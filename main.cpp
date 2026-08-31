#include "adc.h"
#include "display.h"
#include "dsp.h"
#include "pwm.h"

#include "pico/stdlib.h"

// User configuration.
constexpr uint32_t SAMPLE_RATE = 500000u;
constexpr uint32_t AUDIO_SAMPLE_RATE = 10000u;
constexpr uint32_t LO_FREQUENCY = 1500000u;
constexpr uint ADC_GPIO_I = 27u;
constexpr uint ADC_GPIO_Q = 26u;
constexpr uint PWM_GPIO_AUDIO = 16u;
constexpr uint OLED_GPIO_SDA = 18u;
constexpr uint OLED_GPIO_SCL = 19u;
constexpr uint8_t OLED_I2C_ADDRESS = 0x3cu;

// Keep one ADC block equal to one audio DMA block: 2 ms with the defaults.
constexpr size_t ADC_DMA_WORDS = 1000u;
constexpr uint32_t IQ_SAMPLE_RATE = SAMPLE_RATE / 2u;
constexpr uint32_t AUDIO_DECIMATION = IQ_SAMPLE_RATE / AUDIO_SAMPLE_RATE;
constexpr size_t AUDIO_BLOCK_SAMPLES = ADC_DMA_WORDS / 2u / AUDIO_DECIMATION;

static_assert(SAMPLE_RATE % 2u == 0u, "SAMPLE_RATE must be even for I/Q round robin");
static_assert(IQ_SAMPLE_RATE % AUDIO_SAMPLE_RATE == 0u,
              "Choose an integer I/Q-to-audio decimation ratio");
static_assert(ADC_DMA_WORDS % (2u * AUDIO_DECIMATION) == 0u,
              "ADC_DMA_WORDS must contain a whole number of audio samples");

int main() {
    pwm_audio_init(PWM_GPIO_AUDIO, AUDIO_SAMPLE_RATE);
    dsp_init(AUDIO_DECIMATION, pwm_audio_wrap());
    adc_dma_init(ADC_GPIO_I, ADC_GPIO_Q, SAMPLE_RATE);
    display_init(OLED_GPIO_SDA, OLED_GPIO_SCL, OLED_I2C_ADDRESS);

    uint16_t adc_samples[ADC_DMA_WORDS];
    uint16_t audio_samples[AUDIO_BLOCK_SAMPLES];
    DspDebugSample debug{};
    uint32_t block_number = 0u;

    // ADC DMA fills a 2 ms I/Q block while the previous block is emitted by PWM DMA.
    while (true) {
        adc_dma_capture_block(adc_samples, ADC_DMA_WORDS);
        const size_t audio_count = dsp_process_iq_block(
            adc_samples, ADC_DMA_WORDS, audio_samples, AUDIO_BLOCK_SAMPLES, &debug);

        while (pwm_audio_busy()) {
            tight_loop_contents();
        }
        pwm_audio_start_block(audio_samples, audio_count);

        if (++block_number % 100u == 0u) {
            display_update(adc_samples[0] & 0x0fffu, adc_samples[1] & 0x0fffu,
                           debug, block_number);
        }
    }
}
