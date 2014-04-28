#ifndef __I2C_MASTER_H__
#define __I2C_MASTER_H__

#include "ch.h"
#include "hal.h"


typedef struct  {
  I2CDriver	*driver;
  const I2CConfig     *i2ccfg;
  GPIO_TypeDef  *sdaGpio;
  GPIO_TypeDef  *sclGpio;
  uint32_t      sdaPin;
  uint32_t      sclPin;
  uint8_t	alternateFunction;
} I2cMasterConfig ;


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

#ifdef I2C_USE_ADS7828
msg_t i2cGetADC_ADS7828_Val (I2CDriver *i2cd, uint8_t adrOffset, 
			     uint8_t bitmask, float *percent);
#endif


#endif //__I2C_MASTER_H__
