#pragma once
#include "front_panel.h"

constexpr unsigned ENCODER_GPIO_A = 20;
constexpr unsigned ENCODER_GPIO_B = 21;
constexpr unsigned BUTTON_GPIO_BACK_COARSE = 17;
constexpr unsigned BUTTON_GPIO_MENU_OK = 22;
// Change to -1 if your encoder's clockwise direction is reversed.
constexpr int ENCODER_DIRECTION = 1;

void controls_init();
PanelInput controls_read();
