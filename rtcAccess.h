#pragma once

#include <ch.h>
#include <hal.h>
#include <stdlib.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// callback fonction type
typedef void (*RtcChangedCB) (int delta);


void setHour (uint32_t val);
void setUtcHour (uint32_t val);
void setMinute (uint32_t val);
void setSecond (uint32_t val);
void setSecondInMs (uint32_t val);
void setYear (uint32_t val);
void setMonth (uint32_t val);
void setMonthDay (uint32_t val);
void setWeekDay (uint32_t val);


uint32_t 	getHour (void);
uint32_t 	getUtcHour (void);
uint32_t 	getMinute (void);
uint32_t 	getSecond (void);
uint32_t 	getSecondInMs (void);
uint32_t 	getYear (void);
uint32_t 	getMonth (void);
uint32_t 	getMonthDay (void);
uint32_t	getWeekDay (void);
const char*	getWeekDayAscii (void);
uint32_t	getDstOffset (void);

time_t   getTimeUnixSec(void);
uint64_t getTimeUnixMillisec(void);
void     setTimeUnixSec(time_t tt);
void	 registerRtcChanged (RtcChangedCB cb);
bool	 tuneShiftByOffset(RTCDriver *rtcp, int millis);


#ifdef RTC_ISR_WUTWF // RTC V2
bool rtcEnablePeriodicWakeup (RTCDriver *rtcp, uint32_t _second);
bool rtcEnablePeriodicWakeupHires (RTCDriver *rtcp, uint16_t milliseconds);
bool rtcDisablePeriodicWakeup (RTCDriver *rtcp);
#endif


__attribute__((access (read_only, 1, 2)))
void rtcBackupWrite(const void *src, size_t n);

__attribute__((access (write_only, 1, 2)))
void rtcBackupRead(void *dst, size_t n);
  

#ifdef __cplusplus
}
#endif

