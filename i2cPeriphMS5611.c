#include <ch.h>
#include <hal.h>

#if defined I2C_USE_MS5611 && I2C_USE_MS5611 != 0
#include "i2cPeriphMS5611.h"

static bool ms5611_prom_crc_ok(uint16_t *prom);
static bool ms5611_calc(MS5611Data *ms);


msg_t MS5611_init (MS5611Data *baro, I2CDriver *i2cd, const uint8_t i2cAddr)
{
  baro->i2cd = i2cd;
  baro->cacheTimestamp = 0;
  baro->sampleInterval = 0;
  baro->i2cAddr = i2cAddr;
  msg_t status = MSG_OK;
  
  // reset MS5611
  i2cAcquireBus(i2cd);
  I2C_WRITE_REGISTERS (baro->i2cd, baro->i2cAddr, MS5611_SOFT_RESET);
  i2cReleaseBus(i2cd);
  chThdSleepMilliseconds (10);
  
  // get prom from MS5611
  i2cAcquireBus(i2cd);
  for (uint8_t promCnt=0; promCnt<PROM_NB; promCnt++) {
    baro->promCoeffs[promCnt] =0;
    baro->dmaTmpBuf1[0] = MS5611_PROM_READ | (promCnt << 1);
    I2C_READ_WRITE (baro->i2cd, baro->i2cAddr, baro->dmaTmpBuf1,
		    (uint8_t *) &(baro->promCoeffs[promCnt]), sizeof (baro->promCoeffs[0]));
    baro->promCoeffs[promCnt] = SWAP_ENDIAN16(baro->promCoeffs[promCnt]);
    //    DebugTrace ("prom[%d] = 0x%x", promCnt, baro->promCoeffs[promCnt]);
  }
  i2cReleaseBus(i2cd);

  if (ms5611_prom_crc_ok (baro->promCoeffs) != true) {
    status = MSG_RESET;
  }
  return status;
}




msg_t MS5611_getVal  (MS5611Data *baro, float *temp, float *pressure)
{
  msg_t status;
  *temp = 0.0f;
  *pressure = 0.0f;
  
  /* LAUNCH D1 conversion. We use OSR=4096 for maximum resolution */
  i2cAcquireBus(baro->i2cd);
  I2C_WRITE_REGISTERS (baro->i2cd, baro->i2cAddr, MS5611_REG_D1OSR4096);
  i2cReleaseBus(baro->i2cd);
  chThdSleepMicroseconds (9000);

  i2cAcquireBus(baro->i2cd);
  baro->dmaTmpBuf1[0] = MS5611_ADC_READ;
  I2C_READ_WRITE (baro->i2cd, baro->i2cAddr, baro->dmaTmpBuf1,
		  baro->dmaTmpBuf3, sizeof (baro->dmaTmpBuf3));
  baro->d1 = (baro->dmaTmpBuf3[0] << 16) | (baro->dmaTmpBuf3[1] << 8) |
	      baro->dmaTmpBuf3[2];
  
  /* if value is zero, it was read to soon and is invalid, back to idle */
  i2cReleaseBus(baro->i2cd);
  if (baro->d1 == 0) {
    DebugTrace ("Err D1");
    return MSG_RESET;
  }

  /*  LAUNCH D2 conversion. We use OSR=4096 for maximum resolution */
  i2cAcquireBus(baro->i2cd);
  I2C_WRITE_REGISTERS (baro->i2cd, baro->i2cAddr, MS5611_REG_D2OSR4096);
  i2cReleaseBus(baro->i2cd);
  chThdSleepMicroseconds (9000);

  i2cAcquireBus(baro->i2cd);
  baro->dmaTmpBuf1[0] = MS5611_ADC_READ;
  I2C_READ_WRITE (baro->i2cd, baro->i2cAddr, baro->dmaTmpBuf1,
		  baro->dmaTmpBuf3, sizeof (baro->dmaTmpBuf3));
  baro->d2 = (baro->dmaTmpBuf3[0] << 16) | (baro->dmaTmpBuf3[1] << 8) |
    	      baro->dmaTmpBuf3[2];
  i2cReleaseBus(baro->i2cd);
  
  /* if value is zero, it was read to soon and is invalid, back to idle */
  if (baro->d2 == 0) {
    DebugTrace ("Err D2");
    return MSG_RESET;
  }
  
  ms5611_calc (baro);
  *temp = baro->temperature / 100.0f;
  *pressure = baro->pressure / 100.0f;
  return MSG_OK;
}


static bool ms5611_prom_crc_ok(uint16_t *prom)
{
  int32_t i, j;
  uint32_t res = 0;
  uint8_t crc = prom[7] & 0xF;
  prom[7] &= 0xFF00;
  for (i = 0; i < 16; i++) {
    if (i & 1) {
      res ^= ((prom[i >> 1]) & 0x00FF);
    } else {
      res ^= (prom[i >> 1] >> 8);
    }
    for (j = 8; j > 0; j--) {
      if (res & 0x8000) {
        res ^= 0x1800;
      }
      res <<= 1;
    }
  }
  prom[7] |= crc;
  if (crc == ((res >> 12) & 0xF)) {
    return true;
  } else {
    return false;
  }
}


static bool ms5611_calc(MS5611Data *ms)
{
  int64_t dt, tempms, off, sens, t2, off2, sens2;

  /* difference between actual and ref temperature */
  dt = ms->d2 - (int64_t)ms->promCoeffs[5] * (1 << 8);
  /* actual temperature */
  tempms = 2000 + ((int64_t)dt * ms->promCoeffs[6]) / (1 << 23);
  /* offset at actual temperature */
  off = ((int64_t)ms->promCoeffs[2] * (1 << 16)) + ((int64_t)ms->promCoeffs[4] * dt) / (1 << 7);
  /* sensitivity at actual temperature */
  sens = ((int64_t)ms->promCoeffs[1] * (1 << 15)) + ((int64_t)ms->promCoeffs[3] * dt) / (1 << 8);
  /* second order temperature compensation */
  if (tempms < 2000) {
    t2 = (dt * dt) / (1 << 31);
    off2 = 5 * ((int64_t)(tempms - 2000) * (tempms - 2000)) / (1 << 1);
    sens2 = 5 * ((int64_t)(tempms - 2000) * (tempms - 2000)) / (1 << 2);
    if (tempms < -1500) {
      off2 = off2 + 7 * (int64_t)(tempms + 1500) * (tempms + 1500);
      sens2 = sens2 + 11 * ((int64_t)(tempms + 1500) * (tempms + 1500)) / (1 << 1);
    }
    tempms = tempms - t2;
    off = off - off2;
    sens = sens - sens2;
  }

  /* temperature compensated pressure in Pascal (0.01mbar) */
  uint32_t p = (((int64_t)ms->d1 * sens) / (1 << 21) - off) / (1 << 15);
  /* if temp and pressare are in valid bounds, copy and return TRUE (valid) */
  if ((tempms > -4000) && (tempms < 8500) && (p > 1000) && (p < 120000)) {
    /* temperature in deg Celsius with 0.01 degC resolultion */
    ms->temperature = (int32_t)tempms;
    ms->pressure = p;
    return TRUE;
  }
  return FALSE;
}


//msg_t mpu9250AddSlv_MS5611 (Mpu9250Data *imu, MS5611Data *baro);
//msg_t MS5611_cacheVal  (MS5611Data *baro);

#endif
