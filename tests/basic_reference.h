#pragma once
// Frozen BASIC kernel from the instrumented 81dbbd9 working tree.
// Test/benchmark only: retains float DC tracking, per-pair sqrt and group order.
#include <algorithm>
#include <cmath>
#include <cstdint>

struct BasicReference {
    float i_dc = 2048, q_dc = 2048, audio_dc = 0;
    double process(const uint16_t *raw, int16_t *audio, int gain, float *magnitudes = nullptr) {
        double total_mag = 0;
        for (unsigned n = 0; n < 64; ++n) {
            float sum = 0;
            for (unsigned k = 0; k < 16; ++k) {
                unsigned pos = 2*(n*16+k);
                i_dc += (raw[pos]-i_dc)/1024;
                q_dc += (raw[pos+1]-q_dc)/1024;
                float i = raw[pos]-i_dc, q = raw[pos+1]-q_dc;
                sum += std::sqrt(i*i+q*q);
            }
            float mag = sum/16;
            total_mag += mag;
            audio_dc += (mag-audio_dc)/256;
            audio[n] = std::clamp<int>((mag-audio_dc)*(1<<gain),-32767,32767);
            if (magnitudes) magnitudes[n] = mag;
        }
        return total_mag/64;
    }
};
