#include "ch.h"
#include "hal.h"
#include "stdutil.h"
//#include "globalVar.h"
#include "i2cPeriphLIS3MDL.h"
#include <string.h>


static msg_t sendConfig(Lis3mdlDriver *ldp, const Lis3mdlConfigRegister *regs);
static msg_t writeOneRegister(Lis3mdlDriver *ldp, const Lis3_RegAddr reg, const uint8_t value);
static msg_t readOneRegister(Lis3mdlDriver *ldp, const Lis3_RegAddr reg, uint8_t *value);

msg_t lis3mdlStart(Lis3mdlDriver *ldp, const Lis3mdlConfig *cfg)
{
  // if one want to reinit using previous configuration, one can pass NULL as second arg
  if (cfg) {
    ldp->config = cfg;
  } else {
    chDbgAssert(ldp->config != NULL, "init with nullptr implies already init");
  }
  memset(&ldp->raw, 0, sizeof(ldp->raw));
  

  uint8_t    whoAmIResponse;
  msg_t  status = readOneRegister(ldp, LIS3_WHO_AM_I, &whoAmIResponse);
  if (status != MSG_OK) {
    DebugTrace("lis i²c init error status = %ld", status);
    return status;
  }
  
  if (whoAmIResponse != LIS3_WHO_AM_I_ANSWER) {
    DebugTrace("reveice LIS3_WHO_AM 0x%x instead of 0x%x", whoAmIResponse, LIS3_WHO_AM_I_ANSWER);
    return MSG_RESET;
  }

  static const Lis3mdlConfigRegister crs = {
    .cr = {
      [0] = 0,
      [1] = LIS3_CR2_SOFTRESET | LIS3_CR2_REBOOT,
      [2] = 0,
      [3] = LIS3_CR4_LITTLE_ENDIAN,
      [4] = 0},
    .intCfg = LIS3_INT_CFG_DISABLE,
    .threshold = 0U
  };

  status = sendConfig(ldp, &crs);
  chThdSleepMilliseconds(1);

  if (status != MSG_OK) 
    return status;

  status = sendConfig(ldp, &ldp->config->regs);
  chThdSleepMilliseconds(1);
  
  return status;
}


msg_t lis3mdlFetch(Lis3mdlDriver *ldp, const Lis3_RegAddr first,
			  const Lis3_RegAddr last)
{
  msg_t status = MSG_OK;
  chDbgAssert((first >= LIS3_STATUS_REG) &&
	      (last <= LIS3_INT_SRC),
	      "lis3mdlFetch register address range");

  chDbgAssert(!((first <  LIS3_OUT_Z_H) &&
		(last > LIS3_OUT_Z_H)),
	      "lis3mdlFetch cross 0x2D auto increment boundary");

  const uint8_t CACHE_ALIGNED(writeBuffer[]) = {first | 0x80};
  uint8_t *readAddr = &ldp->raw.status + (first - LIS3_STATUS_REG);
  const size_t readLen = 1 + last - first;

#if I2C_USE_MUTUAL_EXCLUSION
  i2cAcquireBus(ldp->config->i2cp);
#endif
  cacheBufferFlush(writeBuffer, sizeof(writeBuffer));
  status = i2cMasterTransmitTimeout(ldp->config->i2cp, ldp->config->numSlave,
				    writeBuffer, sizeof(writeBuffer),        
				    readAddr, readLen, 100) ;
  cacheBufferInvalidate(&ldp->raw, sizeof(ldp->raw));

  
  if (status != MSG_OK) {
    restartI2c(ldp->config->i2cp);
#if I2C_USE_MUTUAL_EXCLUSION
    i2cReleaseBus(ldp->config->i2cp);
#endif
  }
  
#if I2C_USE_MUTUAL_EXCLUSION
  i2cReleaseBus(ldp->config->i2cp);
#endif
  return status;
}


void lis3mdlGetMag(const Lis3mdlDriver *ldp, Vec3f *mag)
{
  float ratio=0.0f;
  switch (ldp->config->regs.cr[1] & LIS3_CR2_SCALE_16_GAUSS) {
  case LIS3_CR2_SCALE_4_GAUSS : ratio = 6842.0f; break;
  case LIS3_CR2_SCALE_8_GAUSS : ratio = 3421.0f; break;
  case LIS3_CR2_SCALE_12_GAUSS : ratio = 2281.0f; break;
  case LIS3_CR2_SCALE_16_GAUSS : ratio = 1711.0f; break;
  }
  
  for (size_t i=0; i<3; i++) 
    mag->v[i] = ldp->raw.out[i] / ratio;
}

void lis3mdlGetNormalizedMag(const Lis3mdlDriver *ldp, Vec3f *normMag)
{
  lis3mdlGetMag(ldp, normMag);
  const float length = sqrtf(normMag->v[0] * normMag->v[0] +
			     normMag->v[1] * normMag->v[1] +
			     normMag->v[2] * normMag->v[2]);
   for (size_t i=0; i<3; i++)
     normMag->v[i] /= length;
}


