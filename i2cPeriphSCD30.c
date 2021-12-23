#include "ch.h"
#include "hal.h"
#include "stdutil.h"
#include <string.h>

#include "i2cPeriphSCD30.h"

static const uint8_t SCD30_I2C_ADDRESS = 0x61;

typedef enum __attribute__ ((__packed__)) {
  SCD30_NOT_INITIALIZED = 0x0,
  SCD30_START_CONTINUOUS = SWAP_ENDIAN16(0x0010), // argument : pressure in mbar or 0 to deactivate altitude compensation
  SCD30_STOP_CONTINUOUS  = SWAP_ENDIAN16(0x0104), // no argument
  SCD30_INTERVAL  = SWAP_ENDIAN16(0x4600), // interval in seconds in the range 2 .. 1800
  SCD30_GET_STATUS = SWAP_ENDIAN16(0x0202), // no argument, read 1 atom
  SCD30_GET_DATA = SWAP_ENDIAN16(0x0300), // no argument, read 6 atoms
  SCD30_SELF_CALIBRATION = SWAP_ENDIAN16(0x5306), // arg : 1 to activate, 0 to deactivate
  SCD30_FORCED_CALIBRATION = SWAP_ENDIAN16(0x5204), // arg : CO2 concentration in ppmm in the range 400 .. 2000
  SCD30_TEMPERATURE_OFFSET = SWAP_ENDIAN16(0x5403), // arg : temperature offset in 1/100 kelvin
  SCD30_ALTITUDE_COMPENSATION = SWAP_ENDIAN16(0x5102), // arg : altitude in m
  SCD30_GET_FIRMWARE_VERSION = SWAP_ENDIAN16(0xD100), // no arg : return FW version
  SCD30_SOFT_RESET = SWAP_ENDIAN16(0xD304) // arg : altitude in m
}  Scd30Command;


typedef struct __attribute__((packed)) {
  union {
    uint16_t data16;
    uint8_t  data8[2];
  };
  uint8_t crc;
} Scd30DataAtom ;


static inline uint8_t crc8_poly31_calc (const uint8_t data[],
					const size_t len);
static bool atomCheck(const Scd30DataAtom *atom);
static void atomCalcCrc(Scd30DataAtom *atom);
static Scd30Status scd30Transaction(const Scd30Driver *scdd, const Scd30Command cmd,
				    Scd30DataAtom *optArg,
				    Scd30DataAtom *optAnswer, size_t nbAnswerAtom);
Scd30Status scd30GetStatus(const Scd30Driver *scdd, bool *status);
static float atom2float(const Scd30DataAtom* atmArr);

