// One scenario per fresh process: isolates all function-static receiver states.
// SDK synchronization shims are single-threaded; this is not a DMA/IRQ test.
#include "rx_dsp.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace {
constexpr double pi=3.14159265358979323846;
rx_dsp receiver; // Same static zero-initialization as the firmware.
uint16_t raw[2048]; int16_t audio[64];
uint16_t adc(double x) { return std::clamp<int>(std::lround(x),0,4095); }
}
int main(int argc, char **argv) {
    if(argc!=3) return 2;
    const int scenario=std::atoi(argv[1]);
    const int mode=scenario>=37 && scenario<=39 ? 4 : scenario>=16 && scenario<28 ? (scenario-16)%6 : 0;
    const bool effects=scenario>=22 && scenario<28;
    const double carrier=scenario>=16 ? 8000 : 1500;
    const double tune=scenario==32 ? -12000 : scenario==33 ? 12000 : scenario>=16 ? 8000 : 0;
    const double depths[]={0.0,0.1,0.3,0.6,0.9};
    const double depth=scenario<5 ? depths[scenario] : 0.3;
    const double amplitude=scenario==5 ? 5 : scenario==6 ? 40 : scenario==7 ? 600 : scenario==11 ? 1600 : 180;
    const int gain=scenario>=37 && scenario<=39 ? 0 : scenario==11 ? 10 : 4;
    receiver.set_mode(mode,scenario==34?0:scenario==35?4:2); receiver.set_frequency_offset_Hz(tune);
    receiver.set_agc_control(effects?0:4,gain); receiver.set_swap_iq(scenario==28 || scenario==29);
    receiver.set_iq_correction(effects || scenario==29 ?1:0); receiver.set_cw_sidetone_Hz(700);
    receiver.set_gain_cal_dB(62); receiver.set_squelch_enabled(false); receiver.set_squelch(0,3);
    receiver.set_deemphasis(effects?2:0); receiver.set_bass(effects?2:0); receiver.set_treble(effects?2:0);
    receiver.set_impulse_threshold(effects?3:0); receiver.set_auto_notch(effects);
    receiver.set_noise_reduction(effects,10,1); receiver.set_spectrum_smoothing(2); receiver.amsync_reset();
    FILE *out=std::fopen(argv[2],"wb"); if(!out) return 3;
    uint8_t spectrum[256]{}; uint8_t scale=0;
    for(unsigned block=0;block<512;++block) {
        if(scenario==36 && block==256) { receiver.set_swap_iq(1); receiver.set_frequency_offset_Hz(-8000); }
        host_device_us=uint64_t(block)*2048*1000000/480000;
        for(unsigned p=0;p<1024;++p) {
            const double t=double(uint64_t(block)*1024+p)/240000;
            const double tq=t+1.0/480000;
            double a=amplitude;
            if(scenario==10) a=block<128?0:block<256?600:180;
            const double bias_i=scenario==8?2200:scenario==9?(block<128?2048:2250):scenario==14?0:2048;
            const double bias_q=scenario==8?1800:scenario==9?(block<128?2048:1800):scenario==14?1:2048;
            double i=a*(1+depth*std::sin(2*pi*1000*t))*std::cos(2*pi*carrier*t);
            double q=a*(1+depth*std::sin(2*pi*1000*tq))*std::sin(2*pi*carrier*tq);
            if(scenario==12 || scenario==14) i=q=0;
            if(scenario==13) { i=180*std::sin(2*pi*1000*t); q=180*std::sin(2*pi*1000*tq); }
            if(scenario==15) { i=2048*std::sin(2*pi*1000*t); q=2048*std::cos(2*pi*1000*tq); }
            if(mode==4) {
                const double beta=scenario==38?0.5:scenario==39?0:1;
                i=a*std::cos(2*pi*carrier*t+beta*std::sin(2*pi*500*t));
                q=a*std::sin(2*pi*carrier*tq+beta*std::sin(2*pi*500*tq));
            }
            raw[2*p]=adc(bias_i+i); raw[2*p+1]=adc(bias_q+q);
            // Match bench.cpp real-IF preparation after per-channel ADC statistics.
            if(scenario==30 || scenario==31 || (scenario>=37 && scenario<=39)) { raw[2*p]=raw[2*p+(scenario==31)]; raw[2*p+1]=2048; }
        }
        if(receiver.process_block(raw,audio,nullptr)!=64) return 4;
        int16_t qi,qq; while(receiver.get_raw_data(qi,qq)) {}
        const int32_t mag=receiver.get_magnitude();
        if(block%64==0) receiver.get_spectrum(spectrum,scale,scenario>=40?scenario-38:1);
        std::fwrite(audio,sizeof(audio),1,out);
        std::fwrite(&mag,sizeof(mag),1,out);
        std::fwrite(spectrum,sizeof(spectrum),1,out);
        std::fwrite(&scale,sizeof(scale),1,out);
    }
    return std::fclose(out)==0?0:5;
}
