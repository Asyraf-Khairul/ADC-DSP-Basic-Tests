#pragma once
#include <cstdint>
#if defined(PICO_ON_DEVICE)
#include "hardware/timer.h"
#endif

// Five receiver boundaries and four filter boundaries, never per sample.
inline uint32_t receiver_profile_clock() {
#if defined(PICO_ON_DEVICE)
    return time_us_32();
#else
    return 0; // Host numerical comparison does not measure target timing.
#endif
}
struct ReceiverProfile {
    uint32_t cic_mix = 0, filter = 0, demod_audio = 0, capture = 0;
    uint32_t fft_forward = 0, fft_bins = 0, fft_inverse = 0;
};
