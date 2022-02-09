#include "ch.h"
#include "hal.h"
#include "stdutil.h"
#include <string.h>
#include "i2cPeriphHM3301.h"




static const uint8_t slaveNum = 0x40;
static const uint8_t i2cMode = 0x88;



void hm3301Start(Hm3301Driver *hmp, I2CDriver *i2cp,
		 Hm3301DmaBuffer *dmab)
{
  _Static_assert(sizeof(Hm3301DmaBuffer) == 29, "Hm3301DmaBuffer size error");

  chDbgAssert((hmp != NULL) && (i2cp != NULL) && (dmab != NULL), "invalid pointers");
  memset(dmab, 0, sizeof(Hm3301DmaBuffer));
  hmp->i2cp = i2cp;
  hmp->dmab = dmab;
}

Hm3301Status         hm3301StartContinuous(Hm3301Driver *hmp)
{
  i2cAcquireBus(hmp->i2cp);
  const msg_t i2cStatus = i2cMasterTransmitTimeout(hmp->i2cp, slaveNum,
						   &i2cMode,  sizeof(i2cMode),
						   NULL,      0,
						   TIME_MS2I(100));
  
  i2cReleaseBus(hmp->i2cp);
  return i2cStatus == MSG_OK ? HM3301_OK : HM3301_I2C_ERROR;
}



Hm3301Status         hm3301FetchData(Hm3301Driver *hmp)
{
  i2cAcquireBus(hmp->i2cp);
  const msg_t i2cStatus = i2cMasterReceiveTimeout(hmp->i2cp, slaveNum,
						  hmp->dmab->data8, sizeof(Hm3301DmaBuffer),
						  TIME_MS2I(100));
  i2cReleaseBus(hmp->i2cp);
  if (i2cStatus != MSG_OK)
    return HM3301_I2C_ERROR;

  uint8_t calcChksum = 0;
  for (size_t i=0; i < (sizeof(Hm3301DmaBuffer) - 1U); i++)
    calcChksum += hmp->dmab->data8[i];

  if (hmp->dmab->data8[28] != calcChksum)
    return HM3301_CHKSUM_ERROR;

  // data is given in big endian (MSB first) 16 bits fields
  // first field is reserved
  for (size_t i=1; i < ARRAY_LEN(hmp->dmab->data16); i++)
    hmp->dmab->data16[i] = __builtin_bswap16(hmp->dmab->data16[i]);
  
  return HM3301_OK;
}
