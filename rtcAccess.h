#ifndef __RTC_ACCESS_H__
#define __RTC_ACCESS_H__

#include <ch.h>

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
uint32_t getDstOffset (void);


#ifdef __cplusplus
}
#endif


#endif //  __RTC_ACCESS_H__
