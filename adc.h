#pragma once

#include <cstddef>
#include <cstdint>

#include "pico/types.h"

// SAMPLE_RATE is the aggregate ADC FIFO rate. With two round-robin channels,
// each I/Q channel is sampled at SAMPLE_RATE / 2.
void adc_dma_init(uint gpio_i, uint gpio_q, uint32_t fifo_sample_rate);
void adc_dma_capture_block(uint16_t *samples, size_t sample_count);
uint32_t adc_dma_actual_fifo_sample_rate();
