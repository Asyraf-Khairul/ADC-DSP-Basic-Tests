#include "display.h"

#include <cstdio>

#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include "ssd1306.h"

namespace {
constexpr uint16_t kDisplayWidth = 128u;
constexpr uint16_t kDisplayHeight = 64u;
constexpr uint32_t kI2cBaudRate = 100000u;
constexpr uint32_t kPowerUpDelayMs = 250u;
constexpr uint32_t kInitRetryDelayMs = 100u;
constexpr uint kInitAttempts = 10u;

ssd1306_t display;
uint8_t display_buffer[(kDisplayWidth + 1u) * (kDisplayHeight / 8u)];

bool write_commands(uint8_t address) {
    const uint8_t commands[] = {
        0x00, 0xAE, 0x20, 0x00, 0xB0, 0xC8, 0x00, 0x10, 0x40,
        0x81, 0xFF, 0xA1, 0xA6, 0xA8, 0x3F, 0xA4, 0xD3, 0x00,
        0xD5, 0x80, 0xD9, 0xF1, 0xDA, 0x12, 0xDB, 0x40, 0x8D,
        0x14, 0xAF,
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
    display.buffer = display_buffer;
    for (uint attempt = 0u; attempt < kInitAttempts; ++attempt) {
        if (write_commands(i2c_address)) {
            break;
        }
        sleep_ms(kInitRetryDelayMs);
    }
    ssd1306_clear(&display, 0);
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
