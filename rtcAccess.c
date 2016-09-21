#include "rtcAccess.h"
#include <ch.h>
#include <hal.h>
#include <stdlib.h>

static uint32_t weekDay (void);
static uint32_t weekDayOfDate (const uint32_t day, const uint32_t month, const uint32_t year);

static struct tm utime;
#if (CH_KERNEL_MAJOR == 2)

#include <chrtclib.h>




void setHour (uint32_t val)
{
  rtcGetTimeTm (&RTCD1, &utime);
  utime.tm_hour = val-getDstOffset();
  utime.tm_isdst = 0;
  rtcSetTimeTm (&RTCD1, &utime);
}

void setMinute (uint32_t val)
{
  rtcGetTimeTm (&RTCD1, &utime);
  utime.tm_min = val;
  rtcSetTimeTm (&RTCD1, &utime);
}

void setSecond (uint32_t val)
{
  rtcGetTimeTm (&RTCD1, &utime);
  utime.tm_sec = val;
  rtcSetTimeTm (&RTCD1, &utime);
}

void setYear (uint32_t val)
{
  rtcGetTimeTm (&RTCD1, &utime);
  utime.tm_year = val-1900;
  rtcSetTimeTm (&RTCD1, &utime);
  setWeekDay (weekDay());
}

void setMonth (uint32_t val)
{
  rtcGetTimeTm (&RTCD1, &utime);
  utime.tm_mon = val-1;
  rtcSetTimeTm (&RTCD1, &utime);
  setWeekDay (weekDay());
}

void setMonthDay (uint32_t val)
{
  rtcGetTimeTm (&RTCD1, &utime);
  utime.tm_mday = val;
  rtcSetTimeTm (&RTCD1, &utime);
  setWeekDay (weekDay());
}

void setWeekDay (uint32_t val)
{
  rtcGetTimeTm (&RTCD1, &utime);
  utime.tm_wday = val;
  rtcSetTimeTm (&RTCD1, &utime);
}

uint32_t getHour (void)
{
  rtcGetTimeTm (&RTCD1, &utime);
  mktime(&utime);
  
  return (utime.tm_hour+getDstOffset()) % 24;
}
uint32_t getMinute (void)
{
  rtcGetTimeTm (&RTCD1, &utime);
  return utime.tm_min;
}
uint32_t getSecond (void)
{
  rtcGetTimeTm (&RTCD1, &utime);
  return utime.tm_sec;
}
uint32_t getYear (void)
{
  rtcGetTimeTm (&RTCD1, &utime);
  return utime.tm_year+1900;
}
uint32_t getMonth (void)
{
  rtcGetTimeTm (&RTCD1, &utime);
  return utime.tm_mon+1;
}
uint32_t getMonthDay (void)
{
  rtcGetTimeTm (&RTCD1, &utime);
  return utime.tm_mday;
}
uint32_t getWeekDay (void)
{
  rtcGetTimeTm (&RTCD1, &utime);
  return utime.tm_wday;
}

time_t getTimeUnixSec(void)
{
  return rtcGetTimeUnixSec(&RTCD1);
}

#else // CH_KERNEL_MAJOR > 2

static RTCDateTime rtctime;
static uint32_t tv_msec;

void setHour (uint32_t val)
{
  rtcGetTime (&RTCD1, &rtctime);
  rtcConvertDateTimeToStructTm (&rtctime, &utime, &tv_msec);
  utime.tm_hour = val-getDstOffset();
  utime.tm_isdst = 0;
  rtcConvertStructTmToDateTime (&utime, tv_msec, &rtctime);
  rtcSetTime (&RTCD1, &rtctime);
}

void setMinute (uint32_t val)
{
  rtcGetTime (&RTCD1, &rtctime);
  rtcConvertDateTimeToStructTm (&rtctime, &utime, &tv_msec);

  utime.tm_min = val;
  rtcConvertStructTmToDateTime (&utime, tv_msec, &rtctime);
  rtcSetTime (&RTCD1, &rtctime);

}

void setSecond (uint32_t val)
{
  rtcGetTime (&RTCD1, &rtctime);
  rtcConvertDateTimeToStructTm (&rtctime, &utime, &tv_msec);

  utime.tm_sec = val;
  rtcConvertStructTmToDateTime (&utime, tv_msec, &rtctime);
  rtcSetTime (&RTCD1, &rtctime);

}

void setYear (uint32_t val)
{
  rtcGetTime (&RTCD1, &rtctime);
  rtcConvertDateTimeToStructTm (&rtctime, &utime, &tv_msec);

  utime.tm_year = val-1900;
  rtcConvertStructTmToDateTime (&utime, tv_msec, &rtctime);
  rtcSetTime (&RTCD1, &rtctime);

  setWeekDay (weekDay());
}

void setMonth (uint32_t val)
{
  rtcGetTime (&RTCD1, &rtctime);
  rtcConvertDateTimeToStructTm (&rtctime, &utime, &tv_msec);

  utime.tm_mon = val-1;
  rtcConvertStructTmToDateTime (&utime, tv_msec, &rtctime);
  rtcSetTime (&RTCD1, &rtctime);

  setWeekDay (weekDay());
}

void setMonthDay (uint32_t val)
{
  rtcGetTime (&RTCD1, &rtctime);
  rtcConvertDateTimeToStructTm (&rtctime, &utime, &tv_msec);

  utime.tm_mday = val;
  rtcConvertStructTmToDateTime (&utime, tv_msec, &rtctime);
  rtcSetTime (&RTCD1, &rtctime);

  setWeekDay (weekDay());
}

