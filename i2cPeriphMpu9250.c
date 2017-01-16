#if I2C_USE_IMU9250

#include "i2cPeriphMpu9250.h"
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



static    msg_t setInitialConfig (Mpu9250Data *imu);
static    msg_t setGyroConfig (Mpu9250Data *imu);
static    msg_t setAccelConfig (Mpu9250Data *imu);
static    msg_t setPowerConfig (Mpu9250Data *imu);
static    msg_t setBehaviorConfig (Mpu9250Data *imu);
static    msg_t setInterruptConfig (Mpu9250Data *imu);
static    msg_t setSampleRate( Mpu9250Data *imu);
//static    msg_t setCompassRate( Mpu9250Data *imu);
static    msg_t resetFifo( Mpu9250Data *imu);
static    msg_t setByPassConfig( Mpu9250Data *imu);
static    msg_t setMasterDelayDivider ( Mpu9250Data *imu);
static    msg_t addSlave (Mpu9250Data *imu, Mpu9250MasterConfig_0_to_3 *mc);


#define MATH_PI 3.14159265358979323846f


msg_t mpu9250_init( Mpu9250Data *imu, const Mpu9250Config* initParam)
{
  msg_t status;

  imu->i2cd =  initParam->i2cd;
  
  imu->slaveAddr = initParam->useAd0 ? MPU9250_ADDRESS1 :  MPU9250_ADDRESS0;
  imu->registerSegmentLen = MPU9250_JUSTIMU_LAST -  MPU9250_REGISTER_BASE +1;
  imu->cacheTimestamp = halGetCounterValue();
  memset (&(imu->mc), 0, sizeof(imu->mc));
  imu->nextSlvFreeSlot = 0;

  status = setInitialConfig(imu);

  /* if (status == RDY_OK) */
  /*   status = resetFifo (imu); */
  
  if (status == RDY_OK) 
    status = mpu9250_setSampleRate (imu, initParam->sampleRate);

 if (status == RDY_OK) 
    status = mpu9250_setAuxSampleRate (imu, initParam->auxSampleRate);
  
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

  if (status == RDY_OK)
    status = mpu9250_setPwr(imu, initParam->pwrMode);

  if (status == RDY_OK)
    status = mpu9250_setBehavior(imu, initParam->behaviorMode);

  if (status == RDY_OK)
    status = mpu9250_setInterrupt(imu, initParam->itrMode);
  
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

msg_t mpu9250_setSampleRate( Mpu9250Data *imu, const uint32_t rate)
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
    imu->gyroScale = MATH_PI / (131.0f * 180.0f);
    break;

  case MPU9250_GYROFSR_500:
    imu->gyroFsr = fsr;
    imu->gyroScale = MATH_PI / (62.5f * 180.0f);
    break;
      
  case MPU9250_GYROFSR_1000:
    imu->gyroFsr = fsr;
    imu->gyroScale = MATH_PI / (32.8f * 180.0f);
    break;
   
  case MPU9250_GYROFSR_2000:
    imu->gyroFsr = fsr;
    imu->gyroScale = MATH_PI / (16.4f * 180.0f);
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

msg_t mpu9250_setAuxSampleRate (Mpu9250Data *imu, const uint32_t rate)
{
  imu->auxSampleRate = rate;
  
  return setMasterDelayDivider (imu);
}



msg_t mpu9250_setBypass( Mpu9250Data *imu, const Mpu9250_PassThroughMode mode)  
{
  imu->byPass = mode;
  return setByPassConfig (imu);
}



msg_t mpu9250_setPwr (Mpu9250Data *imu, const Mpu9250_PowerMode pMode)
{
  if ((pMode & MPU9250_ACC_ENABLED) && (pMode & MPU9250_ACC_DISABLED)) {
    DebugTrace ("cannot have  MPU9250_ACC enabled AND disabled");
    return I2C_EINVAL;
  }
  if (!((pMode & MPU9250_ACC_ENABLED) || (pMode & MPU9250_ACC_DISABLED))) {
    DebugTrace ("cannot have  MPU9250_ACC neither enabled nor disabled");
    return I2C_EINVAL;
  }

  if ((pMode & MPU9250_GYRO_ENABLED) && (pMode & MPU9250_GYRO_DISABLED)) {
    DebugTrace ("cannot have  MPU9250_GYRO enabled AND disabled");
    return I2C_EINVAL;
  }
  if (!((pMode & MPU9250_GYRO_ENABLED) || (pMode & MPU9250_GYRO_DISABLED))) {
    DebugTrace ("cannot have  MPU9250_GYRO neither enabled nor disabled");
    return I2C_EINVAL;
  }


  const Mpu9250_PowerMode pwrBitmask = pMode & (MPU9250_POWERMAX | MPU9250_POWERLOW |
						MPU9250_SLEEP);

  if (__builtin_popcount(pwrBitmask) > 1) {
    DebugTrace ("only one of MPU9250_POWERMAX, MPU9250_POWERLOW, MPU9250_SLEEP can be selected");
    return I2C_EINVAL;
  } else if (__builtin_popcount(pwrBitmask) == 0) {
    DebugTrace ("one of MPU9250_POWERMAX, MPU9250_POWERLOW, MPU9250_SLEEP should be selected");
    return I2C_EINVAL;
  }

  
  imu->pwrMode = pMode;

  return setPowerConfig (imu);
}

msg_t mpu9250_setBehavior (Mpu9250Data *imu, const Mpu9250_BehaviourMode bMode)
{
  switch (bMode) {
  case MPU9250_MODE_I2C:
  case MPU9250_MODE_MOTION_DETECT: break;
  default: 
    DebugTrace ("incorrect value (correct are MPU9250_MODE_I2C, MPU9250_MODE_MOTION_DETECT");
    return I2C_EINVAL;
  }
  
  imu->behaviorMode = bMode;
  return setBehaviorConfig (imu);
}

/*
  msg_t status =  i2cMasterWriteBit (imu->i2cd, imu->slaveAddr,  const uint8_t regAdr,
				     const uint8_t mask, const bool enable);
*/
msg_t mpu9250_setInterrupt (Mpu9250Data *imu, const Mpu9250_ItrMode itrMode)
{
  switch (itrMode) {
  case MPU9250_NO_ITR:
  case MPU9250_ITR_ON_MOTION:
  case MPU9250_ITR_ON_DATA_READY: break;
  default:
    DebugTrace ("incorrect value (correct are MPU9250_NO_ITR, MPU9250_ITR_ON_MOTION, MPU9250_ITR_ON_DATA_READY");
    return I2C_EINVAL;
  }

  imu->itrMode = itrMode;
  return setInterruptConfig (imu);
}

static    msg_t setPowerConfig (Mpu9250Data *imu)
{
  uint8_t pwrmgmt2 = 0U;
  msg_t status;
  if (imu->pwrMode & MPU9250_ACC_DISABLED)
    pwrmgmt2 = MPU9250_PWRM2_DISABLE_ACC;
  if (imu->pwrMode & MPU9250_GYRO_DISABLED)
    pwrmgmt2 |= MPU9250_PWRM2_DISABLE_GYRO;

  i2cAcquireBus(imu->i2cd);
  I2C_WRITE_REGISTERS  (imu->i2cd, imu->slaveAddr, MPU9250_PWR_MGMT_2, pwrmgmt2);
  if (imu->pwrMode & MPU9250_GYRO_DISABLED) {
    i2cMasterWriteBit (imu->i2cd, imu->slaveAddr,  MPU9250_PWR_MGMT_1,  MPU9250_PWRM1_GYROSTANDBY, true);
  }

  if (imu->pwrMode & MPU9250_POWERMAX) {
    i2cMasterWriteBit (imu->i2cd, imu->slaveAddr,  MPU9250_PWR_MGMT_1,  MPU9250_PWRM1_SLEEP, false);
  } else if (imu->pwrMode &  MPU9250_POWERLOW) {
    
  } else if (imu->pwrMode & MPU9250_SLEEP) {
  } 
  i2cReleaseBus(imu->i2cd);
  return status;
}

static    msg_t setBehaviorConfig (Mpu9250Data *imu)
{
  (void) imu;
#warning "do real behaviour config";
  return MSG_OK;
}

static    msg_t setInterruptConfig (Mpu9250Data *imu)
{
  (void) imu;
#warning "do real interrupt config";
  return MSG_OK;
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
    imu->cacheTimestamp = halGetCounterValue();
    status =  mpu9250_cacheVal (imu);
  }

  if (status != RDY_OK) 
    return status;

  // C°
  *temp = ( ((int16_t) ((rawB[6]<<8) | rawB[7])) / 340.0f) + 21.0f;

  // m/s²
  for (int i=0; i< 3; i++) {
    acc->arr[i] =  ((int16_t) ((rawB[i*2]<<8) | rawB[(i*2)+1])) * imu->accelScale  * 9.81f;
  }
  
  // rad/s
 for (int i=0; i< 3; i++) {
    gyro->arr[i] =  ((int16_t) ((rawB[(i*2)+8]<<8) | rawB[(i*2)+9])) * imu->gyroScale;
  }


 
  return status;
}

