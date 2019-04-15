#include "spiMasterUtil.h"
#include "spiPeriphICM20600.h"
#include <math.h>
#include <string.h>

// to use DebugTrace
#include "globalVar.h"
#include "stdutil.h"

/*
#                 _____    _____   __  __             ___     ___      __     ___     ___   
#                |_   _|  / ____| |  \/  |           |__ \   / _ \    / /    / _ \   / _ \  
#                  | |   | |      | \  / |              ) | | | | |  / /_   | | | | | | | | 
#                  | |   | |      | |\/| |             / /  | | | | | '_ \  | | | | | | | | 
#                 _| |_  | |____  | |  | |  ______    / /_  | |_| | | (_) | | |_| | | |_| | 
#                |_____|  \_____| |_|  |_| |______|  |____|  \___/   \___/   \___/   \___/  
*/

static    msg_t setInitialConfig(Icm20600Data *imu);
static    void setGyroConfig(Icm20600Data *imu);
static    void setAccelConfig(Icm20600Data *imu);
static    void setSampleRate( Icm20600Data *imu);

#define MATH_PI 3.14159265358979323846f


msg_t icm20600_init(Icm20600Data *imu, Icm20600Config* config)
{
  msg_t status;

  imu->config = config;
  
  imu->registerSegmentLen = ICM20600_GYRO_ZOUT_L - ICM20600_ACCEL_XOUT_H + 1;
  imu->cacheTimestamp = chSysGetRealtimeCounterX();
  imu->accOnly = false;

  status = setInitialConfig(imu);

  if (status == MSG_OK)  {
    icm20600_setSampleRate(imu, imu->config->sampleRate);
    icm20600_setGyroLpf(imu, imu->config->config);
    icm20600_setAccelLpf(imu, imu->config->accelConf2);
    icm20600_setGyroFsr(imu, imu->config->gyroConfig);
    icm20600_setAccelFsr(imu, imu->config->accelConf);
  }
  
  return status;
}


void icm20600_setGyroLpf( Icm20600Data *imu, const uint8_t lpf)
{
  imu->config->config |= (lpf & 0x7);
  setGyroConfig(imu);
}

void icm20600_setAccelLpf( Icm20600Data *imu, const uint8_t lpf)
{
  imu->config->accelConf2 |= (lpf & 0xf);
  return setAccelConfig(imu);
}


void icm20600_setSampleRate( Icm20600Data *imu, const uint32_t rate)
{
  if ((rate < ICM20600_SAMPLERATE_MIN) || (rate > ICM20600_SAMPLERATE_MAX)) {
    chSysHalt("rate not in range");
  }
  
  imu->sampleInterval = STM32_SYSCLK / rate;
  if (imu->sampleInterval == 0)
    imu->sampleInterval = 1;
  setSampleRate(imu);
}


void icm20600_setGyroFsr( Icm20600Data *imu, const Icm20600_gyroConf fsr)
{
  const Icm20600_gyroConf fsrMask = fsr & ICM20600_RANGE_GYRO_MASK;

  switch (fsrMask) {
  case ICM20600_RANGE_250_DPS:
    imu->gyroScale = MATH_PI / ((1<<17) * 0.180f);
    break;
    
  case ICM20600_RANGE_500_DPS:
    imu->gyroScale = MATH_PI / ((1<<16) * 0.180f);
    break;
    
  case ICM20600_RANGE_1K_DPS:
    imu->gyroScale = MATH_PI / ((1<<15) * 0.180f);
    break;
    
  case ICM20600_RANGE_2K_DPS:
    imu->gyroScale = MATH_PI / ((1<<14) * 0.180f);
    break;

  default:
    chSysHalt("full scale range not in range");
   }
  
  imu->config->gyroConfig = fsr;
  return setGyroConfig(imu);
}



void icm20600_setAccelFsr( Icm20600Data *imu, const Icm20600_accelConf fsr)
{
  const Icm20600_accelConf fsrMask = fsr & ICM20600_RANGE_ACCEL_MASK;
  
   switch (fsrMask) {
    case ICM20600_RANGE_2G:
        imu->accelScale = 1.0d/16384.0d;
	break;
    
    case ICM20600_RANGE_4G:
        imu->accelScale = 1.0d/8192.0d;
	break;
	
    case ICM20600_RANGE_8G:
        imu->accelScale = 1.0d/4096.0d;
	break;

    case ICM20600_RANGE_16G:
        imu->accelScale = 1.0d/2048.0d;
	break;

    default:
      chSysHalt("full scale range not in range");
    }

   imu->config->accelConf = fsr;
   return setAccelConfig(imu);
}

