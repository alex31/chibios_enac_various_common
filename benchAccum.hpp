#pragma once

#include "ch.h"
#include "hal.h"
#include <concepts>
#include <limits>


template<typename ACC_T, bool recMinMax=false>
requires std::unsigned_integral<ACC_T>
class benchAccum {
public:
  benchAccum() {reset();}
  void  reset();
  void  startMonitor();
  void  stopMonitor();
  ACC_T getTotalMicroSeconds();
  ACC_T getTotalSysticks();
  uint32_t getAverageMicroSeconds();
  uint32_t getAverageNanoSeconds();
  ACC_T getMinMicroSeconds();
  ACC_T getMaxMicroSeconds();
  
  
private:
  struct empty_t {};
  struct minmax_t {
    ACC_T minSysTick = {};
    ACC_T maxSysTick = {};  
  };

  ACC_T totalSysTick = {};
  rtcnt_t startMonitorTime = {};
  ACC_T numCall = {};
  static ACC_T rtc2ns(ACC_T systicks);
  static ACC_T rtc2us(ACC_T systicks);
  typename std::conditional<recMinMax, minmax_t, empty_t>::type minmax;
} ;


template<typename ACC_T, bool recMinMax>
void  benchAccum<ACC_T, recMinMax>::reset()
{
  totalSysTick = 0;
  numCall = 0;
  if constexpr (recMinMax) {
    minmax.minSysTick = std::numeric_limits<ACC_T>::max();
    minmax.maxSysTick = 0;
  }
}

template<typename ACC_T, bool recMinMax>
void  benchAccum<ACC_T, recMinMax>::startMonitor()
{
  startMonitorTime = chSysGetRealtimeCounterX();
}

template<typename ACC_T, bool recMinMax>
void  benchAccum<ACC_T, recMinMax>::stopMonitor()
{
  const ACC_T diff = chSysGetRealtimeCounterX() - startMonitorTime;
  totalSysTick += diff;
  numCall++;
  if constexpr (recMinMax) {
    minmax.minSysTick = std::min(minmax.minSysTick, diff);
    minmax.maxSysTick = std::max(minmax.maxSysTick, diff);
  }
}

template<typename ACC_T, bool recMinMax>
ACC_T benchAccum<ACC_T, recMinMax>::getTotalSysticks()
{
  return totalSysTick;
}

#define RTC2USLL(freq, n) ((((n) - 1ULL) / ((freq) / 1000000ULL)) + 1ULL)
template<typename ACC_T, bool recMinMax>
ACC_T benchAccum<ACC_T, recMinMax>::rtc2ns(ACC_T systicks)
{
  if constexpr (sizeof(ACC_T) <= 32) 
    return RTC2US(STM32_SYSCLK, systicks * 1000UL);
  else
    return RTC2USLL(static_cast<ACC_T>(STM32_SYSCLK), systicks * 1000ULL);
}

template<typename ACC_T, bool recMinMax>
ACC_T benchAccum<ACC_T, recMinMax>::rtc2us(ACC_T systicks)
{
  if constexpr (sizeof(ACC_T) <= 32) 
    return RTC2US(STM32_SYSCLK, systicks);
  else
    return RTC2USLL(static_cast<ACC_T>(STM32_SYSCLK), systicks);
}

template<typename ACC_T, bool recMinMax>
ACC_T benchAccum<ACC_T, recMinMax>::getTotalMicroSeconds()
{
  return rtc2us(totalSysTick);
}

template<typename ACC_T, bool recMinMax>
uint32_t benchAccum<ACC_T, recMinMax>::getAverageNanoSeconds()
{
  return rtc2ns(totalSysTick) / numCall;
}

template<typename ACC_T, bool recMinMax>
uint32_t benchAccum<ACC_T, recMinMax>::getAverageMicroSeconds()
{
   return rtc2us(totalSysTick) / numCall;
}

template<typename ACC_T, bool recMinMax>
ACC_T benchAccum<ACC_T, recMinMax>::getMinMicroSeconds()
{
  static_assert(recMinMax == true, "enable recMinMax in template parameter");
  return rtc2us(minmax.minSysTick);
}

template<typename ACC_T, bool recMinMax>
ACC_T benchAccum<ACC_T, recMinMax>::getMaxMicroSeconds()
{
  static_assert(recMinMax == true, "enable recMinMax in template parameter");
  return rtc2us(minmax.maxSysTick);
}
