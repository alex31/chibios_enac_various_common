#pragma once

#include <ch.h>
#include <hal.h>


typedef union  {
  struct {
    uint16_t crc:4;
    uint16_t mantisse:9;
    uint16_t exponent:3;
  };
  uint16_t rawFrame;
} DshotEPeriodPacket;

typedef struct {
  DshotEPeriodPacket ep; // 16 bits packet
  uint32_t	     ef; // 21 bits frame
} DshotErps;



const DshotErps* setFromFrame(DshotErps *derpsp, uint32_t frame);
const DshotErps* setFromRpm(DshotErps *derpsp, uint32_t rpm);
uint32_t getRpm(const DshotErps *derpsp) ;
inline uint32_t getFrame(const DshotErps *derpsp) {return derpsp->ef;}
bool checkCrc4(const DshotErps *derpsp);