__attribute__ ((pure))
static inline uint8_t crc8_poly31_calc(const uint8_t data[],
				       const size_t len)
{
  static const uint8_t
    crc8_poly31[256] = { 0x00, 0x31, 0x62, 0x53, 0xC4, 0xF5, 0xA6, 0x97,
    0xB9, 0x88, 0xDB, 0xEA, 0x7D, 0x4C, 0x1F, 0x2E, 0x43, 0x72, 0x21,
    0x10, 0x87, 0xB6, 0xE5, 0xD4, 0xFA, 0xCB, 0x98, 0xA9, 0x3E, 0x0F,
    0x5C, 0x6D, 0x86, 0xB7, 0xE4, 0xD5, 0x42, 0x73, 0x20, 0x11, 0x3F,
    0x0E, 0x5D, 0x6C, 0xFB, 0xCA, 0x99, 0xA8, 0xC5, 0xF4, 0xA7, 0x96,
    0x01, 0x30, 0x63, 0x52, 0x7C, 0x4D, 0x1E, 0x2F, 0xB8, 0x89, 0xDA,
    0xEB, 0x3D, 0x0C, 0x5F, 0x6E, 0xF9, 0xC8, 0x9B, 0xAA, 0x84, 0xB5,
    0xE6, 0xD7, 0x40, 0x71, 0x22, 0x13, 0x7E, 0x4F, 0x1C, 0x2D, 0xBA,
    0x8B, 0xD8, 0xE9, 0xC7, 0xF6, 0xA5, 0x94, 0x03, 0x32, 0x61, 0x50,
    0xBB, 0x8A, 0xD9, 0xE8, 0x7F, 0x4E, 0x1D, 0x2C, 0x02, 0x33, 0x60,
    0x51, 0xC6, 0xF7, 0xA4, 0x95, 0xF8, 0xC9, 0x9A, 0xAB, 0x3C, 0x0D,
    0x5E, 0x6F, 0x41, 0x70, 0x23, 0x12, 0x85, 0xB4, 0xE7, 0xD6, 0x7A,
    0x4B, 0x18, 0x29, 0xBE, 0x8F, 0xDC, 0xED, 0xC3, 0xF2, 0xA1, 0x90,
    0x07, 0x36, 0x65, 0x54, 0x39, 0x08, 0x5B, 0x6A, 0xFD, 0xCC, 0x9F,
    0xAE, 0x80, 0xB1, 0xE2, 0xD3, 0x44, 0x75, 0x26, 0x17, 0xFC, 0xCD,
    0x9E, 0xAF, 0x38, 0x09, 0x5A, 0x6B, 0x45, 0x74, 0x27, 0x16, 0x81,
    0xB0, 0xE3, 0xD2, 0xBF, 0x8E, 0xDD, 0xEC, 0x7B, 0x4A, 0x19, 0x28,
    0x06, 0x37, 0x64, 0x55, 0xC2, 0xF3, 0xA0, 0x91, 0x47, 0x76, 0x25,
    0x14, 0x83, 0xB2, 0xE1, 0xD0, 0xFE, 0xCF, 0x9C, 0xAD, 0x3A, 0x0B,
    0x58, 0x69, 0x04, 0x35, 0x66, 0x57, 0xC0, 0xF1, 0xA2, 0x93, 0xBD,
    0x8C, 0xDF, 0xEE, 0x79, 0x48, 0x1B, 0x2A, 0xC1, 0xF0, 0xA3, 0x92,
    0x05, 0x34, 0x67, 0x56, 0x78, 0x49, 0x1A, 0x2B, 0xBC, 0x8D, 0xDE,
    0xEF, 0x82, 0xB3, 0xE0, 0xD1, 0x46, 0x77, 0x24, 0x15, 0x3B, 0x0A,
    0x59, 0x68, 0xFF, 0xCE, 0x9D, 0xAC }; uint8_t crc = 0xff;
  
  for (size_t i=0; i < len; ++i) crc = crc8_poly31[crc ^ data[i]];
  return crc;
}

static bool atomCheck(const Scd30DataAtom *atom)
{
  return crc8_poly31_calc(atom->data8, sizeof(atom->data8)) == atom->crc;
}

static void atomCalcCrc(Scd30DataAtom *atom)
{
  atom->crc = crc8_poly31_calc(atom->data8, sizeof(atom->data8));
}


