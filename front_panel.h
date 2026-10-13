#pragma once
#include "bench.h"
#include <cstddef>

constexpr unsigned STATUS_PAGE_COUNT = 5;

// Hardware-independent menu and input decoding, also exercised by host tests.
struct PanelInput {
    int turns = 0, coarse_turns = 0;
    bool ok = false, back = false;
};

class EncoderDecoder {
public:
    void reset(unsigned state) { previous_ = state & 3; progress_ = 0; }
    int update(unsigned state);
private:
    unsigned previous_ = 3;
    int progress_ = 0;
};

class DebouncedButton {
public:
    // Called once per millisecond. Returns true when the debounced state changes.
    bool update(bool pressed);
    bool down() const { return stable_; }
private:
    bool stable_ = false, candidate_ = false;
    unsigned ticks_ = 0;
};

class FrontPanel {
public:
    // Returns a settings request; the caller must acknowledge queue success.
    bool update(PanelInput input, const Settings &current, Settings &next, uint32_t now);
    void acknowledge(bool accepted, uint32_t now);
    void set_page(unsigned value) { page_ = value % STATUS_PAGE_COUNT; }
    unsigned page() const { return page_; }
    bool menu_open() const { return menu_; }
    bool editing() const { return editing_; }
    unsigned selected() const { return selected_; }
    unsigned count() const;
    const char *label(unsigned item) const;
    const char *description(const Settings &current) const;
    const char *notice(uint32_t now) const;
    void value(unsigned item, const Settings &current, char *text, size_t size) const;
private:
    unsigned page_ = 0, selected_ = 0;
    bool menu_ = false, editing_ = false;
    int edit_value_ = 0, tune_step_ = 2;
    const char *notice_ = nullptr;
    uint32_t notice_until_ = 0;
    void message(const char *text, uint32_t now);
};
