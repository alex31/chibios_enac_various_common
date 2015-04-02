#if I2C_USE_IMU9250

#include "i2cPeriphMpu9250.h"
#include <math.h>



/*
#                 __  __   _____    _    _              ___    ___    _____    ___          
#                |  \/  | |  __ \  | |  | |            / _ \  |__ \  | ____|  / _ \         
#                | \  / | | |__) | | |  | |           | (_) |    ) | | |__   | | | |        
#                | |\/| | |  ___/  | |  | |            \__, |   / /  |___ \  | | | |        
#                | |  | | | |      | |__| |  ______      / /   / /_   ___) | | |_| |        
#                |_|  |_| |_|       \____/  |______|    /_/   |____| |____/   \___/         
*/



static    msg_t setInitialConfig ( Mpu9250Data *imu);
static    msg_t setGyroConfig( Mpu9250Data *imu);
static    msg_t setAccelConfig( Mpu9250Data *imu);
static    msg_t setSampleRate( Mpu9250Data *imu);
//static    msg_t setCompassRate( Mpu9250Data *imu);
static    msg_t resetFifo( Mpu9250Data *imu);
static    msg_t setByPassConfig( Mpu9250Data *imu);
//static    msg_t compassSetup( Mpu9250Data *imu);

#define MATH_PI 3.14159265358979323846f


msg_t mpu9250_init( Mpu9250Data *imu, const Mpu9250Config* initParam)
{
  msg_t status;

  imu->i2cd =  initParam->i2cd;
  imu->slaveAddr = imu->useAd1 ? MPU9250_ADDRESS1 :  MPU9250_ADDRESS0;
  imu->registerSegmentLen = MPU9250_JUSTIMU_LAST -  MPU9250_REGISTER_BASE +1;
  imu->cacheTimestamp = halGetCounterValue();

  status = setInitialConfig(imu);
  
  if (status == RDY_OK) 
    status = mpu9250_setSampleRate (imu, initParam->sampleRate);
  
  if (status == RDY_OK)
    status = mpu9250_setGyroLpf (imu, initParam->gyroLpf);
  
  if (status == RDY_OK)
    status = mpu9250_setAccelLpf (imu, initParam->accelLpf);
  
  if (status == RDY_OK)
    status = mpu9250_setGyroFsr(imu, initParam->gyroFsr);
  
  if (status == RDY_OK)
    status = mpu9250_setAccelFsr(imu, initParam->accelFsr);
  
  if (status == RDY_OK)
    status = mpu9250_setBypass(imu, IMU_BYPASS);
  
  return status;
}

msg_t mpu9250_setGyroLpf( Mpu9250Data *imu, const uint8_t lpf)
{
  switch (lpf) {
  case MPU9250_GYRO_LPF_8800:
  case MPU9250_GYRO_LPF_3600:
  case MPU9250_GYRO_LPF_250:
  case MPU9250_GYRO_LPF_184:
  case MPU9250_GYRO_LPF_92:
  case MPU9250_GYRO_LPF_41:
  case MPU9250_GYRO_LPF_20:
  case MPU9250_GYRO_LPF_10:
  case MPU9250_GYRO_LPF_5:
    imu->gyroLpf = lpf;
    break;
    
  default:
    return I2C_EINVAL;
  }

  return setGyroConfig (imu);
}

msg_t mpu9250_setAccelLpf( Mpu9250Data *imu, const uint8_t lpf)
{
  switch (lpf) {
  case MPU9250_ACCEL_LPF_1130:
  case MPU9250_ACCEL_LPF_460:
  case MPU9250_ACCEL_LPF_184:
  case MPU9250_ACCEL_LPF_92:
  case MPU9250_ACCEL_LPF_41:
  case MPU9250_ACCEL_LPF_20:
  case MPU9250_ACCEL_LPF_10:
  case MPU9250_ACCEL_LPF_5:
    imu->accelLpf = lpf;
    break;
    
  default:
    return I2C_EINVAL;
  }

  return setAccelConfig (imu);
}

msg_t mpu9250_setSampleRate( Mpu9250Data *imu, const int32_t rate)
{
  if ((rate < MPU9250_SAMPLERATE_MIN) || (rate > MPU9250_SAMPLERATE_MAX)) {
    return I2C_EINVAL;
  }
  imu->sampleRate = rate;
  imu->sampleInterval = halGetCounterFrequency() / imu->sampleRate;
  if (imu->sampleInterval == 0)
    imu->sampleInterval = 1;
  return setSampleRate (imu);
}

