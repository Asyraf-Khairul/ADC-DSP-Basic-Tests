#pragma once
#include <algorithm>
#include <cstdint>

#if defined(PICO_ON_DEVICE)
#define BASIC_INLINE inline __attribute__((always_inline))
#else
#define BASIC_INLINE inline
#endif

namespace basic_dsp {
// Coefficient sqrt(1 + (index/256)^2), rounded to Q16, for linear interpolation.
extern const uint32_t magnitude_coefficients[257];

// Inputs are bounded by 4095 ADC counts. Q3 squares fit in uint32_t even at
// both rails: 2*(4095*8)^2 = 2146435200. No input pair is discarded.
BASIC_INLINE uint32_t integer_sqrt(uint32_t value) {
    uint32_t root = 0, bit = 1u << 30;
    while (bit > value) bit >>= 2;
    while (bit) {
        if (value >= root+bit) { value -= root+bit; root = (root>>1)+bit; }
        else root >>= 1;
        bit >>= 2;
    }
    return root;
}
BASIC_INLINE int32_t rounded_shift(int32_t value, unsigned bits) {
    // Arithmetic right shift on the project's Arm GCC and host GCC targets.
    // Symmetric round-to-nearest, ties away from zero, without integer division.
    return (value + (value >= 0 ? (1 << (bits-1)) : (1 << (bits-1))-1)) >> bits;
}
BASIC_INLINE void update_dc(uint16_t raw, int32_t &dc) {
    // Emulate the original binary32 subtraction/addition in integer arithmetic.
    // Q16 retains all binary32 values >=128 ADC counts; below that range the
    // additional Q16 quantization is bounded and tested against the reference.
    int32_t difference=(int32_t(raw)<<16)-dc;
    uint32_t absolute=difference<0 ? -difference : difference;
    if (absolute>0x00ffffffu) {
        const unsigned shift=8-__builtin_clz(absolute); // bit length minus 24.
        const uint32_t half=1u<<(shift-1);
        absolute=(absolute+half-1+((absolute>>shift)&1u))>>shift<<shift;
        difference=difference<0 ? -int32_t(absolute) : int32_t(absolute);
    }
    // Split the exact Q26 sum into its Q16 integer and ten fractional bits.
    // This produces the same rounding as a 64-bit sum without 64-bit operations.
    const uint32_t base=uint32_t(dc+(difference>>10));
    const unsigned precision=base>=(2048u<<16) ? 4 : base>=(1024u<<16) ? 3 :
        base>=(512u<<16) ? 2 : base>=(256u<<16) ? 1 : 0;
    const uint32_t quantum=1u<<precision,half=quantum<<9;
    const uint32_t fraction=((base&(quantum-1))<<10)+(uint32_t(difference)&1023u);
    uint32_t value=base&~(quantum-1);
    if (fraction>half || (fraction==half && ((base>>precision)&1u))) value+=quantum;
    dc=int32_t(value);
}
BASIC_INLINE uint32_t magnitude_q8(int32_t i_q16, int32_t q_q16) {
    const int32_t i = rounded_shift(i_q16,8), q = rounded_shift(q_q16,8);
    const uint32_t ai = i < 0 ? -i : i, aq = q < 0 ? -q : q;
    const uint32_t large = std::max(ai,aq), small = std::min(ai,aq);
    if (!large) return 0;
    // Keep fine Q8 resolution for weak signals. Normalize only the ratio operands
    // for large signals so the division is 32-bit and uses the RP2040 divider.
    unsigned normalization=0;
    while ((large>>normalization)>65535u) ++normalization;
    const uint32_t rounding=normalization ? 1u<<(normalization-1) : 0;
    const uint32_t denominator=(large+rounding)>>normalization;
    const uint32_t numerator=(small+rounding)>>normalization;
    // Rounding can produce 65536; the equal-operands shortcut avoids overflow.
    const uint32_t ratio=numerator==denominator ? 65536u : (numerator<<16)/denominator;
    const unsigned index = ratio >> 8;
    uint32_t coefficient = magnitude_coefficients[index];
    if (index < 256) {
        const uint32_t difference = magnitude_coefficients[index+1]-coefficient;
        coefficient += (difference*(ratio&255u)+128u)>>8;
    }
    // Split the Q8*Q16 product into two bounded 32-bit products. This is exactly
    // the rounded 64-bit result, including simultaneous near-rail inputs.
    const uint32_t extra=coefficient-65536u;
    return large+(large>>16)*extra+(((large&65535u)*extra+32768u)>>16);
}
BASIC_INLINE uint32_t magnitude_sqrt_q8(int32_t i_q16, int32_t q_q16) {
    const int32_t i = rounded_shift(i_q16,13), q = rounded_shift(q_q16,13);
    return integer_sqrt(uint32_t(i*i)+uint32_t(q*q)) << 5;
}

// Both candidates retain DC coefficients 1/1024 and 1/256, per-pair magnitude,
// groups of 16 and truncating/saturating PCM conversion. Magnitude is a bounded
// approximation; filter coefficients, sample order and averaging are unchanged.
template<bool Lookup = true> class Kernel {
public:
    int32_t i_dc = 2048<<16, q_dc = 2048<<16;
    int64_t audio_dc = 0; // Q24 avoids a DC deadband at the highest PCM gain.
    template<bool Capture = false>
    BASIC_INLINE int process(const uint16_t *raw, int16_t *audio, int gain, uint32_t *magnitudes = nullptr) {
        uint32_t total_q8 = 0;
        for (unsigned n = 0; n < 64; ++n) {
            uint32_t sum_q8 = 0;
            for (unsigned k = 0; k < 16; ++k) {
                const unsigned pos = 2*(n*16+k);
                const int32_t in_i = int32_t(raw[pos])<<16, in_q = int32_t(raw[pos+1])<<16;
                update_dc(raw[pos],i_dc);
                update_dc(raw[pos+1],q_dc);
                if constexpr (Lookup) sum_q8 += magnitude_q8(in_i-i_dc,in_q-q_dc);
                else sum_q8 += magnitude_sqrt_q8(in_i-i_dc,in_q-q_dc);
            }
            total_q8 += sum_q8;
            const int64_t mag_q24 = int64_t(sum_q8)<<12; // /16 and Q8->Q24 combined.
            const int64_t error = mag_q24-audio_dc;
            audio_dc += (error + (error >= 0 ? 128 : 127))>>8;
            const int64_t delta = mag_q24-audio_dc;
            const unsigned shift = 24-gain; // validated gain 0..10.
            const int level = int(delta >= 0 ? delta>>shift : -((-delta)>>shift));
            audio[n] = std::clamp(level,-32767,32767);
            if constexpr (Capture) magnitudes[n] = sum_q8<<4; // Q16 test capture.
        }
        return int(total_q8>>18); // average all 1024 pair magnitudes, then truncate.
    }
};
class BasicKernel : public Kernel<true> {
public:
    int process(const uint16_t *raw, int16_t *audio, int gain);
};
}
#undef BASIC_INLINE
