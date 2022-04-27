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
  .timeBetweenOp = TIME_MS2I(5),
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
   
    return modbusReadRegs(&k96d->mdrv, 0, 10, k96d->data);
  } else if (modbusStatus != MODBUS_OK)   {
    k96d->status = (K96Status) modbusStatus;
  }
  
  return modbusStatus;  
}

ModbusStatus k96FetchInfo(SenseairK96Driver *k96d, SenseairK96Info *k96i)
{
  ModbusStatus st;
  *k96i = (SenseairK96Info) {0};
  st = modbusReadRam(&k96d->mdrv, 0x20, sizeof(k96i->address), &k96i->address);
  if (st != MODBUS_OK) goto exit;
 
  st = modbusReadRam(&k96d->mdrv, 0x21, sizeof(k96i->pgaGain), &k96i->pgaGain);
  if (st != MODBUS_OK) goto exit;
 
  st = modbusReadRam(&k96d->mdrv, 0x28, sizeof(k96i->meterId), &k96i->meterId);
  if (st != MODBUS_OK) goto exit;
 
  st = modbusReadRam(&k96d->mdrv, 0x2f, sizeof(k96i->mapVersion), &k96i->mapVersion);
  if (st != MODBUS_OK) goto exit;
 
  st = modbusReadRam(&k96d->mdrv, 0x62, sizeof(k96i->fwType), &k96i->fwType);
  if (st != MODBUS_OK) goto exit;
 
  st = modbusReadRam(&k96d->mdrv, 0x63, sizeof(k96i->revMain), &k96i->revMain);
  if (st != MODBUS_OK) goto exit;
 
  st = modbusReadRam(&k96d->mdrv, 0x64, sizeof(k96i->revSub), &k96i->revSub);
  if (st != MODBUS_OK) goto exit;
 
  st = modbusReadRam(&k96d->mdrv, 0x65, sizeof(k96i->revAux), &k96i->revAux);
  if (st != MODBUS_OK) goto exit;
 
  st = modbusReadRam(&k96d->mdrv, 0x1e4, sizeof(k96i->frac), &k96i->frac);
 
 exit:
  return st;
}



#define BS32(f) _Static_assert(sizeof(f) == 4); f = __builtin_bswap32(f)
#define BS16(f) _Static_assert(sizeof(f) == 2); f = __builtin_bswap16(f)
#define BS32_16(f) _Static_assert(sizeof(f) == 6); BS32(f.s32); BS16(f.u16)

ModbusStatus k96FetchIrRaw(SenseairK96Driver *k96d, SenseairK96IrRaw *k96ir)
{
  typedef struct {
    int32_t s32;
    uint16_t u16;
  }  __attribute__ ((__packed__)) S32_16;

  struct {
     int32_t lplIrLow;
     int32_t lplIrHigh;
     int32_t splIrLow;
     int32_t splIrHigh;
     int32_t lplSignal;
     S32_16  lplSignalFiltered;
     uint8_t dummy[6];
     int32_t splSignal;
     S32_16  splSignalFiltered;
  } __attribute__ ((__packed__)) ad170 = {0};
  _Static_assert(sizeof(ad170) == 0x19A - 0x170);

  struct {
    int32_t mplIrLow;
    int32_t mplIrHigh;
  } __attribute__ ((__packed__)) ad350 = {0};
  _Static_assert(sizeof(ad350) == 0x358 - 0x350);
  
  struct {
    int32_t mplSignal;
    S32_16  mplSignalFiltered;
  } __attribute__ ((__packed__)) ad360 = {0};
  _Static_assert(sizeof(ad360) == 0x36A - 0x360);
  
  ModbusStatus st;
  *k96ir = (SenseairK96IrRaw) {0};

  st = modbusReadRam(&k96d->mdrv, 0x170, sizeof(ad170), &ad170);
  if (st != MODBUS_OK) goto exit;
   
  st = modbusReadRam(&k96d->mdrv, 0x350, sizeof(ad350), &ad350);
  if (st != MODBUS_OK) goto exit;
   
  st = modbusReadRam(&k96d->mdrv, 0x360, sizeof(ad360), &ad360);
   
  BS32(ad170.lplIrLow);
  BS32(ad170.lplIrHigh);	       
  BS32(ad170.splIrLow);
  BS32(ad170.splIrHigh);
  BS32(ad170.lplSignal);
  BS32_16(ad170.lplSignalFiltered);
  BS32(ad170.splSignal);       
  BS32_16(ad170.splSignalFiltered);
  BS32(ad350.mplIrLow);
  BS32(ad350.mplIrHigh);	       
  BS32(ad360.mplSignal);       
  BS32_16(ad360.mplSignalFiltered);

  k96ir->lpl.irLow = ad170.lplIrLow;
  k96ir->lpl.irHigh = ad170.lplIrHigh;
  k96ir->lpl.irSignal = ad170.lplSignal;
  k96ir->lpl.irSignalFiltered = ad170.lplSignalFiltered.s32 + (ad170.lplSignalFiltered.u16 / 65536.0d);

  k96ir->spl.irLow = ad170.splIrLow;
  k96ir->spl.irHigh = ad170.splIrHigh;
  k96ir->spl.irSignal = ad170.splSignal;
  k96ir->spl.irSignalFiltered = ad170.splSignalFiltered.s32 + (ad170.splSignalFiltered.u16 / 65536.0d);

  k96ir->mpl.irLow = ad350.mplIrLow;
  k96ir->mpl.irHigh = ad350.mplIrHigh;
  k96ir->mpl.irSignal = ad360.mplSignal;
  k96ir->mpl.irSignalFiltered = ad360.mplSignalFiltered.s32 + (ad360.mplSignalFiltered.u16 / 65536.0d);

 
 exit:
  return st;
}

  
