#include "ch.h"
#include "hal.h"
#include "stdutil.h"
#include "i2cPeriphIAQ_Core.h"


static const uint8_t slaveNum = 0x5a;

static void restartI2cPeripheral(I2CDriver *i2cp);


void iaqCoreStart(IaqCoreDriver *hmp, I2CDriver *i2cp,
		 IaqCoreDmaBuffer *dmab)
{
  _Static_assert(sizeof(IaqCoreDmaBuffer) == 9, "IaqCoreDmaBuffer size error");
  chDbgAssert((hmp != NULL) && (i2cp != NULL) && (dmab != NULL), "invalid pointers");
  chDbgAssert(i2cp->config->clock_speed <= 100000, "Max IAQ Core I²C clock is 100Khz");
  
  hmp->i2cp = i2cp;
  hmp->dmab = dmab;
}


IaqCoreStatus	iaqCoreFetchData(IaqCoreDriver *hmp)
{
  do {
    i2cAcquireBus(hmp->i2cp);
    const msg_t i2cStatus = i2cMasterReceiveTimeout(hmp->i2cp, slaveNum,
						    hmp->dmab->data8, sizeof(IaqCoreDmaBuffer),
						    TIME_MS2I(100));
    i2cReleaseBus(hmp->i2cp);
    
    if (i2cStatus != MSG_OK) {
      restartI2cPeripheral(hmp->i2cp);
      return IAQCORE_I2C_ERROR;
    }
    
    
    if (hmp->dmab->status == IAQCORE_OK) {
      hmp->dmab->co2_ppm = __builtin_bswap16(hmp->dmab->co2_ppm);
      hmp->dmab->resistance = __builtin_bswap32(hmp->dmab->resistance);
      hmp->dmab->tvoc_ppb = __builtin_bswap16(hmp->dmab->tvoc_ppb);
    }
  } while (hmp->dmab->status == IAQCORE_BUSY);

  return hmp->dmab->status;
}


static void	restartI2cPeripheral(I2CDriver *i2cp)
{
  const I2CConfig *cfg = i2cp->config;
  i2cStop(i2cp);
  i2cStart(i2cp, cfg);
}


  
