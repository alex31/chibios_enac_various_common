#pragma once
#include "ch.h"
#include "hal.h"
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef union {
  struct {
    float x;
    float y;
    float z;
  };
  float arr[3];
}  __attribute__ ((__packed__)) ImuVec3f;
  
  
typedef struct  {
  I2CDriver	*driver;
  const I2CConfig     *i2ccfg;
  ioline_t	sda;
  ioline_t      scl;
  iomode_t	alternateFunction;
} I2cMasterConfig ;


#define I2C_EINVAL	  -13
#define I2C_BADID	  -14
#define I2C_MAXSLV_REACH  -15


bool initI2cDriver (const I2cMasterConfig *mconf);


#ifdef I2C_USE_ADXL345
msg_t i2cGetAcc_ADXL345_DevId (I2CDriver *i2cd, uint8_t *devid);
msg_t i2cGetAcc_ADXL345_Val (I2CDriver *i2cd, int16_t  rawBuf[3], float acc[3]);
msg_t i2cInitAcc_ADXL345 (I2CDriver *i2cd);
#endif

#ifdef I2C_USE_ITG3200
msg_t i2cGetGyro_ITG3200_DevId (I2CDriver *i2cd, uint8_t *devid);
msg_t i2cGetGyro_ITG3200_Val (I2CDriver *i2cd, int16_t  rawBuf[4],
			      float *temp, float gyro[3]);
msg_t i2cInitGyro_ITG3200 (I2CDriver *i2cd);
#endif

#ifdef I2C_USE_HMC5883L
msg_t i2cGetCompass_HMC5883L_DevId (I2CDriver *i2cd, uint8_t devid[3]);
msg_t i2cGetCompass_HMC5883L_Val (I2CDriver *i2cd, int16_t  rawBuf[3],
				  float mag[3]);
msg_t i2cInitCompass_HMC5883L (I2CDriver *i2cd);
#endif

#ifdef I2C_USE_MPL3115A2
typedef enum  {MPL3115A2_One_Shot, MPL3115A2_Continuous} MPL3115A2_Mode;
msg_t i2cGetBaro_MPL3115A2_DevId (I2CDriver *i2cd, uint8_t devid[3]);
msg_t i2cGetBaro_MPL3115A2_Val (I2CDriver *i2cd, int32_t  *rawBuf, float *pressure);
msg_t i2cInitBaro_MPL3115A2 (I2CDriver *i2cd, MPL3115A2_Mode mode);
#endif

#ifdef I2C_USE_MPU6050
typedef enum  {MPU6050_Bypass, MPU6050_Master} MPU6050_Mode;
msg_t i2cGetIMU_MPU6050_DevId (I2CDriver *i2cd, uint8_t devid[3]);
msg_t  i2cGetIMU_MPU6050_Val (I2CDriver *i2cd, int8_t  *rawBuf, 
			      float *temp, float gyro[3], float acc[3], 
			      float *pressure, float mag[3]);
msg_t i2cInitIMU_MPU6050 (I2CDriver *i2cd, MPU6050_Mode mode);
#endif

#ifdef I2C_USE_PCF8574
msg_t i2cSetIO_PCF8574 (I2CDriver *i2cd, uint8_t ioVal);
msg_t i2cInitIO_PCF8574(I2CDriver *i2cd);
#endif

#ifdef I2C_USE_MCP23008
msg_t i2cSetIO_MCP23008 (I2CDriver *i2cd, uint8_t ioVal);
msg_t i2cGetIO_PCF8574 (I2CDriver *i2cd, uint8_t *ioVal);
#endif

#ifdef I2C_USE_24AA02
msg_t i2cRead24AA02 (I2CDriver *i2cd, const uint8_t chipAddr, 
		     const uint8_t eepromAddr, uint8_t *buffer, const size_t len);
msg_t i2cWrite24AA02 (I2CDriver *i2cd, const uint8_t chipAddr, 
		      const uint8_t eepromAddr, const uint8_t *buffer, const size_t len);
#endif



/*
  i2cd		: I2C driver
  adrOffset	: pin selectionnable i2c address offset
  bitmask       : mask of channel to be sampled
  useExt_VRef	: use external ref (1) or internal 2.5v ref (0)
  percent	: pointer to an array of 8 float values
 */
#ifdef I2C_USE_ADS7828
msg_t i2cGetADC_ADS7828_Val (I2CDriver *i2cd, const uint8_t adrOffset, 
			     const uint8_t bitmask, const bool useExt_VRef, 
			     float *percent);
#endif

#ifdef I2C_USE_SDP3X
#include "i2cPeriphSDP3X.h"
#endif

#ifdef I2C_USE_LIS3MDL
#include "i2cPeriphLIS3MDL.h"
#endif

#ifdef I2C_USE_BMP3XX
#include "i2cPeriphBMP3XX.h"
#endif

#ifdef I2C_USE_LPS33HW
#include "i2cPeriphLPS33HW.h"
#endif


#include "i2cPeriphMpu9250.h"
#include "i2cPeriphMpu20600.h"
#include "i2cPeriphMPL3115A2.h"
#include "i2cPeriphMS45XX.h"

#ifdef __cplusplus
}
#endif



static inline void imuVec3fAdd (const ImuVec3f *opa, const ImuVec3f *opb, ImuVec3f *res)
{
  for (int i=0; i< 3; i++) {
    res->arr[i]=opa->arr[i]+opb->arr[i];
  }
}

static inline void imuVec3fSub (const ImuVec3f *opa, const ImuVec3f *opb, ImuVec3f *res)
{
  for (int i=0; i< 3; i++) {
    res->arr[i]=opa->arr[i]-opb->arr[i];
  }
}

static inline void imuVec3fAccumulate (ImuVec3f *acc, const ImuVec3f *opa)
{
  for (int i=0; i< 3; i++) {
    acc->arr[i]+=opa->arr[i];
  }
}

static inline void imuVec3fZero (ImuVec3f *v)
{
  for (int i=0; i< 3; i++) {
    v->arr[i]=0.0f;
  }
}

static inline void imuVec3fDiv (ImuVec3f *v, const float div)
{
  for (int i=0; i< 3; i++) {
    v->arr[i] /= div;
  }
}

static inline void imuVec3fMul (ImuVec3f *v, const float mul)
{
  for (int i=0; i< 3; i++) {
    v->arr[i] *= mul;
  }
}


static inline void imuVec3fNormalize (const ImuVec3f *v, ImuVec3f *norm)
{  // normalize vector 3d
  float vlen = 0.0f;
  for (int i=0; i<3; i++) 
    vlen += (v->arr[i] *  v->arr[i]);
  
  vlen = sqrtf (vlen);
  if (vlen != 0.0f) {
    for (int i=0; i<3; i++) 
      norm->arr[i] = v->arr[i] / vlen;
  } else {
    imuVec3fZero (norm);
  }
}
