#pragma once
#include <cstdint>
constexpr unsigned ADC_WORDS = 2048;
constexpr unsigned AUDIO_WORDS = 64;
struct StreamHealth {
    uint32_t adc_drops, adc_overflows, audio_underruns, audio_drops, late_irqs;
};
// All functions except health_snapshot run on core 1.
void stream_init();
bool stream_read(uint16_t *samples);
void stream_audio(const int16_t *samples, unsigned volume);
StreamHealth stream_health();
