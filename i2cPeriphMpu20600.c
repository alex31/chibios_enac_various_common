#if I2C_USE_IMU20600

#include "i2cPeriphMpu20600.h"
#include <math.h>
#include <string.h>

/*

TODO :

* verifier la période d'interrogation
* trouver le bug
* commenter les structures de i2cperiph9150.h et i2cperiphmpl311.h
* commenter les fonctions addslave


 */


/*
#                 __  __   _____    _    _              ___    ___    _____    ___          
#                |  \/  | |  __ \  | |  | |            / _ \  |__ \  | ____|  / _ \         
#                | \  / | | |__) | | |  | |           | (_) |    ) | | |__   | | | |        
#                | |\/| | |  ___/  | |  | |            \__, |   / /  |___ \  | | | |        
#                | |  | | | |      | |__| |  ______      / /   / /_   ___) | | |_| |        
#                |_|  |_| |_|       \____/  |______|    /_/   |____| |____/   \___/         
*/



static    msg_t setInitialConfig(Mpu20600Data *imu);
static    msg_t setGyroConfig(Mpu20600Data *imu);
static    msg_t setAccelConfig(Mpu20600Data *imu);
static    msg_t setSampleRate( Mpu20600Data *imu);
static    msg_t resetFifo( Mpu20600Data *imu);

#define MATH_PI 3.14159265358979323846f


msg_t mpu20600_init( Mpu20600Data *imu, const Mpu20600Config* initParam)
{
  msg_t status;

  imu->i2cd =  initParam->i2cd;
  
  imu->slaveAddr = initParam->useAd0 ? MPU20600_ADDRESS1 :  MPU20600_ADDRESS0;
  imu->registerSegmentLen = MPU20600_JUSTIMU_LAST -  MPU20600_REGISTER_BASE +1;
  imu->cacheTimestamp = chSysGetRealtimeCounterX();
  imu->nextSlvFreeSlot = 0;
  imu->accOnly = false;

  status = setInitialConfig(imu);

  if (status == MSG_OK) 
    status = mpu20600_setSampleRate(imu, initParam->sampleRate);

  if (status == MSG_OK)
    status = mpu20600_setGyroLpf(imu, initParam->gyroLpf);
  
  if (status == MSG_OK)
    status = mpu20600_setAccelLpf(imu, initParam->accelLpf);
  
  if (status == MSG_OK)
    status = mpu20600_setGyroFsr(imu, initParam->gyroFsr);
  
  if (status == MSG_OK)
    status = mpu20600_setAccelFsr(imu, initParam->accelFsr);
  
  return status;
}

msg_t mpu20600_setGyroLpf( Mpu20600Data *imu, const uint8_t lpf)
{
  switch (lpf) {
  case MPU20600_GYRO_LPF_8800:
  case MPU20600_GYRO_LPF_3600:
  case MPU20600_GYRO_LPF_250:
  case MPU20600_GYRO_LPF_184:
  case MPU20600_GYRO_LPF_92:
  case MPU20600_GYRO_LPF_41:
  case MPU20600_GYRO_LPF_20:
  case MPU20600_GYRO_LPF_10:
  case MPU20600_GYRO_LPF_5:
    imu->gyroLpf = lpf;
    break;
    
  default:
    return I2C_EINVAL;
  }

  return setGyroConfig(imu);
}

msg_t mpu20600_setAccelLpf( Mpu20600Data *imu, const uint8_t lpf)
{
  switch (lpf) {
  case MPU20600_ACCEL_LPF_1130:
  case MPU20600_ACCEL_LPF_460:
  case MPU20600_ACCEL_LPF_184:
  case MPU20600_ACCEL_LPF_92:
  case MPU20600_ACCEL_LPF_41:
  case MPU20600_ACCEL_LPF_20:
  case MPU20600_ACCEL_LPF_10:
  case MPU20600_ACCEL_LPF_5:
    imu->accelLpf = lpf;
    break;
    
  default:
    return I2C_EINVAL;
  }

  return setAccelConfig(imu);
}

msg_t mpu20600_setSampleRate( Mpu20600Data *imu, const uint32_t rate)
{
  if ((rate < MPU20600_SAMPLERATE_MIN) || (rate > MPU20600_SAMPLERATE_MAX)) {
    return I2C_EINVAL;
  }
  imu->sampleRate = rate;
  imu->sampleInterval =  STM32_SYSCLK / imu->sampleRate;
  if (imu->sampleInterval == 0)
    imu->sampleInterval = 1;
  return setSampleRate(imu);
}