msg_t mpu9250_setGyroFsr( Mpu9250Data *imu, const uint8_t fsr)
{
  switch (fsr) {
  case MPU9250_GYROFSR_250:
    imu->gyroFsr = fsr;
    imu->gyroScale = MATH_PI / (131.0d * 180.0d);
    break;

  case MPU9250_GYROFSR_500:
    imu->gyroFsr = fsr;
    imu->gyroScale = MATH_PI / (62.5d * 180.0d);
    break;
      
  case MPU9250_GYROFSR_1000:
    imu->gyroFsr = fsr;
    imu->gyroScale = MATH_PI / (32.8d * 180.0d);
    break;
   
  case MPU9250_GYROFSR_2000:
    imu->gyroFsr = fsr;
    imu->gyroScale = MATH_PI / (16.4d * 180.0d);
    break;
            
  default:
    return I2C_EINVAL;
  }


  return setGyroConfig (imu);
}

msg_t mpu9250_setAccelFsr( Mpu9250Data *imu, const uint8_t fsr)
{
   switch (fsr) {
    case MPU9250_ACCELFSR_2:
        imu->accelFsr = fsr;
        imu->accelScale = 1.0d/16384.0d;
	break;
    
    case MPU9250_ACCELFSR_4:
        imu->accelFsr = fsr;
        imu->accelScale = 1.0d/8192.0d;
	break;
	
    case MPU9250_ACCELFSR_8:
        imu->accelFsr = fsr;
        imu->accelScale = 1.0d/4096.0d;
	break;

    case MPU9250_ACCELFSR_16:
        imu->accelFsr = fsr;
        imu->accelScale = 1.0d/2048.0d;
	break;

    default:
        return I2C_EINVAL;
    }

   return setAccelConfig (imu);
}

msg_t mpu9250_setBypass( Mpu9250Data *imu, const PassThroughMode mode)  
{
  imu->byPass = mode;
  return setByPassConfig (imu);
}

msg_t mpu9250_cacheVal ( Mpu9250Data *imu)
{
  msg_t status = RDY_OK;

  
  i2cAcquireBus(imu->i2cd);
  I2C_READLEN_REGISTERS(imu->i2cd, imu->slaveAddr, MPU9250_REGISTER_BASE, imu->rawCache, 
		     imu->registerSegmentLen);
  i2cReleaseBus(imu->i2cd);
  
  return RDY_OK; 
}

msg_t mpu9250_getVal ( Mpu9250Data *imu, float *temp,
		     ImuVec3f *gyro, ImuVec3f *acc)
{
  msg_t status = RDY_OK;

  // alias 
  const uint8_t  *rawB =  imu->rawCache;

  if (!halIsCounterWithin(imu->cacheTimestamp, 
			  imu->cacheTimestamp + imu->sampleInterval)) {
    status =  mpu9250_cacheVal (imu);
    imu->cacheTimestamp = halGetCounterValue();
  }

  if (status != RDY_OK) 
    return status;

  *temp = ( ((int16_t) ((rawB[6]<<8) | rawB[7])) / 340.0f) + 21.0f;


  for (int i=0; i< 3; i++) {
    acc->arr[i] =  ((int16_t) ((rawB[i*2]<<8) | rawB[(i*2)+1])) * imu->accelScale;
  }
  
 for (int i=0; i< 3; i++) {
    gyro->arr[i] =  ((int16_t) ((rawB[(i*2)+8]<<8) | rawB[(i*2)+9])) * imu->gyroScale;
  }

  return status;
}

msg_t mpu9250_getDevid ( Mpu9250Data *imu, uint8_t *devid)
{ 
  msg_t status = RDY_OK;
  i2cAcquireBus(imu->i2cd);
  I2C_READ_REGISTER(imu->i2cd, imu->slaveAddr, MPU9250_WHO_AM_I, devid);
  i2cReleaseBus(imu->i2cd);
  if (*devid != MPU9250_ID)
    status = I2C_BADID;
  return status;
}


//msg_t mpu9250_setCompassRate( Mpu9250Data *imu, const int32_t rate);


static    msg_t setInitialConfig ( Mpu9250Data *imu)
{
  msg_t status = RDY_OK;
  i2cAcquireBus(imu->i2cd);

  I2C_WRITE_REGISTERS (imu->i2cd, imu->slaveAddr, MPU9250_PWR_MGMT_1, 0x80);
  chThdSleepMilliseconds (10);
  I2C_WRITE_REGISTERS (imu->i2cd, imu->slaveAddr, MPU9250_PWR_MGMT_1, 0x1);

  i2cReleaseBus(imu->i2cd);
  return status;
}


