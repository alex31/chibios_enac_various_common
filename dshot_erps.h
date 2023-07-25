#pragma once

#include <ch.h>
#include <hal.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   ERPS telemetry type 
 * @note    last (as of mid 2022) addition to the dshot bidir protocol
 */
  typedef enum  {
    EDT_NOTEDT=0x0,
    EDT_TEMP=0x2, EDT_VOLT=0x4, EDT_CURRENT=0x6,
    EDT_DBG1=0x8, EDT_DBG2=0xA, EDT_DBG3=0xC,
    EDT_STATE=0xE
  }  EdtType;
  
  
/**
 * @brief   ERPS classic rpm frame
 */
typedef union  {
  struct {
    uint16_t crc:4;
    uint16_t mantisse:9;
    uint16_t exponent:3;
  };
  uint16_t rawFrame;
} DshotEPeriodPacket;

/**
 * @brief   ERPS telemetry frame
 */
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


/**
 * @brief   ERPS complete frame, raw and decoded
 */
  typedef struct {
    DshotEPeriodPacket ep; // 16 bits packet
    uint32_t	     ef; // 21 bits frame
  } DshotErps;
  
  
  
  const DshotErps* DshotErpsSetFromFrame(DshotErps *derpsp, uint32_t frame);
  const DshotErps* DshotErpsSetFromRpm(DshotErps *derpsp, uint32_t rpm);
  uint32_t DshotErpsGetRpm(const DshotErps *derpsp) ;
  bool DshotErpsCheckCrc4(const DshotErps *derpsp);

/**
 * @brief   return encoded frame
 *
 * @param[in] derpsp    pointer to the @p DshotErps object
 * @api
 */
  static inline uint32_t DshotErpsGetFrame(const DshotErps *derpsp) {return derpsp->ef;}
/**
 * @brief   return true if current frame is a telemetry frame
 *
 * @param[in] derpsp    pointer to the @p DshotErps object
 * @api
 */
  static inline bool DshotErpsIsEdt(const DshotErps *derpsp)  {
    return 
      ((derpsp->ep.rawFrame & 0x1000) == 0) &&
      ((derpsp->ep.rawFrame & 0xe000) != 0);
  }

/**
 * @brief   return type of a telemetry frame
 *
 * @param[in] derpsp    pointer to the @p DshotErps object
 * @api
 */
 static inline EdtType DshotErpsEdtType(const DshotErps *derpsp) {
  return (DshotEPeriodTelemetry) {.rawFrame = derpsp->ep.rawFrame}.edt_type;
}
/**
 * @brief   return temperature for a temperature telemetry frame
 *
 * @param[in] derpsp    pointer to the @p DshotErps object
 * @api
 */
static inline uint8_t DshotErpsEdtTempCentigrade(const DshotErps *derpsp) {
  return (DshotEPeriodTelemetry) {.rawFrame = derpsp->ep.rawFrame}.edt_value;
}
/**
 * @brief   return voltage for a voltage telemetry frame
 *
 * @param[in] derpsp    pointer to the @p DshotErps object
 * @api
 */
static inline uint16_t DshotErpsEdtDeciVolts(const DshotErps *derpsp) {
  return (DshotEPeriodTelemetry) {.rawFrame = derpsp->ep.rawFrame}.edt_value * 10U / 4U;
}
/**
 * @brief   return current intensity for a current telemetry frame
 *
 * @param[in] derpsp    pointer to the @p DshotErps object
 * @api
 */
static inline uint16_t DshotErpsEdtCurrentAmp(const DshotErps *derpsp) {
  return (DshotEPeriodTelemetry) {.rawFrame = derpsp->ep.rawFrame}.edt_value;
}

#ifdef __cplusplus
}
#endif
