#include "spiMasterUtil.h"
#include "spiPeriphIvensenseV3.h"
#include <math.h>
#include <string.h>

// to use DebugTrace

#include "stdutil.h"

/* static const float odrValues[] = {0, 32000, 16000, 8000, 4000, 2000, 1000, */
/*   200, 100, 50, 25, 12.5f, 6.25f, 3.125f, 1.5625f, 500}; */

static const float accScaleValues[] = {16, 8, 4, 2};
static const float gyroScaleValues[] = {2000, 1000, 500, 250, 125, 62.5f, 31.25f, 15.625f};

static void inv3GetRawVal (SPIDriver *spid, float *temp,  
			   Vec3f *gyro, Vec3f *acc);
static msg_t inv3SelfTestInit(SPIDriver *spid);

#if INVENSENSE3_USE_FIFO
static void  inv3InitFifo(Inv3Driver *inv3d);
#endif

static void setBank(Inv3Driver *inv3d, Inv3Bank b)
{
  chDbgAssert(b <= INV3_BANK4, "incorrect bank index");
  if (b != inv3d->currentBank) {
    inv3d->currentBank = b;
    spiWriteRegister(inv3d->config->spid,  INV3REG_BANK_SEL, b);
  }
}

bool inv3Init(Inv3Driver *inv3d, const Inv3Config* cfg)
{
  inv3d->config = cfg;
  chDbgAssert((cfg->gyroScale >> GYRO_FS_SHIFT) <= ARRAY_LEN(gyroScaleValues),
	      "invalid gyroScale");
  chDbgAssert((cfg->accelScale >> ACCEL_FS_SHIFT) <= ARRAY_LEN(accScaleValues),
	      "invalid accelScale");
  inv3d->currentBank = 0;
  inv3d->gyroScale = gyroScaleValues[cfg->gyroScale >> GYRO_FS_SHIFT];
  inv3d->accScale = accScaleValues[cfg->accelScale >> ACCEL_FS_SHIFT];

  // when using fifo and interruptions, two resets are needed to avoid
  // device stall
  for (int i=0; i < 2; i++) {
    setBank(inv3d, INV3_BANK(INV3REG_INTF_CONFIG0));
    spiWriteRegister(inv3d->config->spid, INV3_REG(INV3REG_DEVICE_CONFIG),
		     BIT_DEVICE_CONFIG_SOFT_RESET_CONFIG);
    chThdSleepMilliseconds(1); // wait 1ms after soft reset
  }

  // if odr is >= 4hz, INT_CONFIG1 must be configured
  // in all case, INT_ASYNC_RESET bit must be cleared
  const  Inv3SensorType sensorType = inv3GetSensorType(inv3d);
  if ((sensorType < INV3_WHOAMI_ICM40605) ||
      (sensorType > INV3_WHOAMI_ICM42670))
    goto fail;
  
  spiWriteRegister(inv3d->config->spid,  INV3_REG(INV3REG_INT_CONFIG0),
		   FIFO_FULL_CLEAR_STATUS_AND_FIFO_READ |
		   FIFO_THRESHOLD_CLEAR_STATUS_AND_FIFO_READ |
		   DATA_READY_CLEAR_STATUS_AND_REG_READ);
  
  spiWriteRegister(inv3d->config->spid,  INV3_REG(INV3REG_INT_CONFIG1),
		   inv3d->config->commonOdr <= COMMON_ODR_4KHZ ?
		   // 4KHz or more
		   BIT_INT_TDEASSERT_ODR4KPLUS |  INT_TPULSE_DURATION_ODR4KPLUS :
		   // less than 4KHz
		   0);
  spiWriteRegister(inv3d->config->spid, INV3_REG(INV3REG_INT_SOURCE0),
		  BIT_UI_DRDY_INT1_EN);
  spiWriteRegister(inv3d->config->spid, INV3_REG(INV3REG_INT_CONFIG),
		   INT1_ACTIVE_HIGH | INT1_PUSHPULL | INT1_PULSED);
  spiWriteRegister(inv3d->config->spid, INV3_REG(INV3REG_INTF_CONFIG0),
		      UI_SIFS_CFG_I2C_DIS | SENSOR_DATA_LITTLE_ENDIAN |
		      FIFO_COUNT_LITTLE_ENDIAN);
  spiWriteRegister(inv3d->config->spid, INV3_REG(INV3REG_GYRO_CONFIG0),
		   inv3d->config->commonOdr | inv3d->config->gyroScale);
  spiWriteRegister(inv3d->config->spid, INV3_REG(INV3REG_ACCEL_CONFIG0),
		   inv3d->config->commonOdr | inv3d->config->accelScale);
  // fix bug in ICM40605 :
  // https://github.com/ArduPilot/ardupilot/commit/6f25ca97740790fcdc75d40cc0f9a63f099c81b4
  spiModifyRegister(inv3d->config->spid, INV3_REG(INV3REG_INTF_CONFIG1),
		    FIFO_COUNT_REC, FIFO_COUNT_REC);
  if (inv3d->config->externClockRef == true) {
    spiModifyRegister(inv3d->config->spid, INV3_REG(INV3REG_INTF_CONFIG1),
		      CLKSEL_EXTERNAL_RTC, CLKSEL_EXTERNAL_RTC);
    setBank(inv3d, INV3_BANK(INV3REG_INTF_CONFIG5));
    spiWriteRegister(inv3d->config->spid, INV3_REG(INV3REG_INTF_CONFIG5),
		     PIN9_CLKIN);
    setBank(inv3d, INV3_BANK(INV3REG_INTF_CONFIG0));
  }

#if INVENSENSE3_USE_FIFO
  inv3InitFifo(inv3d);
#endif
  spiWriteRegister(inv3d->config->spid, INV3_REG(INV3REG_PWR_MGMT0),
		   ACCEL_MODE_LN | GYRO_MODE_LN);
  chThdSleepMilliseconds(1);

  return true;
 fail:
  return false;
}