msg_t mpu9250_getDevid (Mpu9250Data *imu, uint8_t *devid)
{ 
  msg_t status = RDY_OK;
  i2cAcquireBus(imu->i2cd);
  I2C_READ_REGISTER(imu->i2cd, imu->slaveAddr, MPU9250_WHO_AM_I, devid);
  i2cReleaseBus(imu->i2cd);
  if (*devid != MPU9250_ID)
    status = I2C_BADID;
  return status;
}


msg_t mpu9250AddSlv_Ak8963 (Mpu9250Data *imu, Ak8963Data *compass)
{
  msg_t status = RDY_OK;
  
  if (imu->nextSlvFreeSlot > 3) {
    return I2C_MAXSLV_REACH;
  }
  
  uint32_t sumOfLen=0;
  for (uint8_t i=0; i<imu->nextSlvFreeSlot; i++) {
    Mpu9250MasterConfig_0_to_3 *mc03 = &(imu->mc.mc03[0]);
    sumOfLen += mc03->mapLen;
  }

  //DebugTrace ("slot = %d; sumOfLen = %d", imu->nextSlvFreeSlot, sumOfLen);

  Mpu9250MasterConfig_0_to_3 *mc03 = &(imu->mc.mc03[imu->nextSlvFreeSlot]);
  compass->mstConfig = mc03;
  mc03->mpu = imu;
  mc03->cacheAdr = &(imu->rawCache[(sumOfLen+MPU9250_EXT_SENS_DATA_00)-MPU9250_REGISTER_BASE]);
  mc03->slvI2cAdr = AK8963_ADDRESS;
  mc03->slvRegStart = AK8963_REGISTER_BASE;
  mc03->mapLen = (AK8963_REGISTER_LAST+1)-AK8963_REGISTER_BASE;
  imu->registerSegmentLen =  (uint8_t) (imu->registerSegmentLen + mc03->mapLen);
  mc03->way = IMU_TRANSFER_READ;
  mc03->swapMode = IMU_NO_SWAP;
  mc03->useMstDlyPrev = true;


  status = addSlave (imu, mc03);
  if (status == RDY_OK) {
    imu->nextSlvFreeSlot++;
  }
  
  return status;		      
}

