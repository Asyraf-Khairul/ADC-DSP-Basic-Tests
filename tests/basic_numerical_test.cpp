// Compare the original float kernel and fixed-point candidates on identical blocks.
// No receiver algorithms or Pico hardware are involved in this host comparison.
#include "basic_dsp.h"
#include "tests/basic_reference.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <cassert>

namespace {
constexpr double pi=3.14159265358979323846;
constexpr unsigned blocks=1024, settled_start=768;
struct Metrics {
    double magnitude_max=0,magnitude_sum=0,magnitude_square=0;
    double audio_max=0,audio_sum=0,audio_square=0;
    double reference_sum=0,reference_square=0,optimized_sum=0,optimized_square=0;
    uint64_t count=0,reference_clips=0,optimized_clips=0,clip_disagreement=0;
    void add(double reference_magnitude,double magnitude,int reference,int optimized) {
        const double me=magnitude-reference_magnitude, ae=optimized-reference;
        magnitude_max=std::max(magnitude_max,std::abs(me)); magnitude_sum+=me; magnitude_square+=me*me;
        audio_max=std::max(audio_max,std::abs(ae)); audio_sum+=ae; audio_square+=ae*ae;
        reference_sum+=reference; reference_square+=double(reference)*reference;
        optimized_sum+=optimized; optimized_square+=double(optimized)*optimized;
        const bool rc=std::abs(reference)==32767,oc=std::abs(optimized)==32767;
        reference_clips+=rc; optimized_clips+=oc; clip_disagreement+=(rc!=oc); ++count;
    }
    void print(const char *candidate,const char *scenario,int gain,const char *phase) const {
        const double ref_rms=std::sqrt(std::max(0.0,reference_square/count-std::pow(reference_sum/count,2)));
        const double opt_rms=std::sqrt(std::max(0.0,optimized_square/count-std::pow(optimized_sum/count,2)));
        printf("%s,%s,%d,%s,%llu,%.9f,%.9f,%.9f,%.9f,%.9f,%.9f,%.9f,%.9f,",
            candidate,scenario,gain,phase,(unsigned long long)count,
            magnitude_max,magnitude_sum/count,std::sqrt(magnitude_square/count),
            audio_max,audio_sum/count,std::sqrt(audio_square/count),ref_rms,opt_rms);
        if (ref_rms>=16) printf("%.9f",100*(opt_rms/ref_rms-1));
        printf(",%llu,%llu,%llu\n",(unsigned long long)reference_clips,
            (unsigned long long)optimized_clips,(unsigned long long)clip_disagreement);
    }
};
const char *scenarios[]={"quiet_midscale","quiet_biased","noise_floor","small_signal",
    "i_only","q_only","equal_phase","quadrature","unequal_amplitude","bias_steps",
    "amplitude_steps","near_rails","audio_limiting","quiet_low_dc"};
uint16_t adc(double value) { return std::clamp<int>(std::lround(value),0,4095); }
void generate(unsigned scenario,unsigned block,uint16_t *raw,uint32_t &random) {
    for (unsigned p=0;p<1024;++p) {
        const uint64_t index=uint64_t(block)*1024+p;
        const double phase=2*pi*1000*double(index)/240000;
        const double qphase=phase+2*pi*1000/(2*240000);
        double i=2048,q=2048;
        switch(scenario) {
            case 0: break;
            case 1: i=2051; q=2066; break;
            case 2:
                random=random*1664525u+1013904223u; i+=int((random>>16)%3)-1;
                random=random*1664525u+1013904223u; q+=int((random>>16)%3)-1; break;
            case 3: i+=1.5*std::sin(phase); q+=1.5*std::cos(qphase); break;
            case 4: i+=180*std::sin(phase); break;
            case 5: q+=180*std::sin(qphase); break;
            case 6: i+=180*std::sin(phase); q+=180*std::sin(qphase); break;
            case 7: i+=180*std::sin(phase); q+=180*std::cos(qphase); break;
            case 8: i+=180*std::sin(phase); q+=70*std::cos(qphase); break;
            case 9:
                i=(block<384?2048:block<640?2384:2091)+120*std::sin(phase);
                q=(block<384?2048:block<640?1777:2066)+90*std::cos(qphase); break;
            case 10: {
                const double amplitude=block<384?10:block<640?200:25;
                i+=amplitude*std::sin(phase); q+=amplitude*std::cos(qphase); break;
            }
            case 11: i+=2048*std::sin(phase); q+=2048*std::cos(qphase); break;
            case 12:
                i+=1100*(1+0.7*std::sin(phase))*std::cos(phase*8);
                q+=1100*(1+0.7*std::sin(qphase))*std::sin(qphase*8); break;
            case 13: i=0; q=1; break;
        }
        raw[2*p]=adc(i); raw[2*p+1]=adc(q);
    }
}
template<bool Lookup> void compare() {
    const char *candidate=Lookup?"lookup_q8":"sqrt_q3";
    for(unsigned scenario=0;scenario<14;++scenario) for(int gain:{0,4,10}) {
        BasicReference reference;
        basic_dsp::Kernel<Lookup> optimized;
        Metrics all,settled;
        uint32_t random=0x17a30b1u;
        uint16_t raw[2048]; int16_t ref_audio[64],audio[64];
        float ref_magnitude[64]; uint32_t magnitude[64];
        for(unsigned block=0;block<blocks;++block) {
            generate(scenario,block,raw,random);
            reference.process(raw,ref_audio,gain,ref_magnitude);
            optimized.template process<true>(raw,audio,gain,magnitude);
            for(unsigned n=0;n<64;++n) {
                const double m=double(magnitude[n])/65536;
                all.add(ref_magnitude[n],m,ref_audio[n],audio[n]);
                if(block>=settled_start) settled.add(ref_magnitude[n],m,ref_audio[n],audio[n]);
            }
        }
        // Predeclared acceptance: group magnitude error <1 ADC-count equivalent.
        if(all.magnitude_max>=1) {
            fprintf(stderr,"FAIL magnitude bound: %s %s gain=%d error=%f\n",candidate,scenarios[scenario],gain,all.magnitude_max);
            std::exit(1);
        }
        all.print(candidate,scenarios[scenario],gain,"all");
        settled.print(candidate,scenarios[scenario],gain,"settled");
    }
}
void magnitude_bound_test() {
    // Predeclared bound <=0.25 count relative to exact sqrt of Q16-centered inputs.
    // Covers Q8 rounding, ratio normalization, interpolation, coefficient and output rounding.
    uint32_t random=0x6539412u;
    double maximum=0;
    for(unsigned n=0;n<1000000;++n) {
        random=random*1664525u+1013904223u; const int32_t i=int32_t(random%536739841u)-268369920;
        random=random*1664525u+1013904223u; const int32_t q=int32_t(random%536739841u)-268369920;
        const double exact=std::hypot(double(i)/65536,double(q)/65536);
        const double approximate=double(basic_dsp::magnitude_q8(i,q))/256;
        maximum=std::max(maximum,std::abs(approximate-exact));
    }
    for(int i:{-4095,-2048,-1,0,1,2048,4095}) for(int q:{-4095,-2048,-1,0,1,2048,4095}) {
        const double exact=std::hypot(double(i),double(q));
        const double approximate=double(basic_dsp::magnitude_q8(i*65536,q*65536))/256;
        maximum=std::max(maximum,std::abs(approximate-exact));
    }
    assert(maximum<=0.25);
    fprintf(stderr,"PASS standalone lookup magnitude max absolute error=%.9f ADC counts (bound 0.25)\n",maximum);
}
void dc_tracking_test() {
    int32_t dc=2048<<16;
    float reference=2048;
    uint32_t random=193875;
    for(unsigned n=0;n<1000000;++n) {
        random=random*1664525u+1013904223u;
        const uint16_t input=(random>>16)&4095;
        reference+=(input-reference)/1024;
        basic_dsp::update_dc(input,dc);
        assert(double(dc)/65536==reference);
    }
    fprintf(stderr,"PASS DC tracker: 1000000 full-range ADC samples match binary32 reference state exactly\n");
}
}
int main() {
    magnitude_bound_test(); dc_tracking_test();
    puts("candidate,scenario,gain_log2,phase,samples,mag_max_abs,mag_mean_signed,mag_rms_error,pcm_max_abs,pcm_mean_signed,pcm_rms_error,reference_ac_rms,optimized_ac_rms,amplitude_error_percent,reference_clips,optimized_clips,clip_disagreement");
    compare<false>(); compare<true>();
}
