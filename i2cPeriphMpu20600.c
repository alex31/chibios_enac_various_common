#if I2C_USE_IMU20600

#include "i2cPeriphMpu20600.h"
#include <math.h>
#include <string.h>
#include "stdutil.h"
/*

TODO :

* verifier la période d'interrogation
* trouver le bug
* commenter les structures de i2cperiph9150.h et i2cperiphmpl311.h
* commenter les fonctions addslave


 */


/*
#                                            ___   ___    __   ___   ___  
#                                           |__ \ / _ \  / /  / _ \ / _ \ 
#                 _ __ ___  _ __  _   _ ______ ) | | | |/ /_ | | | | | | |
#                | '_ ` _ \| '_ \| | | |______/ /| | | | '_ \| | | | | | |
#                | | | | | | |_) | |_| |     / /_| |_| | (_) | |_| | |_| |
#                |_| |_| |_| .__/ \__,_|    |____|\___/ \___/ \___/ \___/ 
#                          | |                                            
#                          |_|                                            
*/




static    msg_t setInitialConfig(Mpu20600Data *imu);
static    msg_t setGyroConfig(Mpu20600Data *imu);
static    msg_t setAccelConfig(Mpu20600Data *imu);
static    msg_t setSampleRate( Mpu20600Data *imu);
static    msg_t fifoEnable( Mpu20600Data *imu);
static    void  rawToSI(const Mpu20600Data *imu, 
		    ImuVec3f *acc, ImuVec3f *gyro,
			float *temp);
static msg_t readFifo(Mpu20600Data *imu, Mpu20600_Status *mpuStatus);

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
  imu->fifoIndex = imu->fifoLen = 0;

  status = setInitialConfig(imu);

  if (status == MSG_OK) 
    status = mpu20600_setSampleRate(imu, initParam->sampleRate);

  if (status == MSG_OK)
    status = mpu20600_setGyroFsr(imu, initParam->gyroFsr);
  
  if (status == MSG_OK)
    status = mpu20600_setGyroLpf(imu, initParam->gyroLpf);
  
  if (status == MSG_OK)
    status = mpu20600_setAccelFsr(imu, initParam->accelFsr);

  if (status == MSG_OK)
    status = mpu20600_setAccelLpf(imu, initParam->accelLpf);
  
  if ((status == MSG_OK) && (initParam->fifoEnabled))
    status = fifoEnable(imu);
  
  return status;
}

msg_t mpu20600_setGyroLpf( Mpu20600Data *imu, const uint8_t lpf)
{
  switch (lpf) {
  case MPU20600_GYRO_LPF_3281:
  case MPU20600_GYRO_LPF_250:
  case MPU20600_GYRO_LPF_176:
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
  case MPU20600_ACCEL_LPF_1046:
  case MPU20600_ACCEL_LPF_420:
  case MPU20600_ACCEL_LPF_218:
  case MPU20600_ACCEL_LPF_99:
  case MPU20600_ACCEL_LPF_44:
  case MPU20600_ACCEL_LPF_21:
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
    imu->gyroScale = MATH_PI / (131.0d * 180.0d);
    break;

  case MPU20600_GYROFSR_500:
    imu->gyroScale = MATH_PI / (62.5d * 180.0d);
    break;
      
  case MPU20600_GYROFSR_1000:
    imu->gyroScale = MATH_PI / (32.8d * 180.0d);
    break;
   
  case MPU20600_GYROFSR_2000:
    imu->gyroScale = MATH_PI / (16.4d * 180.0d);
    break;
            
  default:
    return I2C_EINVAL;
  }

  imu->gyroFsr = fsr;
  return setGyroConfig(imu);
}

msg_t mpu20600_setAccelFsr( Mpu20600Data *imu, const uint8_t fsr)
{
   switch (fsr) {
    case MPU20600_ACCELFSR_2:
        imu->accelScale = 1.0d / 16384.0d;
	break;
    
    case MPU20600_ACCELFSR_4:
        imu->accelScale = 1.0d / 8192.0d;
	break;
	
    case MPU20600_ACCELFSR_8:
        imu->accelScale = 1.0d / 4096.0d;
	break;

    case MPU20600_ACCELFSR_16:
        imu->accelScale = 1.0d / 2048.0d;
	break;

    default:
        return I2C_EINVAL;
    }

   imu->accelFsr = fsr;
   return setAccelConfig(imu);
}



