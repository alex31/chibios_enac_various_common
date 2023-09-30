#include "i2cPeriphBMP3XX.h"

#include "ch.h"
#include "hal.h"
#include <string.h>

// use DebugTrace for tuning
//#include "stdutil.h"


static int8_t i2cWrite (uint8_t reg_addr, const uint8_t *data, uint32_t len,
			void *userData);
static int8_t i2cRead (uint8_t reg_addr, uint8_t *data, uint32_t len,
		       void *userData);
static void   waitMicroseconds (uint32_t period, void *userData);

msg_t  bmp3xxStart (Bmp3xxDriver *bmpp, Bmp3xxConfig *config)
{
  bmpp->config = config;
  bmpp->bmp3dev.intf = BMP3_I2C_INTF;
  bmpp->bmp3dev.read = &i2cRead;
  bmpp->bmp3dev.write = &i2cWrite;
  bmpp->bmp3dev.delay_us = &waitMicroseconds;
  bmpp->bmp3dev.intf_ptr = bmpp->config;

  uint8_t bmpStatus = bmp3_init(&bmpp->bmp3dev);
  if (bmpStatus == BMP3_OK)
    bmpStatus = bmp3_set_sensor_settings(bmpp->config->settings_sel,
					 &bmpp->config->settings, &bmpp->bmp3dev);
   if (bmpStatus == BMP3_OK)
    bmpStatus = bmp3_set_op_mode(&bmpp->config->settings, &bmpp->bmp3dev);
  
  return bmpStatus == BMP3_OK ? MSG_OK : MSG_RESET;
}

msg_t  bmp3xxFetch(Bmp3xxDriver *bmpp, const Bmp3xRequest request)
{
  int8_t bmpStatus;
  
  /* Temperature and Pressure data are read and stored in the bmp3_data instance */
  bmpStatus = bmp3_get_sensor_data(request, &bmpp->measure, &bmpp->bmp3dev);
  
  return bmpStatus == BMP3_OK ? MSG_OK : MSG_RESET;
}



static int8_t i2cWrite (uint8_t reg_addr, const uint8_t *data, uint32_t len,
			void *userData)
{
  const Bmp3xxConfig *config = (Bmp3xxConfig *) userData;
  const uint8_t wlen = len + 1;

  if (wlen > 128)
    return BMP3_E_INVALID_LEN;

  uint8_t CACHE_ALIGNED(writeBuffer[wlen]);

  //  DebugTrace("write %u b @r%u", len, reg_addr);
  
  writeBuffer[0] = reg_addr;
  memcpy(&writeBuffer[1], data, len);

  const msg_t status = i2cMasterCacheTransmitTimeout(config->i2cp, config->slaveAddr,
						     writeBuffer, wlen,
						     NULL, 0, 100);
  if (status != MSG_OK) {
    restartI2c(config->i2cp);
    return BMP3_E_COMM_FAIL;
  }
  
  return BMP3_OK;
}



// have to manage cache in case of STM32F7XX or STM32H7XX
#if defined __DCACHE_PRESENT && (__DCACHE_PRESENT != 0)
static int8_t i2cRead (uint8_t reg_addr, uint8_t *data, uint32_t len,
		       void *userData)
{
  if (len > 128-32)
    return BMP3_E_INVALID_LEN;
  
   const Bmp3xxConfig *config = (Bmp3xxConfig *) userData;
  const uint8_t CACHE_ALIGNED(writeBuffer[]) = {reg_addr};
  uint8_t       CACHE_ALIGNED(readBuffer[len+32]); // +32 to isolate cache line

  const msg_t status = i2cMasterCacheTransmitTimeout(config->i2cp, config->slaveAddr,
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

static int8_t i2cRead (uint8_t reg_addr, uint8_t *data, uint32_t len,
		       void *userData)
{
  const Bmp3xxConfig *config = (Bmp3xxConfig *) userData;

  const msg_t status = i2cMasterTransmitTimeout(config->i2cp, config->slaveAddr,
						&reg_addr, sizeof(reg_addr),
						data, len, 100);

  /* DebugTrace("read %u b @r%u", len, reg_addr); */
  /* for (size_t i=0; i<len; i++) { */
  /*   DebugTrace("R[%u]=0x%x",  reg_addr+i, data[i]); */
  /* } */

  if (status != MSG_OK) {
    restartI2c(config->i2cp);
    return BMP3_E_COMM_FAIL;
  }
  
  return BMP3_OK;
}


#endif // defined STM32F7XX || defined STM32H7XX

static void waitMicroseconds (uint32_t period, void *)
{
  chThdSleepMicroseconds(period);
}