void inv3SetOdr (Inv3Driver *inv3d, const Inv3Odr odr)
{
  setBank(inv3d, INV3_BANK(INV3REG_GYRO_CONFIG0));
  spiModifyRegister(inv3d->config->spid, INV3_REG(INV3REG_GYRO_CONFIG0),
		    COMMON_ODR_MASK, odr);
  spiModifyRegister(inv3d->config->spid, INV3_REG(INV3REG_ACCEL_CONFIG0),
		    COMMON_ODR_MASK, odr);
}

void inv3SetAccelScale (Inv3Driver *inv3d, const Inv3AccelScale scale)
{
  setBank(inv3d, INV3_BANK(INV3REG_ACCEL_CONFIG0));
  spiModifyRegister(inv3d->config->spid, INV3_REG(INV3REG_ACCEL_CONFIG0),
		    ACCEL_FS_MASK, scale);
}

void inv3SetGyroScale (Inv3Driver *inv3d, const Inv3GyroScale scale)
{
  setBank(inv3d, INV3_BANK(INV3REG_GYRO_CONFIG0));
  spiModifyRegister(inv3d->config->spid, INV3_REG(INV3REG_GYRO_CONFIG0),
		    GYRO_FS_MASK, scale);
}

Inv3SensorType inv3GetSensorType(Inv3Driver *inv3d)
{
  setBank(inv3d, INV3_BANK(INV3REG_WHO_AM_I));
  const Inv3SensorType whoami =
    spiReadOneRegister(inv3d->config->spid, INV3_REG(INV3REG_WHO_AM_I));
  return whoami;
}