msg_t mpu20600_setGyroFsr( Mpu20600Data *imu, const uint8_t fsr)
{
  switch (fsr) {
  case MPU20600_GYROFSR_250:
    imu->gyroFsr = fsr;
    imu->gyroScale = MATH_PI / (131.0d * 180.0d);
    break;

  case MPU20600_GYROFSR_500:
    imu->gyroFsr = fsr;
    imu->gyroScale = MATH_PI / (62.5d * 180.0d);
    break;
      
  case MPU20600_GYROFSR_1000:
    imu->gyroFsr = fsr;
    imu->gyroScale = MATH_PI / (32.8d * 180.0d);
    break;
   
  case MPU20600_GYROFSR_2000:
    imu->gyroFsr = fsr;
    imu->gyroScale = MATH_PI / (16.4d * 180.0d);
    break;
            
  default:
    return I2C_EINVAL;
  }


  return setGyroConfig(imu);
}

msg_t mpu20600_setAccelFsr( Mpu20600Data *imu, const uint8_t fsr)
{
   switch (fsr) {
    case MPU20600_ACCELFSR_2:
        imu->accelFsr = fsr;
        imu->accelScale = 1.0d / 16384.0d;
	break;
    
    case MPU20600_ACCELFSR_4:
        imu->accelFsr = fsr;
        imu->accelScale = 1.0d / 8192.0d;
	break;
	
    case MPU20600_ACCELFSR_8:
        imu->accelFsr = fsr;
        imu->accelScale = 1.0d / 4096.0d;
	break;

    case MPU20600_ACCELFSR_16:
        imu->accelFsr = fsr;
        imu->accelScale = 1.0d / 2048.0d;
	break;

    default:
        return I2C_EINVAL;
    }

   return setAccelConfig(imu);
}



msg_t mpu20600_cacheVal ( Mpu20600Data *imu)
{
  msg_t status = MSG_OK;

  i2cAcquireBus(imu->i2cd);

  // if we get all 9 axes
  if (imu->accOnly == false) {
    I2C_READLEN_REGISTERS(imu->i2cd, imu->slaveAddr, MPU20600_REGISTER_BASE, imu->rawCache, 
			  imu->registerSegmentLen);
  } else { // if we just get 3 axes of accelerations
    I2C_READLEN_REGISTERS(imu->i2cd, imu->slaveAddr, MPU20600_REGISTER_BASE, imu->rawCache, 
			  6);
  }
  i2cReleaseBus(imu->i2cd);
  
  return MSG_OK; 
}

msg_t mpu20600_getVal ( Mpu20600Data *imu, float *temp,
		     ImuVec3f *gyro, ImuVec3f *acc)
{
  msg_t status = MSG_OK;

  // alias 
  const uint8_t  *rawB =  imu->rawCache;

  if (!chSysIsCounterWithinX(chSysGetRealtimeCounterX(),
			     imu->cacheTimestamp, 
			     imu->cacheTimestamp + imu->sampleInterval)) {
    imu->cacheTimestamp = chSysGetRealtimeCounterX();
    status =  mpu20600_cacheVal(imu);
  }

  if (status != MSG_OK) 
    return status;

   // m/s²
  for (int i=0; i < 3; i++) {
    acc->arr[i] =  ((int16_t) ((rawB[i*2]<<8) | rawB[(i*2)+1])) * imu->accelScale  * 9.81f;
  }

  // if we are in low power mode acceleration sensor only, don't calculate other values
  if (imu->accOnly) {
    return status;
  }
  
  // C°
  *temp = ( ((int16_t) ((rawB[6]<<8) | rawB[7])) / 340.0f) + 21.0f;
  
  // rad/s
 for (int i=0; i < 3; i++) {
    gyro->arr[i] =  ((int16_t) ((rawB[(i*2)+8]<<8) | rawB[(i*2)+9])) * imu->gyroScale;
  }


 
  return status;
}

msg_t mpu20600_getDevid(Mpu20600Data *imu, uint8_t *devid)
{ 
  msg_t status = MSG_OK;
  i2cAcquireBus(imu->i2cd);
  I2C_READ_REGISTER(imu->i2cd, imu->slaveAddr, MPU20600_WHO_AM_I, devid);
  i2cReleaseBus(imu->i2cd);
  if (*devid != ICM20600_WHO_AM_I_CONTENT)
    status = I2C_BADID;
  return status;
}





