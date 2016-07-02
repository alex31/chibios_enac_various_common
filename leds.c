#include <ch.h>
#include <hal.h>
#include "bitband.h"
#include "leds.h"
#include <strings.h>
#include <stdnoreturn.h>


#ifndef LED_MAX_NUMBER
#define LED_MAX_NUMBER 8
#endif


typedef struct {
  ioline_t ledL;
  LedState ledS;
} LedData;


static void thdBlinkLed (void *arg);

LedData ledDatas[LED_MAX_NUMBER];
static uint32_t nbLed = 0;



bool ledResisterLine (ioline_t ledl, LedState iniState)
{
  if (nbLed >= LED_MAX_NUMBER)
    return false;

  ledDatas[nbLed].ledL = ledl;
  ledDatas[nbLed].ledS = iniState;

  chThdCreateFromHeap (NULL, 512, NORMALPRIO, 
		       thdBlinkLed, &ledDatas[nbLed]);
  nbLed++;
  
  return true;
}

void ledSet (ioline_t ledl, LedState iniState)
{

  for (uint32_t i=0; i<LED_MAX_NUMBER; i++) {
    if  (ledDatas[i].ledL == ledl) {
      ledDatas[i].ledL = ledl;
      ledDatas[i].ledS = iniState;
      break;
    }
  }
}

static noreturn void thdBlinkLed (void *arg)
{
  const LedData *ledData = (LedData *) arg;

  // give uniq name to threads if several leds are used
  char name[] = "blinkLed_#";
  for (uint32_t i=0; i<LED_MAX_NUMBER; i++) {
    if  (ledDatas[i].ledL == ledData->ledL) {
      *(index(name, '#')) = i + '0';
      break;
    }
  }
  chRegSetThreadName (name);

  while (true) {
    switch (ledData->ledS) {
    case LED_OFF : bb_palClearLine (ledData->ledL); break;
    case LED_BLINKSLOW : 
    case LED_BLINKFAST : bb_palToggleLine (ledData->ledL); break;
    case LED_ON : bb_palSetLine (ledData->ledL); break;
    }

    switch (ledData->ledS) {
    case LED_BLINKSLOW : chThdSleepMilliseconds (1000); break;
    case LED_OFF : 
    case LED_BLINKFAST : 
    case LED_ON : chThdSleepMilliseconds (200); break;
    }
  }
}