#if I2C_USE_MPL3115A2
msg_t mpu9250AddSlv_MPL3115A2 (Mpu9250Data *imu, MPL3115A2Data *baro) 
{
  msg_t status = RDY_OK;
  
  if (imu->nextSlvFreeSlot > 3) {
    return I2C_MAXSLV_REACH;
  }
  
  uint32_t sumOfLen=0;
  for (uint8_t i=0; i<imu->nextSlvFreeSlot; i++) {
    Mpu9250MasterConfig_0_to_3 *mc03 = &(imu->mc.mc03[0]);
    sumOfLen += mc03->mapLen;
  }

  //DebugTrace ("slot = %d; sumOfLen = %d", imu->nextSlvFreeSlot, sumOfLen);

  Mpu9250MasterConfig_0_to_3 *mc03 = &(imu->mc.mc03[imu->nextSlvFreeSlot]);
  baro->mstConfig = mc03;
  mc03->mpu = imu;
  mc03->cacheAdr = &(imu->rawCache[(sumOfLen+MPU9250_EXT_SENS_DATA_00)-MPU9250_REGISTER_BASE]);
  mc03->slvI2cAdr =  MPL3115A2_ADDRESS;
  mc03->slvRegStart = MPL3115A2_OUT_P_MSB;
  mc03->mapLen = (MPL3115A2_OUT_P_LSB+1)-MPL3115A2_OUT_P_MSB;
  imu->registerSegmentLen = (uint8_t) (imu->registerSegmentLen + mc03->mapLen);
  mc03->way = IMU_TRANSFER_READ;
  mc03->swapMode = IMU_NO_SWAP;
  mc03->useMstDlyPrev = true;

  status = addSlave (imu, mc03);
  if (status != RDY_OK) 
    return status;

  imu->nextSlvFreeSlot++;
 
  mc03 = &(imu->mc.mc03[imu->nextSlvFreeSlot]);
  mc03->mpu = imu;
  mc03->slvI2cAdr =  MPL3115A2_ADDRESS;
  mc03->slvRegStart = MPL3115A2_CTRL_REG1;
  mc03->slvDo = MPL3115A2_ONESHOT_INIT | baro->oversampling;
  mc03->mapLen = 1;
  mc03->way = IMU_TRANSFER_WRITE;
  mc03->swapMode = IMU_NO_SWAP;
  mc03->useMstDlyPrev = true; 

  status = addSlave (imu, mc03);
  if (status == RDY_OK) {
    imu->nextSlvFreeSlot++;
  }
  
  return status;		      
}
#endif
static  msg_t addSlave (Mpu9250Data *imu, Mpu9250MasterConfig_0_to_3 *mc)
{
  msg_t status = RDY_OK;

  
  const uint8_t slvRegistersOffset = (uint8_t) ((MPU9250_I2C_SLV1_ADDR - MPU9250_I2C_SLV0_ADDR) *
						imu->nextSlvFreeSlot);

  const uint8_t i2cSlvAddr =  (uint8_t) (MPU9250_I2C_SLV0_ADDR + slvRegistersOffset);
  const uint8_t i2cSlvReg = (uint8_t) (MPU9250_I2C_SLV0_REG + slvRegistersOffset);
  const uint8_t i2cSlvCtrl = (uint8_t) (MPU9250_I2C_SLV0_CTRL + slvRegistersOffset);

  const uint8_t i2cSlvAddrVal = (uint8_t) ((mc->slvI2cAdr & I2C_ID_MSK) | mc->way);
  const uint8_t i2cSlvRegVal =  (uint8_t) (mc->slvRegStart);
  const uint8_t i2cSlvCtrlVal =  (uint8_t) (I2C_SLV_EN | mc->swapMode | (mc->mapLen & I2C_SLV_LENG_MSK));

  uint8_t i2cMasterDelayControl;

  if (mc->mapLen >  I2C_SLV_LENG_MSK)
    return I2C_EINVAL;
  
  i2cAcquireBus(imu->i2cd);
  if (mc->useMstDlyPrev == true) {
    I2C_READ_REGISTER  (imu->i2cd, imu->slaveAddr, MPU9250_I2C_MST_DELAY_CTRL, &i2cMasterDelayControl);
    i2cMasterDelayControl = (uint8_t) (i2cMasterDelayControl | (1U << imu->nextSlvFreeSlot));
  }
  I2C_WRITE_REGISTERS  (imu->i2cd, imu->slaveAddr, 
			(uint8_t) (MPU9250_I2C_SLV0_DO + imu->nextSlvFreeSlot),
			mc->slvDo);
  I2C_WRITE_REGISTERS  (imu->i2cd, imu->slaveAddr, MPU9250_I2C_MST_DELAY_CTRL, i2cMasterDelayControl);
  I2C_WRITE_REGISTERS (imu->i2cd, imu->slaveAddr, i2cSlvAddr, i2cSlvAddrVal);
  I2C_WRITE_REGISTERS (imu->i2cd, imu->slaveAddr, i2cSlvReg, i2cSlvRegVal);
  I2C_WRITE_REGISTERS (imu->i2cd, imu->slaveAddr, i2cSlvCtrl, i2cSlvCtrlVal);
  i2cReleaseBus(imu->i2cd);

  return status;
}




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
    userControl &= ~0x20U;    // I2C_MST_EN = 0 : disable Master
    userControl |=  0x02U;    // Reset I2C Master Module
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
  uint8_t gyroConfig = (uint8_t) (imu->gyroFsr + ((imu->gyroLpf >> 3U) & 3U));
  uint8_t gyroLpf = (uint8_t) (imu->gyroLpf & 7U);
  
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

