#include "bench.h"
#include "display.h"
#include "controls.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/util/queue.h"
#include "pico/critical_section.h"
#include "hardware/clocks.h"
#include <cstdio>
#include <cstring>

namespace {
queue_t commands;
critical_section_t status_lock;
Status shared_status;
uint32_t core1_stack[4096];
void worker() {
    static uint16_t raw[ADC_WORDS];
    static int16_t audio[AUDIO_WORDS];
    static Status st;
    apply_settings(st.settings);
    stream_init();
    for (;;) {
        Settings next;
        if(queue_try_remove(&commands,&next)) {
            st.settings=next; ++st.revision; apply_settings(next);
        }
        if(!stream_read(raw)) { tight_loop_contents(); continue; }
        uint32_t start=time_us_32();
        process_samples(raw,audio,st);
        const uint32_t process_end=time_us_32();
        stream_audio(audio,st.settings.volume);
        const uint32_t output_end=time_us_32();
        st.dsp_us=output_end-start;
        st.timing.process.record(process_end-start);
        st.timing.output.record(output_end-process_end);
        if(st.dsp_us>st.max_us) st.max_us=st.dsp_us;
        if(st.dsp_us>4266) ++st.deadline_misses;
        ++st.blocks;
        if(st.blocks%32==0) {
            st.health=stream_health();
            critical_section_enter_blocking(&status_lock);
            shared_status=st;
            critical_section_exit(&status_lock);
        }
    }
}
void help() {
    puts("\nPico ADC/DSP suite: commands are lowercase, one per line.");
    puts("preset 0..8: BASIC, IQ-AM, IF-AM, IF-AMSYNC, IF-FM, IF-FSK, USB, LSB, IF-CW");
    puts("mode basic|am|amsync|fm|usb|lsb|cw; input iq|i|q; tune -12000..12000 Hz");
    puts("bw 0..4; agc 0=fast 1=medium 2=slow 3=long 4=manual; gain 0..10 (2^n)");
    puts("volume 0..100; squelch -1=off or 0..12; hang 0..7; swap 0|1; iqcorr 0|1");
    puts("deemph 0..2; bass/treble 0..4; blanker 0..6; notch/nr 0|1");
    puts("smooth 1..15; threshold 0=adaptive or 1..6; cw 100..2000 Hz");
    puts("page 0..4 (4=timing); stream 0|1; status; help. Reboot resets settings and DSP history.");
    puts("BASIC ignores receiver controls except gain/volume. IF forces swap/iqcorr off.");
}
void report(const Status &st) {
    const auto &s=st.settings; const auto &h=st.health;
    printf("B=%lu rev=%lu %s %s tune=%d bw=%d agc=%d gain=2^%d vol=%d\n",
        (unsigned long)st.blocks,(unsigned long)st.revision,mode_name(s.mode),input_name(s.input),
        s.tune,s.bw,s.agc,s.gain,s.volume);
    printf("I min/mean/max/rms=%u/%u/%u/%u Q=%u/%u/%u/%u MAG=%d AUD rms/peak=%d/%d clips=%lu\n",
        st.lo[0],st.mean[0],st.hi[0],st.rms[0],st.lo[1],st.mean[1],st.hi[1],st.rms[1],
        st.magnitude,st.audio_rms,st.audio_peak,(unsigned long)st.clips);
    printf("us=%lu max=%lu deadline=%lu ADCdrop=%lu FIFO=%lu AUDunder=%lu AUDdrop=%lu IRQlate=%lu\n",
        (unsigned long)st.dsp_us,(unsigned long)st.max_us,(unsigned long)st.deadline_misses,
        (unsigned long)h.adc_drops,(unsigned long)h.adc_overflows,(unsigned long)h.audio_underruns,
        (unsigned long)h.audio_drops,(unsigned long)h.late_irqs);
    printf("sq=%d hang=%d swap=%d iqcorr=%d deemph=%d bass=%d treble=%d blanker=%d notch=%d nr=%d smooth=%d threshold=%d cw=%d\n",
        s.squelch,s.hang,s.input?0:s.swap,s.input?0:s.iqcorr,s.deemph,s.bass,s.treble,
        s.blanker,s.notch,s.nr,s.smooth,s.threshold,s.cw);
    const auto &t=st.timing;
    printf("TIMING us latest/max PROC=%lu/%lu OUT=%lu/%lu IN=%lu/%lu DSP=%lu/%lu STAT=%lu/%lu FFT=%lu/%lu\n",
        (unsigned long)t.process.latest_us,(unsigned long)t.process.max_us,
        (unsigned long)t.output.latest_us,(unsigned long)t.output.max_us,
        (unsigned long)t.input_stats.latest_us,(unsigned long)t.input_stats.max_us,
        (unsigned long)t.dsp.latest_us,(unsigned long)t.dsp.max_us,
        (unsigned long)t.audio_stats.latest_us,(unsigned long)t.audio_stats.max_us,
        (unsigned long)t.spectrum.latest_us,(unsigned long)t.spectrum.max_us);
}
}

