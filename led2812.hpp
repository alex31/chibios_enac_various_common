#pragma once

#include "stdutil.h"
#include "hal_stm32_dma.h"
#include <algorithm>
#include <array>
#include <tuple>
#include <utility>
#include <cmath>


constexpr uint32_t PERIOD_NS = 1250;
constexpr uint32_t T0H_NS = 400;
constexpr uint32_t T1H_NS = 800;

enum class TimerChannel {C1, C2, C3, C4};

struct LedTiming {
  uint32_t frequency;
  uint16_t period;
  uint16_t t0h;
  uint16_t t1h;
};

typedef struct {
    uint8_t r;       // 0 - 255
    uint8_t g;       // 0 - 255
    uint8_t b;       // 0 - 255
} RGB;

typedef struct {
    float h;       // angle in degrees
    float s;       // a fraction between 0 and 1
    float v;       // a fraction between 0 and 1
} HSV;

[[maybe_unused]] static HSV   constexpr rgb2hsv(const RGB in);
[[maybe_unused]] static RGB   constexpr hsv2rgb(const HSV in);


template<typename T>
concept Addable = requires (T a, T b)
{
    a + b; // "the expression “a + b” is a valid expression that will compile"
};


// template<typename T>
// concept TIMCLK1_is_constexpr = requires(T a)
// {
//   uint32_t frequency = STM32_TIMCLK1;
// };

constexpr  LedTiming getClockByTimer([[maybe_unused]] const PWMDriver *pwmp)
{
#if defined STM32G4XX
  uint32_t frequency = STM32_SYSCLK; // slowest one
#else
  uint32_t frequency = STM32_TIMCLK1; // slowest one
#ifdef USE_MAXIMUM_TIMER_CAPABILITY
#if STM32_PWM_USE_TIM1
  if (pwmp == &PWMD1)
    frequency = STM32_TIMCLK2;
#endif
#if STM32_PWM_USE_TIM8
  if (pwmp == &PWMD8)
    frequency = STM32_TIMCLK2;
#endif
#if STM32_PWM_USE_TIM9
  if (pwmp == &PWMD9)
    frequency = STM32_TIMCLK2;
#endif
#if defined(STM32_PWM_USE_TIM15) && STM32_PWM_USE_TIM15
  if (pwmp == &PWMD15)
    frequency = STM32_TIMCLK2;
#endif
#if defined(STM32_PWM_USE_TIM16) && STM32_PWM_USE_TIM16
  if (pwmp == &PWMD16)
    frequency = STM32_TIMCLK2;
#endif
#if defined(STM32_PWM_USE_TIM17) && STM32_PWM_USE_TIM17
  if (pwmp == &PWMD17)
    frequency = STM32_TIMCLK2;
#endif
#endif
#endif
  
  const float timerPeriodNs = 1e9 / frequency;
  return {frequency,
	  static_cast<uint16_t>(PERIOD_NS/timerPeriodNs),
	  static_cast<uint16_t>(T0H_NS/timerPeriodNs),
	  static_cast<uint16_t>(T1H_NS/timerPeriodNs)};
  
}





template <typename T, T T0H, T T1H>
class Led2812 {
public:
  using ColorArray_t = struct {std::array<T, 8> a;}  __attribute__((packed));
  using timerType = T;
  constexpr Led2812(void) : g{}, r{}, b{} {};
  Led2812(const uint8_t mr, const uint8_t mg, const uint8_t mb) {setRGB(mr, mg, mb);};
  Led2812(const Led2812& other) {g = other.g;r = other.r;b = other.b;};
  const Led2812& operator=(const Led2812& other) {g = other.g;r = other.r;b = other.b;
    return *this;};
  void setG(const uint8_t v) {setColor(g, v);};
  void setR(const uint8_t v) {setColor(r, v);};
  void setB(const uint8_t v) {setColor(b, v);};
  void setRGB(const uint8_t mr, const uint8_t mg, const uint8_t mb) {
    setR(mr); setG(mg); setB(mb);
  };
  void setRGB(const RGB &rgb) {
    setR(rgb.r); setG(rgb.g); setB(rgb.b);
  };
  void  setHSV(const HSV &hsv) {
    setRGB(hsv2rgb(hsv));
  };