void icm20600_fetch( Icm20600Data *imu)
{
  SPIDriver * const spid = imu->config->spid;
  spiAcquireBus(spid);

  // just accel or accel + temp + gyro
  spiReadRegisters(spid, ICM20600_ACCEL_XOUT_H, imu->rawCache,
		   imu->accOnly ? ICM20600_ACCEL_ZOUT_L - ICM20600_ACCEL_XOUT_H + 1
		   : ICM20600_GYRO_ZOUT_L - ICM20600_ACCEL_XOUT_H + 1);
  spiReleaseBus(spid);
}

void icm20600_getVal( Icm20600Data *imu, float *temp,
		     Vec3f *gyro, Vec3f *acc)
{
  // alias 
  const uint8_t  *rawB =  imu->rawCache;

  if (!chSysIsCounterWithinX(chSysGetRealtimeCounterX(),
			     imu->cacheTimestamp, 
			     imu->cacheTimestamp + imu->sampleInterval)) {
    imu->cacheTimestamp = chSysGetRealtimeCounterX();
    icm20600_fetch(imu);
  }

  // m/s²
  for (int i=0; i< 3; i++) {
    acc->v[i] =  ((int16_t) ((rawB[i*2]<<8) | rawB[(i*2)+1])) * imu->accelScale  * 9.81f;
  }

  // if we are in low power mode acceleration sensor only, don't calculate other values
  if (imu->accOnly) {
    return;
  }
  
  // C°
  *temp = ( ((int16_t) ((rawB[6]<<8) | rawB[7])) / 340.0f) + 21.0f;
  
  // rad/s
  for (int i=0; i< 3; i++) {
    gyro->v[i] =  ((int16_t) ((rawB[(i*2)+8]<<8) | rawB[(i*2)+9])) * imu->gyroScale;
  }
}


uint8_t icm20600_getDevid(Icm20600Data *imu)
{
  SPIDriver * const spid = imu->config->spid;
  
  spiAcquireBus(spid);
  const uint8_t whoAmI = spiReadOneRegister(spid, ICM20600_WHO_AM_I);
  spiReleaseBus(spid);
  return whoAmI;
}



static    msg_t setInitialConfig( Icm20600Data *imu)
{
  SPIDriver * const spid = imu->config->spid;
  
  spiAcquireBus(spid);

  SPI_WRITE_REGISTERS(spid, ICM20600_PWR_MGMT_1, ICM20600_RESET);
  chThdSleepMilliseconds(2);
  SPI_WRITE_REGISTERS(spid, ICM20600_PWR_MGMT_1, ICM20600_CLKSEL_AUTO);
  chThdSleepMilliseconds(2);
  SPI_WRITE_REGISTERS(spid, ICM20600_I2C_IF, ICM20600_SPI_MODE);
  chThdSleepMilliseconds(2);
 
  const uint8_t whoAmI = spiReadOneRegister(spid, ICM20600_WHO_AM_I);

  spiReleaseBus(spid);
  return (whoAmI == ICM20600_WHO_AM_I_CONTENT) ? MSG_OK : MSG_RESET;
}


static    void setGyroConfig( Icm20600Data *imu)
{
  SPIDriver * const spid = imu->config->spid;
  
  spiAcquireBus(spid);
  SPI_WRITE_REGISTERS(spid, ICM20600_GYRO_CONFIG, imu->config->gyroConfig);
  SPI_WRITE_REGISTERS(spid, ICM20600_CONFIG, imu->config->config);
  spiReleaseBus(spid);
}

static    void setAccelConfig( Icm20600Data *imu)
{
  SPIDriver * const spid = imu->config->spid;

  spiAcquireBus(spid);
  SPI_WRITE_REGISTERS (spid, ICM20600_ACCEL_CONFIG, imu->config->accelConf);
  SPI_WRITE_REGISTERS (spid, ICM20600_ACCEL_CONFIG2, imu->config->accelConf2);
  spiReleaseBus(spid);
}