uint8_t inv3GetStatus(Inv3Driver *inv3d)
{
  setBank(inv3d, INV3_BANK(INV3REG_INT_STATUS));
  const uint8_t status =
    spiReadOneRegister(inv3d->config->spid, INV3_REG(INV3REG_INT_STATUS));
  return status;
}

uint8_t inv3GetClockDiv(Inv3Driver *inv3d)
{
  setBank(inv3d, INV3_BANK(INV3REG_CLOCKDIV));
  const uint8_t clockdiv =
    spiReadOneRegister(inv3d->config->spid, INV3_REG(INV3REG_CLOCKDIV));
  return clockdiv;
}

static void inv3GetRawVal (SPIDriver *spid, float *temp, 
			   Vec3f *gyro, Vec3f *acc)
{
  union {
    uint8_t buf[14];
    struct {
      int16_t temp;
      int16_t acc[3];
      int16_t gyro[3];
    };
  } transBuf = {};
  
  spiWriteRegister(spid,  INV3REG_BANK_SEL, INV3_BANK(INV3REG_TEMP_DATA1));
  spiReadRegisters(spid, INV3_REG(INV3REG_TEMP_DATA1),
		   transBuf.buf, sizeof(transBuf));
  
  *temp = transBuf.temp;
  for (int i=0; i<3; i++) {
    acc->v[i] = transBuf.acc[i];
    gyro->v[i] =  transBuf.gyro[i];
  }
}

void inv3GetVal (Inv3Driver *inv3d, float *temp, 
		      Vec3f *gyro, Vec3f *acc)
{
  union {
    uint8_t buf[14];
    struct {
      int16_t temp;
      int16_t acc[3];
      int16_t gyro[3];
    };
  } transBuf = {};
  
  setBank(inv3d, INV3_BANK(INV3REG_TEMP_DATA1));
  spiReadRegisters(inv3d->config->spid, INV3_REG(INV3REG_TEMP_DATA1),
		   transBuf.buf, sizeof(transBuf));
  
  *temp = (transBuf.temp / 132.48f) + 25;
  for (int i=0; i<3; i++) {
    acc->v[i] = transBuf.acc[i]  * 9.81f * inv3d->accScale / 16384;
    gyro->v[i] =  transBuf.gyro[i] * inv3d->gyroScale / 32768;
  }
}


static msg_t inv3SelfTestInit(SPIDriver *spid)
{
  // when using fifo and interruptions, two resets are needed to avoid
  // device stall
  for (int i=0; i < 2; i++) {
    spiWriteRegister(spid,  INV3REG_BANK_SEL, INV3_BANK0);
    spiWriteRegister(spid, INV3_REG(INV3REG_DEVICE_CONFIG),
		     BIT_DEVICE_CONFIG_SOFT_RESET_CONFIG);
    chThdSleepMilliseconds(1); // wait 1ms after soft reset
  }
  
  spiWriteRegister(spid, INV3_REG(INV3REG_ACCEL_CONFIG0),
		   COMMON_ODR_1KHZ | ACCEL_FS_SEL_4G);
  spiWriteRegister(spid, INV3_REG(INV3REG_GYRO_CONFIG0),
		   COMMON_ODR_1KHZ | GYRO_FS_SEL_250DPS);
  // set gyro and accel bandwidth to ODR/10
  spiWriteRegister(spid, INV3_REG(INV3REG_GYRO_ACCEL_CONFIG0),
		   0x44);
  spiWriteRegister(spid, INV3_REG(INV3REG_INTF_CONFIG0),
		   UI_SIFS_CFG_I2C_DIS | SENSOR_DATA_LITTLE_ENDIAN);
  
  spiWriteRegister(spid, INV3_REG(INV3REG_PWR_MGMT0),
		   ACCEL_MODE_LN | GYRO_MODE_LN);
  chThdSleepMilliseconds(1); // wait 1ms after soft reset
  return MSG_OK;
}

