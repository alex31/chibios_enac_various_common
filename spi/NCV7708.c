#include <ch.h>
#include <hal.h>
#include "globalVar.h"
#include "printf.h"
#include "stdutil.h"
#include "NCV7708.h"


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
/* 000: fPCLK/2 100: fPCLK/32 */
/* 001: fPCLK/4 101: fPCLK/64 */
/* 010: fPCLK/8 110: fPCLK/128 */
/* 011: fPCLK/16 111: fPCLK/256 */

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


static void spiPrologue (SPIDriver * spid, const SPIConfig *spiCfg);
static void spiEpilogue (SPIDriver * spid);
static HalfBridgePortMask get_NCV7708_status (const HalfBridgeCommandMask latchesBits, 
					      const HalfBridgeCommandMask hbstatus);
static HalfBridgeCommandMask get_NCV7708_latchesBits (const HalfBridgePortMask outMask);



typedef  struct {
  HalfBridgeCommandMask	     options; // control option (over/under voltage/current)
  HalfBridgePortMask out;  // bit field for the 6 half bridge
  HalfBridgePortMask err;  // error field for the 6 half bridge
  
  bool_t overIntensity;
} HalfBridgeData;

static HalfBridgeData halfBridgeData[HALF_BRIDGE_NUMBER] = {{0}};
static HalfBridgeOutputMode outputMode = MODE_PUSHPULL;

typedef struct {
  HalfBridgeIndex hbIdx;
  HalfBridgePortMask hbMsk;
} HalfBridgePort;

const HalfBridgePort halfBridgePort[] = {
    {.hbIdx = HALF_BRIDGE_1, .hbMsk = 1 << BUT_B_3},
    {.hbIdx = HALF_BRIDGE_1, .hbMsk = 1 << BUT_D_1},
    {.hbIdx = HALF_BRIDGE_1, .hbMsk = 1 << BUT_G_1},
    {.hbIdx = HALF_BRIDGE_1, .hbMsk = 1 << BUT_AR_2},
    {.hbIdx = HALF_BRIDGE_1, .hbMsk = 1 << BUT_AV_2},
    {.hbIdx = HALF_BRIDGE_1, .hbMsk = 1 << BUT_H_3},

    {.hbIdx = HALF_BRIDGE_2, .hbMsk = 1 << (BUT_F_5-BUT_F_5)},
    {.hbIdx = HALF_BRIDGE_2, .hbMsk = 1 << (BUT_H_4-BUT_F_5)},
    {.hbIdx = HALF_BRIDGE_2, .hbMsk = 1 << (BUT_B_4-BUT_F_5)},
    {.hbIdx = HALF_BRIDGE_2, .hbMsk = 1 << (BUT_O_5-BUT_F_5)},
    {.hbIdx = HALF_BRIDGE_2, .hbMsk = 1 << (L2_SHORTCUT-BUT_F_5)},
    {.hbIdx = HALF_BRIDGE_2, .hbMsk = 1 << (L1_OPENCIRCUIT-BUT_F_5)}
};

typedef struct {
  HalfBridgeCommandMask off_L;
  HalfBridgeCommandMask on_H;
} HalfBridgeOutputModeMask;

const HalfBridgeOutputModeMask halfBridgeOutputModeMask[MODE_NUMBER] = {
  // MODE_PUSHPULL
  {
    .off_L = 0xffff,
    .on_H = 0xffff
  },
   // MODE_OPENDRAIN_PULLUP
  {
    .off_L = 0xffff,
    .on_H = 0
  },
   //MODE_OPENDRAIN_PULLDOWN
  {
    .off_L = 0,
    .on_H = 0xffff
  }
};



#if HAL_USE_SPI
static  SPIConfig halfBridgeCfg[] = {{
    NULL,
    /* HW dependent part.*/
    GPIOC,
    GPIOC_PIN4_SPI1_CS1,
    SPI_CR1_CPHA | SPI_CR1_BR_2 | SPI_CR1_BR_1 | SPI_CR1_LSBFIRST // Speed 328khz
  },
				     
  {
    NULL,
    /* HW dependent part.*/
    GPIOC,
    GPIOC_PIN5_SPI1_CS2,
    SPI_CR1_CPHA | SPI_CR1_BR_2 | SPI_CR1_BR_1 | SPI_CR1_LSBFIRST // Speed 328khz
  }
};



#endif

void spiSetHalfBridgeMask (const HalfBridgeIndex hbi, const HalfBridgePortMask hbm,
			   const HalfBridgeCommandMask options)
{
  spiChangeHalfBridgeMask (HALF_BRIDGE_SET, hbi, hbm, options);
}

void spiClearHalfBridgeMask (const HalfBridgeIndex hbi, const HalfBridgePortMask hbm,
			   const HalfBridgeCommandMask options)
{
  spiChangeHalfBridgeMask (HALF_BRIDGE_CLEAR, hbi, hbm, options);
}