static    void setSampleRate( Icm20600Data *imu)
{
  SPIDriver * const spid = imu->config->spid;

  spiAcquireBus(spid);
  if (imu->config->sampleRate > 1000) {
    imu->config->gyroConfig = spiReadOneRegister(spid, ICM20600_GYRO_CONFIG);
    imu->config->gyroConfig &= ~0b11;
    
    if (imu->config->sampleRate > 3200)
      imu->config->gyroConfig |= ICM20600_FCHOICE_RATE_32K_BW_8173;
    else
      imu->config->gyroConfig |= ICM20600_FCHOICE_RATE_32K_BW_3281;
    
    SPI_WRITE_REGISTERS(spid, ICM20600_GYRO_CONFIG, imu->config->gyroConfig);
    SPI_WRITE_REGISTERS(spid, ICM20600_SMPLRT_DIV, 0);
  } else { // sampleRate <= 1000
    SPI_WRITE_REGISTERS(spid, ICM20600_SMPLRT_DIV, 
			((uint8_t) ((1000 / imu->config->sampleRate) - 1)));
  }
  
  spiReleaseBus(spid);
}


 Icm20600_interruptStatus icm20600_getItrStatus (Icm20600Data *imu)
{
  SPIDriver * const spid = imu->config->spid;
  spiAcquireBus( spid);
  const  Icm20600_interruptStatus retVal = spiReadOneRegister(spid, ICM20600_INT_STATUS);
  spiReleaseBus(spid);
  return retVal;
}


void icm20600_setModeAccOnly(Icm20600Data *imu)
{
  SPIDriver * const spid = imu->config->spid;
  imu->accOnly = true;
  
  spiAcquireBus(spid); 
  // disable gyro sensors
  SPI_WRITE_REGISTERS(spid, ICM20600_PWR_MGMT_2, ICM20600_DISABLE_GYRO);
  
  // put gyro circuitry in standby
  Icm20600_powerMgmt1 powerMgmt1 = spiReadOneRegister(spid, ICM20600_PWR_MGMT_1);
  powerMgmt1 |= ICM20600_GYRO_STANDBY;
  SPI_WRITE_REGISTERS(spid, ICM20600_PWR_MGMT_1, powerMgmt1);
  spiReleaseBus(spid); 
}


void icm20600_setModeAutoWake(Icm20600Data *imu, const uint16_t frequency)
{
  SPIDriver * const spid = imu->config->spid;
  spiAcquireBus(spid); 
  // set frequency for waking mpu
  const uint8_t smplrtdiv = MAX(1, MIN(256, 1000 / frequency)) -1;
  SPI_WRITE_REGISTERS(spid, ICM20600_SMPLRT_DIV, smplrtdiv);
  
  // set cycle bit to 1
  SPI_WRITE_REGISTERS(spid, ICM20600_PWR_MGMT_1,
		      ICM20600_CLKSEL_AUTO | ICM20600_CYCLE_ENABLE);
  spiReleaseBus(spid); 
}



void icm20600_activateMotionDetect(Icm20600Data *imu,
				    const uint32_t threadsholdInMilliG,
				    const Icm20600_pinControl pinConfigMask)
{
  chSysHalt("not yet implemented, see invensense ICM20600 datasheet page 57"); 
  (void) imu;
  (void) threadsholdInMilliG;
  (void) pinConfigMask;

  
  /* SPIDriver * const spid = imu->config->spid; */
  /* const uint8_t threadshold = MIN((threadsholdInMilliG/4), 255U); */
  
  /* spiAcquireBus(spid); */
  /* (void) pinConfigMask; */
  
  /* // enable wake on motion detection logic */
  /* SPI_WRITE_REGISTERS(spid,  ICM20600_ACCEL_INTEL_CTRL, */
  /* 		      ICM20600_WAKE_ON_MOTION_ENABLE | ICM20600_ONE_AXIS_REACHES_THRESHOLD); */
  
  /* SPI_WRITE_REGISTERS  (spid, ICM20600_INT_PIN_CFG, pinConfigMask); */
  
  /* // interrupt pin fire on motion detection */
  /* SPI_WRITE_REGISTERS(spid, ICM20600_INT_ENABLE, */
  /* 		      ICM20600_INT_ENABLE_WAKE_ON_MOTION); */
  
  /* // set threshold */
  /* SPI_WRITE_REGISTERS(spid, ICM20600_ACCEL_WOM_THRESHOLD, threadshold); */
  
  /* spiReleaseBus(spid);  */
}



void icm20600_setModeDeepSleep(Icm20600Data *imu)
{
  SPIDriver * const spid = imu->config->spid;
  spiAcquireBus(spid);
  // set 9250 in deep sleep mode
  SPI_WRITE_REGISTERS(spid, ICM20600_PWR_MGMT_1, ICM20600_ENTER_SLEEP);
  spiReleaseBus(spid);
}
