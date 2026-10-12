#include "bench.h"
#include "receiver_dsp/rx_dsp.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstdlib>

namespace {
rx_dsp receiver; // Static lifetime is required by the upstream stateful implementation.
float basic_i = 2048, basic_q = 2048, basic_dc = 0;
}
const char *mode_name(int m) {
    static const char *names[] = {"AM","AMSYNC","LSB","USB","FM","CW","BASIC"};
    return names[std::clamp(m,0,6)];
}
const char *input_name(int i) { return i == 1 ? "IF-I" : i == 2 ? "IF-Q" : "IQ"; }
Settings preset(unsigned index) {
    Settings s;
    switch (index % PRESET_COUNT) {
        case 0: break;
        case 1: s.mode=0; break;
        case 2: s.mode=0; s.input=1; s.tune=8000; break;
        case 3: s.mode=1; s.input=1; s.tune=8000; break;
        case 4: s.mode=4; s.input=1; s.tune=8000; s.gain=0; break;
        case 5: s.mode=4; s.input=1; s.tune=8250; s.gain=0; break;
        case 6: s.mode=3; break;
        case 7: s.mode=2; break;
        case 8: s.mode=5; s.input=1; s.tune=8000; break;
    }
    return s;
}
bool parse_setting(const char *line, Settings &s) {
    char key[24], value[24], extra[2];
    if (sscanf(line, "%23s %23s %1s", key, value, extra) != 2) return false;
    if (!strcmp(key,"mode")) {
        for (int m=0;m<7;++m) {
            char name[16]; snprintf(name,sizeof(name),"%s",mode_name(m));
            for (char *p=name;*p;++p) if (*p>='A' && *p<='Z') *p+=32;
            if (!strcmp(name,value)) { s.mode=m; return true; }
        }
        return false;
    }
    if (!strcmp(key,"input")) {
        if (!strcmp(value,"iq")) s.input=0;
        else if (!strcmp(value,"i")) s.input=1;
        else if (!strcmp(value,"q")) s.input=2;
        else return false;
        return true;
    }
    char *end=nullptr;
    long v=strtol(value,&end,10);
    if (!*value || *end) return false;
    if (!strcmp(key,"preset") && v>=0 && v<PRESET_COUNT) { s=preset(v); return true; }
    struct Field { const char *name; int Settings::*member; int lo,hi; };
    static const Field fields[] = {
        {"tune",&Settings::tune,-12000,12000}, {"bw",&Settings::bw,0,4},
        {"agc",&Settings::agc,0,4}, {"gain",&Settings::gain,0,10},
        {"volume",&Settings::volume,0,100}, {"squelch",&Settings::squelch,-1,12},
        {"hang",&Settings::hang,0,7}, {"swap",&Settings::swap,0,1},
        {"iqcorr",&Settings::iqcorr,0,1}, {"deemph",&Settings::deemph,0,2},
        {"bass",&Settings::bass,0,4}, {"treble",&Settings::treble,0,4},
        {"blanker",&Settings::blanker,0,6}, {"notch",&Settings::notch,0,1},
        {"nr",&Settings::nr,0,1}, {"smooth",&Settings::smooth,1,15},
        {"threshold",&Settings::threshold,0,6}, {"cw",&Settings::cw,100,2000}
    };
    for (const auto &f: fields) if (!strcmp(key,f.name)) {
        if (v<f.lo || v>f.hi) return false;
        s.*(f.member)=v; return true;
    }
    return false;
}
void apply_settings(const Settings &s) {
    receiver.set_mode(std::min(s.mode,5),s.bw);
    receiver.set_frequency_offset_Hz(s.tune);
    receiver.set_agc_control(s.agc,s.gain);
    receiver.set_swap_iq(s.input ? 0 : s.swap);
    receiver.set_iq_correction(s.input ? 0 : s.iqcorr);
    receiver.set_cw_sidetone_Hz(s.cw);
    receiver.set_gain_cal_dB(62);
    receiver.set_squelch_enabled(s.squelch>=0);
    receiver.set_squelch(std::max(0,s.squelch),s.hang);
    receiver.set_deemphasis(s.deemph);
    receiver.set_bass(s.bass);
    receiver.set_treble(s.treble);
    receiver.set_impulse_threshold(s.blanker);
    receiver.set_auto_notch(s.notch);
    receiver.set_noise_reduction(s.nr,s.smooth,s.threshold);
    receiver.set_spectrum_smoothing(2);
    receiver.amsync_reset();
}
void process_samples(uint16_t *raw, int16_t *audio, Status &st) {
    for (unsigned ch=0;ch<2;++ch) {
        uint32_t sum=0; uint64_t squares=0;
        uint16_t lo=4095,hi=0;
        for (unsigned n=ch;n<ADC_WORDS;n+=2) {
            auto v=raw[n]&4095u;
            raw[n]=v; sum+=v; squares+=v*v;
            lo=std::min<unsigned>(lo,v); hi=std::max<unsigned>(hi,v);
            if(v<4 || v>4091) ++st.clips;
        }
        double mean=sum/1024.0;
        st.mean[ch]=std::lround(mean); st.lo[ch]=lo; st.hi[ch]=hi;
        st.rms[ch]=std::sqrt(std::max(0.0,squares/1024.0-mean*mean));
    }
    const auto &s=st.settings;
    if(s.mode==6) {
        double total_mag=0;
        for(unsigned n=0;n<AUDIO_WORDS;++n) {
            float sum=0;
            for(unsigned k=0;k<16;++k) {
                unsigned pos=2*(n*16+k);
                basic_i+=(raw[pos]-basic_i)/1024;
                basic_q+=(raw[pos+1]-basic_q)/1024;
                float i=raw[pos]-basic_i, q=raw[pos+1]-basic_q;
                sum+=std::sqrt(i*i+q*q);
            }
            float mag=sum/16;
            total_mag+=mag;
            basic_dc+=(mag-basic_dc)/256;
            audio[n]=std::clamp<int>((mag-basic_dc)*(1<<s.gain),-32767,32767);
        }
        st.magnitude=total_mag/AUDIO_WORDS;
    } else {
        if(s.input) for(unsigned n=0;n<ADC_WORDS;n+=2) {
            raw[n]=raw[n+(s.input==2 ? 1 : 0)];
            raw[n+1]=2048;
        }
        receiver.process_block(raw,audio,nullptr);
        st.magnitude=receiver.get_magnitude();
        // Drain upstream's optional decoder queue; no bit decoder is implemented here.
        int16_t i,q; while(receiver.get_raw_data(i,q)) {}
    }
    uint64_t square=0; st.audio_peak=0;
    for(unsigned n=0;n<AUDIO_WORDS;++n) {
        st.audio[n]=audio[n];
        square+=static_cast<int64_t>(audio[n])*audio[n];
        st.audio_peak=std::max(st.audio_peak,std::abs(int(audio[n])));
    }
    st.audio_rms=std::sqrt(square/double(AUDIO_WORDS));
    // Spectrum generation is deliberately decimated to reduce processing overhead.
    if(s.mode!=6 && st.blocks%64==0) {
        uint8_t spectrum[256], scale;
        receiver.get_spectrum(spectrum,scale,1);
        for(unsigned n=0;n<128;++n) st.spectrum[n]=std::max(spectrum[n*2],spectrum[n*2+1]);
    }
}