msg_t mpu20600_cacheVal ( Mpu20600Data *imu)
{
  msg_t status = MSG_OK;

  i2cAcquireBus(imu->i2cd);

  // if we get all 9 axes
  if (imu->accOnly == false) {
    I2C_READLEN_REGISTERS(imu->i2cd, imu->slaveAddr, MPU20600_REGISTER_BASE,
			  (uint8_t *) imu->fifo, 
			  imu->registerSegmentLen);
  } else { // if we just get 3 axes of accelerations
    I2C_READLEN_REGISTERS(imu->i2cd, imu->slaveAddr, MPU20600_REGISTER_BASE,
			  (uint8_t *) imu->fifo, 
			  6);
  }
  i2cReleaseBus(imu->i2cd);
  for (size_t i=0; i < 7; i++)
    imu->fifo[0].raw[i] = SWAP_ENDIAN16(imu->fifo[0].raw[i]);
  
  return MSG_OK; 
}

msg_t mpu20600_getVal ( Mpu20600Data *imu, float *temp,
		     ImuVec3f *gyro, ImuVec3f *acc)
{
  msg_t status = MSG_OK;

  if (!chSysIsCounterWithinX(chSysGetRealtimeCounterX(),
			     imu->cacheTimestamp, 
			     imu->cacheTimestamp + imu->sampleInterval)) {
    imu->cacheTimestamp = chSysGetRealtimeCounterX();
    status =  mpu20600_cacheVal(imu);
  }

  if (status != MSG_OK) 
    return status;
  imu->fifoIndex = 0;
  rawToSI(imu, acc, gyro, temp);
 
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
  I2C_WRITE_REGISTERS(imu->i2cd, imu->slaveAddr, MPU20600_ACCEL_CONFIG2, imu->accelLpf);
  i2cReleaseBus(imu->i2cd);
  
  return status;
}


static    msg_t setSampleRate( Mpu20600Data *imu)
{
  msg_t status = MSG_OK;

  i2cAcquireBus(imu->i2cd);
  if (imu->sampleRate > 1000) {
    I2C_WRITE_REGISTERS(imu->i2cd, imu->slaveAddr, MPU20600_SMPLRT_DIV, 0);
  } else {
    I2C_WRITE_REGISTERS(imu->i2cd, imu->slaveAddr, MPU20600_SMPLRT_DIV, 
			  ((uint8_t) ((1000U / imu->sampleRate) - 1U)));
  }
  
  i2cReleaseBus(imu->i2cd);
  return status;
}


/* static    msg_t setCompassRate( Mpu20600Data *imu) */
/* { */
/*   return MSG_OK; */
/* } */
static msg_t fifoReset(Mpu20600Data *imu)
{
  msg_t status = MSG_OK;
  // reset fifo operation
  I2C_WRITE_REGISTERS(imu->i2cd, imu->slaveAddr, MPU20600_USER_CTRL,
		      0b100);
  chThdSleepMilliseconds(1);
  
  // enable fifo operation
  I2C_WRITE_REGISTERS(imu->i2cd, imu->slaveAddr, MPU20600_USER_CTRL,
		      0b01000000);
  chThdSleepMilliseconds(1);
  return status;
}

static msg_t fifoFill(Mpu20600Data *imu, bool fill)
{
  msg_t status = MSG_OK;
  I2C_WRITE_REGISTERS(imu->i2cd, imu->slaveAddr, MPU20600_FIFO_EN,
		      fill ? 0b11 << 3 : 0);
  return status;
}

