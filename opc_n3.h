#pragma once

#include <ch.h>
#include <hal.h>

typedef enum {
  OPCN3_OK, OPCN3_CRC_ERROR, OPCN3_NOT_RESPONDING
} OPCN3Status;

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
} OPCN3Config;

typedef struct {
  uint8_t fanOn;
  uint8_t laserDacOn;
  uint8_t fanDacVal;
  uint8_t laserDacVal;
  uint8_t laserSwitch;
  uint8_t gains;
} OPCN3DacAndPowerStatus;

typedef struct {
  uint8_t major;
  uint8_t minor;
} OPCN3FwVersion;

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
} __attribute__ ((__packed__)) OPCN3FwConf;



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
} __attribute__ ((__packed__)) OPCN3Histogram;

typedef struct {
  float	   pm[3];
  uint16_t crc;
} __attribute__ ((__packed__)) OPCN3PMData;


typedef struct {
  const OPCN3Config	*opcn3Cfg;
  SPIConfig   spiCfg;
  OPCN3Status status;
} OPCN3Driver;

#ifdef __cplusplus
extern "C" {
#endif


OPCN3Status OPCN3Start(OPCN3Driver *drv, const OPCN3Config *cfg);
OPCN3Status OPCN3SendPower(OPCN3Driver *drv, Option option);
OPCN3Status OPCN3ReadDacAndPowerStatus(OPCN3Driver *drv, OPCN3DacAndPowerStatus *dps);
OPCN3Status OPCN3SetFanPower(OPCN3Driver *drv, uint8_t power);
OPCN3Status OPCN3SetBinWeighting(OPCN3Driver *drv, uint8_t weight);
OPCN3Status OPCN3ReadInfo(OPCN3Driver *drv, char info[61]);
OPCN3Status OPCN3ReadSerial(OPCN3Driver *drv, char serial[61]);
OPCN3Status OPCN3ReadFwVersion(OPCN3Driver *drv, OPCN3FwVersion *ver);
OPCN3Status OPCN3ReadFwConf(OPCN3Driver *drv, OPCN3FwConf *conf);
OPCN3Status OPCN3WriteFwConf(OPCN3Driver *drv, const OPCN3FwConf *conf);
OPCN3Status OPCN3ReadHistogram(OPCN3Driver *drv, OPCN3Histogram *data);
OPCN3Status OPCN3ReadPMData(OPCN3Driver *drv, OPCN3PMData *data);
OPCN3Status OPCN3SaveConf(OPCN3Driver *drv);
OPCN3Status OPCN3CheckStatus(OPCN3Driver *drv);
OPCN3Status OPCN3Reset(OPCN3Driver *drv);
OPCN3Status OPCN3EnterBootLoaderMode(OPCN3Driver *drv);

#ifdef __cplusplus
}
#endif
