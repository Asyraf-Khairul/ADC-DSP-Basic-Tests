#include "display.h"

#include <cstdio>

#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include "ssd1306.h"

namespace {
constexpr uint16_t kDisplayWidth = 128u;
constexpr uint16_t kDisplayHeight = 64u;
// A typical 128x64 SH1106 panel uses columns 2..129 of its 132-column RAM.
constexpr uint8_t kDisplayColumnOffset = 2u;
constexpr uint32_t kI2cBaudRate = 100000u;
constexpr uint32_t kPowerUpDelayMs = 250u;
constexpr uint32_t kInitRetryDelayMs = 100u;
constexpr uint32_t kChargePumpDelayMs = 100u;
constexpr uint32_t kDisplayOnDelayMs = 150u;
constexpr uint kInitAttempts = 10u;

ssd1306_t display;
uint8_t display_buffer[(kDisplayWidth + 1u) * (kDisplayHeight / 8u)];

bool write_commands(uint8_t address) {
    // SH1106 uses page addressing and AD/8B for its internal DC-DC supply.
    // Do not send SSD1306 horizontal-addressing (20) or charge-pump (8D) commands.
    const uint8_t commands[] = {
        0x00,        // I2C control byte: commands follow.
        0xAE,        // Keep the panel off while configuring and clearing RAM.
        0xD5, 0x50,  // Default oscillator frequency, clock divider 1.
        0xA8, 0x3F,  // 64 multiplexed rows.
        0xD3, 0x00,  // No vertical offset.
        0x40,        // Display start line 0.
        0xA1, 0xC8,  // Segment remap and reversed COM scan.
        0xDA, 0x12,  // Alternative COM pin configuration.
        0x81, 0x80,  // Contrast.
        0xD9, 0x22,  // SH1106 default discharge/precharge periods.
        0xDB, 0x35,  // SH1106 default VCOM deselect level.
        0xAD, 0x8B,  // Enable the SH1106 internal DC-DC converter.
        0xA4, 0xA6,  // Display RAM contents, normal (non-inverted) pixels.
    };
    return i2c_write_timeout_us(i2c1, address, commands, sizeof(commands), false, 10000) ==
           static_cast<int>(sizeof(commands));
}

void draw_line(uint8_t row, const char *text) {
    ssd1306_draw_string(&display, 0, row * 9, 1, text, 1);
}
}  // namespace

void display_init(uint sda_gpio, uint scl_gpio, uint8_t i2c_address) {
    sleep_ms(kPowerUpDelayMs);

    i2c_init(i2c1, kI2cBaudRate);
    gpio_set_function(sda_gpio, GPIO_FUNC_I2C);
    gpio_set_function(scl_gpio, GPIO_FUNC_I2C);
    gpio_pull_up(sda_gpio);
    gpio_pull_up(scl_gpio);

    ssd1306_init(&display, kDisplayWidth, kDisplayHeight, i2c_address, i2c1);
    // The shared drawing driver already sends each page separately.
    display.disp_col_offset = kDisplayColumnOffset;
    display.buffer = display_buffer;
    for (uint attempt = 0u; attempt < kInitAttempts; ++attempt) {
        if (write_commands(i2c_address)) {
            break;
        }
        sleep_ms(kInitRetryDelayMs);
    }
    ssd1306_clear(&display, 0);
    ssd1306_show(&display);
    sleep_ms(kChargePumpDelayMs);
    const uint8_t display_on[] = {0x00, 0xAF};
    i2c_write_timeout_us(i2c1, i2c_address, display_on, sizeof(display_on), false, 10000);
    sleep_ms(kDisplayOnDelayMs);
    draw_line(0, "ADC DSP TEST");
    draw_line(2, "STARTING");
    ssd1306_show(&display);
}

