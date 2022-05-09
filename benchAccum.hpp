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
  ACC_T startMonitorTime = {};
  ACC_T numCall = {};
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

template<typename ACC_T, bool recMinMax>
ACC_T benchAccum<ACC_T, recMinMax>::getTotalMicroSeconds()
{
  return RTC2US(STM32_SYSCLK, totalSysTick);
}

template<typename ACC_T, bool recMinMax>
uint32_t benchAccum<ACC_T, recMinMax>::getAverageNanoSeconds()
{
  return RTC2US(STM32_SYSCLK, totalSysTick * 1000UL) / numCall;
}

template<typename ACC_T, bool recMinMax>
uint32_t benchAccum<ACC_T, recMinMax>::getAverageMicroSeconds()
{
  return RTC2US(STM32_SYSCLK, totalSysTick) / numCall;
}

template<typename ACC_T, bool recMinMax>
ACC_T benchAccum<ACC_T, recMinMax>::getMinMicroSeconds()
{
  static_assert(recMinMax == true, "enable recMinMax in template parameter");
  return RTC2US(STM32_SYSCLK, minmax.minSysTick);
}

template<typename ACC_T, bool recMinMax>
ACC_T benchAccum<ACC_T, recMinMax>::getMaxMicroSeconds()
{
  static_assert(recMinMax == true, "enable recMinMax in template parameter");
  return RTC2US(STM32_SYSCLK, minmax.maxSysTick);
}
