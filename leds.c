#include <ch.h>
#include <hal.h>
#include "bitband.h"
#include "stdutil.h"
#include "leds.h"
#include <strings.h>
#include <stdnoreturn.h>


#ifndef LED_MAX_NUMBER
#define LED_MAX_NUMBER 8
#endif

static volatile bool blinkFast = false;
static volatile bool blinkSlow = false;
static thread_t *genThd = NULL;

typedef struct {
  ioline_t ledL;
  LedState ledS;
} LedData;


static void thdBlinkLed (void *arg);
static void thdBlinkGen (void *arg);

LedData ledDatas[LED_MAX_NUMBER];
static uint32_t nbLed = 0;


static THD_WORKING_AREA(waBlinkGen, 200);
static THD_WORKING_AREA_ARRAY(waBlinkLed, 224, LED_MAX_NUMBER);

bool ledRegisterLine (ioline_t ledl, LedState iniState)
{
  // error if max registered line is hit
  if (nbLed >= LED_MAX_NUMBER)
    return false;

  // error if already registered
  for (uint32_t i=0; i<LED_MAX_NUMBER; i++) {
    if  (ledDatas[i].ledL == ledl) {
      return false;
    }
  }

  // launch synchronous blink source thread
  if (genThd == NULL) {
    genThd = chThdCreateStatic (waBlinkGen, sizeof(waBlinkGen), NORMALPRIO, 
				thdBlinkGen, NULL);
  }

  ledDatas[nbLed].ledL = ledl;
  ledDatas[nbLed].ledS = iniState;

  chThdCreateStatic (waBlinkLed[nbLed], sizeof(waBlinkLed[0]), NORMALPRIO, 
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

static noreturn void thdBlinkGen (void *arg)
{
  (void) arg;

  chRegSetThreadName ("blinkGen");

  uint32_t c=0;
  while (true) {
    blinkFast = !blinkFast;
    if ((++c%5) == 0)
      blinkSlow = !blinkSlow;
    
    chThdSleepMilliseconds (200);
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
    case LED_BLINKSLOW : bb_palWriteLine (ledData->ledL, blinkSlow); break;
    case LED_BLINKFAST : bb_palWriteLine (ledData->ledL, blinkFast); break;
    case LED_ON : bb_palSetLine (ledData->ledL); break;
    }
    
    chThdSleepMilliseconds (20); 
  }
  
}
