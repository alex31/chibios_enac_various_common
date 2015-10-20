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

HalfBridgeNCV7719_Status HalfBridgeNCV7719_spiExchange(HalfBridgeNCV7719 *hb)
{
  typedef union  {
    uint16_t cmd[2];
    uint16_t status[2];
    uint8_t  bytes[4];
  } SpiData;

  SPIDriver *driver = hb->periphCfg->driver;
  const SPIConfig *spiCfg =  hb->spiCfg;

  spiStart (driver, spiCfg);
  spiAcquireBus (driver);
  spiSelect (driver);
  
  // reset SSR, HDSEL acordingly, no [underload, overvoltage] shutdown
  SpiData spiCmd = {.cmd = {NCV7719_OVER_VOLTAGE_LOCKOUT,
			    NCV7719_OVER_VOLTAGE_LOCKOUT | NCV7719_CHANNELGROUP_SELECT}};
  SpiData spiStatus = {.status = {0,0}};
  
  // set bits for enabling half bridge and output level
  // channel 1-6
  for (uint32_t i=0; i<6; i++) {
    const HalfBridgeNCV7719_Cmd cmd = ((hb->cmdBitField >> (i*2)) & 0b11);
    switch (cmd) {
    case NCV7719_HighZ: 
      break;
    case NCV7719_Low: 
      spiCmd.cmd[0] |= (1 << (i+7)); // enabled
      break;
    case NCV7719_High: 
      spiCmd.cmd[0] |= (1 << (i+7)); // enabled
      spiCmd.cmd[0] |= (1 << (i+1)); // output = high
      break;
    default: break;
    }
  }

  // channel 7-8
  for (uint32_t i=6; i<8; i++) {
    const HalfBridgeNCV7719_Cmd cmd = ((hb->cmdBitField >> (i*2)) & 0b11);
    switch (cmd) {
    case NCV7719_HighZ: 
      break;
    case NCV7719_Low: 
      spiCmd.cmd[1] |= 1 << (i+1); // enabled
      break;
    case NCV7719_High: 
      spiCmd.cmd[1] |= 1 << (i+1); // enabled
      spiCmd.cmd[1] |= 1 << (i-5); // output = high
      break;
    default: break;
    }
  }

  // in one pass : should be tested
  //spiExchange (driver, sizeof(SpiData), spiCmd.bytes , spiStatus.bytes);

  //  const halrtcnt_t firstTransactionTS = halGetCounterValue();
  if (spiCmd.cmd[0] != hb->lastSpiCmd[0]) {
    spiExchange (driver, 1, &spiCmd.bytes[0] , &spiStatus.bytes[0]);
  }

  if ((spiCmd.cmd[0] != hb->lastSpiCmd[0]) && (spiCmd.cmd[1] != hb->lastSpiCmd[1])) {
    spiUnselect (driver);
    // wait 5 microsecond
    const halrtcnt_t now =  halGetCounterValue();
    const halrtcnt_t until =  now+US2RTT(4); // minimum time between two transactions is 5 microseconds
    while (halIsCounterWithin (now, until)) {};
    spiSelect (driver);
  }
  
  //  const halrtcnt_t secondTransactionTS = halGetCounterValue();
  if (spiCmd.cmd[1] != hb->lastSpiCmd[1]) {
    spiExchange (driver, 1, &spiCmd.bytes[2] , &spiStatus.bytes[2]);
  }

  /*  for (int i=0; i<= 1; i++) {
    if (hb->lastSpiCmd[i] != spiCmd.cmd[i]) {
      DebugTrace ("send[%d] 0x%x => receive 0x%x", i,
		  spiCmd.cmd[i], spiStatus.status[i]);
    }
  }
  */
  
  hb->lastSpiCmd[0] = spiCmd.cmd[0];
  hb->lastSpiCmd[1] = spiCmd.cmd[1];
  //  DebugTrace ("micro second betwen T1 T2 = %d µs", (secondTransactionTS-firstTransactionTS)/168);

  
  // we combine status error of both groups and eliminate channels output and config
  const uint16_t outAndConfBits = (spiStatus.status[0] | spiStatus.status[1]) & 
    0b0001111111111110;

  const  HalfBridgeNCV7719_Status allZero = 
    outAndConfBits ? 0 : NCV7719_StatusAllZeroMask;
  const  HalfBridgeNCV7719_Status allOne = 
    (outAndConfBits == 0b0001111111111110) ? NCV7719_StatusAllOneMask : 0;

  hb->statusBitField = (spiStatus.status[0] | spiStatus.status[1]) &
    (NCV7719_ThermalWarningMask |
     NCV7719_IndexErrorMask |
     NCV7719_UnderloadMask |
     NCV7719_UnderOverVoltageMask | 
     NCV7719_OverCurrentMask | 
     NCV7719_ThermalShutdownMask |
     allZero | allOne);
  
  spiUnselect(driver); 
  spiReleaseBus(driver);
  spiStop(driver);

  return hb->statusBitField;
}

