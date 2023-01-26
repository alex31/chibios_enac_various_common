#pragma once

#include <ch.h>
#include <hal.h>

typedef enum {
  OPCR2_OK, OPCR2_CRC_ERROR, OPCR2_NOT_RESPONDING
} OPCR2Status;

typedef enum {
  FAN_OFF = 0x02,
  FAN_ON = 0x03,
  LASER_OFF = 0x06,
  LASER_ON= 0x07
} Option;

typedef struct {
  SPIDriver *spid;
  ioline_t  ssline;
  ioline_t  sckline;
  uint8_t   sckaf;
} OPCR2Config;

typedef struct {
  uint8_t fanOn;
  uint8_t laserDacOn;
  uint8_t fanDacVal;
  uint8_t laserDacVal;
  uint8_t laserSwitch;
  uint8_t gains;
} OPCR2DacAndPowerStatus;

typedef struct {
  uint8_t major;
  uint8_t minor;
} OPCR2FwVersion;

typedef struct {
  uint16_t adcBound[25];
  uint16_t diameterBound[25];
  uint16_t binWeight[24];
  uint16_t pwDiameterA;
  uint16_t pwDiameterB;
  uint16_t pwDiameterC;
  uint16_t maxTimeOfFlight;
  uint16_t samplingIntervalCount;
  uint16_t idleIntervalCount;
  uint16_t maxDataArrayInFile;
  uint8_t  amOnlySavePMData;
  uint8_t  amLaserOnInIdle;
  uint8_t  tof2sfr;
  uint8_t  particleValidationPeriod;
  uint8_t  binWeightingIndex;
} __attribute__ ((__packed__)) OPCR2FwConf;



typedef struct {
  uint16_t bin[24];
  uint8_t  bin1Mtof;
  uint8_t  bin3Mtof;
  uint8_t  bin5Mtof;
  uint8_t  bin7Mtof;
  uint16_t samplingPeriod;
  uint16_t sampleFlowRate;
  uint16_t temperature;
  uint16_t relativeHumidity;
  float	   pm[3];
  uint16_t rejectCountGlitch;
  uint16_t rejectCountLongTOF;
  uint16_t rejectCountRatio;
  uint16_t rejectCountOutOfRange;
  uint16_t fanRPM;
  uint16_t laserStatus;
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
OPCR2Status OPCR2ReadDacAndPowerStatus(OPCR2Driver *drv, OPCR2DacAndPowerStatus *dps);
OPCR2Status OPCR2SetFanPower(OPCR2Driver *drv, uint8_t power);
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