void spiChangeHalfBridgeMask (const HalfBridgeOperation hbo, const HalfBridgeIndex hbi, 
			      const HalfBridgePortMask hbm, const HalfBridgeCommandMask options)
{
#if HAL_USE_SPI
  HalfBridgeCommandMask spiOrder, spiStatus;
  
  if (hbi >= HALF_BRIDGE_NUMBER) {
    DebugTrace ("hbi [%d] >= HALF_BRIDGE_NUMBER [%d]", hbi, HALF_BRIDGE_NUMBER);
    return;
  }

  if (hbo == HALF_BRIDGE_SET) {
    halfBridgeData[hbi].out |= hbm;
  } else {
    halfBridgeData[hbi].out &= (~hbm);
  }
   
  halfBridgeData[hbi].options = options;

  spiOrder = get_NCV7708_latchesBits (halfBridgeData[hbi].out);
  spiOrder |= ((NCV7708E_RESET_STATUS | NCV7708E_OVERLOAD_CONTROL | 
		NCV7708E_UNDERLOAD_CONTROL |  NCV7708E_OVERVOLTAGE_CONTROL) & options);
  
  spiPrologue (&SPID1, &halfBridgeCfg[hbi]);
  spiExchange (&SPID1, sizeof(spiOrder), &spiOrder, &spiStatus);
  spiEpilogue (&SPID1);
  //  DebugTrace ("Change: spiOrder = 0x%x, spiStatus = 0x%x", spiOrder, spiStatus);
  
#endif
}

bool_t spiCheckHalfBridgeMask (const HalfBridgeIndex hbi, bool_t *overLoad, 
			       bool_t *underLoad, bool_t *supplyFailure, bool_t *thermalWarning,
			       HalfBridgePortMask *portStatusMsk)
{
#if HAL_USE_SPI
  HalfBridgeCommandMask spiOrder, spiStatus;
  
   if (hbi >= HALF_BRIDGE_NUMBER) {
    DebugTrace ("hbi [%d] >= HALF_BRIDGE_NUMBER [%d]", hbi, HALF_BRIDGE_NUMBER);
    return true;
  }
 
  spiOrder = get_NCV7708_latchesBits (halfBridgeData[hbi].out);
  spiOrder |= ((NCV7708E_RESET_STATUS | NCV7708E_OVERLOAD_CONTROL | 
		NCV7708E_UNDERLOAD_CONTROL |  NCV7708E_OVERVOLTAGE_CONTROL) & 
	       halfBridgeData[hbi].options);

  spiPrologue (&SPID1, &halfBridgeCfg[hbi]);
  spiExchange (&SPID1, sizeof(spiOrder), &spiOrder, &spiStatus);
  spiEpilogue (&SPID1);

  //  DebugTrace ("Check: spiOrder = 0x%x, spiStatus = 0x%x", spiOrder, spiStatus);

  *portStatusMsk = get_NCV7708_status (spiOrder, spiStatus);
  *overLoad = spiStatus & NCV7708E_STATUS_OVERLOAD;
  *underLoad = spiStatus & NCV7708E_STATUS_UNDERLOAD;
  *supplyFailure = spiStatus & NCV7708E_STATUS_OVERVOLTAGE;
  *thermalWarning = spiStatus &  NCV7708E_STATUS_THERMAL_WARNING;

  return *portStatusMsk || *overLoad || *underLoad
    || *supplyFailure || *thermalWarning;
#endif
}

void setHalfBridgeOutputMode (const HalfBridgeOutputMode _outputMode)
{
  if (outputMode < MODE_NUMBER) 
    outputMode = _outputMode;
}

static void spiPrologue (SPIDriver * spid, const SPIConfig *spiCfg)
{
  spiAcquireBus(spid); 
  spiStart(spid, spiCfg);
  spiSelect(spid);
  halPolledDelay (16); // 100 nano seconds required between CS active and clock raise
}

static void spiEpilogue (SPIDriver * spid)
{
  halPolledDelay (16); // 100 nano seconds required between last clock fall and CS inactive
  spiUnselect (spid);
  spiStop (spid);
  spiReleaseBus(spid);    
}

static HalfBridgeCommandMask get_NCV7708_latchesBits (const HalfBridgePortMask outMask)
{
  HalfBridgeCommandMask latchBits = 0;
  for (uint32_t i=0; i<NCV7708E_NB_PORT; i++) {
    const HalfBridgeCommandMask shift = i+i;
    const HalfBridgeCommandMask lowLatch =  (NCV7708E_OUT_L1 << shift) & 
      halfBridgeOutputModeMask[outputMode].off_L;
    const HalfBridgeCommandMask highLatch = (NCV7708E_OUT_H1 << shift) & 
      halfBridgeOutputModeMask[outputMode].on_H;
    if (outMask & (1<<i)) {
      // set out to high level setting hightLatch to 1 and lowLatch to 0
      latchBits |= highLatch;
    } else {
      // set out to low level setting hightLatch to 0 and lowLatch to 1
      latchBits |= lowLatch;
    }
  }

  return latchBits;
}

static HalfBridgePortMask get_NCV7708_status (const HalfBridgeCommandMask latchesBits, 
					      const HalfBridgeCommandMask hbstatus)
{
  HalfBridgePortMask latchStatus=0;
  
  for (uint32_t i=0; i<NCV7708E_NB_PORT; i++) {
    const HalfBridgeCommandMask shift = i+i;
    const HalfBridgeCommandMask latchMsk =  (NCV7708E_OUT_L1 << shift) | (NCV7708E_OUT_H1 << shift);
    
    // if status bits is different from previous order, something is wrong on this bit
    if ((latchesBits & latchMsk) != (hbstatus & latchMsk)) {
      latchStatus |= (1 << i);
    }
  }
  
  return latchStatus;
}