static msg_t fifoEnable( Mpu20600Data *imu) 
{
  msg_t status = MSG_OK;
  
  i2cAcquireBus(imu->i2cd);
  // fifo enabled for accelero and gyro and temperature which
  // cannot be disabled
  fifoFill(imu, true);

  fifoReset(imu);
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

/*
  read itr status : store  overflow condition in boolan hasOverflown
  read fifoCount
  // disable sampling (really needed ?)
  copy (fifoCount - fifoCount%14) bytes from 20600 to cache
  // enable sampling  (really needed ?)
  fifoIndex = 0; fifoLen = blockCount
  if overflow : reset fifo circuitry
 */

static msg_t readFifo(Mpu20600Data *imu, Mpu20600_Status *mpuStatus)
{
  msg_t status = MSG_OK;
  uint8_t itrStatus;
  uint16_t fifoCount = 0;
  imu->fifoIndex = imu->fifoLen = 0;
  i2cAcquireBus( imu->i2cd);
  I2C_READ_REGISTER(imu->i2cd, imu->slaveAddr, MPU20600_INT_STATUS, &itrStatus);
  const bool hasOverflown =  (itrStatus & MPU20600_INT_FIFO_OFLOW) != 0;
  if (hasOverflown) {
    *mpuStatus |=  MPU20600_FIFO_FULL;
  }
  I2C_READLEN_REGISTERS(imu->i2cd, imu->slaveAddr, MPU20600_FIFO_COUNT_H,
			(uint8_t *) &fifoCount, sizeof(fifoCount));
  fifoCount = SWAP_ENDIAN16(fifoCount);
  if ((fifoCount == 0) || (fifoCount > 1008)) {
     goto fifoResetAndExit;
  }
  // read by block of 14 bytes
  const uint16_t fifoNbBlocks = fifoCount / sizeof(Mpu20600FifoData);
  I2C_READLEN_REGISTERS(imu->i2cd, imu->slaveAddr, MPU20600_FIFO_R_W,
			(uint8_t *) imu->fifo, fifoNbBlocks * sizeof(Mpu20600FifoData));
  if (status == MSG_OK) {
    int16_t *samplesPtr = imu->fifo[0].raw;
    fifoCount = 0;
    // fix endianness of all values
    for (size_t sampleIdx = 0;
	 sampleIdx < fifoNbBlocks * sizeof(Mpu20600FifoData) / sizeof(int16_t);
	 sampleIdx++) {
      samplesPtr[sampleIdx] = SWAP_ENDIAN16(samplesPtr[sampleIdx]);
    }
    imu->fifoLen = fifoNbBlocks;
    if (hasOverflown) {
    fifoResetAndExit:
      fifoReset(imu);
    }
  } 
  
  i2cReleaseBus(imu->i2cd);
  return status;
}

static void rawToSI(const Mpu20600Data *imu, 
		    ImuVec3f *acc, ImuVec3f *gyro,
		    float *temp)
{
  const Mpu20600FifoData *fifoData = &imu->fifo[imu->fifoIndex];

  if (acc) 
    for (int i=0; i < 3; i++) 
      acc->arr[i] =  fifoData->acc[i] * imu->accelScale  * 9.81f;
  
  if (gyro) 
    for (int i=0; i < 3; i++) 
      gyro->arr[i] = fifoData->gyr[i] * imu->gyroScale;
  
  if (temp) 
    *temp = fifoData->temperature / 340.0f + 21.0f;
  
}


bool  mpu20600_popFifo(Mpu20600Data *imu, ImuVec3f *acc, ImuVec3f *gyro,
		       float *dt, Mpu20600_Status *mpuStatus)
{
  *dt = 1.0f / imu->sampleRate;
  // if we have poped all data from fifo cache, get from IMU
  if (imu->fifoIndex == imu->fifoLen) {
    const msg_t status = readFifo(imu, mpuStatus);
    if (status != MSG_OK) {
      *mpuStatus |= MPU20600_I2C_ERROR;
      fifoReset(imu);
    }
  }
  // transform from raw value to SI value for the current index
  rawToSI(imu, acc, gyro, nullptr);
  // return true if there is remaining data in fifo cache
  return (++imu->fifoIndex != imu->fifoLen);
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