bool inv3RunSelfTest(SPIDriver *spid,
			 Vec3f *accelDiff, Vec3f *gyroDiff,
			 Vec3f *accelRatio, Vec3f *gyroRatio)
{
  bool statusOk = true;
  
  struct {
    float temp;
    Vec3f gyro, acc;
  } nominal, selftest;
  struct {
    uint8_t v[3];
  } calibAcc, calibGyro;
  Vec3f dummy;
  *accelDiff = *gyroDiff = *accelRatio = *gyroRatio = (Vec3f){};
  inv3SelfTestInit(spid);
  chThdSleepMilliseconds(100);
  
  inv3GetRawVal(spid, &nominal.temp, &nominal.gyro, &nominal.acc);
  // 
  spiWriteRegister(spid, INV3_REG(INV3REG_SELF_TEST_CONFIG),
		   SELFTEST_ENABLE_ACCEL);
  chThdSleepMilliseconds(100);
  inv3GetRawVal(spid, &selftest.temp, &dummy, &selftest.acc);
  spiWriteRegister(spid, INV3_REG(INV3REG_SELF_TEST_CONFIG),
		   SELFTEST_ENABLE_GYRO);
  chThdSleepMilliseconds(100);
  inv3GetRawVal(spid, &selftest.temp, &selftest.gyro, &dummy);
  spiWriteRegister(spid, INV3_REG(INV3REG_SELF_TEST_CONFIG),
		   SELFTEST_DISABLE);

  *accelDiff = vec3fSub(&nominal.acc, &selftest.acc);
  *accelDiff = vec3fAbs(accelDiff);
  *gyroDiff = vec3fSub(&nominal.gyro, &selftest.gyro);
  *gyroDiff = vec3fAbs(gyroDiff);

  spiWriteRegister(spid,  INV3REG_BANK_SEL, INV3_BANK(INV3REG_XG_ST_DATA));
  spiReadRegisters(spid, INV3_REG(INV3REG_XG_ST_DATA),
		   calibGyro.v, sizeof(calibGyro));
  spiWriteRegister(spid,  INV3REG_BANK_SEL, INV3_BANK(INV3REG_XA_ST_DATA));
  spiReadRegisters(spid, INV3_REG(INV3REG_XA_ST_DATA),
		   calibAcc.v, sizeof(calibAcc));

 /*
00> accel diff should be between 50 and 1200 mg
00> gyro diff should be > 60 dps
00> accel and gyro ratios should be between 50% and 150%
 */
  static const float minAccLimit = 0.050f * 32768 / 4;
  static const float maxAccLimit = 1.2f * 32768 / 4;
  static const float minGyroLimit = 60 * 32768 / 250;
  for(size_t i=0; i<3; i++) {
    accelRatio->v[i] =  accelDiff->v[i] / (1310.0f * powf(1.01f, calibAcc.v[i] - 1U) + 0.5f);
    gyroRatio->v[i] =  gyroDiff->v[i] / (2620.0f * powf(1.01f, calibGyro.v[i] - 1U) + 0.5f);
    statusOk = statusOk && (accelDiff->v[i] > minAccLimit && accelDiff->v[i] < maxAccLimit);
    statusOk = statusOk && (gyroDiff->v[i] > minGyroLimit);
    statusOk = statusOk && (accelRatio->v[i] > 0.5f && accelRatio->v[i] < 1.5f);
    statusOk = statusOk && (gyroRatio->v[i] > 0.5f && gyroRatio->v[i] < 1.5f);
  }
  
  spiWriteRegister(spid,  INV3REG_BANK_SEL, INV3_BANK0);
  return statusOk;
}

