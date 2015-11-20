#pragma once

#include <ch.h>
#include <hal.h>

typedef enum  {NCV7719_HighZ=0b00, NCV7719_Reserved=0b01, NCV7719_Low=0b10, NCV7719_High=0b11}
  HalfBridgeNCV7719_Cmd;

typedef enum  {NCV7719_Bridge_HighZ, NCV7719_Bridge_Short, NCV7719_Forward, NCV7719_Reverse}
  BridgeNCV7719_Cmd;

typedef enum  {
  NCV7719_noOptions=0,
  NCV7719_FromIsr=1<<0,
  NCV7719_DoSpiExchange=1<<1,
  NCV7719_ResetStatusReg=1<<2
}  NCV7719_Options;

typedef enum {
  NCV7719_ThermalWarningMask=1<<0, 
  NCV7719_IndexErrorMask=1<<1,
  NCV7719_StatusAllZeroMask=1<<2,
  NCV7719_StatusAllOneMask=1<<3,
  NCV7719_UnderloadMask=1<<13,
  NCV7719_UnderOverVoltageMask=1<<14, 
  NCV7719_OverCurrentMask=1<<15, 
  NCV7719_ThermalShutdownMask=1<<16,
  NCV7719_AsyncDriverNotReady=1<<17
}  HalfBridgeNCV7719_Status ;

typedef struct  {
  SPIDriver       *driver;
  GPTDriver	  *timer;
  GPIO_TypeDef    *misoGpio;
  GPIO_TypeDef    *mosiGpio;
  GPIO_TypeDef    *sckGpio;
  uint32_t        misoPin;
  uint32_t        mosiPin;
  uint32_t        sckPin;
  uint8_t         alternateFunction;
} SpiPeriphConfig ;

typedef  union  {
  uint16_t cmd[2];
  uint16_t status[2];
  uint8_t  bytes[4];
} SpiData;


typedef struct {
  SPIConfig *spiCfg;
  const     SpiPeriphConfig *periphCfg;
  volatile uint32_t  statusBitField;
  uint16_t  lastSpiCmd[2];
  uint16_t  cmdBitField;
  SpiData   spiCmd;
  SpiData   spiStatus;
} HalfBridgeNCV7719;

#include "NCV7719_conf.h"

void HalfBridgeNCV7719_init (void);
HalfBridgeNCV7719_Status HalfBridgeNCV7719_setHalfBridge (const uint32_t outIndex, // 1-8 
							  HalfBridgeNCV7719_Cmd cmd, 
							  const NCV7719_Options options);
HalfBridgeNCV7719_Status HalfBridgeNCV7719_toggleHalfBridge (const uint32_t outIndex, 
							     const NCV7719_Options options);
HalfBridgeNCV7719_Status HalfBridgeNCV7719_getStatus (void);
HalfBridgeNCV7719_Status HalfBridgeNCV7719_spiExchange (const NCV7719_Options options);

HalfBridgeNCV7719_Status HalfBridgeNCV7719_setBridge (const uint32_t bridgeIndex, // 1-4 
						      const BridgeNCV7719_Cmd cmd,
						      NCV7719_Options options);


