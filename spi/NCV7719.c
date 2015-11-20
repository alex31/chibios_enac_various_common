#include <ch.h>
#include <hal.h>
#include "globalVar.h"
#include "stdutil.h"
#include "spi/NCV7719.h"
#include "alloca.h"

/* If CPOL is reset, the SCK pin has a low-level idle state. If CPOL is */
/* set, the SCK pin has a high-level idle state.  */


/* If the CPHA (clock */
/* phase) bit is set, the second edge on the SCK pin (falling edge if the */
/* CPOL bit is reset, rising edge if the CPOL bit is set) is the MSBit */
/* capture strobe. Data are latched on the occurrence of the second clock */
/* transition. If the CPHA bit is reset, the first edge on the SCK pin */
/* (falling edge if CPOL bit is set, rising edge if CPOL bit is reset) is */
/* the MSBit capture strobe. Data are latched on the occurrence of the */
/* first clock transition. */


/* BR[2:0]: Baud rate control */
/* 000: fPCLK/2  ||  100: fPCLK/32  */
/* 001: fPCLK/4  ||  101: fPCLK/64  */
/* 010: fPCLK/8  ||  110: fPCLK/128 */
/* 011: fPCLK/16 ||  111: fPCLK/256 */

/*
 * SPI1 configuration structure.
 * clock is Mhz, BR[0-2] bit set prescaler from 2 to 256 so
 * spi freq can grom from 238khz to 42mhz 
 * CPOL clock to 1 when idle
 * CPHA The second clock transition is the first data capture edge
 * not LSBFIRST  MSb transmitted first
 * Speed 328khz, CPHA=1, CPOL=1, 8bits frames,.
 * *
 */

#define NCV7719_OVER_VOLTAGE_LOCKOUT (1<<0)
#define NCV7719_HALFBRIDGE_1_CONF (1<<1)
#define NCV7719_HALFBRIDGE_1_ENABLE (1<<7)
#define NCV7719_UNDERLOAD_SHUTDOWN (1<<13)
#define NCV7719_CHANNELGROUP_SELECT (1<<14)
#define NCV7719_RESET_REGISTER (1 << 15)

//static void printCmd (uint16_t command);


static void gptcb(GPTDriver *gptp);

static const GPTConfig gptcfg = {
  1000000,    /* 1Mhz timer clock.*/
  gptcb,   /* Timer callback.*/
  0
};



static void asyncTransfertCompleted (SPIDriver *spip);

HalfBridgeNCV7719_Status HalfBridgeNCV7719_spiExchange(const NCV7719_Options options)
{
  SPIDriver *driver = hb1.periphCfg->driver;
  const bool fromIsr = options & NCV7719_FromIsr;
  
  if (hb1.periphCfg->driver->state != SPI_READY)
    return NCV7719_AsyncDriverNotReady;

   if (fromIsr) {
      chSysLockFromIsr();
      spiSelectI (driver);
      chSysUnlockFromIsr();
    } else {
     spiSelect (driver);
   }
  
  // reset SSR, HDSEL acordingly, no [underload, overvoltage] shutdown
  hb1.spiCmd.cmd[0] = NCV7719_OVER_VOLTAGE_LOCKOUT;
  hb1.spiCmd.cmd[1] = NCV7719_OVER_VOLTAGE_LOCKOUT | NCV7719_CHANNELGROUP_SELECT;
  if (options & NCV7719_ResetStatusReg) {
    hb1.spiCmd.cmd[0] |= NCV7719_RESET_REGISTER;
    hb1.spiCmd.cmd[1] |= NCV7719_RESET_REGISTER;
  }
  
  // set bits for enabling half bridge and output level
  // channel 1-6
  for (uint32_t i=0; i<6; i++) {
    const HalfBridgeNCV7719_Cmd cmd = ((hb1.cmdBitField >> (i*2)) & 0b11);
    switch (cmd) {
    case NCV7719_HighZ: 
      break;
    case NCV7719_Low: 
      hb1.spiCmd.cmd[0] |= (1 << (i+7)); // enabled
      break;
    case NCV7719_High: 
      hb1.spiCmd.cmd[0] |= (1 << (i+7)); // enabled
      hb1.spiCmd.cmd[0] |= (1 << (i+1)); // output = high
      break;
    default: break;
    }
  }

  // channel 7-8
  for (uint32_t i=6; i<8; i++) {
    const HalfBridgeNCV7719_Cmd cmd = ((hb1.cmdBitField >> (i*2)) & 0b11);
    switch (cmd) {
    case NCV7719_HighZ:
      break;
    case NCV7719_Low:
      hb1.spiCmd.cmd[1] |= 1 << (i+1); // enabled
      break;
    case NCV7719_High:
      hb1.spiCmd.cmd[1] |= 1 << (i+1); // enabled
      hb1.spiCmd.cmd[1] |= 1 << (i-5); // output = high
      break;
    default: break;
    }
  }

  if (hb1.spiCmd.cmd[0] != hb1.lastSpiCmd[0]) {
    hb1.lastSpiCmd[0] = hb1.spiCmd.cmd[0];
    if (fromIsr) {
      chSysLockFromIsr();
      spiStartExchangeI (driver, 1, &hb1.spiCmd.bytes[0] , &hb1.spiStatus.bytes[0]);
      chSysUnlockFromIsr();
    } else {
      spiStartExchange (driver, 1, &hb1.spiCmd.bytes[0] , &hb1.spiStatus.bytes[0]);
    }
  } else if (hb1.spiCmd.cmd[1] != hb1.lastSpiCmd[1]) {
    hb1.lastSpiCmd[1] = hb1.spiCmd.cmd[1];
    if (fromIsr) {
      chSysLockFromIsr();
      spiStartExchangeI (driver, 1, &hb1.spiCmd.bytes[2] , &hb1.spiStatus.bytes[2]);
      chSysUnlockFromIsr();
    } else {
      spiStartExchange (driver, 1, &hb1.spiCmd.bytes[2] , &hb1.spiStatus.bytes[2]);
    }
  } 
  
  return hb1.statusBitField;
}

