#pragma once

#include <cstdint>

#include "pico/types.h"
#include "dsp.h"

void display_init(uint sda_gpio, uint scl_gpio, uint8_t i2c_address);
void display_update(uint16_t raw_i, uint16_t raw_q, const DspDebugSample &sample,
                    uint32_t block_number);