static    msg_t setByPassConfig ( Mpu9250Data *imu)
{
  msg_t status = RDY_OK;
  uint8_t userControl;
  i2cAcquireBus(imu->i2cd);
  
  I2C_READ_REGISTER  (imu->i2cd, imu->slaveAddr, MPU9250_USER_CTRL, &userControl);
  if (imu->byPass == IMU_BYPASS) {
    userControl &= ~0x20; // I2C_MST_EN = 0 : disable Master
    userControl |= 2; // Reset I2C Master Module
    I2C_WRITE_REGISTERS  (imu->i2cd, imu->slaveAddr, MPU9250_USER_CTRL, userControl);
    I2C_WRITE_REGISTERS  (imu->i2cd, imu->slaveAddr, MPU9250_INT_PIN_CFG, 0x2);
  } else {
    userControl |= 0x20; // enable Master
    I2C_WRITE_REGISTERS  (imu->i2cd, imu->slaveAddr, MPU9250_USER_CTRL, userControl);
    chThdSleepMilliseconds (50);
    I2C_WRITE_REGISTERS (imu->i2cd, imu->slaveAddr, MPU9250_INT_PIN_CFG, 0x0);
    //    I2C_WRITE_REGISTERS (imu->i2cd, imu->slaveAddr, MPU9250_I2C_MST_CTRL, 0x0d);
    //    I2C_WRITE_REGISTERS (imu->i2cd, imu->slaveAddr, MPU9250_I2C_MST_DELAY_CTRL, 0x0);
  }
  
  i2cReleaseBus(imu->i2cd);
  return status;
}


static    msg_t setGyroConfig ( Mpu9250Data *imu)
{
  msg_t status = RDY_OK;
  uint8_t gyroConfig = imu->gyroFsr + ((imu->gyroLpf >> 3) & 3);
  uint8_t gyroLpf = imu->gyroLpf & 7;
  
  I2C_WRITE_REGISTERS  (imu->i2cd, imu->slaveAddr, MPU9250_GYRO_CONFIG, gyroConfig);
  I2C_WRITE_REGISTERS  (imu->i2cd, imu->slaveAddr, MPU9250_GYRO_LPF, gyroLpf);

  return status;
}

static    msg_t setAccelConfig ( Mpu9250Data *imu)
{
  msg_t status = RDY_OK;

  I2C_WRITE_REGISTERS  (imu->i2cd, imu->slaveAddr, MPU9250_ACCEL_CONFIG, imu->accelFsr);
  I2C_WRITE_REGISTERS  (imu->i2cd, imu->slaveAddr, MPU9250_ACCEL_LPF, imu->accelLpf);

  return status;
}

static    msg_t setSampleRate ( Mpu9250Data *imu)
{
  msg_t status = RDY_OK;
  if (imu->sampleRate > 1000)
    return RDY_OK;                                        // SMPRT not used above 1000Hz
  
  I2C_WRITE_REGISTERS  (imu->i2cd, imu->slaveAddr, MPU9250_SMPRT_DIV, 
			((uint8_t) (1000 / imu->sampleRate - 1)));
  return status;
}


/* static    msg_t setCompassRate( Mpu9250Data *imu) */
/* { */
/*   return RDY_OK; */
/* } */

static __attribute__((__unused__)) msg_t resetFifo( Mpu9250Data *imu) 
{
  msg_t status = RDY_OK;

  I2C_WRITE_REGISTERS  (imu->i2cd, imu->slaveAddr, MPU9250_INT_ENABLE, 0);
  I2C_WRITE_REGISTERS  (imu->i2cd, imu->slaveAddr, MPU9250_FIFO_EN, 0);
  I2C_WRITE_REGISTERS  (imu->i2cd, imu->slaveAddr, MPU9250_USER_CTRL, 0);
  I2C_WRITE_REGISTERS  (imu->i2cd, imu->slaveAddr, MPU9250_USER_CTRL, 0x04);
  I2C_WRITE_REGISTERS  (imu->i2cd, imu->slaveAddr, MPU9250_USER_CTRL, 0x60);
  chThdSleepMilliseconds (50);
  I2C_WRITE_REGISTERS  (imu->i2cd, imu->slaveAddr, MPU9250_INT_ENABLE, 1);
  I2C_WRITE_REGISTERS  (imu->i2cd, imu->slaveAddr, MPU9250_FIFO_EN, 0x78);

  return status;
}

/* static    msg_t compassSetup( Mpu9250Data *imu) */
/* { */
/*   return RDY_OK; */
/* } */




/*
#                  ___    _  __             ___     ___      __    ____          
#                 / _ \  | |/ /            / _ \   / _ \    / /   |___ \         
#                | |_| | | ' /            | (_) | | (_) |  / /_     __) |        
#                |  _  | |  <              > _ <   \__, | | '_ \   |__ <         
#                | | | | | . \   ______   | (_) |    / /  | (_) |  ___) |        
#                |_| |_| |_|\_\ |______|   \___/    /_/    \___/  |____/         
*/