void HalfBridgeNCV7719_init (void)
{
  hb1.cmdBitField = 0;
  hb1.statusBitField=0;
  hb1.lastSpiCmd[0] =0xffff;
  hb1.lastSpiCmd[1] =0xffff;
  hb1.spiCfg->end_cb = &asyncTransfertCompleted;
  
  // disable chip output
  palSetPad (hb1.spiCfg->ssport, hb1.spiCfg->sspad);
  //  palSetPad (hb1.enGpio, hb1.enPin);

  palSetPadMode (hb1.spiCfg->ssport, hb1.spiCfg->sspad, PAL_MODE_OUTPUT_PUSHPULL);
  
  palSetPadMode (hb1.periphCfg->misoGpio, hb1.periphCfg->misoPin, 
		 PAL_MODE_ALTERNATE(hb1.periphCfg->alternateFunction));
  palSetPadMode (hb1.periphCfg->mosiGpio, hb1.periphCfg->mosiPin, 
		 PAL_MODE_ALTERNATE(hb1.periphCfg->alternateFunction));
  palSetPadMode (hb1.periphCfg->sckGpio, hb1.periphCfg->sckPin, 
		 PAL_MODE_ALTERNATE(hb1.periphCfg->alternateFunction));


  spiStart (hb1.periphCfg->driver, hb1.spiCfg);
  gptStart (hb1.periphCfg->timer, &gptcfg);
}

HalfBridgeNCV7719_Status HalfBridgeNCV7719_toggleHalfBridge (const uint32_t outIndex, 
							     const NCV7719_Options options)
{
  // on utilise les 16  bits du mot de 16 bits cmdBitField
  const uint32_t idx = outIndex-1;
  if (idx > 7)
    return NCV7719_IndexErrorMask;
  
  // on commence par recuperer le bit concerné
  const uint32_t bitmask = 0b01 << (idx*2);

  // ou exclusif avec le masque pour inverser le bit concerné
  hb1.cmdBitField ^= bitmask;
  
  if (options & NCV7719_DoSpiExchange) {
    HalfBridgeNCV7719_spiExchange (options);
  }
  return hb1.statusBitField;
}



HalfBridgeNCV7719_Status HalfBridgeNCV7719_setHalfBridge (const uint32_t outIndex, 
							  HalfBridgeNCV7719_Cmd cmd, 
							  const NCV7719_Options options)
{
  // on utilise les 16  bits du mot de 16 bits cmdBitField
  const uint32_t idx = outIndex-1;
  if (idx > 7)
    return NCV7719_IndexErrorMask;
  
  // on commence par effacer les deux bits concernés
  const uint32_t eraseBitmask = 0b11 << (idx*2);
  hb1.cmdBitField &= ~eraseBitmask;

  // puis on fait un ou logique avec le paramètre en entrée
  hb1.cmdBitField |= (cmd << (idx*2));
  //  printCmd (hb1.cmdBitField);
  if (options & NCV7719_DoSpiExchange) {
    HalfBridgeNCV7719_spiExchange (options);
  }
  return hb1.statusBitField;
}

HalfBridgeNCV7719_Status HalfBridgeNCV7719_getStatus (void)
{
  return hb1.statusBitField;
}


