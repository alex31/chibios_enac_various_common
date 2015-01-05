#include <ch.h>
#include <hal.h>
#include "globalVar.h"
#include "stdutil.h"
#include "rowColKeypad.h"

// when finish to debug (printf need big stack), shrink  WORKING_AREA to 256
static WORKING_AREA(waThdKeypadScan, 256);

static GPIO_TypeDef  * getGpioPtrFromEnum (const Keypad_Gpio kpg);
static void configureGpio (const Keypad_Def *kd);
static Keypad_key scanKeypad (const Keypad_Def *kd);
static bool  keypadKeyAreEqual (const Keypad_key k1, const Keypad_key k2);
static bool  keypadKeyAreNotEqual (const Keypad_key k1, const Keypad_key k2);
static msg_t thdKeypadScan(void *arg) ;
static const Keypad_key noPress = {KEYPAD_NO_PRESS, KEYPAD_NO_PRESS};



typedef struct {
  keypadCbType cb;
  void *userData;
  const Keypad_Def *kd;
} WorkerThreadArgs;



void launchScanKeypad (keypadCbType keyCb, const Keypad_Def *kd, void *userData)
{
  static WorkerThreadArgs wta ;
  wta.cb=keyCb;
  wta.userData=userData;
  wta.kd=kd;

  configureGpio (kd);
  chThdCreateStatic(waThdKeypadScan, sizeof(waThdKeypadScan),
                    NORMALPRIO, thdKeypadScan, &wta);
}



static GPIO_TypeDef  * getGpioPtrFromEnum (const Keypad_Gpio kpg)
{
  switch (kpg) {
  case Keypad_None : return NULL;
     case Keypad_GpioA : return GPIOA;
     case Keypad_GpioB : return GPIOB;
     case Keypad_GpioC : return GPIOC;
     case Keypad_GpioD : return GPIOD;
     case Keypad_GpioE : return GPIOE;
     case Keypad_GpioF : return GPIOF;
     case Keypad_GpioG : return GPIOG;
     case Keypad_GpioH : return GPIOH;
     case Keypad_GpioI : return GPIOI;
  }
  return NULL;
}





static msg_t thdKeypadScan(void *arg) 
{
  WorkerThreadArgs *wta = (WorkerThreadArgs *) arg;
  chRegSetThreadName("keypadScan");
  Keypad_key lastKey = noPress;

  while (!chThdShouldTerminate()) {
    Keypad_key kk = scanKeypad (wta->kd);
    if ((keypadKeyAreNotEqual(kk, noPress)) && (keypadKeyAreEqual(kk, lastKey))) {
      (wta->cb) (kk, wta->userData); 
      while (keypadKeyAreNotEqual (scanKeypad (wta->kd), noPress)) {
	chThdSleepMilliseconds (10);
      }
      lastKey = noPress;
      chThdSleepMilliseconds (100);
    } else {
      lastKey = kk;
      chThdSleepMilliseconds (5);
    }

  }
  
  return RDY_OK;
}

static void configureGpio (const Keypad_Def *kd)
{
  for (int r = 0; r < KEYPAD_NUM_OF_ROWS; r++) {
    palSetPadMode (getGpioPtrFromEnum (kd->kpRow[r].kpGgpio), kd->kpRow[r].kpPin,
		   PAL_MODE_OUTPUT_OPENDRAIN | PAL_STM32_OSPEED_HIGHEST);
  }
  for (int c = 0; c < KEYPAD_NUM_OF_COLS; c++) {
    palSetPadMode (getGpioPtrFromEnum (kd->kpCol[c].kpGgpio), kd->kpCol[c].kpPin,
		   PAL_MODE_INPUT_PULLUP | PAL_STM32_OSPEED_HIGHEST);
  }
}

static Keypad_key scanKeypad (const Keypad_Def *kd)
{
  for (int r = 0; r < KEYPAD_NUM_OF_ROWS; r++) {
    palSetPad (getGpioPtrFromEnum (kd->kpRow[r].kpGgpio), kd->kpRow[r].kpPin);
  }

  for (int r = 0; (r < KEYPAD_NUM_OF_ROWS) && (kd->kpRow[r].kpGgpio !=  Keypad_None); r++) {
    palClearPad (getGpioPtrFromEnum (kd->kpRow[r].kpGgpio), kd->kpRow[r].kpPin);
 
    // this should be minimised to 1 microsecond
    chThdSleepMilliseconds (1);

    for (int c = 0; (c < KEYPAD_NUM_OF_COLS) && (kd->kpCol[c].kpGgpio !=  Keypad_None); c++) {
      int level = palReadPad (getGpioPtrFromEnum (kd->kpCol[c].kpGgpio), kd->kpCol[c].kpPin);
      if (level == 0) {// keypress
	palSetPad (getGpioPtrFromEnum (kd->kpRow[r].kpGgpio), kd->kpRow[r].kpPin);
	const Keypad_key kk = {r,c};
	return kk;
      }
    }
      
    palSetPad (getGpioPtrFromEnum (kd->kpRow[r].kpGgpio), kd->kpRow[r].kpPin);
  }
  

  return noPress;
}

static bool  keypadKeyAreEqual (const Keypad_key k1, const Keypad_key k2)
{
  return ((k1.row) == (k2.row)) && ((k1.col) == (k2.col));
}

static bool  keypadKeyAreNotEqual (const Keypad_key k1, const Keypad_key k2)
{
  return ! keypadKeyAreEqual (k1, k2);
}