float lis3mdlGetTemp(const Lis3mdlDriver *ldp)
{
  return (ldp->raw.temp / 8.0f) + 25.0f;
}

Lis3_Status lis3mdlGetStatus(const Lis3mdlDriver *ldp)
{
  return ldp->raw.status;
}

Lis3_InterruptSource lis3mdlGetIntSource(const Lis3mdlDriver *ldp)
{
   return ldp->raw.intSrc;
}

msg_t lis3mdlWaitUntilDataReady(Lis3mdlDriver *ldp)
{
  msg_t i2cStatus;

  while (true) {
    Lis3_Status lisStatus;
    _Static_assert(sizeof(lisStatus) == 1, "enum Lis3_Status size should be 1 byte");
    
    if ((i2cStatus = readOneRegister(ldp, LIS3_STATUS_REG, &lisStatus)) != MSG_OK)
      return i2cStatus;
    if ((lisStatus & LIS3_STATUS_ZYX_AVAIL))
      break;
    else
      chThdYield();
  }
  
  return MSG_OK;
}


 Lis3_ErrorMask lis3mdlLaunchTest(Lis3mdlDriver *ldp)
{
  // follow procedure described in
  // https://www.st.com/content/ccc/resource/technical/document/application_note/69/d7/05/fe/a9/3e/4b/ea/DM00136626.pdf/files/DM00136626.pdf/jcr:content/translations/en.DM00136626.pdf
  // page 19

  // initialize sensor
  static const float gaussFactor12g = 2281.0f;
  static const int32_t loop = 5;
  
  if (writeOneRegister(ldp, LIS3_CTRL_REG1, 0x1C) != MSG_OK)
    return  LIS3_I2C_ERROR;
  // turn on sensor
  if (writeOneRegister(ldp, LIS3_CTRL_REG2, 0x40) != MSG_OK)
    return  LIS3_I2C_ERROR;
  chThdSleepMilliseconds(20);
  // FS = 12 gauss, continous, odr=80hz
  if (writeOneRegister(ldp, LIS3_CTRL_REG3, 0x0) != MSG_OK)
    return  LIS3_I2C_ERROR;
  // wait for data
  if (lis3mdlWaitUntilDataReady(ldp) != MSG_OK)
    return  LIS3_I2C_ERROR;
  // read and discard
  if (lis3mdlFetch(ldp, LIS3_OUT_X_L, LIS3_OUT_Z_H) != MSG_OK)
    return  LIS3_I2C_ERROR;

  float out_nost[3] = {0.0f, 0.0f, 0.0f};

  // accumulate 5 samples
  for (int32_t i=0; i<loop; i++) {
    // wait for data
    if (lis3mdlWaitUntilDataReady(ldp) != MSG_OK)
      return  LIS3_I2C_ERROR;
    // read and sum
    if (lis3mdlFetch(ldp, LIS3_OUT_X_L, LIS3_OUT_Z_H) != MSG_OK)
      return  LIS3_I2C_ERROR;
    for (int32_t j=0; j<3; j++) {
      out_nost[j] += ldp->raw.out[j];
    }
  }
  // average
  for (int32_t j=0; j<3; j++) {
    out_nost[j] /= loop;
  }

  // enable selftest
  if (writeOneRegister(ldp, LIS3_CTRL_REG1, 0x1D) != MSG_OK)
    return  LIS3_I2C_ERROR;
  chThdSleepMilliseconds(60);

    // wait for data
  if (lis3mdlWaitUntilDataReady(ldp) != MSG_OK)
    return  LIS3_I2C_ERROR;
  // read and discard
  if (lis3mdlFetch(ldp, LIS3_OUT_X_L, LIS3_OUT_Z_H) != MSG_OK)
    return  LIS3_I2C_ERROR;

  float out_st[3] = {0.0f, 0.0f, 0.0f};

  // accumulate 5 samples
  for (int32_t i=0; i<loop; i++) {
    // wait for data
    if (lis3mdlWaitUntilDataReady(ldp) != MSG_OK)
      return  LIS3_I2C_ERROR;
    // read and sum
    if (lis3mdlFetch(ldp, LIS3_OUT_X_L, LIS3_OUT_Z_H) != MSG_OK)
      return  LIS3_I2C_ERROR;
    for (int32_t j=0; j<3; j++) {
      out_st[j] += ldp->raw.out[j];
    }
  }
  // average
  for (int32_t j=0; j<3; j++) {
    out_st[j] /= loop;
  }

  const float diff[3] = {
    fabs((out_st[0]-out_nost[0])) / gaussFactor12g,
    fabs((out_st[1]-out_nost[1])) / gaussFactor12g,
    fabs((out_st[2]-out_nost[2])) / gaussFactor12g
  };

  /* for (int32_t j=0; j<3; j++) { */
  /*   DebugTrace("diff[%u] = %.3f", j, diff[j]); */
  /* } */

  if (writeOneRegister(ldp, LIS3_CTRL_REG1, 0x1C) != MSG_OK)
    return  LIS3_I2C_ERROR;

  if (writeOneRegister(ldp, LIS3_CTRL_REG3, 0x03) != MSG_OK)
    return  LIS3_I2C_ERROR;

  if (lis3mdlStart(ldp, NULL) != MSG_OK)
    return  LIS3_I2C_ERROR;
    
  Lis3_ErrorMask mask = 0;
  if ((diff[0] < 1.0f) || (diff[0] > 3.0f)) {
    DebugTrace("LIS3MDL self test X fail D=%.2f not in [1.0 - 3.0]", diff[0]);
    mask |= LIS3_X_TEST_FAIL;
  }
  if ((diff[1] < 1.0f) || (diff[1] > 3.0f)) {
    DebugTrace("LIS3MDL self test Y fail D=%.2f not in [1.0 - 3.0]", diff[1]);
    mask |= LIS3_Y_TEST_FAIL;
  }
  if ((diff[2] < 0.1f) || (diff[2] > 1.0f)) {
    DebugTrace("LIS3MDL self test Z fail D=%.2f not in [0.1 - 1.0]", diff[2]);
    mask |= LIS3_Z_TEST_FAIL;
  }

  
  return mask;
}


