#pragma once
#include <ch.h>
#include <hal.h>


typedef enum {LED_OFF, LED_BLINKSLOW, LED_BLINKFAST, LED_ON} LedState;


bool ledResisterLine (ioline_t ledl, LedState iniState);
void ledSet (ioline_t ledl, LedState iniState);

