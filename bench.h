#pragma once
#include <cstdint>
#include "stream.h"
#include "timing_capture.h"
#include "receiver_dsp/profile.h"
struct Settings {
    int mode = 6; // receiver AM=0, AMSYNC=1, LSB=2, USB=3, FM=4, CW=5; BASIC=6
    int input = 0; // IQ=0, real IF on I=1 or Q=2
    int tune = 0, bw = 2, agc = 4, gain = 4, volume = 25;
    int squelch = -1, hang = 3, swap = 0, iqcorr = 0;
    int deemph = 0, bass = 0, treble = 0, blanker = 0, notch = 0;
    int nr = 0, smooth = 10, threshold = 1, cw = 700;
};
struct StageTiming {
    uint32_t latest_us = 0, max_us = 0;
    void record(uint32_t elapsed_us) {
        latest_us = elapsed_us;
        if (elapsed_us > max_us) max_us = elapsed_us;
    }
};
struct ProcessingTiming {
    StageTiming input_stats, dsp, audio_stats, spectrum, process, output;
};
struct Status {
    Settings settings;
    uint32_t revision = 0, blocks = 0, dsp_us = 0, max_us = 0, deadline_misses = 0;
    StreamHealth health{};
    ProcessingTiming timing{};
    ReceiverProfile receiver_profile{};
    TimingCapture capture{};
    uint16_t lo[2]{}, mean[2]{}, hi[2]{}, rms[2]{};
    uint32_t clips = 0;
    int magnitude = 0, audio_rms = 0, audio_peak = 0;
    uint8_t spectrum[128]{};
    int16_t audio[64]{};
};
const char *mode_name(int mode);
const char *input_name(int input);
Settings preset(unsigned index);
constexpr unsigned PRESET_COUNT = 9;
// Parses one command without changing settings on failure.
bool parse_setting(const char *line, Settings &settings);
void apply_settings(const Settings &s);
void process_samples(uint16_t *raw, int16_t *audio, Status &status);
