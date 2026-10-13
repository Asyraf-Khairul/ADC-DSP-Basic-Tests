#include "controls.h"
#include "pico/stdlib.h"
#include "hardware/sync.h"
#include <algorithm>

namespace {
EncoderDecoder encoder;
DebouncedButton back_button, ok_button;
repeating_timer_t button_timer;
volatile int fine_turns = 0, coarse_turns = 0;
volatile bool back_event = false, ok_event = false;
volatile bool coarse_used = false;

unsigned encoder_state() {
    const uint32_t pins = gpio_get_all();
    return ((pins >> ENCODER_GPIO_A) & 1u) * 2u + ((pins >> ENCODER_GPIO_B) & 1u);
}
void encoder_irq(uint gpio, uint32_t events) {
    (void)events;
    if (gpio != ENCODER_GPIO_A && gpio != ENCODER_GPIO_B) return;
    const int turn = encoder.update(encoder_state()) * ENCODER_DIRECTION;
    if (!turn) return;
    // Capture movement even while the foreground is sending a full OLED frame.
    if (!gpio_get(BUTTON_GPIO_BACK_COARSE)) {
        coarse_turns = std::clamp(int(coarse_turns) + turn, -128, 128);
        coarse_used = true;
    } else fine_turns = std::clamp(int(fine_turns) + turn, -128, 128);
}
bool sample_buttons(repeating_timer_t *) {
    if (back_button.update(!gpio_get(BUTTON_GPIO_BACK_COARSE)) && !back_button.down()) {
        if (!coarse_used) back_event = true;
        coarse_used = false;
    }
    if (ok_button.update(!gpio_get(BUTTON_GPIO_MENU_OK)) && !ok_button.down()) ok_event = true;
    return true;
}
}
void controls_init() {
    const unsigned pins[] = {ENCODER_GPIO_A, ENCODER_GPIO_B, BUTTON_GPIO_BACK_COARSE, BUTTON_GPIO_MENU_OK};
    for (unsigned pin : pins) {
        gpio_init(pin); gpio_set_dir(pin, GPIO_IN); gpio_pull_up(pin);
    }
    encoder.reset(encoder_state());
    gpio_set_irq_enabled_with_callback(ENCODER_GPIO_A, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, encoder_irq);
    gpio_set_irq_enabled(ENCODER_GPIO_B, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    hard_assert(add_repeating_timer_ms(-1, sample_buttons, nullptr, &button_timer));
}
PanelInput controls_read() {
    const uint32_t interrupts = save_and_disable_interrupts();
    const PanelInput input{fine_turns, coarse_turns, ok_event, back_event};
    fine_turns = coarse_turns = 0;
    ok_event = back_event = false;
    restore_interrupts(interrupts);
    return input;
}
