#include "spiMasterUtil.h"
#include "spiPeriphICM20600.h"
#include <math.h>
#include <string.h>

// to use DebugTrace

#include "stdutil.h"

/*
#                 _____    _____   __  __             ___     ___      __     ___     ___   
#                |_   _|  / ____| |  \/  |           |__ \   / _ \    / /    / _ \   / _ \  
#                  | |   | |      | \  / |              ) | | | | |  / /_   | | | | | | | | 
#                  | |   | |      | |\/| |             / /  | | | | | '_ \  | | | | | | | | 
#                 _| |_  | |____  | |  | |  ______    / /_  | |_| | | (_) | | |_| | | |_| | 
#                |_____|  \_____| |_|  |_| |______|  |____|  \___/   \___/   \___/   \___/  
*/

#define NB_AVERAGE 200
#define MIN_ABSOLUTE_DEVIATION_DPS 60
#define MAX_ABSOLUTE_OFFSET_DPS 10
#define MAX_ABSOLUTE_DEVIATION_MILLI_G 675
#define MIN_ABSOLUTE_DEVIATION_MILLI_G 225



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
  MODIFY_REG(imu->config->config, 0x7, lpf);
  setGyroConfig(imu);
}

void icm20600_setAccelLpf( Icm20600Data *imu, const uint8_t lpf)
{
  MODIFY_REG(imu->config->accelConf2, 0xf, lpf);
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

  MODIFY_REG(imu->config->gyroConfig, ICM20600_RANGE_GYRO_MASK, fsrMask);
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

   MODIFY_REG(imu->config->accelConf, ICM20600_RANGE_ACCEL_MASK, fsrMask);
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
  spiSetBitsRegister(spid, ICM20600_PWR_MGMT_1, ICM20600_GYRO_STANDBY);

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

static const uint16_t stCodeToOtp[256] = {
	2620, 2646, 2672, 2699, 2726, 2753, 2781, 2808,
	2837, 2865, 2894, 2923, 2952, 2981, 3011, 3041,
	3072, 3102, 3133, 3165, 3196, 3228, 3261, 3293,
	3326, 3359, 3393, 3427, 3461, 3496, 3531, 3566,
	3602, 3638, 3674, 3711, 3748, 3786, 3823, 3862,
	3900, 3939, 3979, 4019, 4059, 4099, 4140, 4182,
	4224, 4266, 4308, 4352, 4395, 4439, 4483, 4528,
	4574, 4619, 4665, 4712, 4759, 4807, 4855, 4903,
	4953, 5002, 5052, 5103, 5154, 5205, 5257, 5310,
	5363, 5417, 5471, 5525, 5581, 5636, 5693, 5750,
	5807, 5865, 5924, 5983, 6043, 6104, 6165, 6226,
	6289, 6351, 6415, 6479, 6544, 6609, 6675, 6742,
	6810, 6878, 6946, 7016, 7086, 7157, 7229, 7301,
	7374, 7448, 7522, 7597, 7673, 7750, 7828, 7906,
	7985, 8065, 8145, 8227, 8309, 8392, 8476, 8561,
	8647, 8733, 8820, 8909, 8998, 9088, 9178, 9270,
	9363, 9457, 9551, 9647, 9743, 9841, 9939, 10038,
	10139, 10240, 10343, 10446, 10550, 10656, 10763, 10870,
	10979, 11089, 11200, 11312, 11425, 11539, 11654, 11771,
	11889, 12008, 12128, 12249, 12371, 12495, 12620, 12746,
	12874, 13002, 13132, 13264, 13396, 13530, 13666, 13802,
	13940, 14080, 14221, 14363, 14506, 14652, 14798, 14946,
	15096, 15247, 15399, 15553, 15709, 15866, 16024, 16184,
	16346, 16510, 16675, 16842, 17010, 17180, 17352, 17526,
	17701, 17878, 18057, 18237, 18420, 18604, 18790, 18978,
	19167, 19359, 19553, 19748, 19946, 20145, 20347, 20550,
	20756, 20963, 21173, 21385, 21598, 21814, 22033, 22253,
	22475, 22700, 22927, 23156, 23388, 23622, 23858, 24097,
	24338, 24581, 24827, 25075, 25326, 25579, 25835, 26093,
	26354, 26618, 26884, 27153, 27424, 27699, 27976, 28255,
	28538, 28823, 29112, 29403, 29697, 29994, 30294, 30597,
	30903, 31212, 31524, 31839, 32157, 32479, 32804
};

static void diffVecI32(int32_t *op1, int32_t *op2, int32_t *diff)
{
  for (int j=0; j< 3; j++) {
    diff[j] = op1[j] - op2[j];
  }
}

static void getAverage(Icm20600Data *imu, int32_t *gyroAvg, int32_t *accAvg)
{
  const uint8_t  *rawB =  imu->rawCache;
  int32_t gyroSum[3] = {0,0,0};
  int32_t accSum[3] = {0,0,0};
  
  for (int32_t i=0; i<NB_AVERAGE; i++) {
    icm20600_fetch(imu);
    for (int32_t j=0; j<3; j++) {
      accSum[j] +=  (int16_t) ((rawB[j*2]<<8) | rawB[(j*2)+1]);
      gyroSum[j] += (int16_t) ((int16_t)((rawB[(j*2)+8]<<8) | rawB[(j*2)+9]));
    }
    chThdSleepMilliseconds(1);
  }

  for (int32_t j=0; j< 3; j++) {
    gyroAvg[j] = gyroSum[j] / NB_AVERAGE;
    accAvg[j] = accSum[j] / NB_AVERAGE;
  }
  
}

static Icm20600TestResult selfTest (Icm20600Data *imu)
{
  SPIDriver * const spid = imu->config->spid;
  Icm20600TestResult res = {100,true,true};
  int32_t gyroNormalAvg[3];
  int32_t gyroTestAvg[3];
  int32_t gyroDiff[3];
  int32_t accNormalAvg[3];
  int32_t accTestAvg[3];
  int32_t accDiff[3];
  uint8_t accStCode[3];
  uint8_t gyroStCode[3];

  // get OTP values
  SPI_READ_REGISTERS(spid, ICM20600_SELF_TEST_X_ACCEL, accStCode);
  SPI_READ_REGISTERS(spid, ICM20600_SELF_TEST_X_GYRO, gyroStCode);
  
  // get average of measures in normal mode
  getAverage(imu, gyroNormalAvg, accNormalAvg);

  // switch to test mode
  spiSetBitsRegister(spid, ICM20600_GYRO_CONFIG, ICM20600_X_GYRO_SELFTEST |
		     ICM20600_Y_GYRO_SELFTEST | ICM20600_Z_GYRO_SELFTEST);
  
  spiSetBitsRegister(spid, ICM20600_ACCEL_CONFIG, ICM20600_X_ACCEL_SELFTEST |
		     ICM20600_Y_ACCEL_SELFTEST | ICM20600_Z_ACCEL_SELFTEST);

  // wait 20ms for oscillations to stabilize
  chThdSleepMilliseconds(20);
  
  // get average of measures in test mode
  getAverage(imu, gyroTestAvg, accTestAvg);

  // calculate difference
  diffVecI32(gyroTestAvg, gyroNormalAvg, gyroDiff);
  diffVecI32(accTestAvg, accNormalAvg, accDiff);

  // calculate ratio and check criteria
  for (size_t j=0; j< 3; j++) {
    if (gyroStCode[j]) {
      const float gyroDiffRatio = ((float) gyroDiff[j]) / stCodeToOtp[gyroStCode[j]];
      const float normEval = 100 - (fabsf(1.0f-gyroDiffRatio) * 200.0f);
      const uint8_t normClamp = clampTo(0,100,normEval);
      res.factory = MIN(normClamp, res.factory);
    } else {
//      DebugTrace("axe %u ratio gyro OTP is ZERO", j);
      if ((gyroDiff[j] * imu->gyroScale) < MIN_ABSOLUTE_DEVIATION_DPS)
	res.factory = 0;
    }

    if (accStCode[j]) {
      const float accDiffRatio = ((float) accDiff[j]) / stCodeToOtp[accStCode[j]];
      const float normEval = 100 - (fabsf(1.0f-accDiffRatio) * 200.0f);
      const uint8_t normClamp = clampTo(0,100,normEval);
      res.factory = MIN(normClamp, res.factory);
     } else {
      //      DebugTrace("axe %u ratio acc OTP is ZERO", j);
      const float ast = 1000 * accDiff[j] * imu->accelScale;
      if ((ast < MIN_ABSOLUTE_DEVIATION_MILLI_G) || (ast > MAX_ABSOLUTE_DEVIATION_MILLI_G))
	res.factory = 0;
    }

    if ((gyroNormalAvg[j] * imu->gyroScale) > MAX_ABSOLUTE_OFFSET_DPS)
      res.bias = false;
    //    DebugTrace ("gyro bias[%u] = %.2f", j, (gyroNormalAvg[j] * imu->gyroScale));
  }

  res.passed = (res.factory != 0) && res.bias;

  //  DebugTrace("overall factory note = %u bias=%d passed=%d",
  //	     res.factory, res.bias, res.passed);
  
  return res;
};



Icm20600TestResult icm20600_runSelfTests (Icm20600Data *imu)
{
  Icm20600TestResult res;

  // save pointer to use provided configuration
  Icm20600Config *savedConfig = imu->config;

  // recommended config for selftests cf
  // https://github.com/kriswiner/MPU9250/blob/master/Documents/AN-MPU-9250A-03%20MPU-9250%20Accel%20Gyro%20and%20Compass%20Self-Test%20Implementation%20v1%200_062813.pdf
  
  Icm20600Config stConfig = {
    .spid = savedConfig->spid,
    .sampleRate = 8000,
    .config = ICM20600_GYRO_RATE_1K_BW_92,
    .gyroConfig = ICM20600_RANGE_250_DPS,
    .accelConf = ICM20600_RANGE_2G,
    .accelConf2 = ICM20600_ACC_RATE_1K_BW_99
  };



  // reset and reinit IMU for selftest
  if (icm20600_init(imu, &stConfig) != MSG_OK) {
    res = (Icm20600TestResult) {0,false,false};
    goto restoreAndExit;
  }
  chThdSleepMilliseconds(20);
  res = selfTest(imu);
  
 restoreAndExit:
  // restore configuration
  imu->config = savedConfig;
  icm20600_init(imu, savedConfig);
  
  return res;
}
