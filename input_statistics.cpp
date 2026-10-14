#include "input_statistics.h"
#include <algorithm>
#include <cmath>

namespace {
inline __attribute__((always_inline)) void sample(uint16_t &raw, uint32_t &sum,
        uint32_t &squares, uint16_t &lo, uint16_t &hi, uint32_t &near) {
    const uint32_t v=raw&4095u;
    raw=v; sum+=v; squares+=v*v;
    lo=std::min<uint16_t>(lo,v); hi=std::max<uint16_t>(hi,v);
    near += v<4 || v>4091;
}
}
#if defined(PICO_ON_DEVICE)
__attribute__((section(".time_critical.input_statistics"), noinline))
#endif
void collect_input_statistics(uint16_t *raw, InputStatistics &stats) {
    stats.near_rail=0;
    for(unsigned ch=0;ch<2;++ch) {
        uint32_t sum=0, s0=0,s1=0,s2=0,s3=0,near=0;
        uint16_t lo=4095,hi=0;
        // Each lane accumulates exactly 256 squares: 256*4095^2 < UINT32_MAX.
        // Combine in 64 bits once per channel; original exact sum is preserved.
        for(unsigned n=ch;n<2048;n+=8) {
            sample(raw[n],sum,s0,lo,hi,near);
            sample(raw[n+2],sum,s1,lo,hi,near);
            sample(raw[n+4],sum,s2,lo,hi,near);
            sample(raw[n+6],sum,s3,lo,hi,near);
        }
        const uint64_t squares=uint64_t(s0)+s1+s2+s3;
        const double mean=sum/1024.0;
        stats.mean[ch]=std::lround(mean); stats.lo[ch]=lo; stats.hi[ch]=hi;
        stats.rms[ch]=std::sqrt(std::max(0.0,squares/1024.0-mean*mean));
        stats.near_rail+=near;
    }
}