static msg_t sendConfig(Lis3mdlDriver *ldp, const Lis3mdlConfigRegister *regs)
{
  uint8_t CACHE_ALIGNED(writeBuffer[sizeof(regs->cr)+1]) = {[0] =  LIS3_CTRL_REG1 | 0X80};
  memcpy(&writeBuffer[1], &regs->cr, sizeof(regs->cr));

#if I2C_USE_MUTUAL_EXCLUSION
  i2cAcquireBus(ldp->config->i2cp);
#endif
  msg_t status = i2cMasterCacheTransmitTimeout(ldp->config->i2cp, ldp->config->numSlave,
					       writeBuffer, sizeof(regs->cr) +1,        
					       NULL, 0, 100) ;
  
  if (status != MSG_OK) goto i2cError;

  writeBuffer[0] = LIS3_INT_CFG;
  writeBuffer[1] = regs->intCfg;
  status = i2cMasterCacheTransmitTimeout(ldp->config->i2cp, ldp->config->numSlave,
					 writeBuffer, 2,        
					 NULL, 0, 100) ;
  if (status != MSG_OK) goto i2cError;

  writeBuffer[0] = LIS3_INT_THS_L | 0X80;
  writeBuffer[1] = 0xff & regs->threshold >> 8;
  writeBuffer[2] = 0xff & regs->threshold;
  status = i2cMasterCacheTransmitTimeout(ldp->config->i2cp, ldp->config->numSlave,
					 writeBuffer, 3,        
					 NULL, 0, 100) ;
  if (status != MSG_OK) goto i2cError;

#if I2C_USE_MUTUAL_EXCLUSION
  i2cReleaseBus(ldp->config->i2cp);
#endif
  return status;

 i2cError:
  restartI2c(ldp->config->i2cp);
#if I2C_USE_MUTUAL_EXCLUSION
  i2cReleaseBus(ldp->config->i2cp);
#endif
  return status;
}

static msg_t writeOneRegister(Lis3mdlDriver *ldp, const Lis3_RegAddr reg, const uint8_t value)
{
  const uint8_t CACHE_ALIGNED(writeBuffer[]) = {reg, value};
#if I2C_USE_MUTUAL_EXCLUSION
  i2cAcquireBus(ldp->config->i2cp);
#endif
  msg_t status = i2cMasterCacheTransmitTimeout(ldp->config->i2cp, ldp->config->numSlave,
					       writeBuffer, sizeof(writeBuffer),  
					       NULL, 0, 100) ;
  if (status != MSG_OK) {
    restartI2c(ldp->config->i2cp);
  }
  
#if I2C_USE_MUTUAL_EXCLUSION
  i2cReleaseBus(ldp->config->i2cp);
#endif
  return status;
}

static msg_t readOneRegister(Lis3mdlDriver *ldp, const Lis3_RegAddr reg, uint8_t *value)
{
  const uint8_t CACHE_ALIGNED(writeBuffer[]) = {reg};
  uint8_t       CACHE_ALIGNED(readBuffer[32]); // 32 to use all cache line

#if I2C_USE_MUTUAL_EXCLUSION
  i2cAcquireBus(ldp->config->i2cp);
#endif
  msg_t status = i2cMasterCacheTransmitTimeout(ldp->config->i2cp, ldp->config->numSlave,
					       writeBuffer, sizeof(writeBuffer),  
					       readBuffer, 1, 100) ;
  *value = readBuffer[0];
  if (status != MSG_OK) {
    restartI2c(ldp->config->i2cp);
#if I2C_USE_MUTUAL_EXCLUSION
    i2cReleaseBus(ldp->config->i2cp);
#endif
  }
  
  
#if I2C_USE_MUTUAL_EXCLUSION
  i2cReleaseBus(ldp->config->i2cp);
#endif
  return status;
}




