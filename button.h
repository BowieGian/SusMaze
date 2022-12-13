// all buttoms are in P8
#ifndef BUTTOM_H
#define BUTTOM_H

#include "gpio.h"

#define MODE_GPIO_NO 47
#define BASE_GPIO_NO 46
#define SNARE_GPIO_NO 27
#define HIHAT_GPIO_NO 65

#define GAME_BUTTON_GPIO_NO 20

// P8_XX
#define MODE_GPIO_PIN 15
#define BASE_GPIO_PIN 16
#define SNARE_GPIO_PIN 17
#define HIHAT_GPIO_PIN 18

#define GAME_BUTTON_GPIO_PIN 41

#define NUM_BUTTOMS 4

void ButtonInit();

bool isButtonPressed();

#endif