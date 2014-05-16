#include "rtcAccess.h"
#include <ch.h>
#include <hal.h>
#include <chrtclib.h>
#include <stdlib.h>


static struct tm utime;
static uint32_t weekDay (void);
static uint32_t weekDayOfDate (const uint32_t day, const uint32_t month, const uint32_t year);
static uint32_t getDstOffset (void);

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


static uint32_t getDstOffset (void)
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
