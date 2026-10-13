#include "front_panel.h"
#include <algorithm>
#include <cstdio>

namespace {
enum Style { Number, Preset, Mode, Input, Hertz, Step, Bandwidth, Agc, Gain,
             Percent, Squelch, Hang, Toggle, Deemphasis, Level, Threshold };
struct Item {
    const char *label;
    int Settings::*member;
    int low, high, increment;
    Style style;
    const char *help;
};
constexpr int tune_steps[] = {1, 10, 100, 1000};
constexpr Item items[] = {
    {"Preset", nullptr, 0, 8, 1, Preset, "Resets all settings"},
    {"Mode", &Settings::mode, 0, 6, 1, Mode, "Demodulation mode"},
    {"Input", &Settings::input, 0, 2, 1, Input, "IQ or single IF pin"},
    {"Tune", &Settings::tune, -12000, 12000, 1, Hertz, "Digital offset in Hz"},
    {"Tune step", nullptr, 0, 3, 1, Step, "Encoder fine step"},
    {"Bandwidth", &Settings::bw, 0, 4, 1, Bandwidth, "0 narrow / 4 wide"},
    {"AGC", &Settings::agc, 0, 4, 1, Agc, "Automatic/manual gain"},
    {"Gain", &Settings::gain, 0, 10, 1, Gain, "Gain or AGC ceiling"},
    {"Volume", &Settings::volume, 0, 100, 1, Percent, "PWM output level"},
    {"Squelch", &Settings::squelch, -1, 12, 1, Squelch, "Mute weak signals"},
    {"Hang", &Settings::hang, 0, 7, 1, Hang, "Squelch hold time"},
    {"IQ swap", &Settings::swap, 0, 1, 1, Toggle, "Exchange I and Q"},
    {"IQ corr", &Settings::iqcorr, 0, 1, 1, Toggle, "Correct IQ imbalance"},
    {"Deemph", &Settings::deemph, 0, 2, 1, Deemphasis, "FM de-emphasis"},
    {"Bass", &Settings::bass, 0, 4, 1, Level, "Bass processing"},
    {"Treble", &Settings::treble, 0, 4, 1, Level, "Treble processing"},
    {"Blanker", &Settings::blanker, 0, 6, 1, Level, "Impulse rejection"},
    {"Notch", &Settings::notch, 0, 1, 1, Toggle, "Remove steady tones"},
    {"Noise red", &Settings::nr, 0, 1, 1, Toggle, "Noise reduction"},
    {"NR smooth", &Settings::smooth, 1, 15, 1, Number, "Noise smoothing"},
    {"NR thresh", &Settings::threshold, 0, 6, 1, Threshold, "0 adaptive / 1..6"},
    {"CW tone", &Settings::cw, 100, 2000, 10, Hertz, "Nominal sidetone Hz"},
};
constexpr unsigned item_count = sizeof(items) / sizeof(items[0]);
const char *preset_names[] = {"0 BASIC", "1 IQ-AM", "2 IF-AM", "3 IF-SAM",
    "4 IF-FM", "5 IF-FSK", "6 USB", "7 LSB", "8 IF-CW"};

int matching_preset(const Settings &s) {
    for (unsigned p = 0; p < PRESET_COUNT; ++p) {
        const Settings expected = preset(p);
        bool same = true;
        for (const auto &item : items)
            if (item.member && s.*(item.member) != expected.*(item.member)) same = false;
        if (same) return p;
    }
    return -1;
}
int adjusted(int value, int turns, int step, int low, int high) {
    return static_cast<int>(std::clamp<int64_t>(int64_t(value) + int64_t(turns)*step, low, high));
}
}

int EncoderDecoder::update(unsigned state) {
    state &= 3;
    // Reverse transitions cancel bounce. A diagonal transition is invalid.
    static constexpr int8_t transitions[] = {0,-1,1,0, 1,0,0,-1, -1,0,0,1, 0,1,-1,0};
    if ((previous_ ^ state) == 3) progress_ = 0;
    else progress_ += transitions[(previous_ << 2) | state];
    previous_ = state;
    if (progress_ >= 4) { progress_ = 0; return 1; }
    if (progress_ <= -4) { progress_ = 0; return -1; }
    return 0;
}

bool DebouncedButton::update(bool pressed) {
    if (pressed != candidate_) { candidate_ = pressed; ticks_ = 0; }
    if (ticks_ < 20) ++ticks_;
    if (ticks_ == 20 && stable_ != candidate_) { stable_ = candidate_; return true; }
    return false;
}

