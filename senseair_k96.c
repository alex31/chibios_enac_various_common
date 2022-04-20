#include <ch.h>
#include <hal.h>
#include <string.h>

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

ModbusStatus k96FetchInfo(SenseairK96Driver *k96d, SenseairK96Info *k96i)
{
  ModbusStatus st;
  memset(k96i, 0, sizeof(SenseairK96Info));
  st = modbusReadRam(&k96d->mdrv, 0x20, sizeof(k96i->address), &k96i->address);
  if (st != MODBUS_OK) goto exit;
  chThdSleepMilliseconds(5);
  st = modbusReadRam(&k96d->mdrv, 0x21, sizeof(k96i->pgaGain), &k96i->pgaGain);
  if (st != MODBUS_OK) goto exit;
  chThdSleepMilliseconds(5);
  st = modbusReadRam(&k96d->mdrv, 0x28, sizeof(k96i->meterId), &k96i->meterId);
  if (st != MODBUS_OK) goto exit;
  chThdSleepMilliseconds(5);
  st = modbusReadRam(&k96d->mdrv, 0x2f, sizeof(k96i->mapVersion), &k96i->mapVersion);
  if (st != MODBUS_OK) goto exit;
  chThdSleepMilliseconds(5);
  st = modbusReadRam(&k96d->mdrv, 0x62, sizeof(k96i->fwType), &k96i->fwType);
  if (st != MODBUS_OK) goto exit;
  chThdSleepMilliseconds(5);
  st = modbusReadRam(&k96d->mdrv, 0x63, sizeof(k96i->revMain), &k96i->revMain);
  if (st != MODBUS_OK) goto exit;
  chThdSleepMilliseconds(5);
  st = modbusReadRam(&k96d->mdrv, 0x64, sizeof(k96i->revSub), &k96i->revSub);
  if (st != MODBUS_OK) goto exit;
  chThdSleepMilliseconds(5);
  st = modbusReadRam(&k96d->mdrv, 0x65, sizeof(k96i->revAux), &k96i->revAux);
  if (st != MODBUS_OK) goto exit;
  chThdSleepMilliseconds(5);
  st = modbusReadRam(&k96d->mdrv, 0x1e4, sizeof(k96i->frac), &k96i->frac);
  chThdSleepMilliseconds(5);
 exit:
  return st;
}
