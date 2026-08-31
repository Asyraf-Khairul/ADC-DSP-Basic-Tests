#include "pwm.h"

#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/structs/pwm.h"
#include "pico/assert.h"

namespace {
constexpr size_t kMaximumAudioSamples = 64u;

uint audio_slice;
uint16_t audio_wrap;
int audio_dma_channel;
dma_channel_config audio_dma_config;
uint16_t pwm_dma_samples[kMaximumAudioSamples * PWM_INTERPOLATION];
}  // namespace

void pwm_audio_init(uint gpio, uint32_t audio_sample_rate) {
    hard_assert(audio_sample_rate > 0u);
    const uint32_t carrier_rate = audio_sample_rate * PWM_INTERPOLATION;
    const uint32_t wrap = clock_get_hz(clk_sys) / carrier_rate - 1u;
    hard_assert(wrap <= 0xffffu);

    gpio_set_function(gpio, GPIO_FUNC_PWM);
    audio_slice = pwm_gpio_to_slice_num(gpio);
    audio_wrap = static_cast<uint16_t>(wrap);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 1.0f);
    pwm_config_set_wrap(&config, audio_wrap);
    pwm_init(audio_slice, &config, true);
    pwm_set_gpio_level(gpio, audio_wrap / 2u);

    audio_dma_channel = dma_claim_unused_channel(true);
    audio_dma_config = dma_channel_get_default_config(audio_dma_channel);
    channel_config_set_transfer_data_size(&audio_dma_config, DMA_SIZE_16);
    channel_config_set_read_increment(&audio_dma_config, true);
    channel_config_set_write_increment(&audio_dma_config, false);
    channel_config_set_dreq(&audio_dma_config, DREQ_PWM_WRAP0 + audio_slice);
}

uint16_t pwm_audio_wrap() {
    return audio_wrap;
}

bool pwm_audio_busy() {
    return dma_channel_is_busy(audio_dma_channel);
}

void pwm_audio_start_block(const uint16_t *samples, size_t sample_count) {
    hard_assert(sample_count <= kMaximumAudioSamples);
    hard_assert(!pwm_audio_busy());

    for (size_t sample = 0; sample < sample_count; ++sample) {
        for (uint32_t repeat = 0; repeat < PWM_INTERPOLATION; ++repeat) {
            pwm_dma_samples[sample * PWM_INTERPOLATION + repeat] = samples[sample];
        }
    }

    dma_channel_configure(audio_dma_channel, &audio_dma_config,
                          &pwm_hw->slice[audio_slice].cc, pwm_dma_samples,
                          sample_count * PWM_INTERPOLATION, true);
}
