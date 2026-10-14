#include "spectrum_log.h"
float spectrum_log_values[SPECTRUM_LOG_COUNT];
void initialise_spectrum_logs() {
    static bool ready=false;
    if(ready) return;
    spectrum_log_values[0]=-INFINITY;
    for(unsigned n=1;n<SPECTRUM_LOG_COUNT;++n) spectrum_log_values[n]=log10f(float(n));
    ready=true;
}
