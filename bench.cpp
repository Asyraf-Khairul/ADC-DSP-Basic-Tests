#include "bench.h"
#include "basic_dsp.h"
#include "input_statistics.h"
#include "receiver_dsp/rx_dsp.h"
#include "hardware/timer.h"
#include <algorithm>
#include <cmath>

namespace {
rx_dsp receiver; // Static lifetime is required by the upstream stateful implementation.
basic_dsp::BasicKernel basic;
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
    uint32_t stage_start = time_us_32();
    InputStatistics input;
    collect_input_statistics(raw,input);
    for(unsigned ch=0;ch<2;++ch) {
        st.mean[ch]=input.mean[ch]; st.lo[ch]=input.lo[ch];
        st.hi[ch]=input.hi[ch]; st.rms[ch]=input.rms[ch];
    }
    st.clips+=input.near_rail;
    uint32_t stage_end = time_us_32();
    st.timing.input_stats.record(stage_end - stage_start);
    stage_start = stage_end;
    const auto &s=st.settings;
    if(s.mode==6) {
        st.receiver_profile = {};
        st.magnitude=basic.process(raw,audio,s.gain);
    } else {
        if(s.input) for(unsigned n=0;n<ADC_WORDS;n+=2) {
            raw[n]=raw[n+(s.input==2 ? 1 : 0)];
            raw[n+1]=2048;
        }
        receiver.process_block(raw,audio,nullptr);
        st.receiver_profile = receiver.profile;
        st.magnitude=receiver.get_magnitude();
        // Drain upstream's optional decoder queue; no bit decoder is implemented here.
        int16_t i,q; while(receiver.get_raw_data(i,q)) {}
    }
    stage_end = time_us_32();
    st.timing.dsp.record(stage_end - stage_start);
    stage_start = stage_end;
    uint64_t square=0; st.audio_peak=0;
    for(unsigned n=0;n<AUDIO_WORDS;++n) {
        st.audio[n]=audio[n];
        square+=static_cast<int64_t>(audio[n])*audio[n];
        st.audio_peak=std::max(st.audio_peak,std::abs(int(audio[n])));
    }
    st.audio_rms=std::sqrt(square/double(AUDIO_WORDS));
    stage_end = time_us_32();
    st.timing.audio_stats.record(stage_end - stage_start);
    stage_start = stage_end;
    // Spectrum generation is deliberately decimated to reduce processing overhead.
    if(s.mode!=6 && st.blocks%64==0) {
        uint8_t spectrum[256], scale;
        receiver.get_spectrum(spectrum,scale,1);
        for(unsigned n=0;n<128;++n) st.spectrum[n]=std::max(spectrum[n*2],spectrum[n*2+1]);
        st.timing.spectrum.record(time_us_32() - stage_start);
    } else st.timing.spectrum.record(0);
}
