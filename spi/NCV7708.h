#pragma once

#include <ch.h>
#include <hal.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NCV7708E_NB_PORT		6

#define NCV7708E_RESET_STATUS		((uint16_t) (1 << 0))
#define NCV7708E_NORESET_STATUS		((uint16_t) (0 << 0))
#define NCV7708E_OUT_L1 		((uint16_t) (1 << 1))
#define NCV7708E_OUT_H1 		((uint16_t) (1 << 2))
#define NCV7708E_OUT_L2 		((uint16_t) (1 << 3))
#define NCV7708E_OUT_H2 		((uint16_t) (1 << 4))
#define NCV7708E_OUT_L3 		((uint16_t) (1 << 5))
#define NCV7708E_OUT_H3 		((uint16_t) (1 << 6))
#define NCV7708E_OUT_L4 		((uint16_t) (1 << 7))
#define NCV7708E_OUT_H4 		((uint16_t) (1 << 8))
#define NCV7708E_OUT_L5 		((uint16_t) (1 << 9))
#define NCV7708E_OUT_H5 		((uint16_t) (1 << 10))
#define NCV7708E_OUT_L6 		((uint16_t) (1 << 11))
#define NCV7708E_OUT_H6 		((uint16_t) (1 << 12))
#define NCV7708E_OVERLOAD_CONTROL	((uint16_t) (1 << 13))
#define NCV7708E_OVERLOAD_NOCONTROL	((uint16_t) (0 << 13))
#define NCV7708E_UNDERLOAD_CONTROL	((uint16_t) (1 << 14))
#define NCV7708E_UNDERLOAD_NOCONTROL	((uint16_t) (0 << 14))
#define NCV7708E_POWERFAILURE_CONTROL	((uint16_t) (1 << 15))
#define NCV7708E_POWERFAILURE_NOCONTROL	((uint16_t) (0 << 15))


#define NCV7708E_STATUS_THERMAL_WARNING ((uint16_t) (1 << 0))
#define NCV7708E_STATUS_OUT_L1 		((uint16_t) (1 << 1))
#define NCV7708E_STATUS_OUT_H1 		((uint16_t) (1 << 2))
#define NCV7708E_STATUS_OUT_L2 		((uint16_t) (1 << 3))
#define NCV7708E_STATUS_OUT_H2 		((uint16_t) (1 << 4))
#define NCV7708E_STATUS_OUT_L3 		((uint16_t) (1 << 5))
#define NCV7708E_STATUS_OUT_H3 		((uint16_t) (1 << 6))
#define NCV7708E_STATUS_OUT_L4 		((uint16_t) (1 << 7))
#define NCV7708E_STATUS_OUT_H4 		((uint16_t) (1 << 8))
#define NCV7708E_STATUS_OUT_L5 		((uint16_t) (1 << 9))
#define NCV7708E_STATUS_OUT_H5 		((uint16_t) (1 << 10))
#define NCV7708E_STATUS_OUT_L6 		((uint16_t) (1 << 11))
#define NCV7708E_STATUS_OUT_H6 		((uint16_t) (1 << 12))
#define NCV7708E_STATUS_OVERLOAD	((uint16_t) (1 << 13))
#define NCV7708E_STATUS_UNDERLOAD	((uint16_t) (1 << 14))
#define NCV7708E_STATUS_POWERFAILURE	((uint16_t) (1 << 15))



typedef uint8_t HalfBridgePortMask;
typedef uint16_t HalfBridgeCommandMask;

typedef enum {
  HALF_BRIDGE_1, HALF_BRIDGE_2, HALF_BRIDGE_NUMBER
} HalfBridgeIndex;

typedef enum {
  MODE_PUSHPULL=0, MODE_OPENDRAIN_PULLUP, MODE_OPENDRAIN_PULLDOWN, MODE_NUMBER
} HalfBridgeOutputMode;

typedef enum {
  HALF_BRIDGE_CLEAR=0, HALF_BRIDGE_SET
} HalfBridgeOperation;

typedef enum {BUT_B_3=0x00, BUT_D_1, BUT_G_1, BUT_AR_2, BUT_AV_2, BUT_H_3,
	      BUT_F_5, BUT_H_4, BUT_B_4, BUT_O_5, L2_SHORTCUT, L1_OPENCIRCUIT
} HalfBridgeFunction;
  

void setHalfBridgeOutputMode (const HalfBridgeOutputMode _outputMode);

void spiSetHalfBridgeMask (const HalfBridgeIndex hbi, const HalfBridgePortMask hbm, 
			   const HalfBridgeCommandMask options);
void spiClearHalfBridgeMask (const HalfBridgeIndex hbi, const HalfBridgePortMask hbm, 
			     const HalfBridgeCommandMask options);
void spiChangeHalfBridgeMask (const HalfBridgeOperation hbo, const HalfBridgeIndex hbi, 
			      const HalfBridgePortMask hbm, const HalfBridgeCommandMask options);

bool_t spiCheckHalfBridgeMask (const HalfBridgeIndex hbi, bool_t *overLoad, 
			       bool_t *underLoad, bool_t *powerFailure, bool_t *thermalWarning,
			       HalfBridgePortMask *portStatusMsk);


#ifdef __cplusplus
}
#endif