static    msg_t setMasterDelayDivider ( Mpu9250Data *imu)
{
  msg_t status = RDY_OK;
  uint8_t delay = (uint8_t) ((imu->sampleRate / imu->auxSampleRate) -1U);
  delay = MIN (delay  , 31);


  uint8_t slv4Ctrl;
  I2C_READ_REGISTER  (imu->i2cd, imu->slaveAddr, MPU9250_I2C_SLV4_CTRL, &slv4Ctrl);
  slv4Ctrl |= delay;
  I2C_WRITE_REGISTERS  (imu->i2cd, imu->slaveAddr, MPU9250_I2C_SLV4_CTRL, slv4Ctrl);

  return status;
}

static    msg_t setSampleRate ( Mpu9250Data *imu)
{
  msg_t status = RDY_OK;
  if (imu->sampleRate > 1000) {
    I2C_WRITE_REGISTERS  (imu->i2cd, imu->slaveAddr, MPU9250_SMPRT_DIV, 0);
  } else {
    I2C_WRITE_REGISTERS  (imu->i2cd, imu->slaveAddr, MPU9250_SMPRT_DIV, 
			  ((uint8_t) ((1000 / imu->sampleRate) - 1)));
  }
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
  static uint8_t IN_DMA_SECTION (asa[3]);
  compass->cacheTimestamp = halGetCounterValue();
  compass->i2cd = i2cd;
  compass->mstConfig = NULL;

  i2cAcquireBus(compass->i2cd);
  I2C_WRITE_REGISTERS(compass->i2cd, AK8963_ADDRESS, AK8963_CNTL1, AK8963_POWERDOWN);
  I2C_WRITE_REGISTERS(compass->i2cd, AK8963_ADDRESS, AK8963_CNTL1, AK8963_FUSE_ROM);
  I2C_READ_REGISTERS(compass->i2cd, AK8963_ADDRESS, AK8963_ASAX, asa);
  i2cReleaseBus(compass->i2cd);

  compass->compassAdjust.x = ((float)asa[0] - 128.0f) / 256.0f + 1.0f;
  compass->compassAdjust.y = ((float)asa[1] - 128.0f) / 256.0f + 1.0f;
  compass->compassAdjust.z = ((float)asa[2] - 128.0f) / 256.0f + 1.0f;

  /* DebugTrace ("adjust: x= %.2f, y=%.2f, z=%.2f", */
  /* 	      compass->compassAdjust.x, */
  /* 	      compass->compassAdjust.y, */
  /* 	      compass->compassAdjust.z); */

  ak8963_setCompassCntl (compass, AK8963_CONTINUOUS_100HZ);
  return status;
}