static    msg_t setInitialConfig( Mpu20600Data *imu)
{
  msg_t status = MSG_OK;
  i2cAcquireBus(imu->i2cd);

  // reset the entire 20600 (if compass I²C has been shutdown, this will not restore it
  I2C_WRITE_REGISTERS(imu->i2cd, imu->slaveAddr, MPU20600_PWR_MGMT_1, 0x80);
  chThdSleepMilliseconds(10);
  I2C_WRITE_REGISTERS(imu->i2cd, imu->slaveAddr, MPU20600_PWR_MGMT_1, 0x1);

  i2cReleaseBus(imu->i2cd);
  return status;
}




static    msg_t setGyroConfig( Mpu20600Data *imu)
{
  msg_t status = MSG_OK;
  uint8_t gyroConfig = (uint8_t) (imu->gyroFsr | ((imu->gyroLpf >> 3U) & 3U));
  uint8_t gyroLpf = (uint8_t) (imu->gyroLpf & 7U);

  i2cAcquireBus(imu->i2cd);
  I2C_WRITE_REGISTERS(imu->i2cd, imu->slaveAddr, MPU20600_GYRO_CONFIG, gyroConfig);
  I2C_WRITE_REGISTERS(imu->i2cd, imu->slaveAddr, MPU20600_GYRO_LPF, gyroLpf);
  i2cReleaseBus(imu->i2cd);

  return status;
}

static    msg_t setAccelConfig( Mpu20600Data *imu)
{
  msg_t status = MSG_OK;

  i2cAcquireBus(imu->i2cd);
  I2C_WRITE_REGISTERS(imu->i2cd, imu->slaveAddr, MPU20600_ACCEL_CONFIG, imu->accelFsr);
  I2C_WRITE_REGISTERS(imu->i2cd, imu->slaveAddr, MPU20600_ACCEL_LPF, imu->accelLpf);
  i2cReleaseBus(imu->i2cd);
  
  return status;
}


static    msg_t setSampleRate( Mpu20600Data *imu)
{
  msg_t status = MSG_OK;

  i2cAcquireBus(imu->i2cd);
  if (imu->sampleRate > 1000) {
    I2C_WRITE_REGISTERS(imu->i2cd, imu->slaveAddr, MPU20600_SMPRT_DIV, 0);
  } else {
    I2C_WRITE_REGISTERS(imu->i2cd, imu->slaveAddr, MPU20600_SMPRT_DIV, 
			  ((uint8_t) ((1000 / imu->sampleRate) - 1)));
  }
  
  i2cReleaseBus(imu->i2cd);
  return status;
}


/* static    msg_t setCompassRate( Mpu20600Data *imu) */
/* { */
/*   return MSG_OK; */
/* } */

static __attribute__((__unused__)) msg_t resetFifo( Mpu20600Data *imu) 
{
  msg_t status = MSG_OK;
  
  i2cAcquireBus(imu->i2cd);
  I2C_WRITE_REGISTERS(imu->i2cd, imu->slaveAddr, MPU20600_INT_ENABLE, 0);
  I2C_WRITE_REGISTERS(imu->i2cd, imu->slaveAddr, MPU20600_FIFO_EN, 0);
  I2C_WRITE_REGISTERS(imu->i2cd, imu->slaveAddr, MPU20600_USER_CTRL, 0);
  I2C_WRITE_REGISTERS(imu->i2cd, imu->slaveAddr, MPU20600_USER_CTRL, 0x04);
  I2C_WRITE_REGISTERS(imu->i2cd, imu->slaveAddr, MPU20600_USER_CTRL, 0x60);
  chThdSleepMilliseconds(50);
  I2C_WRITE_REGISTERS(imu->i2cd, imu->slaveAddr, MPU20600_INT_ENABLE, 1);
  I2C_WRITE_REGISTERS(imu->i2cd, imu->slaveAddr, MPU20600_FIFO_EN, 0x78);
  i2cReleaseBus(imu->i2cd);
  
  return status;
}

/* static    msg_t compassSetup( Mpu20600Data *imu) */
/* { */
/*   return MSG_OK; */
/* } */





msg_t mpu20600_getItrStatus (Mpu20600Data *imu, uint8_t *itrStatus)
{
  msg_t status = MSG_OK;
  i2cAcquireBus( imu->i2cd);
  I2C_READ_REGISTER(imu->i2cd, imu->slaveAddr, MPU20600_INT_STATUS, itrStatus);
  i2cReleaseBus(imu->i2cd);
  return status;
}






