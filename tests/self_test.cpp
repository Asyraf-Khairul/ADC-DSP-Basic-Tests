// Synthetic tests of the actual bench/receiver pipeline on a Pico.
// No generator needed. Does NOT exercise ADC, DMA, PWM, or real-time deadlines.
#include "bench.h"
#include "display.h"
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include <cmath>
#include <cstdio>
#include <algorithm>
#include <cstring>

namespace {
constexpr double pi=3.14159265358979323846;
enum Stimulus { Quiet, Carrier, Am, Fm, PositiveIq, NegativeIq };
static uint16_t raw[ADC_WORDS];
static uint16_t wave[960]; // 2 ms period fits all stimuli; avoid per-sample trig on Pico.
static int16_t audio[AUDIO_WORDS];
static Status st;
unsigned failures=0, checks=0;
void check(const char *name, bool ok, double value) {
    ++checks; if(!ok) ++failures;
    printf("%s %s value=%.3f\n",ok?"PASS":"FAIL",name,value);
}
// 1024-block runs; discard first half to settle DC, filter and PLL state.
double measure(Settings settings, Stimulus source, double audio_frequency=1000) {
    st={}; st.settings=settings; apply_settings(settings);
    double re=0,im=0,energy=0; unsigned count=0;
    const double step=2*pi*audio_frequency/15000;
    const double step_re=cos(step), step_im=sin(step);
    double osc_re=1, osc_im=0;
    for(unsigned n=0;n<960;++n) {
            const double t=double(n)/480000.0;
            const double angle=2*pi*8000*t;
            double sample=0;
            if(source==PositiveIq || source==NegativeIq) {
                sample=160*((n&1)? (source==PositiveIq?1:-1)*sin(2*pi*1000*t):cos(2*pi*1000*t));
            } else if((n&1)==(settings.input==2?1u:0u)) {
                if(source==Carrier) sample=160*cos(angle);
                if(source==Am) sample=160*(1+0.3*cos(2*pi*1000*t))*cos(angle);
                if(source==Fm) sample=160*cos(angle+sin(2*pi*500*t)); // +/-500 Hz
            }
            wave[n]=std::clamp<int>(std::lround(2048+sample),0,4095);
    }
    for(unsigned block=0;block<1024;++block) {
        for(unsigned n=0;n<ADC_WORDS;++n) raw[n]=wave[(block*ADC_WORDS+n)%960];
        st.blocks=block;
        process_samples(raw,audio,st);
        if(block>=512) for(unsigned n=0;n<AUDIO_WORDS;++n) {
            re+=audio[n]*osc_re; im+=audio[n]*osc_im;
            const double next_re=osc_re*step_re-osc_im*step_im;
            osc_im=osc_im*step_re+osc_re*step_im; osc_re=next_re;
            energy+=double(audio[n])*audio[n]; ++count;
        }
        if(block%128==0) { display_status(st,0); sleep_ms(1); }
    }
    double amplitude=2*sqrt(re*re+im*im)/count;
    printf("  %s %s tone=%.3f rms=%.3f\n",mode_name(settings.mode),
           input_name(settings.input),amplitude,sqrt(energy/count));
    return amplitude;
}
}
int main() {
    set_sys_clock_khz(120000,true);
    stdio_init_all(); display_init(18,19,0x3c);
    // Wait for a console so results are not silently lost. OLED shows STARTING.
    while(!stdio_usb_connected()) sleep_ms(100);
    sleep_ms(1000);
    puts("SOFTWARE SELF TEST -- no ADC/PWM verification");
    Settings s=preset(2); s.gain=0;
    auto original=s;
    check("reject invalid bandwidth",!parse_setting("bw 99",s) && s.bw==original.bw,s.bw);
    check("reject trailing tokens",!parse_setting("nr 1 extra",s),0);
    check("accept mode",parse_setting("mode fm",s) && s.mode==4,s.mode);
    s=preset(2); s.gain=0;
    double silent=measure(s,Quiet);
    check("AM quiet",silent<5,silent);
    double unmod=measure(s,Carrier);
    check("AM removes steady carrier",unmod<5,unmod);
    double am=measure(s,Am);
    check("AM 1k recovery",am>5 && am<500,am);
    s.input=2;
    double amq=measure(s,Am);
    check("IF Q input agrees with I",amq>am*0.7 && amq<am*1.3,amq);
    s=preset(3); s.gain=0;
    double sync=measure(s,Am);
    check("AMSYNC 1k recovery",sync>5,sync);
    s=preset(4);
    double fm=measure(s,Fm,500);
    check("FM 500Hz recovery",fm>500 && fm<5000,fm);
    s=preset(6); s.gain=0;
    double usb=measure(s,PositiveIq);
    double image=measure(s,NegativeIq);
    double rejection=20*log10(std::max(usb,0.001)/std::max(image,0.001));
    check("USB desired tone",usb>20,usb);
    check("USB image rejection >15dB",rejection>15,rejection);
    s=preset(7); s.gain=0;
    double lsb=measure(s,NegativeIq);
    check("LSB desired tone",lsb>20,lsb);
    s=preset(8); s.gain=0;
    double cw_hz=floor(s.cw*2048.0/15000)*15000/2048;
    double cw=measure(s,Carrier,cw_hz);
    check("CW sidetone",cw>10,cw);
    // Stress silent input in correction/NR paths (formerly zero denominators).
    s=preset(6); s.iqcorr=1; s.nr=1; s.threshold=0;
    double quiet=measure(s,Quiet);
    check("IQ correction + adaptive NR silence",quiet<5,quiet);
    // Smoke-test every processing switch; not a quantitative quality test.
    s=preset(4); s.deemph=2; s.bass=4; s.treble=4; s.blanker=6;
    s.notch=1; s.nr=1; s.threshold=6; s.smooth=15; s.agc=0; s.gain=4;
    double effects=measure(s,Fm,500);
    check("combined effects completed",std::isfinite(effects),effects);
    printf("DONE %u checks, %u failures\n",checks,failures);
    puts("Power-cycle between independent benchmark runs; flash normal UF2 for AFG tests.");
    for(;;) { sleep_ms(5000); printf("DONE %u checks, %u failures\n",checks,failures); }
}
