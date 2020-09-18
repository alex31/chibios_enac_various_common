#include <ch.h>
#include <hal.h>
#include <math.h>

#include "smartLed.hpp"

namespace {
  static THD_WORKING_AREA(waLed, 512);

  constexpr std::array<float, 20> getSinPowTable(void)
  {
    std::array<float, 20> ret{};
    const float inc = 3.1415726  / ret.size();
    float angle = 0;
    for (auto &e : ret) {
      e = fabsf(powf(sinf(angle), 5));
      angle += inc;
    }
    return ret;
  }

  constexpr auto sinPowTable = getSinPowTable();
  
  static void ledPeriodic (void *arg)
  {
    FrontLed *fl = (FrontLed *) arg;
    chRegSetThreadName("ledPeriodic");
    size_t periodic(0), colIndex(0);
    HSV hsv{0,1,1};
    
    while (!chThdShouldTerminateX()) {
      if (fl->getPeriod() != 0) {
	const float &f = sinPowTable[periodic];
	fl->leds[0].setRGB(fl->ledColor.c[colIndex][0] * f * 255,
			   fl->ledColor.c[colIndex][1] * f * 255,
			   fl->ledColor.c[colIndex][2] * f * 255);
	fl->leds.emitFrame();
	chThdSleepMilliseconds (fl->getPeriod());
	if (periodic == sinPowTable.size()-1) {
	  colIndex = (colIndex+1)%2;
	}
	periodic = (periodic+1)%sinPowTable.size();
      } else { // led demo mode at start : cycle over colors until initialisation is done
	fl->leds[0].setHSV(hsv);
	fl->leds.emitFrame();
	hsv.h = fmod(hsv.h + 0.005f, 1.0f);
	chThdSleepMilliseconds(10);
      }
    }
    chThdExit(0);
  }
} // end of anonymous namespace



thread_t *FrontLed::tp = nullptr;


FrontLed::FrontLed() : leds(ledPwm,
			    ledTiming,
			    STM32_TIM1_UP_DMA_STREAM,
			    STM32_TIM1_UP_DMA_CHANNEL),
		       ledColor({0,0,0}, {0,0,0})
{
  if (tp == nullptr) {
    tp =  chThdCreateStatic(waLed, sizeof(waLed), LOWPRIO+1, &ledPeriodic, this);
  }
}


void FrontLed::setError(const LedCode code)
{
  switch (code) {
  case LedCode::Starting :
    period = 0;
    break;
  case LedCode::Optimal :
    period = 50;
    ledColor = {{0, 1, 0}, {0, 1, 0}};
    break;
  case LedCode::DirtyBit :
    period = 50;
    ledColor = {{0, 1, 0}, {1, 0.3, 0}};
    break;
  case LedCode::FatFsError :
    period = 16;
    ledColor = {{1, 0.3, 0}, {1, 0.3, 0}};
    break;
  case LedCode::ConfigError :
    period = 16;
    ledColor = {{0, 1, 0}, {0.5, 0.5, 0.5}};
    break;
  case LedCode::UsbStorageVBus :
    period = 16;
    ledColor = {{0, 1, 0}, {0, 1, 0}};
    break;
  case LedCode::UsbStorageOff :
    period = 50;
    ledColor = {{0, 0, 1}, {0, 0, 1}};
    break;
  case LedCode::UsbStorageOn :
    period = 8;
    ledColor = {{0, 0, 1}, {0, 0, 1}};
    break;
  case LedCode::HardFault :
    period = 16;
    ledColor = {{1, 0, 0}, {1, 0, 0}};
    break;
  case LedCode::SwdioModeStart :
    period = 16;
    ledColor = {{1, 0.5, 0}, {0, 0, 0}};
    break;
  case LedCode::SwdioModeWait :
    period = 16;
    ledColor = {{1, 0.5, 0}, {1, 0.5, 0}};
    break;
  }
}

void FrontLed::powerOff(void)
{
  if (tp) {
    chThdTerminate(tp);
    tp = nullptr;
  }
  leds[0].setRGB(0, 0, 0);
  leds.emitFrame();
}