msg_t mpu20600_setModeAccOnly(Mpu20600Data *imu)
{
  msg_t status;
  // if compass not already set in sleep mode, do it

  
  i2cAcquireBus(imu->i2cd); 
  
  // disable gyro sensors
  I2C_WRITE_REGISTERS(imu->i2cd, imu->slaveAddr, MPU20600_PWR_MGMT_2, MPU20600_PWRM2_DISABLE_GYRO);
  
  // put gyro circuitry in standby
  status = i2cMasterWriteBit(imu->i2cd, imu->slaveAddr,
			      MPU20600_PWR_MGMT_1,  MPU20600_PWRM1_GYROSTANDBY, true);
  
  i2cReleaseBus(imu->i2cd); 
  return status;
}

msg_t mpu20600_setModeAutoWake(Mpu20600Data *imu, 
			      Mpu20600_LowPowerAccelerometerFrequencyCycle lpodr)
{
  msg_t status = MSG_RESET;
  // if compass not already set in sleep mode, do it
  
  i2cAcquireBus(imu->i2cd); 
  if (lpodr != MPU20600_NORMAL_POWER_ODR) {
    // set frequency for waking mpu
    I2C_WRITE_REGISTERS(imu->i2cd, imu->slaveAddr, MPU20600_ACCEL_ODR, lpodr);
    
    // set cycle bit to 1
    I2C_WRITE_REGISTERS(imu->i2cd, imu->slaveAddr, MPU20600_PWR_MGMT_1, 0b00100001);
  }
  
  i2cReleaseBus(imu->i2cd); 
  return status;
}


msg_t mpu20600_activateMotionDetect(Mpu20600Data *imu, const uint32_t threadsholdInMilliG, const uint8_t pinConfigMask)
{
  msg_t status;
  const uint8_t threadshold = MIN((threadsholdInMilliG/4), 255U);
  
  i2cAcquireBus(imu->i2cd);
  
  // enable wake on motion detection logic
  I2C_WRITE_REGISTERS(imu->i2cd, imu->slaveAddr,  MPU20600_ACCEL_ITR_CTRL,
		       MPU20600_INTEL_ENABLE | MPU20600_INTEL_MODE_COMPARE);
  
  // interrupt pin is active @level low, opendrain, 50µs pulse, clear on any register read
  I2C_WRITE_REGISTERS(imu->i2cd, imu->slaveAddr, MPU20600_INT_PIN_CFG, pinConfigMask);
  
  // interrupt pin fire on motion detection
  I2C_WRITE_REGISTERS(imu->i2cd, imu->slaveAddr, MPU20600_INT_ENABLE,
			MPU20600_INT_ENABLE_WAKE_ON_MOTION);
  
  // set threshold
  I2C_WRITE_REGISTERS(imu->i2cd, imu->slaveAddr, MPU20600_ACCEL_WOM_THRESHOLD, threadshold);
  
  
  i2cReleaseBus(imu->i2cd); 
  return status;
}

/* static msg_t mpu20600_inactiveMotionDetect (Mpu20600Data *imu) */
/* { */
/*   msg_t status; */
  
/*   i2cAcquireBus(imu->i2cd); */
  
/*   // enable wake on motion detection logic */
/*   I2C_WRITE_REGISTERS(imu->i2cd, imu->slaveAddr,  MPU20600_ACCEL_ITR_CTRL, */
/* 		       MPU20600_INTEL_DISABLE); */
  
/*   // interrupt pin is active @level low, opendrain, 50µs pulse, clear on any register read */
/*   I2C_WRITE_REGISTERS  (imu->i2cd, imu->slaveAddr, MPU20600_INT_PIN_CFG, */
/* 			MPU20600_INT_PIN_CFG_ACTIVE_LOW | MPU20600_INT_PIN_CFG_OPENDRAIN); */
  
/*   // interrupt pin fire on motion detection */
/*   I2C_WRITE_REGISTERS  (imu->i2cd, imu->slaveAddr, MPU20600_INT_ENABLE, */
/* 			MPU20600_INT_DISABLE_WAKE_ON_MOTION); */
  
/*   // set threshold */
/*   I2C_WRITE_REGISTERS  (imu->i2cd, imu->slaveAddr, MPU20600_ACCEL_WOM_THRESHOLD, 255); */
  
/*   i2cReleaseBus(imu->i2cd);  */
/*   return status; */
/* } */

msg_t mpu20600_setModeDeepSleep(Mpu20600Data *imu)
{
  msg_t status;
  
  
 
  i2cAcquireBus(imu->i2cd);
  // set 20600 in deep sleep mode
  I2C_WRITE_REGISTERS(imu->i2cd, imu->slaveAddr, MPU20600_PWR_MGMT_1,  MPU20600_PWRM1_SLEEP);
  i2cReleaseBus(imu->i2cd);
  
  
  return status;
}

#endif
