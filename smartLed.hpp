#pragma once
#include "led2812.hpp"

/*
code de la led pour smartprobe :

1 led (3 couleur)

1 tableau precalculé pow(sin(x),3)) pour  x de 0 à PI/2 sur 20 valeurs
1 thread qui fait le dmaTransmit
vert à 20 hertz : pas de soucis (sleep 50ms)
vert+orange à 20hz : dirty bit, mais on continue quand même
orange à 60hz : hardware ok mais montage carte sd impossible
vert+blanc à 60hz : hardware ok sd ok, mais pb de config
rouge à 60 hertz :  (sleep 16ms) : problème hardware à l'init
*/

enum class LedCode {Starting, Optimal, DirtyBit, FatFsError, ConfigError, HardFault, UsbStorageVBus,
		    UsbStorageOff, UsbStorageOn};

static constexpr PWMDriver *ledPwm = &PWMD1;
static constexpr LedTiming ledTiming = getClockByTimer(ledPwm);
using Led_t = Led2812<uint16_t, ledTiming.t0h, ledTiming.t1h> ;



class FrontLed
{
public:
  struct Colors {
    Colors(const std::array<float, 3>& c0, const std::array<float, 3>&c1) {
      c[0] = c0; c[1] = c1;
    }
    std::array<std::array<float, 3>, 2> c;
  };
  FrontLed();
  void setError(const LedCode code);
  void powerOff(void);
  uint32_t getPeriod(void) const {return period;};
  Led2812Strip<1, Led_t> leds;
  Colors ledColor;
private:
  uint32_t period;
  static  thread_t *tp;
};
