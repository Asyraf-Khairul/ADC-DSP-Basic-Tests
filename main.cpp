#include "bench.h"
#include "display.h"
#include "controls.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/util/queue.h"
#include "pico/critical_section.h"
#include "hardware/clocks.h"
#include "build_identity.h"
#include <cstdio>
#include <cstring>

namespace {
queue_t commands;
queue_t capture_commands;
critical_section_t status_lock;
Status shared_status;
uint32_t core1_stack[4096];
uint32_t timer_probe_us = 0;
CaptureCounters capture_counters(const Status &st) {
    return {st.blocks, st.deadline_misses, st.clips, st.revision, stream_health(), time_us_64()};
}
void publish(Status &st) {
    st.health=stream_health();
    if (st.capture.active) st.capture.end=capture_counters(st);
    critical_section_enter_blocking(&status_lock);
    shared_status=st;
    critical_section_exit(&status_lock);
}
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
        uint8_t capture_command;
        if (queue_try_remove(&capture_commands, &capture_command)) {
            if (capture_command == 1) st.capture.begin(capture_counters(st));
            else st.capture.finish(capture_counters(st));
            publish(st);
        }
        if(!stream_read(raw)) { tight_loop_contents(); continue; }
        const bool has_spectrum = st.settings.mode != 6 && st.blocks%64 == 0;
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
        const uint32_t bookkeeping_start = time_us_32();
        const auto &r=st.receiver_profile;
        const uint32_t values[CAPTURE_STAGES] = {st.dsp_us,
            st.timing.input_stats.latest_us,st.timing.dsp.latest_us,
            st.timing.audio_stats.latest_us,st.timing.spectrum.latest_us,
            st.timing.output.latest_us,r.cic_mix,r.filter,r.demod_audio,r.capture,
            r.fft_forward,r.fft_bins,r.fft_inverse};
        st.capture.record(has_spectrum, values, st.revision);
        if (st.capture.active) {
            const uint32_t overhead = time_us_32() - bookkeeping_start;
            st.capture.bookkeeping_max_us = std::max(st.capture.bookkeeping_max_us, overhead);
            st.capture.bookkeeping_sum_us += overhead;
        }
        ++st.blocks;
        if(st.blocks%32==0) {
            publish(st);
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
    puts("measure start|stop: all-block timing window; cumulative diagnostics never reset.");
    printf("BUILD label=%s source_sha256=%s base=%s\n", BENCH_BUILD_LABEL, BENCH_SOURCE_SHA256, BENCH_BASE_COMMIT);
    printf("INSTRUMENT sys_clock_Hz=%lu timer_1024_reads_us=%lu extra_receiver_timer_reads_per_block=9\n",
        (unsigned long)clock_get_hz(clk_sys),(unsigned long)timer_probe_us);
}
void report_capture_counters(const char *name, const CaptureCounters &c) {
    printf("%s B=%lu rev=%lu deadline=%lu clips=%lu ADCdrop=%lu FIFO=%lu AUDunder=%lu AUDdrop=%lu IRQlate=%lu device_us=%llu\n",
        name,(unsigned long)c.blocks,(unsigned long)c.revision,(unsigned long)c.late,
        (unsigned long)c.clips,(unsigned long)c.health.adc_drops,
        (unsigned long)c.health.adc_overflows,(unsigned long)c.health.audio_underruns,
        (unsigned long)c.health.audio_drops,(unsigned long)c.health.late_irqs,
        (unsigned long long)c.device_us);
}
void report_group(const char *name, const TimingGroup &g) {
    const char *names[CAPTURE_STAGES] = {"TOTAL","IN","DSP","STAT","FFT","OUT",
        "CICMIX","FILTER","DEMOD","CAPTURE","FFTFWD","FFTBINS","FFTINV"};
    printf("WINDOW_%s N=%lu LATE=%lu",name,(unsigned long)g.count,(unsigned long)g.late);
    for (unsigned k=0;k<CAPTURE_STAGES;++k)
        printf(" %s=%lu/%llu",names[k],(unsigned long)g.max_us[k],(unsigned long long)g.sum_us[k]);
    puts("");
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
    const auto &r=st.receiver_profile;
    printf("RX latest_us CICMIX=%lu FILTER=%lu DEMOD=%lu CAPTURE=%lu FFTFWD=%lu FFTBINS=%lu FFTINV=%lu\n",
        (unsigned long)r.cic_mix,(unsigned long)r.filter,(unsigned long)r.demod_audio,
        (unsigned long)r.capture,(unsigned long)r.fft_forward,(unsigned long)r.fft_bins,
        (unsigned long)r.fft_inverse);
    printf("MEASURE id=%lu active=%u settings_changed=%u\n",(unsigned long)st.capture.id,
        st.capture.active,st.capture.settings_changed);
    report_capture_counters("WINDOW_START",st.capture.start);
    report_capture_counters("WINDOW_END",st.capture.end);
    report_group("ORDINARY",st.capture.ordinary);
    report_group("SPECTRUM",st.capture.spectrum);
    printf("BOOKKEEP max_us=%lu sum_us=%llu\n",(unsigned long)st.capture.bookkeeping_max_us,
        (unsigned long long)st.capture.bookkeeping_sum_us);
    puts("ENDSTATUS");
}
}

int main() {
    set_sys_clock_khz(120000,true);
    stdio_init_all();
    volatile uint32_t timer_sink = 0;
    const uint32_t timer_start = time_us_32();
    for(unsigned i=0;i<1024;++i) timer_sink=time_us_32();
    timer_probe_us=time_us_32()-timer_start;
    (void)timer_sink;
    queue_init(&commands,sizeof(Settings),8);
    queue_init(&capture_commands,sizeof(uint8_t),4);
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
                    else if(!strcmp(line,"measure start") || !strcmp(line,"measure stop")) {
                        const uint8_t command = !strcmp(line,"measure start") ? 1 : 2;
                        puts(queue_try_add(&capture_commands,&command) ? "OK measurement queued" : "ERR busy; retry");
                    }
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