HalfBridgeNCV7719_Status HalfBridgeNCV7719_setBridge (const uint32_t bridgeIndex,
						      const BridgeNCV7719_Cmd cmd,
						      const NCV7719_Options options)
{
  HalfBridgeNCV7719_Status st=0;
  const uint32_t idx = bridgeIndex-1;
  const  NCV7719_Options fromIsr = options & NCV7719_FromIsr;
  const  NCV7719_Options doSpiExchange = options & NCV7719_DoSpiExchange;

  if (idx > 3)
    return NCV7719_IndexErrorMask;


  const uint32_t index1 = (idx*2)+1;
  const uint32_t index2 = index1+1;

  switch (cmd) {
  case NCV7719_Bridge_HighZ  :
    HalfBridgeNCV7719_setHalfBridge (index1, NCV7719_HighZ, fromIsr);
    st=HalfBridgeNCV7719_setHalfBridge (index2, NCV7719_HighZ, fromIsr | doSpiExchange);
    break;
  case NCV7719_Bridge_Short  :
    HalfBridgeNCV7719_setHalfBridge (index1,  NCV7719_Low, fromIsr);
    st=HalfBridgeNCV7719_setHalfBridge (index2,  NCV7719_Low, fromIsr | doSpiExchange);
    break;
  case NCV7719_Forward  :
    HalfBridgeNCV7719_setHalfBridge (index1, NCV7719_Low, fromIsr);
    st=HalfBridgeNCV7719_setHalfBridge (index2, NCV7719_High, fromIsr | doSpiExchange);
    break;
  case NCV7719_Reverse  :
    HalfBridgeNCV7719_setHalfBridge (index1, NCV7719_High, fromIsr);
    st=HalfBridgeNCV7719_setHalfBridge (index2, NCV7719_Low, fromIsr | doSpiExchange);
    break;
  }

  return st;
}

static void asyncTransfertCompleted (SPIDriver *spip)
{
  //  DebugTrace ("micro second betwen T1 T2 = %d µs", (secondTransactionTS-firstTransactionTS)/168);

  
  // we combine status error of both groups and eliminate channels output and config
  const uint16_t outAndConfBits = (hb1.spiStatus.status[0] | hb1.spiStatus.status[1]) & 
    0b0001111111111110;

  const  HalfBridgeNCV7719_Status allZero = 
    outAndConfBits ? 0 : NCV7719_StatusAllZeroMask;
  const  HalfBridgeNCV7719_Status allOne = 
    (outAndConfBits == 0b0001111111111110) ? NCV7719_StatusAllOneMask : 0;

  hb1.statusBitField = (hb1.spiStatus.status[0] | hb1.spiStatus.status[1]) &
    (NCV7719_ThermalWarningMask |
     NCV7719_IndexErrorMask |
     NCV7719_UnderloadMask |
     NCV7719_UnderOverVoltageMask | 
     NCV7719_OverCurrentMask | 
     NCV7719_ThermalShutdownMask);
  hb1.statusBitField |= (allZero | allOne);

  chSysLockFromIsr ();
  spiUnselectI(spip); 
  // if there is another word to transfert
  if ((hb1.spiCmd.cmd[1] != hb1.lastSpiCmd[1]) ||
      (hb1.spiCmd.cmd[0] != hb1.lastSpiCmd[0])) {
    if (hb1.periphCfg->timer->state == GPT_READY)  {
      gptStartOneShotI(hb1.periphCfg->timer, 5); // 5 microsecond with enable at HIGH
    }
  }
  chSysUnlockFromIsr ();
}


static void gptcb(GPTDriver *gptp)
{
  (void) gptp;
  
  if ((hb1.spiCmd.cmd[0] == hb1.lastSpiCmd[0]) && (hb1.spiCmd.cmd[1] == hb1.lastSpiCmd[1]))
    return;

  SPIDriver *driver = hb1.periphCfg->driver;
  if (driver->state != SPI_READY) {
    // if not ready, reschedule
    if (hb1.periphCfg->timer->state == GPT_READY)  {
      gptStartOneShotI(hb1.periphCfg->timer, 10); // wait 10 microsecond 
    }
    return;
  }



  chSysLockFromIsr();
  spiSelectI (driver);
  
  if (hb1.spiCmd.cmd[0] != hb1.lastSpiCmd[0]) {
    hb1.lastSpiCmd[0] = hb1.spiCmd.cmd[0];
    spiStartExchangeI (driver, 1, &hb1.spiCmd.bytes[0] , &hb1.spiStatus.bytes[0]);
  } else if (hb1.spiCmd.cmd[1] != hb1.lastSpiCmd[1]) {
    hb1.lastSpiCmd[1] = hb1.spiCmd.cmd[1];
    spiStartExchangeI (driver, 1, &hb1.spiCmd.bytes[2] , &hb1.spiStatus.bytes[2]);
  }
  chSysUnlockFromIsr();
}


/*
static void printCmd (uint16_t command)
{
  chprintf (chp, "command = ");
  for (int i=0; i<8; i++) {
    const uint16_t hbCommand = (command >> (i*2)) & 0b11;
    switch (hbCommand) {
    case NCV7719_HighZ : chprintf (chp, "%d:Z ", i+1); break;
    case NCV7719_Reserved : chprintf (chp, "%d:Reserved ", i+1); break;
    case NCV7719_Low : chprintf (chp, "%d:L ", i+1); break;
    case NCV7719_High : chprintf (chp, "%d:H ", i+1); break;
    }
    
  }
  chprintf (chp, "\r\n");
}
*/
