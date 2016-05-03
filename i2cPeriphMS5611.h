#if I2C_USE_MS5611

#pragma once

#include "ch.h"
#include "hal.h"
#include "i2cMaster.h"
#include "ms5611_regs.h"



typedef struct  
{
  I2CDriver *i2cd;
  //  Mpu9250MasterConfig_0_to_3 *mstConfig;
  uint32_t cacheTimestamp; 
  uint32_t sampleInterval;                       // interval between samples in tick
  //  uint8_t rawCache[MS5611_OUT_P_LSB-MS5611_OUT_P_MSB+1];
  uint16_t promCoeffs[PROM_NB];
  uint8_t  dmaTmpBuf3[3];
  uint8_t  dmaTmpBuf1[1];
  uint8_t  i2cAddr;
  uint32_t pressure;    ///< pressure in Pascal (0.01mbar)
  int32_t temperature;  ///< temperature with 0.01 degrees Celsius resolution
  uint32_t d1;
  uint32_t d2;
} MS5611Data;


msg_t MS5611_init (MS5611Data *baro, I2CDriver *i2cd, const uint8_t i2cAddr);
//msg_t MS5611_cacheVal  (MS5611Data *baro);
msg_t MS5611_getVal  (MS5611Data *baro, float *temp, float *pressure);
//msg_t mpu9250AddSlv_MS5611 (Mpu9250Data *imu, MS5611Data *baro);

#endif
