#pragma once
#include "ch.h"
#include "hal.h"


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
} ImuVec3f;
  
  
typedef struct  {
  I2CDriver	*driver;
  const I2CConfig     *i2ccfg;
  GPIO_TypeDef  *sdaGpio;
  GPIO_TypeDef  *sclGpio;
  uint32_t      sdaPin;
  uint32_t      sclPin;
  iomode_t	alternateFunction;
} I2cMasterConfig ;


#define I2C_EINVAL	  -13
#define I2C_BADID	  -14
#define I2C_MAXSLV_REACH  -15


bool_t initI2cDriver (const I2cMasterConfig *mconf);


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


#include "i2cPeriphMpu9250.h"
#include "i2cPeriphMPL3115A2.h"
#ifdef __cplusplus
}
#endif



