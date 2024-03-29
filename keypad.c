#include <ch.h>
#include <hal.h>
#include "stdutil.h"
#include "keypad.h"


static THD_WORKING_AREA(waThdKeypadScan, 256);

static void configureLines (const Keypad_Def *kd);
static Keypad_key scanKeypad (const Keypad_Def *kd);
static bool  keypadKeyAreEqual (const Keypad_key k1, const Keypad_key k2);
static bool  keypadKeyAreNotEqual (const Keypad_key k1, const Keypad_key k2);
static void  thdKeypadScan(void *arg) ;
static const Keypad_key noPress = {KEYPAD_NO_PRESS, KEYPAD_NO_PRESS};

static const Keypad_Def keypadWiring = {
  .kpRow = {KEYPAD_GPIO_ROW},
  .kpCol = {KEYPAD_GPIO_COL}
};



typedef struct {
  keypadCbType cb;
  void *userData;
  const Keypad_Def *kd;
} WorkerThreadArgs;



void launchScanKeypad (keypadCbType keyCb, void *userData)
{
  static WorkerThreadArgs wta ;
  wta.cb=keyCb;
  wta.userData=userData;
  wta.kd=&keypadWiring;

  configureLines(wta.kd);
  chThdCreateStatic(waThdKeypadScan, sizeof(waThdKeypadScan),
                    NORMALPRIO, thdKeypadScan, &wta);
}


static void thdKeypadScan(void *arg) 
{
  WorkerThreadArgs *wta = (WorkerThreadArgs *) arg;
  chRegSetThreadName("keypadScan");
  Keypad_key lastKey = noPress;

  while (!chThdShouldTerminateX()) {
    Keypad_key kk = scanKeypad (wta->kd);
    if ((keypadKeyAreNotEqual(kk, noPress)) && (keypadKeyAreEqual(kk, lastKey))) {
      const Keypad_Symbol ks = (kk.row * KEYPAD_GPIO_COL_SIZE) + kk.col;
      (wta->cb) (ks, wta->userData);
      // wait for release
      uint32_t cnt=0;
      while (keypadKeyAreNotEqual (scanKeypad (wta->kd), noPress)) {
	if (cnt++ == 300) {
	  DebugTrace ("spurious long key press row=%u col=%u", kk.row, kk.col);
	}
	chThdSleepMilliseconds (10);
      }
      lastKey = noPress;
      chThdSleepMilliseconds (10);
    } else {
      lastKey = kk;
      chThdSleepMilliseconds (5);
    }

  }
  chThdExit (MSG_OK);
}

static void configureLines (const Keypad_Def *kd)
{
  for (int r = 0; r < KEYPAD_GPIO_ROW_SIZE; r++) {
    palSetLineMode (kd->kpRow[r], PAL_MODE_OUTPUT_OPENDRAIN);
  }
  for (int c = 0; c < KEYPAD_GPIO_COL_SIZE; c++) {
    palSetLineMode (kd->kpCol[c],  PAL_MODE_INPUT_PULLUP);
  }
}

static Keypad_key scanKeypad (const Keypad_Def *kd)
{
  for (int r = 0; r < KEYPAD_GPIO_ROW_SIZE; r++) {
    palSetLine (kd->kpRow[r]);
  }

  for (int r = 0; r < KEYPAD_GPIO_ROW_SIZE; r++) {
    palClearLine (kd->kpRow[r]);
 
    chThdSleepMilliseconds (1);
    for (int c=0; c < KEYPAD_GPIO_COL_SIZE; c++) {
      int level = palReadLine (kd->kpCol[c]);
      if (level == 0) {// keypress
	palSetLine (kd->kpRow[r]);
	const Keypad_key kk = {r,c};
	return kk;
      }
    }
      
    palSetLine (kd->kpRow[r]);
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

