#pragma once

#include <ch.h>
#include <stdlib.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif


void setHour (uint32_t val);
void setMinute (uint32_t val);
void setSecond (uint32_t val);
void setYear (uint32_t val);
void setMonth (uint32_t val);
void setMonthDay (uint32_t val);
void setWeekDay (uint32_t val);


uint32_t 	getHour (void);
uint32_t 	getMinute (void);
uint32_t 	getSecond (void);
uint32_t 	getYear (void);
uint32_t 	getMonth (void);
uint32_t 	getMonthDay (void);
uint32_t	getWeekDay (void);
const char*	getWeekDayAscii (void);
uint32_t	getDstOffset (void);

time_t   getTimeUnixSec(void);
uint64_t getTimeUnixMillisec(void);
void     setTimeUnixSec(time_t tt);

#ifdef __cplusplus
}
#endif

