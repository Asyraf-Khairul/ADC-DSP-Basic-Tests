#include "stream.h"
#include <algorithm>
#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"
#include "hardware/regs/adc.h"

namespace {
constexpr unsigned PWM_WORDS = AUDIO_WORDS * 16;
uint16_t adc_buffers[2][ADC_WORDS];
uint16_t pwm_buffers[2][PWM_WORDS];
uint16_t expanded[PWM_WORDS];
queue_t adc_queue, audio_queue;
int adc_ch[2], pwm_ch[2];
uint32_t adc_mask, pwm_mask;
unsigned slice;
bool playing = false;
int previous = 250;
volatile uint32_t adc_drops = 0, adc_overflows = 0, audio_underruns = 0;
volatile uint32_t audio_drops = 0, late_irqs = 0;

void __not_in_flash_func(adc_irq)() {
    const uint32_t pending = dma_hw->ints0 & adc_mask;
    if (pending == adc_mask) ++late_irqs;
    for (unsigned b = 0; b < 2; ++b) {
        if (!(pending & (1u << adc_ch[b]))) continue;
        dma_hw->ints0 = 1u << adc_ch[b];
        // Other DMA channel is filling now. Copy before this buffer is reused.
        if (!queue_try_add(&adc_queue, adc_buffers[b])) ++adc_drops;
        dma_channel_set_write_addr(adc_ch[b], adc_buffers[b], false);
        dma_channel_set_trans_count(adc_ch[b], ADC_WORDS, false);
    }
    if (adc_hw->fcs & ADC_FCS_OVER_BITS) {
        ++adc_overflows;
        hw_set_bits(&adc_hw->fcs, ADC_FCS_OVER_BITS);
    }
}

void __not_in_flash_func(audio_irq)() {
    const uint32_t pending = dma_hw->ints1 & pwm_mask;
    if (pending == pwm_mask) ++late_irqs;
    for (unsigned b = 0; b < 2; ++b) {
        if (!(pending & (1u << pwm_ch[b]))) continue;
        dma_hw->ints1 = 1u << pwm_ch[b];
        if (!queue_try_remove(&audio_queue, pwm_buffers[b])) {
            std::fill_n(pwm_buffers[b], PWM_WORDS, 250);
            ++audio_underruns;
        }
        dma_channel_set_read_addr(pwm_ch[b], pwm_buffers[b], false);
        dma_channel_set_trans_count(pwm_ch[b], PWM_WORDS, false);
    }
}
}

void stream_init() {
    queue_init(&adc_queue, sizeof(adc_buffers[0]), 6);
    queue_init(&audio_queue, sizeof(pwm_buffers[0]), 6);
    adc_init();
    adc_gpio_init(27);
    adc_gpio_init(26);
    adc_select_input(1); // GP27 I first, then GP26 Q.
    adc_set_round_robin(3);
    adc_fifo_setup(true, true, 1, false, false);
    // SDK divider is conversion start period minus one, in 48 MHz clock ticks.
    adc_set_clkdiv(99.0f); // 480 kword/s, 240 ksample/s per ADC input.
    adc_fifo_drain();
    for (unsigned b = 0; b < 2; ++b) {
        adc_ch[b] = dma_claim_unused_channel(true);
        pwm_ch[b] = dma_claim_unused_channel(true);
    }
    for (unsigned b = 0; b < 2; ++b) {
        auto c = dma_channel_get_default_config(adc_ch[b]);
        channel_config_set_transfer_data_size(&c, DMA_SIZE_16);
        channel_config_set_read_increment(&c, false);
        channel_config_set_write_increment(&c, true);
        channel_config_set_dreq(&c, DREQ_ADC);
        channel_config_set_chain_to(&c, adc_ch[b ^ 1]);
        dma_channel_configure(adc_ch[b], &c, adc_buffers[b], &adc_hw->fifo, ADC_WORDS, false);
        dma_channel_set_irq0_enabled(adc_ch[b], true);
        adc_mask |= 1u << adc_ch[b];
    }
    irq_set_exclusive_handler(DMA_IRQ_0, adc_irq);
    irq_set_enabled(DMA_IRQ_0, true);

    gpio_set_function(16, GPIO_FUNC_PWM);
    slice = pwm_gpio_to_slice_num(16);
    auto pc = pwm_get_default_config();
    pwm_config_set_wrap(&pc, 499);
    pwm_config_set_clkdiv(&pc, 1.0f); // 120 MHz / 500 = 240 kHz.
    pwm_init(slice, &pc, false);
    pwm_set_gpio_level(16, 250);
    for (unsigned b = 0; b < 2; ++b) {
        std::fill_n(pwm_buffers[b], PWM_WORDS, 250);
        auto c = dma_channel_get_default_config(pwm_ch[b]);
        channel_config_set_transfer_data_size(&c, DMA_SIZE_16);
        channel_config_set_read_increment(&c, true);
        channel_config_set_write_increment(&c, false);
        channel_config_set_dreq(&c, pwm_get_dreq(slice));
        channel_config_set_chain_to(&c, pwm_ch[b ^ 1]);
        dma_channel_configure(pwm_ch[b], &c, &pwm_hw->slice[slice].cc,
                              pwm_buffers[b], PWM_WORDS, false);
        dma_channel_set_irq1_enabled(pwm_ch[b], true);
        pwm_mask |= 1u << pwm_ch[b];
    }
    irq_set_exclusive_handler(DMA_IRQ_1, audio_irq);
    irq_set_enabled(DMA_IRQ_1, true);
    dma_start_channel_mask(1u << adc_ch[0]);
    adc_run(true);
}

bool stream_read(uint16_t *samples) { return queue_try_remove(&adc_queue, samples); }

void stream_audio(const int16_t *samples, unsigned volume) {
    for (unsigned i = 0; i < AUDIO_WORDS; ++i) {
        const int next = std::clamp(250 + static_cast<int>(
            static_cast<int64_t>(samples[i]) * 249 * volume / (32768 * 100)), 0, 499);
        for (unsigned j = 0; j < 16; ++j)
            expanded[i * 16 + j] = previous + ((next - previous) * int(j + 1)) / 16;
        previous = next;
    }
    if (!queue_try_add(&audio_queue, expanded)) ++audio_drops;
    if (!playing && queue_get_level(&audio_queue) >= 4) {
        queue_try_remove(&audio_queue, pwm_buffers[0]);
        queue_try_remove(&audio_queue, pwm_buffers[1]);
        playing = true;
        dma_start_channel_mask(1u << pwm_ch[0]);
        pwm_set_enabled(slice, true);
    }
}

StreamHealth stream_health() {
    return {adc_drops, adc_overflows, audio_underruns, audio_drops, late_irqs};
}
