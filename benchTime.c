
#include "benchTime.h"
#include "stdutil.h"


benchResults doBench(const benchmarkFn_t fn, const size_t iter,
		 void *userData)
{
  benchResults br = {0, 0, UINT32_MAX, 0};
  rtcnt_t tsall = chSysGetRealtimeCounterX();

  size_t i = iter;
  while (i--) {
    const rtcnt_t ts = chSysGetRealtimeCounterX();
    fn(userData);
    const rtcnt_t diff = rtcntDiff(ts, chSysGetRealtimeCounterX());
    br.minMicroSeconds = MIN(br.minMicroSeconds, diff);
    br.maxMicroSeconds = MAX(br.minMicroSeconds, diff);
  }
  br.totalMicroSeconds = rtcntDiff(tsall, chSysGetRealtimeCounterX());
  br.meanMicroSeconds = br.totalMicroSeconds / iter;

  br.minMicroSeconds = RTC2US(STM32_SYSCLK, br.minMicroSeconds);
  br.maxMicroSeconds = RTC2US(STM32_SYSCLK, br.maxMicroSeconds);
  br.totalMicroSeconds = RTC2US(STM32_SYSCLK, br.totalMicroSeconds);
  br.meanMicroSeconds = RTC2US(STM32_SYSCLK, br.meanMicroSeconds);
  
  return br;
}
