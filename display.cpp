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
    return i2c_write_blocking(i2c1, address, commands, sizeof(commands), false) ==
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
    i2c_write_blocking(i2c1, i2c_address, display_on, sizeof(display_on), false);
    sleep_ms(kDisplayOnDelayMs);
    draw_line(0, "ADC DSP TEST");
    draw_line(2, "STARTING");
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
