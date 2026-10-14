#include "timing_capture.h"
#include <cassert>
#include <cstdio>
int main() {
    CaptureCounters old{96,17,300,4,{5,6,7,8,9},123456};
    TimingCapture c;
    c.begin(old);
    uint32_t values[CAPTURE_STAGES]{};
    for(unsigned b=96;b<256;++b) {
        const bool fft=b%64==0;
        values[0]=fft?7000:3000; values[4]=fft?3500:0;
        c.record(fft,values,4);
    }
    auto end=old; end.blocks+=160; end.late+=2;
    c.finish(end);
    assert(c.ordinary.count==158 && c.spectrum.count==2);
    assert(c.spectrum.late==2 && c.ordinary.late==0);
    assert(c.spectrum.max_us[0]==7000 && c.spectrum.max_us[4]==3500);
    assert(c.ordinary.sum_us[0]==158*3000 && c.start.late==17 && c.end.late==19);
    assert(!c.settings_changed && !c.active);
    c.record(true,values,9);
    assert(c.spectrum.count==2); // Closed windows freeze even if snapshots continue.
    c.begin(end);
    c.record(false,values,5);
    assert(c.id==2 && c.settings_changed && c.start.late==19);
    assert(c.ordinary.count==1 && c.spectrum.count==0);
    puts("PASS all-block window: unpublished spectrum peaks, counts, sums, boundaries, revision detection and history preservation");
}
