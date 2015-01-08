#include "ch.h"
#include "hal.h"
#include "stdutil.h"
#include "globalVar.h"
#include "sonar_sr04.h"



static  float	        lastDistance= 0.0f;
static  uint32_t	lastPeriod= 0;
static  float		meanDistance = 0.0f;

static  const SonarGen    *pwmGen;
static  const SonarInput  *pwmInput;



static void setPwmValue (const SonarGen *s, const uint32_t value);
static void icuwidthcb(ICUDriver *icup);
static void icuperiodcb(ICUDriver *icup);


void sonarInit (const SonarGen *_pwmGen, const SonarInput *_pwmInput) 
{
  pwmGen = _pwmGen;
  pwmInput = _pwmInput;

  static PWMConfig pwmcfgTim = {
    200000,                                 /* 200 khz PWM clock frequency.   */
    20000,                                 /* Initial PWM period 0.1s   */
    NULL,                                   /* no callback on PWM counter reset */
    {
      {PWM_OUTPUT_ACTIVE_HIGH, NULL},
      {PWM_OUTPUT_DISABLED, NULL},
      {PWM_OUTPUT_DISABLED, NULL},
      {PWM_OUTPUT_DISABLED, NULL}
    },
    0,
    0
  };
  
  pwmStart(pwmGen->driver, &pwmcfgTim);
  palSetPadMode (pwmGen->gpio, pwmGen->pin, PAL_MODE_ALTERNATE(pwmGen->alt));
  setPwmValue (pwmGen, 2); // 10 µs pulse 


  static ICUConfig icucfgTim = {
    ICU_INPUT_ACTIVE_HIGH,
    100000,                                    /* 100 khz ICU clock frequency.   */
    icuwidthcb,
    icuperiodcb,
    NULL,
    ICU_CHANNEL_1,
    0
  };
  
  icuStart(pwmInput->driver, &icucfgTim);
  palSetPadMode (pwmInput->gpio, pwmInput->pin, PAL_MODE_ALTERNATE(pwmInput->alt));
  icuEnable(pwmInput->driver);
}

float sonarGetDistanceInCm (void)
{
  return  meanDistance;
}

uint32_t  sonarGetPeriodInMilliseconds (void)
{
  return lastPeriod/100;
}

/*
#                 _ __           _                    _                   
#                | '_ \         (_)                  | |                  
#                | |_) |  _ __   _   __   __   __ _  | |_     ___         
#                | .__/  | '__| | |  \ \ / /  / _` | | __|   / _ \        
#                | |     | |    | |   \ V /  | (_| | \ |_   |  __/        
#                |_|     |_|    |_|    \_/    \__,_|  \__|   \___|        
*/


static void setPwmValue (const SonarGen *s, const uint32_t value)
{
  pwmEnableChannel(s->driver, s->channel, PWM_FRACTION_TO_WIDTH(s->driver, 100, value)); 
}

static void icuwidthcb(ICUDriver *icup) 
{
  const uint32_t lastWidth = icuGetWidth(icup);
  lastDistance =  lastWidth * (10.0e-6f * 34000.0f /2.0f);
  
  if ((lastDistance > 4.0f) && (lastDistance <= 400.0f)) {
    meanDistance = ((4.0f*meanDistance)+lastDistance)/5.0f;
  }
}

static void icuperiodcb(ICUDriver *icup) 
{
  lastPeriod = icuGetPeriod(icup);
}