static Scd30Status scd30Transaction(const Scd30Driver *scdd, const Scd30Command cmd,
				    Scd30DataAtom *optArg,
				    Scd30DataAtom *optAnswer, size_t nbAnswerAtom)
{
  static IN_DMA_SECTION(uint8_t tx[sizeof(cmd) + sizeof(*optArg)]);
  static IN_DMA_SECTION(uint8_t rx[sizeof(*optAnswer) * 6]);
  const size_t txSize = sizeof(cmd) + (optArg ? sizeof(*optArg) : 0);
  const size_t rxSize = sizeof(*optAnswer) * nbAnswerAtom;
  Scd30Status status = SCD30_OK;

 
  chDbgAssert(nbAnswerAtom <= 6U, "maximum answer size is 6 atoms");
  if (optAnswer)
    chDbgAssert(nbAnswerAtom != 0, "non null optAnswer implies non null nbAnswerAtom");
  
  memcpy(tx, &cmd, sizeof(cmd));
  if (optArg) {
    atomCalcCrc(optArg);
    memcpy(tx + sizeof(cmd), optArg, sizeof(*optArg));
  }
  i2cAcquireBus(scdd->i2cp);
  if (scdd->i2cp->state != I2C_READY) {
    i2cReleaseBus(scdd->i2cp);
    return SCD30_I2C_ERROR;
  }
   msg_t i2cStatus = i2cMasterTransmitTimeout(scdd->i2cp, SCD30_I2C_ADDRESS,
					     tx, txSize,
					     NULL, 0, TIME_MS2I(100));
  if ((i2cStatus == MSG_OK) && optAnswer) {
    chThdSleepMilliseconds(3); // at least 3ms between write and read
    i2cStatus = i2cMasterReceiveTimeout(scdd->i2cp, SCD30_I2C_ADDRESS,
					rx, rxSize, TIME_MS2I(100));
  }
  if (i2cStatus == MSG_OK) {
    if (optAnswer) {
      memcpy(optAnswer, rx, rxSize);
      for (size_t i=0; i< nbAnswerAtom; i++) {
	if (! atomCheck(&(optAnswer[i]))) {
	  status = SCD30_CRC_ERROR;
	  break;
	}
      }
    }
  } else {
    status = SCD30_I2C_ERROR;
  }
  i2cReleaseBus(scdd->i2cp);
  return status;
}

void        scd30Start(Scd30Driver *sdpp, I2CDriver *i2cp)
{
  chDbgAssert((sdpp != NULL) && (i2cp != NULL),
	      "please give valid object pointers");
  sdpp->i2cp = i2cp;
}

Scd30Status scd30StartContinuous(const Scd30Driver *scdd, uint16_t optPressure)
{
  chDbgAssert((optPressure == 0) ||
	      ((optPressure >= 700) && (optPressure <= 1400)),
	     "optional pressure must be 0 (disabled) or in range 700 .. 1400");
  Scd30DataAtom atom = {.data16 = __builtin_bswap16(optPressure)};
  return scd30Transaction(scdd, SCD30_START_CONTINUOUS, &atom, NULL, 0);
}

Scd30Status scd30StopContinuous(const Scd30Driver *scdd)
{
  return scd30Transaction(scdd, SCD30_STOP_CONTINUOUS, NULL, NULL, 0);
}

Scd30Status scd30Reset(const Scd30Driver *scdd)
{
  return scd30Transaction(scdd, SCD30_SOFT_RESET, NULL, NULL, 0);
}

Scd30Status scd30SetInterval(const Scd30Driver *scdd, const uint16_t interval)
{
  Scd30DataAtom atom = {.data16 = __builtin_bswap16(interval)};
  return scd30Transaction(scdd, SCD30_INTERVAL, &atom, NULL, 0);
}

Scd30Status scd30GetInterval(const Scd30Driver *scdd, uint16_t *interval)
{
  Scd30DataAtom atom;
  Scd30Status status = scd30Transaction(scdd, SCD30_INTERVAL, NULL, &atom, 1);
  if (status == SCD30_OK) {
    *interval = __builtin_bswap16(atom.data16);
  }
  return status;
}

Scd30Status scd30GetStatus(const Scd30Driver *scdd, bool *ready)
{
  Scd30DataAtom atom;
  Scd30Status status = scd30Transaction(scdd, SCD30_GET_STATUS, NULL, &atom, 1);
  if (status == SCD30_OK) {
    *ready = __builtin_bswap16(atom.data16) ? true : false;
  }
  return status;
}

Scd30Status scd30FetchData(Scd30Driver *scdd)
{
  Scd30DataAtom atom[6];
  bool ready = false;
  while (ready == false) {
    const Scd30Status status = scd30GetStatus(scdd, &ready);
    if (status != SCD30_OK)
      return SCD30_OK;
  }
  const Scd30Status status = scd30Transaction(scdd, SCD30_GET_DATA, NULL, atom, 6);
  if (status == SCD30_OK) {
    scdd->co2 = atom2float(atom);
    scdd->temperature = atom2float(atom + 2U);
    scdd->humidity = atom2float(atom + 4U);
  }
  return status;
}

