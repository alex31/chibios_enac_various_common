#pragma once
#include <ch.h>
#include <hal.h>


// set LED_MAX_NUMBER for static array mem reservation, in mcuconf.h
// otherwite default value of 8 is used
#ifdef __cplusplus
extern "C" {
#endif

typedef enum {LED_OFF, LED_BLINKSLOW, LED_BLINKFAST, LED_ON} LedState;


bool ledRegisterLine (ioline_t ledl, LedState iniState);
void ledSet (ioline_t ledl, LedState iniState);

#ifdef __cplusplus
}
#endif
