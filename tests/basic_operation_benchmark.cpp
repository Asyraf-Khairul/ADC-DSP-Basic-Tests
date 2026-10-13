// Separate, synthetic operation timings. Does not initialize ADC, DMA or PWM.
#include "basic_dsp.h"
#include "tests/basic_reference.h"
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include <cmath>
#include <cstdio>

namespace {
uint16_t raw[2048];
float centers[2048];
int32_t fixed_centers[2048];
int16_t audio[64];
volatile float float_sink;
volatile uint32_t integer_sink;
template<class Operation> void measure(const char *name, Operation operation) {
    uint32_t minimum = ~0u, maximum = 0;
    uint64_t total = 0;
    for (unsigned repeat=0; repeat<24; ++repeat) {
        const uint32_t start=time_us_32(); operation();
        const uint32_t elapsed=time_us_32()-start;
        minimum=std::min(minimum,elapsed); maximum=std::max(maximum,elapsed); total+=elapsed;
    }
    printf("BENCH %s min=%lu mean=%.1f max=%lu us per 1024 pairs\n",name,
        (unsigned long)minimum,double(total)/24,(unsigned long)maximum);
}
}
int main() {
    set_sys_clock_khz(120000,true); stdio_init_all();
    while(!stdio_usb_connected()) sleep_ms(10);
    sleep_ms(300);
    puts("BASIC OPERATION BENCHMARK -- synthetic, no streaming or loss counters");
    for (unsigned pair=0; pair<1024; ++pair) {
        raw[2*pair]=2048+int(180*std::cos(pair*0.071));
        raw[2*pair+1]=2048+int(140*std::sin(pair*0.071));
        centers[2*pair]=int(raw[2*pair])-2048;
        centers[2*pair+1]=int(raw[2*pair+1])-2048;
    }
    measure("float_dc_tracking",[] {
        float i=2048,q=2048;
        for(unsigned p=0;p<1024;++p) {
            i+=(raw[2*p]-i)/1024; q+=(raw[2*p+1]-q)/1024;
            centers[2*p]=raw[2*p]-i; centers[2*p+1]=raw[2*p+1]-q;
        }
        float_sink=i+q;
    });
    measure("float_magnitude",[] {
        float sum=0;
        for(unsigned p=0;p<1024;++p) {
            const float i=centers[2*p],q=centers[2*p+1];
            sum+=std::sqrt(i*i+q*q);
        }
        float_sink=sum;
    });
    measure("fixed_dc_tracking",[] {
        int32_t i=2048<<16,q=2048<<16;
        for(unsigned p=0;p<1024;++p) {
            i+=basic_dsp::rounded_shift((int32_t(raw[2*p])<<16)-i,10);
            q+=basic_dsp::rounded_shift((int32_t(raw[2*p+1])<<16)-q,10);
            fixed_centers[2*p]=(int32_t(raw[2*p])<<16)-i;
            fixed_centers[2*p+1]=(int32_t(raw[2*p+1])<<16)-q;
        }
        integer_sink=uint32_t(i)+uint32_t(q);
    });
    measure("integer_sqrt_magnitude",[] {
        uint32_t sum=0;
        for(unsigned p=0;p<1024;++p) sum+=basic_dsp::magnitude_sqrt_q8(fixed_centers[2*p],fixed_centers[2*p+1]);
        integer_sink=sum;
    });
    measure("compatible_dc_tracking",[] {
        int32_t i=2048<<16,q=2048<<16;
        for(unsigned p=0;p<1024;++p) {
            basic_dsp::update_dc(raw[2*p],i); basic_dsp::update_dc(raw[2*p+1],q);
            fixed_centers[2*p]=(int32_t(raw[2*p])<<16)-i;
            fixed_centers[2*p+1]=(int32_t(raw[2*p+1])<<16)-q;
        }
        integer_sink=uint32_t(i)+uint32_t(q);
    });
    measure("lookup_magnitude",[] {
        uint32_t sum=0;
        for(unsigned p=0;p<1024;++p) sum+=basic_dsp::magnitude_q8(fixed_centers[2*p],fixed_centers[2*p+1]);
        integer_sink=sum;
    });
    measure("original_full_kernel",[] { BasicReference s; float_sink=s.process(raw,audio,4); });
    measure("fixed_sqrt_full_kernel",[] { basic_dsp::Kernel<false> s; integer_sink=s.process(raw,audio,4); });
    measure("fixed_lookup_full_kernel",[] { basic_dsp::BasicKernel s; integer_sink=s.process(raw,audio,4); });
    puts("BENCH DONE -- flash normal firmware before hardware streaming tests");
    for (;;) sleep_ms(1000);
}
