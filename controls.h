#pragma once
#include "front_panel.h"

constexpr unsigned ENCODER_GPIO_A = 20;
constexpr unsigned ENCODER_GPIO_B = 21;
constexpr unsigned BUTTON_GPIO_BACK_COARSE = 17;
constexpr unsigned BUTTON_GPIO_MENU_OK = 22;
// Clockwise moves down the menu on this encoder; use 1 for the opposite wiring.
constexpr int ENCODER_DIRECTION = -1;

void controls_init();
PanelInput controls_read();