Scd30Status scd30SetSelfCalibration(Scd30Driver *scdd, const bool activate)
{
  Scd30DataAtom atom = {.data16 = __builtin_bswap16(activate ? 1U : 0U)};
  return scd30Transaction(scdd, SCD30_SELF_CALIBRATION, &atom, NULL, 0);
}

Scd30Status scd30GetSelfCalibration(Scd30Driver *scdd, bool *activate)
{
  Scd30DataAtom atom;
  Scd30Status status = scd30Transaction(scdd, SCD30_SELF_CALIBRATION, NULL, &atom, 1);
  if (status == SCD30_OK) {
    *activate = __builtin_bswap16(atom.data16) ? true : false;
  }
  return status;
}


Scd30Status scd30SetForcedCalibration(Scd30Driver *scdd, const uint16_t ppm)
{
  chDbgAssert((ppm >= 400) && (ppm <= 2000),
	     "ppm must be in range 400 .. 2000");
  Scd30DataAtom atom = {.data16 = __builtin_bswap16(ppm)};
  return scd30Transaction(scdd, SCD30_FORCED_CALIBRATION, &atom, NULL, 0);
}

Scd30Status scd30GetForcedCalibration(Scd30Driver *scdd, uint16_t *ppm)
{
  Scd30DataAtom atom;
  Scd30Status status = scd30Transaction(scdd, SCD30_FORCED_CALIBRATION, NULL, &atom, 1);
  if (status == SCD30_OK) {
    *ppm = __builtin_bswap16(atom.data16);
  }
  return status;
}

Scd30Status scd30SetTemperatureOffset(Scd30Driver *scdd, const float offset)
{
  chDbgAssert(offset >= 0.0f, "offset must be positive");
  const uint16_t offset16 = offset * 100;
  Scd30DataAtom atom = {.data16 = __builtin_bswap16(offset16)};
  return scd30Transaction(scdd, SCD30_TEMPERATURE_OFFSET, &atom, NULL, 0);
}

Scd30Status scd30GetTemperatureOffset(Scd30Driver *scdd, float *offset)
{
  Scd30DataAtom atom;
  Scd30Status status = scd30Transaction(scdd, SCD30_TEMPERATURE_OFFSET,
					NULL, &atom, 1);
  if (status == SCD30_OK) {
    *offset = __builtin_bswap16(atom.data16) / 100.0f;
  }
  return status;
}

Scd30Status scd30SetAltitudeCompensation(Scd30Driver *scdd, const uint16_t altitude)
{
  Scd30DataAtom atom = {.data16 = __builtin_bswap16(altitude)};
  return scd30Transaction(scdd, SCD30_ALTITUDE_COMPENSATION, &atom, NULL, 0);
}


Scd30Status scd30GetAltitudeCompensation(Scd30Driver *scdd, uint16_t *altitude)
{
  Scd30DataAtom atom;
  Scd30Status status = scd30Transaction(scdd, SCD30_ALTITUDE_COMPENSATION, NULL, &atom, 1);
  if (status == SCD30_OK) {
    *altitude = __builtin_bswap16(atom.data16);
  }
  return status;
}


Scd30Status scd30GetFwVersion(const Scd30Driver *scdd, uint16_t *version)
 {
  Scd30DataAtom atom;
  Scd30Status status = scd30Transaction(scdd, SCD30_GET_FIRMWARE_VERSION, NULL, &atom, 1);
  if (status == SCD30_OK) {
    *version = __builtin_bswap16(atom.data16);
  }
  return status;
}
 

static float atom2float(const Scd30DataAtom* atmArr)
{
  const union {
    struct {
      uint8_t llsb;
      uint8_t lmsb;
      uint8_t mlsb;
      uint8_t mmsb;
    };
    float spfloat;
  } swapVal = {
    .mmsb = atmArr[0].data8[0],
    .mlsb = atmArr[0].data8[1],
    .lmsb = atmArr[1].data8[0],
    .llsb = atmArr[0].data8[1]};

  return swapVal.spfloat;
}
