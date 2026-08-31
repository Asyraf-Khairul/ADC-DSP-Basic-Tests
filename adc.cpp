#include "adc.h"

#include "hardware/adc.h"
#include "hardware/dma.h"
#include "pico/assert.h"

namespace {
constexpr uint32_t kAdcClockHz = 48'000'000;
constexpr uint32_t kConversionClocks = 96;

int adc_dma_channel;
dma_channel_config adc_dma_config;
uint32_t actual_fifo_sample_rate;

uint gpio_to_adc_channel(uint gpio) {
    return gpio - 26u;
}
}  // namespace

void adc_dma_init(uint gpio_i, uint gpio_q, uint32_t fifo_sample_rate) {
    hard_assert(gpio_i >= 26u && gpio_i <= 29u);
    hard_assert(gpio_q >= 26u && gpio_q <= 29u);
    hard_assert(gpio_i != gpio_q);
    hard_assert(fifo_sample_rate > 0u);

    const float divider = static_cast<float>(kAdcClockHz) /
                              static_cast<float>(kConversionClocks * fifo_sample_rate) -
                          1.0f;
    adc_init();
    adc_gpio_init(gpio_i);
    adc_gpio_init(gpio_q);
    adc_select_input(gpio_to_adc_channel(gpio_i));
    adc_set_round_robin((1u << gpio_to_adc_channel(gpio_i)) |
                        (1u << gpio_to_adc_channel(gpio_q)));
    adc_set_clkdiv(divider > 0.0f ? divider : 0.0f);
    adc_fifo_setup(true, true, 1, false, false);

    actual_fifo_sample_rate = static_cast<uint32_t>(
        static_cast<float>(kAdcClockHz) /
        (kConversionClocks * (divider > 0.0f ? divider + 1.0f : 1.0f)));

    adc_dma_channel = dma_claim_unused_channel(true);
    adc_dma_config = dma_channel_get_default_config(adc_dma_channel);
    channel_config_set_transfer_data_size(&adc_dma_config, DMA_SIZE_16);
    channel_config_set_read_increment(&adc_dma_config, false);
    channel_config_set_write_increment(&adc_dma_config, true);
    channel_config_set_dreq(&adc_dma_config, DREQ_ADC);
}

void adc_dma_capture_block(uint16_t *samples, size_t sample_count) {
    adc_run(false);
    adc_fifo_drain();
    dma_channel_configure(adc_dma_channel, &adc_dma_config, samples, &adc_hw->fifo,
                          sample_count, false);
    adc_run(true);
    dma_channel_start(adc_dma_channel);
    dma_channel_wait_for_finish_blocking(adc_dma_channel);
    adc_run(false);
}

uint32_t adc_dma_actual_fifo_sample_rate() {
    return actual_fifo_sample_rate;
}
