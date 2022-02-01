#include "rtcAccess.h"
#include <ch.h>
#include <hal.h>
#include <stdlib.h>

static uint32_t weekDay (void);
static uint32_t weekDayOfDate (const uint32_t day, const uint32_t month, const uint32_t year);
static bool isInDTSPeriod (void);

static struct tm utime;
static RtcChangedCB callback = NULL;
static void cbCheck (int delta) {
  if (callback)
    callback (delta);
}


#if (CH_KERNEL_MAJOR == 2)

#include <chrtclib.h>


void setUtcHour (uint32_t val)
{
  rtcGetTimeTm (&RTCD1, &utime);
  utime.tm_hour = val;
  utime.tm_isdst = 0;
  rtcSetTimeTm (&RTCD1, &utime);
}

void setHour (uint32_t val)
{
  setUtcHour ((24+val-getDstOffset() % 24);
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

 uint32_t getUtcHour (void)
{
  rtcGetTimeTm (&RTCD1, &utime);
  mktime(&utime);
  
  return utime.tm_hour;
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

void setUtcHour (uint32_t val)
{
  rtcGetTime (&RTCD1, &rtctime);
  rtcConvertDateTimeToStructTm (&rtctime, &utime, &tv_msec);
  const int delta = (val - utime.tm_hour) * 3600;
  utime.tm_hour = val;
  utime.tm_isdst = 0;
  rtcConvertStructTmToDateTime (&utime, tv_msec, &rtctime);
  rtcSetTime (&RTCD1, &rtctime);
  cbCheck (delta);
}

void setHour (uint32_t val)
{
  setUtcHour ((24+val-getDstOffset()) % 24);
}

void setMinute (uint32_t val)
{
  rtcGetTime (&RTCD1, &rtctime);
  rtcConvertDateTimeToStructTm (&rtctime, &utime, &tv_msec);
  const int delta = (val - utime.tm_min) * 60;
  utime.tm_min = val;
  rtcConvertStructTmToDateTime (&utime, tv_msec, &rtctime);
  rtcSetTime (&RTCD1, &rtctime);
  cbCheck (delta);
}

void setSecond (uint32_t val)
{
  rtcGetTime (&RTCD1, &rtctime);
  rtcConvertDateTimeToStructTm (&rtctime, &utime, &tv_msec);
  const int delta = val - utime.tm_sec;
  utime.tm_sec = val;
  rtcConvertStructTmToDateTime (&utime, tv_msec, &rtctime);
  rtcSetTime (&RTCD1, &rtctime);
  cbCheck (delta);
}

// TODO : rewrite low level hal_rtc_lld.c to permit subsecond SSR setting via
// RTC_SHIFTR register
 void setSecondInMs (uint32_t val)
{
  const int sec = val/1000;
  const int millisec = val%1000;
  rtcGetTime (&RTCD1, &rtctime);
  rtcConvertDateTimeToStructTm (&rtctime, &utime, &tv_msec);
  const int delta = sec - utime.tm_sec;
  utime.tm_sec =  sec;
  tv_msec = millisec;
  rtcConvertStructTmToDateTime (&utime, tv_msec, &rtctime);
  rtcSetTime (&RTCD1, &rtctime);
  cbCheck (delta);
}

void setYear (uint32_t val)
{
  rtcGetTime (&RTCD1, &rtctime);
  rtcConvertDateTimeToStructTm (&rtctime, &utime, &tv_msec);
  const int delta = (val - utime.tm_year) * 365 * 86400;
  utime.tm_year = val-1900;
  rtcConvertStructTmToDateTime (&utime, tv_msec, &rtctime);
  rtcSetTime (&RTCD1, &rtctime);

  setWeekDay (weekDay());
  cbCheck (delta);
}

void setMonth (uint32_t val)
{
  rtcGetTime (&RTCD1, &rtctime);
  rtcConvertDateTimeToStructTm (&rtctime, &utime, &tv_msec);
  const int delta = (val - utime.tm_mon) * 30 * 86400;
  utime.tm_mon = val-1;
  rtcConvertStructTmToDateTime (&utime, tv_msec, &rtctime);
  rtcSetTime (&RTCD1, &rtctime);

  setWeekDay (weekDay());
  cbCheck (delta);
}

void setMonthDay (uint32_t val)
{
  rtcGetTime (&RTCD1, &rtctime);
  rtcConvertDateTimeToStructTm (&rtctime, &utime, &tv_msec);
  const int delta = (val - utime.tm_mday) * 86400;
  utime.tm_mday = val;
  rtcConvertStructTmToDateTime (&utime, tv_msec, &rtctime);
  rtcSetTime (&RTCD1, &rtctime);

  setWeekDay (weekDay());
  cbCheck (delta);
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

  uint32_t getUtcHour (void)
{
  rtcGetTime (&RTCD1, &rtctime);
  rtcConvertDateTimeToStructTm (&rtctime, &utime, &tv_msec);

  mktime(&utime);
  
  return utime.tm_hour;
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
uint32_t getSecondInMs (void)
{
  rtcGetTime (&RTCD1, &rtctime);
  rtcConvertDateTimeToStructTm (&rtctime, &utime, &tv_msec);

  return (utime.tm_sec *1000) + (rtctime.millisecond % 1000);
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
  const time_t previousVal =  getTimeUnixSec();
  RTCDateTime timespec;
  rtcConvertStructTmToDateTime (gmtime (&tt), 0, &timespec);
  rtcSetTime (&RTCD1, &timespec);
  cbCheck (tt-previousVal);
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
  return isInDTSPeriod() ? 2 : 1;
}

static bool isInDTSPeriod (void)
{
  const  uint32_t startMonth=3;
  const  uint32_t endMonth=10;
  const uint32_t day = getMonthDay();
  const uint32_t month = getMonth();
  const uint32_t year = getYear();
  static uint32_t lastSundayOfMonth=0;

  
  if ((month < startMonth) || (month > endMonth))
    return false;

  if ((month > startMonth) && (month < endMonth))
      return true;

  if (lastSundayOfMonth==0) {
    for (uint32_t d=31; d>=24; d--) {
      if (weekDayOfDate (d, month, year) == 0) {
	lastSundayOfMonth=d;
	break;
      }
    }
  }

  if (month == startMonth) 
    return day >= lastSundayOfMonth;
  else
    return day < lastSundayOfMonth;
}


void	 registerRtcChanged (RtcChangedCB cb)
{
  callback = cb;
}

#ifdef RTC_ISR_WUTWF // RTC V2
bool rtcEnablePeriodicWakeup (RTCDriver *rtcp, uint32_t _second)
{
  if (--_second > ((2 * (1<<16))-1))
    return false;
  
  uint16_t second = _second;
#ifdef STM32_RTCSEL_LSE
  if (STM32_RTCSEL !=  STM32_RTCSEL_LSE)
    return false;
#endif
  
  rtcp->rtc->CR &= ~RTC_CR_WUTE; // Clear WUTE in RTC_CR to disable the wakeup timer.
  while ((rtcp->rtc->ISR & RTC_ISR_WUTWF) == 0)  {};
  if (_second < (1<<16)) {
    MODIFY_REG(rtcp->rtc->CR, RTC_CR_WUCKSEL, 0b100);
  } else {
    MODIFY_REG(rtcp->rtc->CR, RTC_CR_WUCKSEL, 0b110);
  }

  rtcp->rtc->WUTR = second ; 
  rtcp->rtc->CR |= RTC_CR_WUTIE;  // enable rtc wakeup ISR and Event
  rtcp->rtc->CR |= RTC_CR_WUTE; // Set WUTE in RTC_CR to enable the wakeup timer.

  while (rtcp->rtc->ISR & RTC_ISR_WUTWF)  {};
  return true;
}

bool rtcEnablePeriodicWakeupHires (RTCDriver *rtcp, uint16_t millisec)
{
#ifdef STM32_RTCSEL_LSE
  if (STM32_RTCSEL !=  STM32_RTCSEL_LSE)
    return false;
#endif
  
  rtcp->rtc->CR &= ~RTC_CR_WUTE; // Clear WUTE in RTC_CR to disable the wakeup timer.
  while ((rtcp->rtc->ISR & RTC_ISR_WUTWF) == 0)  {};
  MODIFY_REG(rtcp->rtc->CR, RTC_CR_WUCKSEL, 0b000); // RTC CLOCK DIV16

  uint32_t wutr = (millisec * (STM32_LSECLK / 16U)) / 1000U;
  if (wutr > (1U << 16))
    return false;
      
  rtcp->rtc->WUTR = wutr ; 
  rtcp->rtc->CR |= RTC_CR_WUTIE;  // enable rtc wakeup ISR and Event
  rtcp->rtc->CR |= RTC_CR_WUTE; // Set WUTE in RTC_CR to enable the wakeup timer.

  while (rtcp->rtc->ISR & RTC_ISR_WUTWF)  {};
  return true;
}

bool rtcDisablePeriodicWakeup (RTCDriver *rtcp)
{
#ifdef STM32_RTCSEL_LSE
  if (STM32_RTCSEL !=  STM32_RTCSEL_LSE)
    return false;
#endif
  
  rtcp->rtc->CR &= ~RTC_CR_WUTE; // Clear WUTE in RTC_CR to disable the wakeup timer.
  while ((rtcp->rtc->ISR & RTC_ISR_WUTWF) == 0)  {};

  rtcp->rtc->CR &= ~RTC_CR_WUTIE;  // disable rtc wakeup ISR and Event

  return true;
}
#endif

bool	 tuneShiftByOffset(RTCDriver *rtcp, int millis)
{
  chDbgAssert((millis < 1000) && (millis > -1000), "sub second correction allowed only");
  
  if ((rtcp->rtc->SSR & (1U<<15)) ||   // risk of overflow
      (rtcp->rtc->ISR & RTC_ISR_SHPF)) // already pending operation
    return false;
  
  uint32_t shiftVal = 0;
  
  // no correction, nothing to do
  if (millis == 0)
    return true;

  /* Disable write protection. */
  rtcp->rtc->WPR = 0xCA;
  rtcp->rtc->WPR = 0x53;

  if (millis > 0) {
    shiftVal = RTC_SHIFTR_ADD1S;
    millis = 1000 - millis;
  } else {
    millis = -millis;
  }

  shiftVal |= (((uint32_t) millis * STM32_RTC_PRESS_VALUE) / 1000U);
  rtcp->rtc->SHIFTR = shiftVal;
  return true;
}