#if INVENSENSE3_USE_FIFO
/* configure :
     ° mode fifo : ok
     ° packet mode 3 : ok (temperature obligatoire ?)
     ° fifo threashold : ok
     ° interruption for fifo threshold instead of dataready : ok
*/
static void  inv3InitFifo(Inv3Driver *inv3d)
{
  const Inv3Config* cfg = inv3d->config;
  inv3d->watermarkNbPacket = (2048U * cfg->watermarkPercent / 100U) /
			      sizeof(Inv3Packet3);
  const uint16_t watermarkBytes = inv3d->watermarkNbPacket * sizeof(Inv3Packet3);

  setBank(inv3d, INV3_BANK(INV3REG_FIFO_CONFIG1));
  spiWriteRegister(inv3d->config->spid, INV3_REG(INV3REG_FIFO_CONFIG),
		   FIFO_CONFIG_MODE_STREAM_TO_FIFO | BIT_FIFO_CONFIG1_TMST_FSYNC_EN);
  spiWriteRegister(inv3d->config->spid, INV3_REG(INV3REG_FIFO_CONFIG1),
		   BIT_FIFO_CONFIG1_ACCEL_EN | BIT_FIFO_CONFIG1_GYRO_EN |
		   BIT_FIFO_CONFIG1_TEMP_EN |
		   BIT_FIFO_CONFIG1_RESUME_PARTIAL_RD);
  spiWriteRegister(inv3d->config->spid, INV3_REG(INV3REG_FIFO_CONFIG2),
		   watermarkBytes & 0xff);
  spiWriteRegister(inv3d->config->spid, INV3_REG(INV3REG_FIFO_CONFIG3),
		   (watermarkBytes >> 8) & 0x0f);
  spiWriteRegister(inv3d->config->spid, INV3_REG(INV3REG_INT_SOURCE0),
		   BIT_FIFO_THS_INT1_EN);
  spiSetBitsRegister(inv3d->config->spid, INV3_REG(INV3REG_TMST_CONFIG), 
		     BIT_TMST_CONFIG_TMST_EN /*| BIT_TMST_RES*/); 
}

uint16_t inv3PopFifo (Inv3Driver *inv3d)
{
  uint16_t fifoCount;
  setBank(inv3d, INV3_BANK(INV3REG_FIFO_COUNTH));
  spiReadRegisters(inv3d->config->spid, INV3_REG(INV3REG_FIFO_COUNTH),
		   (uint8_t *) &fifoCount, 2U);
  if ((fifoCount >= sizeof(Inv3Packet3)) && (fifoCount <= 2080)) {
    spiDirectReadRegisters(inv3d->config->spid, INV3REG_FIFO_DATA,
			   (uint8_t *) inv3d->config->fifoBuffer->fifoBuf,
			   fifoCount);
  } 
  inv3d->config->fifoBuffer->fifoCount = fifoCount;
  return fifoCount;
}

void inv3GetAverageVal (Inv3Driver *inv3d, float *temp, 
			Vec3f *gyro, Vec3f *acc)
{
  const Inv3Packet3FifoBuffer *pfb = inv3d->config->fifoBuffer;
  *temp = 0;
  size_t validPacket=0;
  Vec3f sumAcc = {}, sumGyro = {};
  for(size_t idx=0; idx < pfb->fifoCount / sizeof(Inv3Packet3); idx++) {
    if (!pfb->fifoBuf[idx].fifoEmpty) {
      validPacket++;
      for (size_t j=0; j<3; j++) { 
	sumAcc.v[j] += pfb->fifoBuf[idx].acc[j];
	sumGyro.v[j] += pfb->fifoBuf[idx].gyro[j];
      }
      *temp += ((pfb->fifoBuf[idx].temp / 2.07f) + 25);
    } else {
      break;
    }
  }
  *gyro = vec3fDiv(&sumGyro, validPacket);
  *acc = vec3fDiv(&sumAcc, validPacket);
  *temp /= validPacket;
  for (size_t j=0; j<3; j++) {
    acc->v[j] = acc->v[j] * 9.81f * inv3d->accScale / 32768;
    gyro->v[j] = gyro->v[j]  * inv3d->gyroScale / 32768;
  }
}




#endif
