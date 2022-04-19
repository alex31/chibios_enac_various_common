#include <ch.h>
#include <hal.h>

#include "senseair_k96.h"


static const SerialConfig serialCfg = {
  .speed = 115200,
  .cr1 = 0,
  .cr2 = USART_CR2_STOP2_BITS,
  .cr3 = 0
};

static ModbusConfig mconf = {
  .endianness = MODBUS_BIG_ENDIAN,
  .slaveAddr = 0x68
};


void k96Start(SenseairK96Driver *k96d, const SenseairK96Config *cfg)
{
  mconf.sd = cfg->sd;
  sdStart(cfg->sd, &serialCfg);
  modbusStart(&k96d->mdrv, &mconf);
}


ModbusStatus k96Fetch(SenseairK96Driver *k96d)
{
  ModbusStatus modbusStatus = modbusReadRegs(&k96d->mdrv, 15, 1, (uint16_t *) &k96d->status);

  if ((modbusStatus == MODBUS_OK) && (k96d->status == K96_OK)) {
    chThdSleepMilliseconds(5);
    return modbusReadRegs(&k96d->mdrv, 0, 10, k96d->data);
  } else if (modbusStatus != MODBUS_OK)   {
    k96d->status = (K96Status) modbusStatus;
  }
  
  return modbusStatus;  
}