  uint8_t getR(void) const {return getColor(r);};
  uint8_t getG(void) const {return getColor(g);};
  uint8_t getB(void) const {return getColor(b);};
  RGB  getRGB() const {
    return {getR(), getG(), getB()};
  };
  HSV  getHSV() const {
    return rgb2hsv(getRGB());
  };
   /*
    prévoir : routines HSV, routines de modif : shift +- sur H, S et V
   */
  void setH(const float val) {
    HSV hsv = getHSV();
    hsv.h = val;
    setHSV(hsv);
  }
  void setS(const float val) {
    HSV hsv = getHSV();
    hsv.s = val;
    setHSV(hsv);
  }
  void setV(const float val) {
    HSV hsv = getHSV();
    hsv.v = val;
    setHSV(hsv);
  }
  void alterH(const float shift) {
    HSV hsv = getHSV();
    hsv.h = fmodf(hsv.h+shift, 1.0f);
    setHSV(hsv);
  }
  void alterS(const float shift) {
    HSV hsv = getHSV();
    hsv.s = fmodf(hsv.s+shift, 1.0f);
    setHSV(hsv);
  }
  void alterV(const float shift) {
    HSV hsv = getHSV();
    hsv.v = fmodf(hsv.v+shift, 1.0f);
    setHSV(hsv);
  }
  
  constexpr static size_t elemSize(void) {return sizeof(T);};
private:
  struct {
  ColorArray_t g;
  ColorArray_t r;
  ColorArray_t b;
  } __attribute__((packed));
  static void setColor(ColorArray_t &colArr, uint8_t value);
  static uint8_t getColor(const ColorArray_t &colArr);
} ;



template <size_t N, typename LT>
class Led2812Strip {
public:
  Led2812Strip(PWMDriver *m_pwmd, const LedTiming &m_ledTiming,
	       const uint32_t stream
#if STM32_DMA_SUPPORTS_DMAMUX
	       , const uint32_t  dmamux
#else
	       , const uint8_t channel
#endif
	       , const TimerChannel m_timChannel = TimerChannel::C1, const size_t nbLed = N
	       );
  LT& operator[](const size_t index) {return leds[index];};
  void emitFrame(void);
  void rotate(int32_t n);
  constexpr static size_t elemSize(void)  {return LT::elemSize();};
  constexpr static size_t valuesPerLed(void) {return sizeof(LT)/elemSize();};
  size_t size(void) const {return
      2 + (activeLedCount*valuesPerLed()) + 2;};
  static_assert(sizeof(LT) % elemSize() == 0, "Led2812 size must align with timer element size");
private:
  void start();
  struct {
    // DMA some timetime hangs on firsts value, so we keep them to 0
    const typename LT::timerType preamble[2] = {0,0};
    std::array<LT, N> leds{};
    // the pwm will continue to run so last width must be zero
    // to keep line low between frames
    // const typename LT::timerType end[8] = {0,0,0,0,0,0,0,0};
    const typename LT::timerType end[2] = {0,0};
  };
  const size_t activeLedCount;
  const LedTiming &ledTiming;
  const TimerChannel timChannel;
  PWMDriver *pwmd;
  PWMConfig pwmCfg;
  DMADriver dmap;
  DMAConfig dmaCfg;
};


template <typename T, T T0H, T T1H>
void Led2812<T, T0H, T1H>::setColor(ColorArray_t &colArr, const uint8_t value)
{
  for (size_t i=0; i<8; i++) {
    colArr.a[i] = value & (1U<<(7U-i)) ?  T1H : T0H;
  }
}

template <typename T, T T0H, T T1H>
uint8_t Led2812<T, T0H, T1H>::getColor(const ColorArray_t &colArr)
{
  uint8_t value =0U;
  for (size_t i=0; i<8; i++) {
    value |= colArr.a[i] == T1H ? (1U<<(7-i)) : 0U;
  }
  return value;
}

template <size_t N, typename LT>
Led2812Strip<N, LT>::Led2812Strip(PWMDriver *m_pwmd, const LedTiming &m_ledTiming,
				  const uint32_t stream
#if STM32_DMA_SUPPORTS_DMAMUX
	       , const uint32_t  dmamux
#else
	       , const uint8_t channel
#endif
	       , const TimerChannel m_timChannel, const size_t nbLed) :
  activeLedCount(std::min(nbLed, N)),
  ledTiming(m_ledTiming), timChannel(m_timChannel), pwmd(m_pwmd)
{
  pwmCfg = {
	    .frequency = ledTiming.frequency,
	    .period    = ledTiming.period, 
	    .callback  = NULL,             
	    .channels  = {
			  {.mode = PWM_OUTPUT_DISABLED, .callback = NULL},
			  {.mode = PWM_OUTPUT_DISABLED, .callback = NULL},
			  {.mode = PWM_OUTPUT_DISABLED, .callback = NULL},
			  {.mode = PWM_OUTPUT_DISABLED, .callback = NULL},
			  },
	    .cr2  =  STM32_TIM_CR2_CCDS,
	    .bdtr = 0,
	    .dier =  STM32_TIM_DIER_UDE
  };
  pwmCfg.channels[std::to_underlying(timChannel)] = {.mode = PWM_OUTPUT_ACTIVE_HIGH, .callback = NULL};
  dmaCfg = (DMAConfig) {
	     .stream = stream,
#if STM32_DMA_SUPPORTS_DMAMUX
	     .dmamux = dmamux,
#else
	     .channel = channel,
#endif
	     .inc_peripheral_addr = false,
	     .inc_memory_addr = true,
	     .op_mode = DMA_ONESHOT,
	     .end_cb = NULL,
	     .error_cb = NULL,
	     .direction = DMA_DIR_M2P,
	     .dma_priority = 0,
	     .irq_priority = 12,
	     .psize = elemSize(),//sizeof(uint16_t),
	     .msize = elemSize(),
#if defined __DCACHE_PRESENT &&  __DCACHE_PRESENT
	     .activate_dcache_sync = false,
#endif
#if STM32_DMA_ADVANCED
	     .pburst = 0,
	     .mburst = 0,
	     .fifo = 4,
	     .periph_inc_size_4 = false,
	     .transfert_end_ctrl_by_periph = false
#endif
  };
  start();
};

