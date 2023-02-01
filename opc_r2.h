#pragma once

#include <ch.h>
#include <hal.h>

typedef enum {
  OPCR2_OK, OPCR2_CRC_ERROR, OPCR2_INVALID_PARAM, OPCR2_NOT_RESPONDING
} OPCR2Status;

typedef enum {
  FAN_LASER_OFF = 0x00,
  FAN_LASER_ON = 0x03
} Option;

typedef struct {
  SPIDriver *spid;
  ioline_t  ssline;
  ioline_t  sckline;
  uint8_t   sckaf;
} OPCR2Config;


typedef struct {
  uint8_t major;
  uint8_t minor;
} OPCR2FwVersion;

typedef struct {
  uint16_t adcBound[16];
  float    diameterBound[16];
  float    binWeight[16];
  float	   gainScalingCoeff;
  float	   particuleMassConcentration[3];
  uint8_t  tof2sfr;
  uint8_t  particleValidationPeriod;
  uint16_t maxTimeOfFlight;
} __attribute__ ((__packed__)) OPCR2FwConf;



typedef struct {
  uint16_t bin[16];
  uint8_t  bin1Mtof;
  uint8_t  bin3Mtof;
  uint8_t  bin5Mtof;
  uint8_t  bin7Mtof;
  float    sampleFlowRate;
  uint16_t temperature;
  uint16_t relativeHumidity;
  float	   samplePeriod;
  uint8_t  rejectCountGlitch;
  uint8_t  rejectCountLong;
  float	   pm[3];
  uint16_t crc;
} __attribute__ ((__packed__)) OPCR2Histogram;

typedef struct {
  float	   pm[3];
  uint16_t crc;
} __attribute__ ((__packed__)) OPCR2PMData;


typedef struct {
  const OPCR2Config	*opcr2Cfg;
  SPIConfig   spiCfg;
  OPCR2Status status;
} OPCR2Driver;

#ifdef __cplusplus
extern "C" {
#endif


OPCR2Status OPCR2Start(OPCR2Driver *drv, const OPCR2Config *cfg);
OPCR2Status OPCR2SendPower(OPCR2Driver *drv, Option option);
OPCR2Status OPCR2SetLaserPower(OPCR2Driver *drv, uint8_t power);
OPCR2Status OPCR2SetBinWeighting(OPCR2Driver *drv, uint8_t weight);
OPCR2Status OPCR2ReadInfo(OPCR2Driver *drv, char info[61]);
OPCR2Status OPCR2ReadSerial(OPCR2Driver *drv, char serial[61]);
OPCR2Status OPCR2ReadFwVersion(OPCR2Driver *drv, OPCR2FwVersion *ver);
OPCR2Status OPCR2ReadFwConf(OPCR2Driver *drv, OPCR2FwConf *conf);
OPCR2Status OPCR2WriteFwConf(OPCR2Driver *drv, const OPCR2FwConf *conf);
OPCR2Status OPCR2ReadHistogram(OPCR2Driver *drv, OPCR2Histogram *data);
OPCR2Status OPCR2ReadPMData(OPCR2Driver *drv, OPCR2PMData *data);
OPCR2Status OPCR2SaveConf(OPCR2Driver *drv);
OPCR2Status OPCR2CheckStatus(OPCR2Driver *drv);
OPCR2Status OPCR2Reset(OPCR2Driver *drv);
OPCR2Status OPCR2EnterBootLoaderMode(OPCR2Driver *drv);

#ifdef __cplusplus
}
#endif
