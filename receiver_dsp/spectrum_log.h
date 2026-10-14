#pragma once
#include <cstdint>
#include <cmath>

// Exact memoization of the existing target log10f, not a logarithm approximation.
// Constants are computed once before ADC/DMA start; no signal-dependent work is
// deferred. Larger unsigned magnitudes retain the original log10f path.
constexpr unsigned SPECTRUM_LOG_COUNT=32768;
extern float spectrum_log_values[SPECTRUM_LOG_COUNT];
void initialise_spectrum_logs();
inline float spectrum_log(uint16_t value) {
    return value<SPECTRUM_LOG_COUNT ? spectrum_log_values[value] : log10f(value);
}