msg_t ak8963_getDevid (Ak8963Data *compass, uint8_t *devid)
{
  msg_t status = RDY_OK;
  static IN_DMA_SECTION (uint8_t did);

  
  i2cAcquireBus(compass->i2cd);
  I2C_READ_REGISTER(compass->i2cd, AK8963_ADDRESS, AK8963_WHO_AM_I, &did);
  i2cReleaseBus(compass->i2cd);

  *devid = did;
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
  

  if (compass->mstConfig == NULL) {
    i2cAcquireBus(compass->i2cd);
    I2C_READLEN_REGISTERS(compass->i2cd, AK8963_ADDRESS, AK8963_REGISTER_BASE, 
			  compass->rawCache, sizeof(compass->rawCache));
    i2cReleaseBus(compass->i2cd);
  } else {
    status = mpu9250_cacheVal (compass->mstConfig->mpu);
  }  

  return status; 
}

msg_t ak8963_getVal  (Ak8963Data *compass, Ak8963Value *val)
{
  msg_t status = RDY_OK;

  if (!halIsCounterWithin(compass->cacheTimestamp, 
			  compass->cacheTimestamp + compass->sampleInterval)) {
    compass->cacheTimestamp = halGetCounterValue();
    status =  ak8963_cacheVal (compass);
    if (status != RDY_OK) {
      return status;
    }
  }
  const uint8_t  *rawB =   (compass->mstConfig == NULL) ? compass->rawCache :
    compass->mstConfig->cacheAdr;
  
  const uint8_t status1 = rawB[0];
  const int16_t magx = *((int16_t *) (rawB+1));
  const int16_t magy = *((int16_t *) (rawB+3));
  const int16_t magz = *((int16_t *) (rawB+5));
  const uint8_t status2 = rawB[7];
  
  // align axis same way than the imu part of 9250 (PS-MPU-9250A-01.pdf page 38)
  val->mag.x = magy * compass->compassAdjust.y;
  val->mag.y = magx * compass->compassAdjust.x;
  val->mag.z = -(magz * compass->compassAdjust.z);

  val->dataReady = status1 & AK8963_ST1_DATAREADY;
  val->overrun = status1 & AK8963_ST1_OVERRUN;
  val->overflow = status2 & AK8963_ST2_OVERFLOW;
  
  return status;
}


static msg_t setCompassCntl (Ak8963Data *compass)
{
  msg_t status = RDY_OK;
  i2cAcquireBus(compass->i2cd);
  I2C_WRITE_REGISTERS(compass->i2cd, AK8963_ADDRESS, AK8963_CNTL1, compass->cntl1);
  i2cReleaseBus(compass->i2cd);
  return status;
}





#endif
