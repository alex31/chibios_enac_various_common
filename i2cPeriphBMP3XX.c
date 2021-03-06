#include "i2cPeriphBMP3XX.h"

#include "ch.h"
#include "hal.h"
#include <string.h>

// use DebugTrace for tuning
//#include "stdutil.h"


static int8_t i2cWrite (uint8_t dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len,
			void *userData);
static int8_t i2cRead (uint8_t dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len,
		       void *userData);
static void   waitMilliseconds (uint32_t period);

msg_t  bmp3xxStart (Bmp3xxDriver *bmpp, Bmp3xxConfig *config)
{
  bmpp->config = config;
  bmpp->config->bm3dev.intf = BMP3_I2C_INTF;
  bmpp->config->bm3dev.read = &i2cRead;
  bmpp->config->bm3dev.write = &i2cWrite;
  bmpp->config->bm3dev.delay_ms = &waitMilliseconds;
  bmpp->config->bm3dev.user_data = bmpp->config->i2cp;

  uint8_t bmpStatus = bmp3_init(&bmpp->config->bm3dev);
  if (bmpStatus == BMP3_OK)
    bmpStatus = bmp3_set_sensor_settings(bmpp->config->settings, &bmpp->config->bm3dev);
   if (bmpStatus == BMP3_OK)
    bmpStatus = bmp3_set_op_mode(&bmpp->config->bm3dev);
  
  return bmpStatus == BMP3_OK ? MSG_OK : MSG_RESET;
}

msg_t  bmp3xxFetch(Bmp3xxDriver *bmpp, const Bmp3xRequest request)
{
  int8_t bmpStatus;
  
  /* Temperature and Pressure data are read and stored in the bmp3_data instance */
  bmpStatus = bmp3_get_sensor_data(request, &bmpp->measure, &bmpp->config->bm3dev);
  
  return bmpStatus == BMP3_OK ? MSG_OK : MSG_RESET;
}



static int8_t i2cWrite (uint8_t dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len,
			void *userData)
{
  I2CDriver *i2cp = (I2CDriver *) userData;
  const uint8_t wlen = len + 1;

  if (wlen > 128)
    return BMP3_E_INVALID_LEN;

  uint8_t CACHE_ALIGNED(writeBuffer[wlen]);

  //  DebugTrace("write %u b @r%u", len, reg_addr);
  
  writeBuffer[0] = reg_addr;
  memcpy(&writeBuffer[1], data, len);

  const msg_t status = i2cMasterCacheTransmitTimeout(i2cp, dev_id,
						     writeBuffer, wlen,
						     NULL, 0, 100);
  if (status != MSG_OK) {
    restartI2c(i2cp);
    return BMP3_E_COMM_FAIL;
  }
  
  return BMP3_OK;
}



// have to manage cache in case of STM32F7XX or STM32H7XX
#if (__DCACHE_PRESENT != 0)
static int8_t i2cRead (uint8_t dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len,
		       void *userData)
{
  if (len > 128-32)
    return BMP3_E_INVALID_LEN;
  
  I2CDriver * const i2cp = (I2CDriver *) userData;
  const uint8_t CACHE_ALIGNED(writeBuffer[]) = {reg_addr};
  uint8_t       CACHE_ALIGNED(readBuffer[len+32]); // +32 to isolate cache line

  const msg_t status = i2cMasterCacheTransmitTimeout(i2cp, dev_id,
						     writeBuffer, sizeof(writeBuffer),
						     readBuffer, len, 100);
  memcpy(data, readBuffer, len);
  /* DebugTrace("read %u b @r%u", len, reg_addr); */
  /* for (size_t i=0; i<len; i++) { */
  /*   DebugTrace("R[%u]=0x%x",  reg_addr+i, data[i]); */
  /* } */

  if (status != MSG_OK) {
    restartI2c(i2cp);
    return BMP3_E_COMM_FAIL;
  }
  
  return BMP3_OK;
}

#else // Mcu without cache

static int8_t i2cRead (uint8_t dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len,
		       void *userData)
{
  I2CDriver *i2cp = (I2CDriver *) userData;

  const msg_t status = i2cMasterTransmitTimeout(i2cp, dev_id,
						&reg_addr, sizeof(reg_addr),
						data, len, 100);

  /* DebugTrace("read %u b @r%u", len, reg_addr); */
  /* for (size_t i=0; i<len; i++) { */
  /*   DebugTrace("R[%u]=0x%x",  reg_addr+i, data[i]); */
  /* } */

  if (status != MSG_OK) {
    restartI2c(i2cp);
    return BMP3_E_COMM_FAIL;
  }
  
  return BMP3_OK;
}


#endif // defined STM32F7XX || defined STM32H7XX

static void waitMilliseconds (uint32_t period)
{
  chThdSleepMilliseconds(period);
}

