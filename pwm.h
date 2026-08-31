#pragma once

#include <cstddef>
#include <cstdint>

#include "pico/types.h"

constexpr uint32_t PWM_INTERPOLATION = 16u;

void pwm_audio_init(uint gpio, uint32_t audio_sample_rate);
uint16_t pwm_audio_wrap();
bool pwm_audio_busy();
void pwm_audio_start_block(const uint16_t *samples, size_t sample_count);