int main() {
    set_sys_clock_khz(120000,true);
    stdio_init_all();
    queue_init(&commands,sizeof(Settings),8);
    critical_section_init(&status_lock);
    display_init(18,19,0x3c);
    controls_init();
    multicore_launch_core1_with_stack(worker,core1_stack,sizeof(core1_stack));
    Settings requested;
    Status st;
    FrontPanel panel;
    char line[96]{}; unsigned length=0;
    bool overflow=false, streaming=false;
    uint32_t next_display=0, next_report=0;
    help();
    for(;;) {
        critical_section_enter_blocking(&status_lock); st=shared_status; critical_section_exit(&status_lock);
        int c=getchar_timeout_us(0);
        if(c!=PICO_ERROR_TIMEOUT) {
            if(c=='\r' || c=='\n') {
                line[length]=0;
                if(overflow) puts("ERR line too long");
                else if(length) {
                    if(!strcmp(line,"help")) help();
                    else if(!strcmp(line,"status")) report(st);
                    else if(!strcmp(line,"stream 0")) streaming=false;
                    else if(!strcmp(line,"stream 1")) streaming=true;
                    else if(!strncmp(line,"page ",5) && strlen(line)==6 && line[5]>='0' && line[5]<'0'+STATUS_PAGE_COUNT) panel.set_page(line[5]-'0');
                    else {
                        Settings candidate=requested;
                        if(!parse_setting(line,candidate)) puts("ERR unknown command/value; type help");
                        else if(!queue_try_add(&commands,&candidate)) puts("ERR busy; retry");
                        else { requested=candidate; puts("OK queued; allow 2 s settling"); }
                    }
                }
                length=0; overflow=false;
            } else if(c==8 || c==127) { if(length) --length; }
            else if(c>=32 && c<127) {
                if(length<sizeof(line)-1) line[length++]=c; else overflow=true;
            }
        }
        uint32_t now=to_ms_since_boot(get_absolute_time());
        const PanelInput input = controls_read();
        Settings candidate;
        if (panel.update(input, requested, candidate, now)) {
            const bool accepted = queue_try_add(&commands, &candidate);
            if (accepted) requested = candidate;
            panel.acknowledge(accepted, now);
        }
        // Redraw immediately on input; GPIO interrupts keep recording turns during I2C.
        if(input.turns || input.coarse_turns || input.ok || input.back) next_display=now;
        if(int32_t(now-next_display)>=0) {
            next_display=now+250;
            if(panel.menu_open()) display_menu(panel,requested,now);
            else display_status(st,panel.page(),panel.notice(now));
        }
        if(streaming && int32_t(now-next_report)>=0) { next_report=now+1000; report(st); }
        sleep_ms(1);
    }
}