HalfBridgeNCV7719_Status HalfBridgeNCV7719_spiGetStatus (HalfBridgeNCV7719 *hb)
{
  typedef union  {
    uint16_t cmd[2];
    uint16_t status[2];
    uint8_t  bytes[4];
  } SpiData;

  SpiData spiCmd = {.cmd = {hb->lastSpiCmd[0], hb->lastSpiCmd[1]}};
  SpiData spiStatus = {.status = {0,0}};

  
  SPIDriver *driver = hb->periphCfg->driver;
  const SPIConfig *spiCfg =  hb->spiCfg;
  
  spiStart (driver, spiCfg);
  spiAcquireBus (driver);
  spiSelect (driver);
  
  
  spiExchange (driver, 1,  &spiCmd.bytes[0],  &spiStatus.bytes[0]);
  spiUnselect (driver);
  // wait 5 microsecond
  const halrtcnt_t now =  halGetCounterValue();
  const halrtcnt_t until =  now+US2RTT(4); // minimum time between two transactions is 5 microseconds
  while (halIsCounterWithin (now, until)) {};
  spiSelect (driver);
  
  spiExchange (driver, 1,  &spiCmd.bytes[2],  &spiStatus.bytes[2]);
  
  const uint16_t outAndConfBits = (spiStatus.status[0] | spiStatus.status[1]) & 
    0b0001111111111110;
  
  const  HalfBridgeNCV7719_Status allZero = 
    outAndConfBits ? 0 : NCV7719_StatusAllZeroMask;
  const  HalfBridgeNCV7719_Status allOne = 
    (outAndConfBits == 0b0001111111111110) ? NCV7719_StatusAllOneMask : 0;
  
  hb->statusBitField = (spiStatus.status[0] | spiStatus.status[1]) &
    (NCV7719_ThermalWarningMask |
     NCV7719_IndexErrorMask |
     NCV7719_UnderloadMask |
     NCV7719_UnderOverVoltageMask | 
     NCV7719_OverCurrentMask | 
     NCV7719_ThermalShutdownMask |
     allZero | allOne);
  
  spiUnselect(driver); 
  spiReleaseBus(driver);
  spiStop(driver);
  
  return hb->statusBitField;
}


void HalfBridgeNCV7719_init (HalfBridgeNCV7719 *hb)
{
  hb->cmdBitField = 0;
  hb->statusBitField=0;
  hb->lastSpiCmd[0] =0xffff;
  hb->lastSpiCmd[1] =0xffff;
  chMtxInit(&(hb->mtx));
  HalfBridgeNCV7719_spiExchange (hb);
  
  // disable chip output
  palSetPad (hb->spiCfg->ssport, hb->spiCfg->sspad);
  //  palSetPad (hb->enGpio, hb->enPin);
}

HalfBridgeNCV7719_Status HalfBridgeNCV7719_toggleHalfBridge (HalfBridgeNCV7719 *hb, const uint32_t outIndex, 
							 bool doSpiExch)
{
  // on utilise les 16  bits du mot de 16 bits cmdBitField
  const uint32_t idx = outIndex-1;
  if (idx > 7)
    return NCV7719_IndexErrorMask;
  
  chMtxLock (&(hb->mtx));
  // on commence par recuperer le bit concerné
  const uint32_t bitmask = 0b01 << (idx*2);

  // ou exclusif avec le masque pour inverser le bit concerné
  hb->cmdBitField ^= bitmask;
  
  if (doSpiExch) {
    HalfBridgeNCV7719_spiExchange (hb);
  }
  chMtxUnlock();
  return hb->statusBitField;
}



HalfBridgeNCV7719_Status HalfBridgeNCV7719_setHalfBridge (HalfBridgeNCV7719 *hb, 
						      const uint32_t outIndex, 
						      HalfBridgeNCV7719_Cmd cmd, bool doSpiExch)
{
  // on utilise les 16  bits du mot de 16 bits cmdBitField
  const uint32_t idx = outIndex-1;
  if (idx > 7)
    return NCV7719_IndexErrorMask;
  
  chMtxLock (&(hb->mtx));
  // on commence par effacer les deux bits concernés
  const uint32_t eraseBitmask = 0b11 << (idx*2);
  hb->cmdBitField &= ~eraseBitmask;

  // puis on fait un ou logique avec le paramètre en entrée
  hb->cmdBitField |= (cmd << (idx*2));
  //  printCmd (hb->cmdBitField);
  if (doSpiExch) {
    HalfBridgeNCV7719_spiExchange (hb);
  }
  chMtxUnlock();
  return hb->statusBitField;
}

HalfBridgeNCV7719_Status HalfBridgeNCV7719_getStatus (HalfBridgeNCV7719 *hb)
{
  HalfBridgeNCV7719_spiGetStatus (hb);
  return hb->statusBitField;
}


HalfBridgeNCV7719_Status HalfBridgeNCV7719_setBridge (HalfBridgeNCV7719 *hb,
						      const uint32_t bridgeIndex, 
						      BridgeNCV7719_Cmd cmd)
{
  HalfBridgeNCV7719_Status st=0;
  const uint32_t idx = bridgeIndex-1;
  if (idx > 3)
    return NCV7719_IndexErrorMask;

  const uint32_t index1 = (idx*2)+1;
  const uint32_t index2 = index1+1;

  switch (cmd) {
  case NCV7719_Bridge_HighZ  :
    HalfBridgeNCV7719_setHalfBridge (hb, index1, NCV7719_HighZ, false);
    st=HalfBridgeNCV7719_setHalfBridge (hb, index2, NCV7719_HighZ, true);
    break;
  case NCV7719_Bridge_Short  :
    HalfBridgeNCV7719_setHalfBridge (hb, index1,  NCV7719_Low, false);
    st=HalfBridgeNCV7719_setHalfBridge (hb, index2,  NCV7719_Low, true);
    break;
  case NCV7719_Forward  :
    HalfBridgeNCV7719_setHalfBridge (hb, index1, NCV7719_Low, false);
    st=HalfBridgeNCV7719_setHalfBridge (hb, index2, NCV7719_High, true);
    break;
  case NCV7719_Reverse  :
    HalfBridgeNCV7719_setHalfBridge (hb, index1, NCV7719_High, false);
    st=HalfBridgeNCV7719_setHalfBridge (hb, index2, NCV7719_Low, true);
    break;
  }

  return st;
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