void display_status(const Status &st, unsigned page, const char *footer) {
    char line[24];
    ssd1306_clear(&display,0);
    snprintf(line,sizeof(line),"%s %s P%u",mode_name(st.settings.mode),input_name(st.settings.input),page);
    draw_line(0,line);
    if(page==0) {
        snprintf(line,sizeof(line),"I %u %u %u",st.lo[0],st.mean[0],st.hi[0]); draw_line(1,line);
        snprintf(line,sizeof(line),"Q %u %u %u",st.lo[1],st.mean[1],st.hi[1]); draw_line(2,line);
        snprintf(line,sizeof(line),"RMS %u %u",st.rms[0],st.rms[1]); draw_line(3,line);
        snprintf(line,sizeof(line),"MAG %d AUD %d",st.magnitude,st.audio_rms); draw_line(4,line);
        snprintf(line,sizeof(line),"T%d BW%d G%d",st.settings.tune,st.settings.bw,st.settings.gain); draw_line(5,line);
    } else if(page==1) {
        snprintf(line,sizeof(line),"ADC D%lu F%lu",(unsigned long)st.health.adc_drops,(unsigned long)st.health.adc_overflows); draw_line(1,line);
        snprintf(line,sizeof(line),"AUD U%lu D%lu",(unsigned long)st.health.audio_underruns,(unsigned long)st.health.audio_drops); draw_line(2,line);
        snprintf(line,sizeof(line),"MAX %lu us",(unsigned long)st.max_us); draw_line(3,line);
        snprintf(line,sizeof(line),"LATE %lu IRQ %lu",(unsigned long)st.deadline_misses,(unsigned long)st.health.late_irqs); draw_line(4,line);
        snprintf(line,sizeof(line),"B%lu C%lu",(unsigned long)st.blocks,(unsigned long)st.clips); draw_line(5,line);
    } else if(page==2) {
        if(st.settings.mode==6) { draw_line(2,"NO FFT IN BASIC"); }
        else {
            draw_line(1,"FFT -15k 0 +15k");
            for(unsigned x=0;x<128;++x) {
                unsigned height=st.spectrum[x]*42/255;
                if(height) ssd1306_fill_rectangle(&display,x,63-height,1,height,1);
            }
        }
    } else if(page==3) {
        draw_line(1,"AUDIO 64 @15k");
        for(unsigned x=0;x<128;++x) {
            int y=42-(int(st.audio[x/2])*20/32768);
            ssd1306_draw_pixel(&display,x,y,1);
        }
    } else if(page==4) {
        const StageTiming *stages[] = {&st.timing.input_stats, &st.timing.dsp,
            &st.timing.audio_stats, &st.timing.spectrum, &st.timing.output};
        const char *names[] = {"IN", "DSP", "STAT", "FFT", "OUT"};
        for (unsigned row=0; row<5; ++row) {
            snprintf(line,sizeof(line),"%s %lu/%lu",names[row],
                (unsigned long)stages[row]->latest_us,(unsigned long)stages[row]->max_us);
            draw_line(row+1,line);
        }
        draw_line(6,"us latest / max");
    }
    if (page < 2) draw_line(6, footer ? footer : "B1:Page B2:Menu");
    ssd1306_show(&display);
}

void display_menu(const FrontPanel &panel, const Settings &settings, uint32_t now) {
    char line[22], value[22];
    ssd1306_clear(&display, 0);
    if (panel.editing()) {
        snprintf(line, sizeof(line), "EDIT %s", panel.label(panel.selected()));
        draw_line(0, line);
        panel.value(panel.selected(), settings, value, sizeof(value));
        draw_line(2, value);
        draw_line(4, panel.description(settings));
        draw_line(5, panel.notice(now) ? panel.notice(now) : "Turn to change");
        draw_line(6, "B1:Cancel B2:Apply");
    } else {
        snprintf(line, sizeof(line), "SETTINGS %u/%u", panel.selected()+1, panel.count());
        draw_line(0, line);
        const unsigned first = (panel.selected()/4)*4;
        for (unsigned row = 0; row < 4 && first+row < panel.count(); ++row) {
            const unsigned item = first+row;
            panel.value(item, settings, value, sizeof(value));
            snprintf(line, sizeof(line), "%c%-9.9s %.10s", item == panel.selected() ? '>' : ' ', panel.label(item), value);
            draw_line(row+1, line);
        }
        draw_line(5, panel.notice(now) ? panel.notice(now) : "Turn to browse");
        draw_line(6, "B1:Back B2:Select");
    }
    ssd1306_show(&display);
}

void display_update(uint16_t raw_i, uint16_t raw_q, const DspDebugSample &sample,
                    uint32_t block_number) {
    char line[22];
    ssd1306_clear(&display, 0);
    draw_line(0, "ADC DSP RUN");

    snprintf(line, sizeof(line), "RAW %4u %4u", static_cast<unsigned>(raw_i),
             static_cast<unsigned>(raw_q));
    draw_line(1, line);
    snprintf(line, sizeof(line), "I/Q %5d %5d", sample.i, sample.q);
    draw_line(2, line);
    snprintf(line, sizeof(line), "MAG %6ld", static_cast<long>(sample.magnitude));
    draw_line(3, line);
    snprintf(line, sizeof(line), "AUD %6ld", static_cast<long>(sample.audio));
    draw_line(4, line);
    snprintf(line, sizeof(line), "PWM %4u B%lu",
             static_cast<unsigned>(sample.pwm_level), static_cast<unsigned long>(block_number));
    draw_line(5, line);

    const uint32_t bar_width = sample.magnitude < 0 ? 0u :
        (static_cast<uint32_t>(sample.magnitude) > 1024u ? 128u :
         static_cast<uint32_t>(sample.magnitude) / 8u);
    ssd1306_draw_rectangle(&display, 0, 56, 127, 7, 1);
    if (bar_width > 0u) {
        ssd1306_fill_rectangle(&display, 1, 57, bar_width - 1u, 5, 1);
    }
    ssd1306_show(&display);
}
