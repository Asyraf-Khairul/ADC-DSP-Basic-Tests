#pragma once

#include <cstdint>

#include "pico/types.h"
#include "dsp.h"
#include "bench.h"
#include "front_panel.h"

void display_init(uint sda_gpio, uint scl_gpio, uint8_t i2c_address);
void display_status(const Status &status, unsigned page, const char *footer = nullptr);
void display_menu(const FrontPanel &panel, const Settings &settings, uint32_t now);
void display_update(uint16_t raw_i, uint16_t raw_q, const DspDebugSample &sample,
                    uint32_t block_number);
