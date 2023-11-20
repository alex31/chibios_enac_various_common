#include "i2cPeriphLPS33HW.h"
#include <string.h>

static int32_t i2cWrite (void *userData, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
  LPS33HWDriver *lpsp = (LPS33HWDriver *) userData;
  const uint8_t wlen = len + 1;

  if (wlen > 128)
    return MSG_RESET;

  uint8_t CACHE_ALIGNED(writeBuffer[wlen]);

  //  DebugTrace("write %u b @r%u", len, reg_addr);
  
  writeBuffer[0] = reg_addr;
  memcpy(&writeBuffer[1], data, len);

  const msg_t status = i2cMasterCacheTransmitTimeout(lpsp->config->i2cp,
						     lpsp->config->slaveAdr,
						     writeBuffer, wlen,
						     NULL, 0, TIME_MS2I(100));
  if (status != MSG_OK) {
    restartI2c(lpsp->config->i2cp);
    return MSG_RESET;
  }
  
  return MSG_OK;
}



#if (__DCACHE_PRESENT != 0)
static int32_t i2cRead (void *userData, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
  if (len > 128-32)
    return MSG_RESET;
  
  
  const uint8_t CACHE_ALIGNED(writeBuffer[]) = {reg_addr};
  uint8_t       CACHE_ALIGNED(readBuffer[len+32]); // +32 to isolate cache line
  LPS33HWDriver *lpsp = (LPS33HWDriver *) userData;
  
  const msg_t status = i2cMasterCacheTransmitTimeout(lpsp->config->i2cp, lpsp->config->slaveAdr,
						     writeBuffer, sizeof(writeBuffer),
						     readBuffer, len, TIME_MS2I(100));
  memcpy(data, readBuffer, len);
  /* DebugTrace("read %u b @r%u", len, reg_addr); */
  /* for (size_t i=0; i<len; i++) { */
  /*   DebugTrace("R[%u]=0x%x",  reg_addr+i, data[i]); */
  /* } */

  if (status != MSG_OK) {
    restartI2c(lpsp->config->i2cp);
    return MSG_RESET;
  }
  
  return MSG_OK;
}

#else // Mcu without cache

static int32_t i2cRead (void *userData, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
  I2CDriver *i2cp = (I2CDriver *) userData;

  const msg_t status = i2cMasterTransmitTimeout(lpsp->config->i2cp, lpsp->config->slaveAdr,
						&reg_addr, sizeof(reg_addr),
						data, len, TIME_MS2I(100));

  /* DebugTrace("read %u b @r%u", len, reg_addr); */
  /* for (size_t i=0; i<len; i++) { */
  /*   DebugTrace("R[%u]=0x%x",  reg_addr+i, data[i]); */
  /* } */

  if (status != MSG_OK) {
    restartI2c(lpsp->config->i2cp);
    return MSG_RESET;
  }
  
  return MSG_OK;
}
#endif


msg_t  lps33Start(LPS33HWDriver *lpsp, const LPS33HWConfig *config)
{
  lpsp->config = config;
  lpsp->pressure_hPa = lpsp->temperature_degC = 0.0f;
  lpsp->dev_ctx.write_reg = &i2cWrite;
  lpsp->dev_ctx.read_reg = &i2cRead;
  lpsp->dev_ctx.handle = lpsp;
  uint8_t whoami, rst;
  msg_t status;

  
  /*
   *  Check device ID
   */
  lps33hw_device_id_get(&lpsp->dev_ctx, &whoami);
  if (whoami != LPS33HW_ID) {
    return MSG_RESET;
  } 

  /*
   *  Restore default configuration
   */
  lps33hw_reset_set(&lpsp->dev_ctx, PROPERTY_ENABLE);
  do {
    lps33hw_reset_get(&lpsp->dev_ctx, &rst);
    chThdYield();
  } while (rst);
  
  /*
   *  Enable Block Data Update
   */
  if (config->blockDataUpdateEnable == true)
    lps33hw_block_data_update_set(&lpsp->dev_ctx, PROPERTY_ENABLE);
  /*
   * Set Output Data Rate
   */
  lps33hw_data_rate_set(&lpsp->dev_ctx, config->odr);
  /*
   * Set Low pass filter
   */
  status = lps33hw_low_pass_filter_mode_set(&lpsp->dev_ctx, config->lpf);


  /*
   * interrupt on data ready, active low, opendrain (pullup needed on MCU side)
   */
  if (config->dataReadyItrEnable) {
    lps33hw_drdy_on_int_set(&lpsp->dev_ctx, PROPERTY_ENABLE);
    lps33hw_int_polarity_set(&lpsp->dev_ctx, LPS33HW_ACTIVE_LOW);
    status = lps33hw_pin_mode_set(&lpsp->dev_ctx, LPS33HW_OPEN_DRAIN);
  }
  
  return status;
}

msg_t  lps33Fetch(LPS33HWDriver *lpsp, LPS33HWFetch fetch)
{
  msg_t status = MSG_OK;

  int32_t data_raw_pressure=0U;
  int16_t data_raw_temperature=0U;

  if (fetch & LPS33HW_FETCH_PRESS) {
    status = lps33hw_pressure_raw_get(&lpsp->dev_ctx, (uint8_t *) &data_raw_pressure);
    lpsp->pressure_hPa = lps33hw_from_lsb_to_hpa(data_raw_pressure);
  }

  if (fetch & LPS33HW_FETCH_TEMP) {
    lps33hw_temperature_raw_get(&lpsp->dev_ctx, (uint8_t *) &data_raw_temperature);
    lpsp->temperature_degC = lps33hw_from_lsb_to_degc(data_raw_temperature);
  }
  
  return status;
}
