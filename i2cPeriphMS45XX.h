#ifndef I2C_USE_MS45XX
#define I2C_USE_MS45XX 0
#endif

#if I2C_USE_MS45XX

#pragma once

#include "ch.h"
#include "hal.h"
#include "i2cMaster.h"



typedef struct  
{
  I2CDriver *i2cd;
  uint8_t  i2cAddr;
  uint8_t raw[4];	// raw data from the sensor
  float diff_pressure;   ///< differential pressure in Pascal
  int16_t temperature;   ///< temperature in 0.1 deg Celcius
  float airspeed;        ///< Airspeed in m/s estimated from differential pressure.
  float airspeed_scale;  ///< quadratic scale factor to convert differential pressure to airspeed
  float pressure_scale;  ///< scaling factor from raw measurement to Pascal
  float pressure_offset; ///< offset in Pascal
} MS45XXData;


msg_t MS45XX_init (MS45XXData *ms, I2CDriver *i2cd, const uint8_t i2cAddr);
msg_t MS45XX_getVal  (MS45XXData *ms, float *temp, float *pressure);


#endif
