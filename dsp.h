#pragma once

#include <cstddef>
#include <cstdint>

struct DspDebugSample {
    int16_t i;
    int16_t q;
    int32_t magnitude;
    int32_t audio;
    uint16_t pwm_level;
};

void dsp_init(uint32_t decimation, uint16_t pwm_wrap);
size_t dsp_process_iq_block(const uint16_t *interleaved_iq, size_t word_count,
                            uint16_t *pwm_samples, size_t pwm_capacity,
                            DspDebugSample *first_sample);

