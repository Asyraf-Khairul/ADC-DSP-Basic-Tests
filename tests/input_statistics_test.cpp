#include "input_statistics.h"
#include "receiver_dsp/spectrum_log.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstdint>
void original(uint16_t *raw, InputStatistics &st) {
    for(unsigned ch=0;ch<2;++ch) {
        uint32_t sum=0; uint64_t squares=0; uint16_t lo=4095,hi=0;
        for(unsigned n=ch;n<2048;n+=2) {
            auto v=raw[n]&4095u; raw[n]=v; sum+=v; squares+=v*v;
            lo=std::min<unsigned>(lo,v); hi=std::max<unsigned>(hi,v);
            if(v<4 || v>4091) ++st.near_rail;
        }
        const double mean=sum/1024.0;
        st.mean[ch]=std::lround(mean); st.lo[ch]=lo; st.hi[ch]=hi;
        st.rms[ch]=std::sqrt(std::max(0.0,squares/1024.0-mean*mean));
    }
}
int main() {
    uint32_t random=0xf9187u;
    uint16_t a[2048],b[2048];
    for(unsigned trial=0;trial<20000;++trial) {
        for(unsigned n=0;n<2048;++n) {
            random=random*1664525u+1013904223u;
            a[n]=trial==0?4095:trial==1?0:trial==2?(n&1?4095:0):
                trial==3?(n%4==0?3:n%4==1?4:n%4==2?4091:4092):
                trial==4?2048:trial==5?65535:uint16_t(random>>16);
        }
        std::memcpy(b,a,sizeof(a)); InputStatistics ref{},candidate{};
        original(a,ref); collect_input_statistics(b,candidate);
        assert(std::memcmp(a,b,sizeof(a))==0);
        assert(std::memcmp(&ref,&candidate,sizeof(ref))==0);
    }
    initialise_spectrum_logs();
    for(unsigned n=1;n<=65535;++n) {
        const float reference=log10f(float(n)), candidate=spectrum_log(n);
        assert(std::memcmp(&reference,&candidate,sizeof(float))==0);
    }
    puts("PASS ADC statistics: 20000 blocks, exact masked samples/mean/RMS/min/max/near-rail counts including all rails and flag bits");
    puts("PASS spectrum log memoization: all 65535 nonzero uint16 inputs bit-identical to host log10f, including fallback range");
}
