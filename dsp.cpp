#include "dsp.h"

#include <cmath>

#include "pico/assert.h"

namespace {
constexpr float kDcTracking = 1.0f / 1024.0f;
constexpr float kAudioDcTracking = 1.0f / 256.0f;
constexpr float kAudioGain = 3.0f;

float i_dc;
float q_dc;
float audio_dc;
uint32_t decimation_factor;
uint32_t decimation_count;
uint16_t output_wrap;

uint16_t to_pwm_level(float audio) {
    const float mid_scale = static_cast<float>(output_wrap) * 0.5f;
    float level = mid_scale + audio * kAudioGain;
    if (level < 0.0f) {
        level = 0.0f;
    } else if (level > static_cast<float>(output_wrap)) {
        level = static_cast<float>(output_wrap);
    }
    return static_cast<uint16_t>(level);
}
}  // namespace

void dsp_init(uint32_t decimation, uint16_t pwm_wrap) {
    hard_assert(decimation > 0u);
    i_dc = 2048.0f;
    q_dc = 2048.0f;
    audio_dc = 0.0f;
    decimation_factor = decimation;
    decimation_count = 0u;
    output_wrap = pwm_wrap;
}

size_t dsp_process_iq_block(const uint16_t *interleaved_iq, size_t word_count,
                            uint16_t *pwm_samples, size_t pwm_capacity,
                            DspDebugSample *first_sample) {
    size_t output_count = 0u;
    bool first_set = false;

    for (size_t index = 0; index + 1u < word_count; index += 2u) {
        const float raw_i = static_cast<float>(interleaved_iq[index] & 0x0fffu);
        const float raw_q = static_cast<float>(interleaved_iq[index + 1u] & 0x0fffu);
        i_dc += (raw_i - i_dc) * kDcTracking;
        q_dc += (raw_q - q_dc) * kDcTracking;
        const float i = raw_i - i_dc;
        const float q = raw_q - q_dc;
        const float magnitude = sqrtf(i * i + q * q);

        audio_dc += (magnitude - audio_dc) * kAudioDcTracking;
        const float audio = magnitude - audio_dc;
        const uint16_t pwm_level = to_pwm_level(audio);

        if (!first_set) {
            *first_sample = {
                static_cast<int16_t>(i), static_cast<int16_t>(q),
                static_cast<int32_t>(magnitude), static_cast<int32_t>(audio), pwm_level};
            first_set = true;
        }

        if (++decimation_count == decimation_factor) {
            decimation_count = 0u;
            if (output_count < pwm_capacity) {
                pwm_samples[output_count++] = pwm_level;
            }
        }
    }
    return output_count;
}
