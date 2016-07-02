#pragma once
#include <ch.h>
#include <hal.h>


// set LED_MAX_NUMBER for static array mem reservation, in mcuconf.h
// otherwite default value of 8 is used

typedef enum {LED_OFF, LED_BLINKSLOW, LED_BLINKFAST, LED_ON} LedState;


bool ledResisterLine (ioline_t ledl, LedState iniState);
void ledSet (ioline_t ledl, LedState iniState);

