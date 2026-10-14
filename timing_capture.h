#pragma once
#include "stream.h"
#include <algorithm>
#include <cstdint>

// Explicit measurement windows reset only these new accumulators, never health
// counters or cumulative timings. Core 1 owns mutation; core 0 prints snapshots.
constexpr unsigned CAPTURE_STAGES = 13;
struct CaptureCounters {
    uint32_t blocks = 0, late = 0, clips = 0, revision = 0;
    StreamHealth health{};
    uint64_t device_us = 0;
};
struct TimingGroup {
    uint32_t count = 0, late = 0;
    uint32_t max_us[CAPTURE_STAGES]{};
    uint64_t sum_us[CAPTURE_STAGES]{};
    void record(const uint32_t (&values)[CAPTURE_STAGES]) {
        ++count;
        if (values[0] > 4266) ++late;
        for (unsigned k=0; k<CAPTURE_STAGES; ++k) {
            max_us[k] = std::max(max_us[k], values[k]);
            sum_us[k] += values[k];
        }
    }
};
struct TimingCapture {
    uint32_t id = 0;
    bool active = false, settings_changed = false;
    uint32_t bookkeeping_max_us = 0;
    uint64_t bookkeeping_sum_us = 0;
    CaptureCounters start{}, end{};
    TimingGroup ordinary{}, spectrum{};
    void begin(const CaptureCounters &counters) {
        const uint32_t next = id+1;
        *this = {};
        id = next; active = true; start = end = counters;
    }
    void finish(const CaptureCounters &counters) { end = counters; active = false; }
    void record(bool has_spectrum, const uint32_t (&values)[CAPTURE_STAGES], uint32_t revision) {
        if (!active) return;
        if (revision != start.revision) settings_changed = true;
        (has_spectrum ? spectrum : ordinary).record(values);
    }
};
