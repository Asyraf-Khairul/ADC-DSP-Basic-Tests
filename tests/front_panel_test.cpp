// Host-side behavioral tests: no Pico SDK or attached hardware required.
// g++ -std=c++17 -I. tests/front_panel_test.cpp front_panel.cpp settings.cpp -o build/front_panel_test.exe
#include "front_panel.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <initializer_list>

namespace {
constexpr PanelInput ok{0,0,true,false}, back{0,0,false,true};
bool update(FrontPanel &panel, PanelInput input, const Settings &s, Settings &next) {
    return panel.update(input, s, next, 100);
}
void select(FrontPanel &panel, const char *label, const Settings &s) {
    Settings next;
    if (!panel.menu_open()) assert(!update(panel,ok,s,next));
    for (unsigned n = 0; n < panel.count() && strcmp(panel.label(panel.selected()),label); ++n)
        assert(!update(panel,{1},s,next));
    assert(!strcmp(panel.label(panel.selected()),label));
    assert(!update(panel,ok,s,next));
    assert(panel.editing());
}
void encoder_tests() {
    EncoderDecoder encoder;
    encoder.reset(3);
    int turns = 0;
    for (unsigned state : {1u,0u,2u,3u}) turns += encoder.update(state);
    assert(turns == 1);
    for (unsigned state : {2u,0u,1u,3u}) turns += encoder.update(state);
    assert(turns == 0);
    // Contact bounce and a partial reversal must not produce phantom detents.
    for (unsigned state : {1u,3u,1u,0u,1u,0u,2u,3u}) turns += encoder.update(state);
    assert(turns == 1);
    encoder.reset(3);
    assert(encoder.update(0) == 0); // Impossible diagonal transition.
    assert(encoder.update(3) == 0);
    for (unsigned state : {1u,0u,2u}) assert(encoder.update(state) == 0);
    assert(encoder.update(3) == 1);
    for (unsigned n = 0; n < 80; ++n) {
        for (unsigned state : {1u,0u,2u}) assert(encoder.update(state) == 0);
        assert(encoder.update(3) == 1);
    }
}
void button_tests() {
    DebouncedButton button;
    for (unsigned n = 0; n < 15; ++n) assert(!button.update(true));
    assert(!button.update(false));
    for (unsigned n = 0; n < 19; ++n) assert(!button.update(true));
    assert(button.update(true) && button.down());
    for (unsigned n = 0; n < 100; ++n) assert(!button.update(true));
    for (unsigned n = 0; n < 19; ++n) assert(!button.update(false));
    assert(button.update(false) && !button.down());
}
void navigation_tests() {
    FrontPanel panel;
    Settings s, next;
    for (unsigned p = 1; p <= STATUS_PAGE_COUNT; ++p) {
        assert(!update(panel,back,s,next)); assert(panel.page() == p%STATUS_PAGE_COUNT);
    }
    panel.set_page(4); assert(panel.page() == 4);
    panel.set_page(STATUS_PAGE_COUNT); assert(panel.page() == 0);
    assert(!update(panel,{5},s,next)); // BASIC has no digital tuning.
    assert(next.tune == 0 && panel.notice(100));
    select(panel,"Volume",s);
    assert(!update(panel,{10},s,next));
    assert(!update(panel,back,s,next)); // Cancel keeps the old setting.
    assert(next.volume == 25 && !panel.editing() && panel.menu_open());
    assert(!update(panel,ok,s,next));
    assert(!update(panel,{5},s,next));
    assert(update(panel,ok,s,next) && next.volume == 30);
    panel.acknowledge(false,100); // Full queue: keep draft for retry.
    assert(panel.editing());
    assert(update(panel,ok,s,next) && next.volume == 30);
    panel.acknowledge(true,100); s=next;
    assert(!panel.editing() && panel.notice(101) && !panel.notice(2100));
    select(panel,"Preset",s);
    assert(!update(panel,{2},s,next));
    assert(update(panel,ok,s,next));
    assert(next.mode == 0 && next.input == 1 && next.tune == 8000 && next.volume == 25);
    panel.acknowledge(true,100); s=next;
    assert(!update(panel,back,s,next) && !panel.menu_open());
    assert(update(panel,{1},s,next) && next.tune == 8100); s=next;
    assert(update(panel,{0,1},s,next) && next.tune == 9100); s=next;
    assert(update(panel,{0,128},s,next) && next.tune == 12000); s=next;
    assert(!update(panel,{0,1},s,next));
    select(panel,"Tune step",s);
    assert(!update(panel,{-2},s,next));
    assert(!update(panel,ok,s,next) && !panel.editing());
    assert(!update(panel,back,s,next));
    assert(update(panel,{-1},s,next) && next.tune == 11999);
}
void all_settings_tests() {
    struct Field { const char *label; int Settings::*member; int low,high; };
    const Field fields[] = {
        {"Mode",&Settings::mode,0,6}, {"Input",&Settings::input,0,2},
        {"Tune",&Settings::tune,-12000,12000}, {"Bandwidth",&Settings::bw,0,4},
        {"AGC",&Settings::agc,0,4}, {"Gain",&Settings::gain,0,10},
        {"Volume",&Settings::volume,0,100}, {"Squelch",&Settings::squelch,-1,12},
        {"Hang",&Settings::hang,0,7}, {"IQ swap",&Settings::swap,0,1},
        {"IQ corr",&Settings::iqcorr,0,1}, {"Deemph",&Settings::deemph,0,2},
        {"Bass",&Settings::bass,0,4}, {"Treble",&Settings::treble,0,4},
        {"Blanker",&Settings::blanker,0,6}, {"Notch",&Settings::notch,0,1},
        {"Noise red",&Settings::nr,0,1}, {"NR smooth",&Settings::smooth,1,15},
        {"NR thresh",&Settings::threshold,0,6}, {"CW tone",&Settings::cw,100,2000}
    };
    for (const auto &field : fields) {
        FrontPanel panel;
        Settings s=preset(2), next;
        select(panel,field.label,s);
        assert(!update(panel,{100000},s,next));
        char value[22]; panel.value(panel.selected(),s,value,sizeof(value));
        assert(strlen(value) <= 21);
        assert(update(panel,ok,s,next) && next.*(field.member) == field.high);
        panel.acknowledge(true,100); s=next;
        assert(!update(panel,ok,s,next));
        assert(!update(panel,{-100000},s,next));
        assert(update(panel,ok,s,next) && next.*(field.member) == field.low);
    }
    for (unsigned p=0; p<PRESET_COUNT; ++p) {
        FrontPanel panel;
        Settings s=preset(p); char value[22];
        panel.value(0,s,value,sizeof(value)); assert(value[0] == char('0'+p));
    }
    Settings s; assert(parse_setting("volume 30",s) && s.volume == 30);
    assert(!parse_setting("volume 101",s) && s.volume == 30);
}
}
int main() {
    encoder_tests(); button_tests(); navigation_tests(); all_settings_tests();
    puts("PASS: encoder bounce/direction, button debounce, menu cancel/retry, presets, tuning and all setting limits");
}