template <size_t N, typename LT>
void Led2812Strip<N, LT>::start()
{
  dmaObjectInit(&dmap);
  dmaStart(&dmap, &dmaCfg);
  pwmStart(pwmd, &pwmCfg);
  pwmEnableChannel(pwmd, std::to_underlying(timChannel), getClockByTimer(pwmd).t0h);
}


template <size_t N, typename LT>
void Led2812Strip<N, LT>::emitFrame(void) 
{
  if (activeLedCount < N) {
    auto *footer = reinterpret_cast<typename LT::timerType *>(leds.data()) + (activeLedCount * valuesPerLed());
    footer[0] = 0;
    footer[1] = 0;
  }
  dmaTransfert(&dmap, &pwmd->tim->CCR[std::to_underlying(timChannel)], (void *) &preamble, size());
  chThdSleepMicroseconds(50);
}

template <size_t N, typename LT>
void Led2812Strip<N, LT>::rotate(int32_t n) 
{
  if (!n || !activeLedCount) return;

  auto first = leds.begin();
  auto last = first + activeLedCount;

  if (n > 0) {
    n %= static_cast<int32_t>(activeLedCount);
    if (!n) return;
    std::rotate(first, last - n, last);
  } else {
    n = -n % static_cast<int32_t>(activeLedCount);
    if (!n) return;
    std::rotate(first, first + n, last);
  }
}



constexpr HSV rgb2hsv(const RGB in)
{
    HSV         out;
    float      min, max, delta;
    const float inr(in.r/255.0), ing(in.g/255.0), inb(in.b/255.0);

    min = inr < ing ? inr : ing;
    min = min  < inb ? min  : inb;

    max = inr > ing ? inr : ing;
    max = max  > inb ? max  : inb;

    out.v = max;                                // v
    delta = max - min;
    if (delta < 0.00001f)
    {
        out.s = 0;
        out.h = 0; // undefined, maybe nan?
        return out;
    }
    if( max > 0.0f ) { // NOTE: if Max is == 0, this divide would cause a crash
        out.s = (delta / max);                  // s
    } else {
        // if max is 0, then r = g = b = 0              
        // s = 0, h is undefined
        out.s = 0.0f;
        out.h = NAN;                            // its now undefined
        return out;
    }
    if( inr >= max )                           // > is bogus, just keeps compilor happy
        out.h = ( ing - inb ) / delta;        // between yellow & magenta
    else
    if( ing >= max )
        out.h = 2.0f + ( inb - inr ) / delta;  // between cyan & yellow
    else
        out.h = 4.0f + ( inr - ing ) / delta;  // between magenta & cyan

    out.h *= 60.0f;                              // degrees

    if( out.h < 0.0f )
        out.h += 360.0f;

    out.h /= 360.0f;
    return out;
}


constexpr RGB hsv2rgb(HSV in)
{
    float      hh, p, q, t, ff;
    long        i;
    float outr(0), outg(0), outb(0);

    in.h *= 360.0f;
    
    if(in.s <= 0.0f) {       // < is bogus, just shuts up warnings
      return  {0,0,0};
    }
    hh = in.h;
    if(hh >= 360.0f) hh = 0.0;
    hh /= 60.0f;
    i = (long)hh;
    ff = hh - i;
    p = in.v * (1.0f - in.s);
    q = in.v * (1.0f - (in.s * ff));
    t = in.v * (1.0f - (in.s * (1.0f - ff)));

    switch(i) {
    case 0:
        outr = in.v;
        outg = t;
        outb = p;
        break;
    case 1:
        outr = q;
        outg = in.v;
        outb = p;
        break;
    case 2:
        outr = p;
        outg = in.v;
        outb = t;
        break;

    case 3:
        outr = p;
        outg = q;
        outb = in.v;
        break;
    case 4:
        outr = t;
        outg = p;
        outb = in.v;
        break;
    case 5:
    default:
        outr = in.v;
        outg = p;
        outb = q;
        break;
    }
    
    return {uint8_t(outr*255), uint8_t(outg*255), uint8_t(outb*255)};     
}
