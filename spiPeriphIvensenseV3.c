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

msg_t inv3Init(Inv3Driver *inv3d, const Inv3Config* cfg)
{
  inv3d->config = cfg;
  chDbgAssert((cfg->gyroScale >> GYRO_FS_SHIFT) <= ARRAY_LEN(gyroScaleValues),
	      "invalid gyroScale");
  chDbgAssert((cfg->accelScale >> ACCEL_FS_SHIFT) <= ARRAY_LEN(accScaleValues),
	      "invalid accelScale");
  inv3d->currentBank = 0;
  inv3d->gyroScale = gyroScaleValues[cfg->gyroScale >> GYRO_FS_SHIFT];
  inv3d->accScale = accScaleValues[cfg->accelScale >> ACCEL_FS_SHIFT];
  setBank(inv3d, INV3_BANK(INV3REG_INTF_CONFIG0));
  
  spiWriteRegister(inv3d->config->spid, INV3_REG(INV3REG_DEVICE_CONFIG),
		      BIT_DEVICE_CONFIG_SOFT_RESET_CONFIG);
  chThdSleepMilliseconds(1); // wait 1ms after soft reset
 
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

  return MSG_OK;
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
    acc->v[i] = transBuf.acc[i]  * 9.81f * inv3d->accScale / 32768;
    gyro->v[i] =  transBuf.gyro[i] * inv3d->gyroScale / 32768;
  }
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
		   FIFO_CONFIG_MODE_STREAM_TO_FIFO);
  spiWriteRegister(inv3d->config->spid, INV3_REG(INV3REG_FIFO_CONFIG1),
		   BIT_FIFO_CONFIG1_ACCEL_EN | BIT_FIFO_CONFIG1_GYRO_EN
		   | BIT_FIFO_CONFIG1_TEMP_EN |  BIT_FIFO_CONFIG1_WM_GT_TH |
		   BIT_FIFO_CONFIG1_RESUME_PARTIAL_RD);
  spiWriteRegister(inv3d->config->spid, INV3_REG(INV3REG_FIFO_CONFIG2),
		   watermarkBytes & 0xff);
  spiWriteRegister(inv3d->config->spid, INV3_REG(INV3REG_FIFO_CONFIG3),
		   (watermarkBytes >> 8) & 0x0f);
  spiWriteRegister(inv3d->config->spid, INV3_REG(INV3REG_INT_SOURCE0),
		   BIT_FIFO_THS_INT1_EN);


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
			   fifoCount + (2U * sizeof(Inv3Packet3)));
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
