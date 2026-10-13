#include "bench.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <cstdlib>

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
    if (!strcmp(key,"preset") && v>=0 && v<static_cast<long>(PRESET_COUNT)) { s=preset(v); return true; }
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