static msg_t setCompassCntl (Ak8963Data *compass);

msg_t ak8963_init (Ak8963Data *compass,  I2CDriver *i2cd)
{
  msg_t status = RDY_OK;
  uint8_t asa[3];
  compass->cacheTimestamp = halGetCounterValue();
  compass->byPass =  IMU_BYPASS;
  compass->i2cd = i2cd;
  i2cAcquireBus(compass->i2cd);
  I2C_WRITE_REGISTERS(compass->i2cd, AK8963_ADDRESS, AK8963_CNTL, AK8963_POWERDOWN);
  I2C_WRITE_REGISTERS(compass->i2cd, AK8963_ADDRESS, AK8963_CNTL, AK8963_FUSE_ROM);
  I2C_READ_REGISTERS(compass->i2cd, AK8963_ADDRESS, AK8963_ASAX, asa);
  i2cReleaseBus(compass->i2cd);

  compass->compassAdjust.x = ((float)asa[0] - 128.0f) / 256.0f + 1.0f;
  compass->compassAdjust.y = ((float)asa[1] - 128.0f) / 256.0f + 1.0f;
  compass->compassAdjust.z = ((float)asa[2] - 128.0f) / 256.0f + 1.0f;

  ak8963_setCompassCntl (compass, AK8963_CONTINUOUS_100HZ);
  return status;
}

msg_t ak8963_getDevid (Ak8963Data *compass, uint8_t *devid)
{
  msg_t status = RDY_OK;
  i2cAcquireBus(compass->i2cd);
  I2C_READ_REGISTER(compass->i2cd, AK8963_ADDRESS, AK8963_WHO_AM_I, devid);
  i2cReleaseBus(compass->i2cd);

  if (*devid != AK8963_ID)
    status = I2C_BADID;
  return status;
}

msg_t ak8963_setCompassCntl (Ak8963Data *compass, const uint8_t cntl)
{
  switch (cntl) {
  case AK8963_POWERDOWN : break;      
  case AK8963_SINGLE_MESURE  :break;    
  case AK8963_CONTINUOUS_8HZ :  
    compass->sampleInterval = halGetCounterFrequency() / 8;
    break;    
  case AK8963_CONTINUOUS_100HZ :
    compass->sampleInterval = halGetCounterFrequency() / 100;
    break ;
  default :
    return I2C_EINVAL;
  }

  compass->cntl1 = cntl|AK8963_16BITS;
  setCompassCntl (compass);
  return RDY_OK;
}

msg_t ak8963_cacheVal  (Ak8963Data *compass)
{
  msg_t status = RDY_OK;
  
  i2cAcquireBus(compass->i2cd);
  I2C_READLEN_REGISTERS(compass->i2cd, AK8963_ADDRESS, AK8963_REGISTER_BASE, 
			compass->rawCache, sizeof(compass->rawCache));
  i2cReleaseBus(compass->i2cd);
  
  return RDY_OK; 
}

msg_t ak8963_getVal  (Ak8963Data *compass, Ak8963Value *val)
{
  msg_t status = RDY_OK;

  if (!halIsCounterWithin(compass->cacheTimestamp, 
			  compass->cacheTimestamp + compass->sampleInterval)) {
    status =  ak8963_cacheVal (compass);
    if (status != RDY_OK)
      return status;
    compass->cacheTimestamp = halGetCounterValue();
  }
  const uint8_t  *rawB =  compass->rawCache;
  
  const uint8_t status1 = rawB[0];
  const int16_t magx = *((int16_t *) (&rawB[1]));
  const int16_t magy = *((int16_t *) (&rawB[3]));
  const int16_t magz = *((int16_t *) (&rawB[5]));
  const uint8_t status2 = rawB[7];

  val->mag.x = magx * compass->compassAdjust.x;
  val->mag.y = magy * compass->compassAdjust.y;
  val->mag.z = magz * compass->compassAdjust.y;

  val->dataReady = status1 & AK8963_ST1_DATAREADY;
  val->overrun = status1 & AK8963_ST1_OVERRUN;
  val->overflow = status2 & AK8963_ST2_OVERFLOW;
  
  return status;
}


static msg_t setCompassCntl (Ak8963Data *compass)
{
  msg_t status = RDY_OK;
  i2cAcquireBus(compass->i2cd);
  I2C_WRITE_REGISTERS(compass->i2cd, AK8963_ADDRESS, AK8963_CNTL, compass->cntl1);
  i2cReleaseBus(compass->i2cd);
  return status;
}





#endif
