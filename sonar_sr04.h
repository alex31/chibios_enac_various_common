#pragma once

#include "ch.h"
#ifdef __cplusplus
extern "C" {
#endif
  
  typedef struct   {
    PWMDriver     *driver;
    uint32_t      channel;
    GPIO_TypeDef  *gpio;
    uint32_t      pin;
    uint8_t	alt;
  } SonarGen ;
  
  typedef struct   {
    ICUDriver     *driver;
    uint32_t      channel;
    GPIO_TypeDef  *gpio;
    uint32_t      pin;
    uint8_t	alt;
  } SonarInput;
  
  
  void     sonarInit (const SonarGen *pwmGen, const SonarInput *pwmInput) ;
  float    sonarGetDistanceInCm (void);
  uint32_t sonarGetPeriodInMilliseconds (void);
  bool_t   sonarGetValidity (void);
#ifdef __cplusplus
}
#endif
