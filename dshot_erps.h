#pragma once

#include <ch.h>
#include <hal.h>

#ifdef __cplusplus
extern "C" {
#endif

  typedef enum  {
    EDT_NOTEDT=0x0,
    EDT_TEMP=0x2, EDT_VOLT=0x4, EDT_CURRENT=0x6,
    EDT_DBG1=0x8, EDT_DBG2=0xA, EDT_DBG3=0xC,
    EDT_STATE=0xE
  }  EdtType;
  
  
// typedef union  {
//   struct {
//     uint16_t crc:4;
//     union  {
//       struct {
// 	uint16_t mantisse:9;
// 	uint16_t exponent:3;
//       };
//       // for Extended DShot Telemetry (EDT)
//       struct {
// 	uint8_t  edt_value:8;
// 	EdtType  edt_type:4;
//       };
//     };
//   };
//   uint16_t rawFrame;
// } DshotEPeriodPacket;

typedef union  {
  struct {
    uint16_t crc:4;
    uint16_t mantisse:9;
    uint16_t exponent:3;
  };
  uint16_t rawFrame;
} DshotEPeriodPacket;

typedef union  {
  struct {
    uint16_t crc:4;
    uint16_t  edt_value:8;
    EdtType edt_type:4;
  } __attribute__ ((__packed__)) ;
  uint16_t rawFrame;
} DshotEPeriodTelemetry;

_Static_assert(sizeof(DshotEPeriodPacket) == sizeof(uint16_t), "DshotEPeriodPacket size error");
_Static_assert(sizeof(DshotEPeriodTelemetry) == sizeof(uint16_t), "DshotEPeriodTelemetry size error");
  
  typedef struct {
    DshotEPeriodPacket ep; // 16 bits packet
    uint32_t	     ef; // 21 bits frame
  } DshotErps;
  
  
  
  const DshotErps* DshotErpsSetFromFrame(DshotErps *derpsp, uint32_t frame);
  const DshotErps* DshotErpsSetFromRpm(DshotErps *derpsp, uint32_t rpm);
  uint32_t DshotErpsGetRpm(const DshotErps *derpsp) ;
  bool DshotErpsCheckCrc4(const DshotErps *derpsp);
  static inline uint32_t DshotErpsGetFrame(const DshotErps *derpsp) {return derpsp->ef;}
  static inline bool DshotErpsIsEdt(const DshotErps *derpsp)  {
    return 
      ((derpsp->ep.rawFrame & 0x1000) == 0) &&
      ((derpsp->ep.rawFrame & 0xe000) != 0);
  }

static inline EdtType DshotErpsEdtType(const DshotErps *derpsp) {
  return (DshotEPeriodTelemetry) {.rawFrame = derpsp->ep.rawFrame}.edt_type;
}
static inline uint8_t DshotErpsEdtTempCentigrade(const DshotErps *derpsp) {
  return (DshotEPeriodTelemetry) {.rawFrame = derpsp->ep.rawFrame}.edt_value;
}
static inline uint16_t DshotErpsEdtDeciVolts(const DshotErps *derpsp) {
  return (DshotEPeriodTelemetry) {.rawFrame = derpsp->ep.rawFrame}.edt_value * 10U / 4U;
}
static inline uint16_t DshotErpsEdtCurrentAmp(const DshotErps *derpsp) {
  return (DshotEPeriodTelemetry) {.rawFrame = derpsp->ep.rawFrame}.edt_value;
}

#ifdef __cplusplus
}
#endif