void setWeekDay (uint32_t val)
{
  rtcGetTime (&RTCD1, &rtctime);
  rtcConvertDateTimeToStructTm (&rtctime, &utime, &tv_msec);

  utime.tm_wday = val;
  rtcConvertStructTmToDateTime (&utime, tv_msec, &rtctime);
  rtcSetTime (&RTCD1, &rtctime);

}

uint32_t getHour (void)
{
  rtcGetTime (&RTCD1, &rtctime);
  rtcConvertDateTimeToStructTm (&rtctime, &utime, &tv_msec);

  mktime(&utime);
  
  return (utime.tm_hour+getDstOffset()) % 24;
}
uint32_t getMinute (void)
{
  rtcGetTime (&RTCD1, &rtctime);
  rtcConvertDateTimeToStructTm (&rtctime, &utime, &tv_msec);

  return utime.tm_min;
}
uint32_t getSecond (void)
{
  rtcGetTime (&RTCD1, &rtctime);
  rtcConvertDateTimeToStructTm (&rtctime, &utime, &tv_msec);

  return utime.tm_sec;
}
uint32_t getYear (void)
{
  rtcGetTime (&RTCD1, &rtctime);
  rtcConvertDateTimeToStructTm (&rtctime, &utime, &tv_msec);

  return utime.tm_year+1900;
}
uint32_t getMonth (void)
{
  rtcGetTime (&RTCD1, &rtctime);
  rtcConvertDateTimeToStructTm (&rtctime, &utime, &tv_msec);

  return utime.tm_mon+1;
}
uint32_t getMonthDay (void)
{
  rtcGetTime (&RTCD1, &rtctime);
  rtcConvertDateTimeToStructTm (&rtctime, &utime, &tv_msec);

  return utime.tm_mday;
}
uint32_t getWeekDay (void)
{
  rtcGetTime (&RTCD1, &rtctime);
  rtcConvertDateTimeToStructTm (&rtctime, &utime, &tv_msec);

  return utime.tm_wday;
}

time_t getTimeUnixSec(void)
{
  struct tm tim;
  RTCDateTime timespec;
  
  rtcGetTime(&RTCD1, &timespec);
  rtcConvertDateTimeToStructTm(&timespec, &tim, NULL);
  return mktime(&tim);
}

void setTimeUnixSec(time_t tt)
{
  // convert from time_t to struct tm
  // convert from  struct tm to RTCDateTime
  // set RTCDateTime
 RTCDateTime timespec;
 rtcConvertStructTmToDateTime (gmtime (&tt), 0, &timespec);
 rtcSetTime (&RTCD1, &timespec);
}



#endif


uint64_t getTimeUnixMillisec(void)
{
  //  return (getTimeUnixSec()*1000) +  RTC_GetSubSecond();
  struct tm tim;
  RTCDateTime timespec;
  
  rtcGetTime(&RTCD1, &timespec);
  rtcConvertDateTimeToStructTm(&timespec, &tim, NULL);
  return  ((mktime(&tim)) * 1000ULL) +  (timespec.millisecond % 1000);
}


const char *getWeekDayAscii (void)
{
  static const char* weekDays[] = {"Sunday", "Monday", "Tuesday", "Wednesday", 
				   "Thursday", "Friday", "Saturday"};

  return weekDays[(getWeekDay())];
}

static uint32_t weekDay (void)
{
  const uint32_t day = getMonthDay();
  const uint32_t month = getMonth();
  const uint32_t year = getYear();
  
  return weekDayOfDate (day, month, year);
}

static uint32_t weekDayOfDate (const uint32_t day, const uint32_t month, const uint32_t year)
{
  return  (day+1                                                    
    + ((153 * (month + 12 * ((14 - month) / 12) - 3) + 2) / 5) 
    + (365 * (year + 4800 - ((14 - month) / 12)))              
    + ((year + 4800 - ((14 - month) / 12)) / 4)                
    - ((year + 4800 - ((14 - month) / 12)) / 100)              
    + ((year + 4800 - ((14 - month) / 12)) / 400)              
	   - 32045) % 7;
}

/* static uint32_t RTC_GetSubSecond(void) */
/* { */
/*   uint32_t tmpreg = 0; */
  
/*   /\* Get sub seconds values from the correspondent registers*\/ */
/*   tmpreg = (uint32_t)(RTC->SSR); */
  
/*   /\* Read DR register to unfroze calendar registers *\/ */
/*   (void) (RTC->DR); */
  
/*   return 1000 - ((tmpreg * 1000) / 0x3FF); */
/* } */

  
uint32_t getDstOffset (void)
{
  const  uint32_t startMonth=3;
  const  uint32_t endMonth=10;
  const uint32_t day = getMonthDay();
  const uint32_t month = getMonth();
  const uint32_t year = getYear();
  static uint32_t lastSundayOfMonth=0;

  
  if ((month < startMonth) || (month > endMonth))
    return 0;

  if ((month > startMonth) && (month < endMonth))
      return 1;

  if (lastSundayOfMonth==0) {
    for (uint32_t d=31; d>=24; d--) {
      if (weekDayOfDate (d, month, year) == 0) {
	lastSundayOfMonth=d;
	break;
      }
    }
  }

  if (month == startMonth) 
    return (day >= lastSundayOfMonth) ? 1 : 0;
  else
    return (day < lastSundayOfMonth) ? 1 : 0;
}