unsigned FrontPanel::count() const { return item_count; }
const char *FrontPanel::label(unsigned item) const { return items[item % item_count].label; }
void FrontPanel::message(const char *text, uint32_t now) { notice_ = text; notice_until_ = now + 2000; }
const char *FrontPanel::notice(uint32_t now) const {
    return notice_ && int32_t(now - notice_until_) < 0 ? notice_ : nullptr;
}
const char *FrontPanel::description(const Settings &s) const {
    const auto member = items[selected_].member;
    if (s.mode == 6 && member && member != &Settings::mode && member != &Settings::gain &&
        member != &Settings::volume) return "Inactive in BASIC";
    if (s.input && (member == &Settings::swap || member == &Settings::iqcorr)) return "IQ input only";
    return items[selected_].help;
}
void FrontPanel::value(unsigned index, const Settings &s, char *text, size_t size) const {
    const auto &item = items[index % item_count];
    int v = item.member ? s.*(item.member) : item.style == Step ? tune_step_ : matching_preset(s);
    if (editing_ && index == selected_) v = edit_value_;
    switch (item.style) {
        case Preset: snprintf(text, size, "%s", v < 0 ? "Custom" : preset_names[v]); break;
        case Mode: snprintf(text, size, "%s", mode_name(v)); break;
        case Input: snprintf(text, size, "%s", input_name(v)); break;
        case Step: snprintf(text, size, "%d Hz", tune_steps[v]); break;
        case Hertz: snprintf(text, size, "%d Hz", v); break;
        case Bandwidth: {
            const char *names[] = {"0 V.narrow","1 Narrow","2 Normal","3 Wide","4 V.wide"};
            snprintf(text, size, "%s", names[v]); break;
        }
        case Agc: {
            const char *names[] = {"Fast","Medium","Slow","Long","Manual"};
            snprintf(text, size, "%s", names[v]); break;
        }
        case Gain: snprintf(text, size, "%d x%d", v, 1 << v); break;
        case Percent: snprintf(text, size, "%d%%", v); break;
        case Hang: {
            constexpr int times[] = {50,100,200,500,1000,2000,3000,5000};
            snprintf(text, size, "%d ms", times[v]); break;
        }
        case Toggle: snprintf(text, size, "%s", v ? "On" : "Off"); break;
        case Deemphasis: snprintf(text, size, "%s", v == 0 ? "Off" : v == 1 ? "50 us" : "75 us"); break;
        case Squelch: if (v < 0) { snprintf(text, size, "Off"); break; } [[fallthrough]];
        case Level: if (v == 0 && item.style == Level) { snprintf(text, size, "Off"); break; } [[fallthrough]];
        case Threshold: if (v == 0 && item.style == Threshold) { snprintf(text, size, "Adaptive"); break; } [[fallthrough]];
        default: snprintf(text, size, "%d", v); break;
    }
}

bool FrontPanel::update(PanelInput input, const Settings &current, Settings &next, uint32_t now) {
    next = current;
    if (input.back) {
        if (editing_) editing_ = false;
        else if (menu_) menu_ = false;
        else page_ = (page_ + 1) % STATUS_PAGE_COUNT;
        notice_ = nullptr;
        return false;
    }
    if (!menu_) {
        if (input.ok) { menu_ = true; notice_ = nullptr; return false; }
        if (!input.turns && !input.coarse_turns) return false;
        if (current.mode == 6) { message("Tune: receiver only", now); return false; }
        const int64_t delta = int64_t(input.turns)*tune_steps[tune_step_] +
            int64_t(input.coarse_turns)*std::max(1000, tune_steps[tune_step_]*10);
        next.tune = static_cast<int>(std::clamp<int64_t>(int64_t(current.tune)+delta, -12000, 12000));
        page_ = 0;
        return next.tune != current.tune;
    }
    const int turns = input.turns + input.coarse_turns;
    if (!editing_) {
        selected_ = static_cast<unsigned>((int64_t(selected_) + turns % int(item_count) + item_count) % item_count);
        if (input.ok) {
            const auto &item = items[selected_];
            edit_value_ = item.member ? current.*(item.member) :
                item.style == Step ? tune_step_ : std::max(0, matching_preset(current));
            editing_ = true;
            notice_ = nullptr;
        }
        return false;
    }
    const auto &item = items[selected_];
    edit_value_ = adjusted(edit_value_, turns,
        item.member == &Settings::tune ? tune_steps[tune_step_] : item.increment, item.low, item.high);
    if (!input.ok) return false;
    if (item.style == Step) {
        tune_step_ = edit_value_; editing_ = false; message("Step set", now); return false;
    }
    if (item.style == Preset) next = preset(edit_value_);
    else next.*(item.member) = edit_value_;
    return true;
}
void FrontPanel::acknowledge(bool accepted, uint32_t now) {
    if (accepted) { editing_ = false; message("Queued; wait 2s", now); }
    else message("Busy: press OK again", now);
}
