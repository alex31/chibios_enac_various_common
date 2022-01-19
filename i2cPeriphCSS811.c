#include "ch.h"
#include "hal.h"
#include "stdutil.h"
#include <string.h>

#include "i2cPeriphCSS811.h"

static const uint8_t CSS811_I2C_ADDRESS = 0x5A;

typedef enum __attribute__ ((__packed__)) {
  CSS811_STATUS = 0x0U,			// R   1
  CSS811_MEAS_MODE = 0x1U,		// R/W 1
  CSS811_ALG_RESULT_DATA = 0x2U,	// R   8
  CSS811_RAW_DATA = 0x3U,		// R   2
  CSS811_ENV_DATA = 0x5U,		// W   4
  CSS811_THRESHOLDS = 0x10U,		// W   4
  CSS811_BASELINE = 0x11U,		// R/W 2
  CSS811_HW_ID = 0x20U,			// R   1 -> 0x81
  CSS811_HW_VERSION = 0x21U,		// R   1
  CSS811_FW_BOOT_VERSION = 0x23U,	// R   2
  CSS811_FW_APP_VERSION = 0x24U,	// R   2
  CSS811_INTERNAL_STATE = 0xA0,		// R   1
  CSS811_ERROR_ID = 0xE0,		// R   1
  CSS811_APP_ERASE = 0xF1,		// W   4, sequence is (0xE7 0xA7 0xE6 0x09)
  CSS811_APP_DATA = 0xF2,		// W   9
  CSS811_APP_VERIFY = 0xF3,		// W   -
  CSS811_APP_START = 0xF4,		// W   -
  CSS811_SW_RESET = 0xFF		// W   4, sequence is (0x11 0xE5 0x72 0x8A)
}  Css811RegAddress;

typedef enum __attribute__ ((__packed__)) {
  CSS811_STATUS_ERROR = 1U << 0,
  CSS811_STATUS_DATA_READY = 1U << 3,
  CSS811_STATUS_APP_VALID = 1U << 4,
  CSS811_STATUS_APP_VERIFY = 1U << 5, // boot mode only
  CSS811_STATUS_APP_ERASE = 1U << 6,  // boot mode only
  CSS811_STATUS_FW_MODE = 1U << 7,    // 0 : boot mode, 1 : app mode
} Css811StatusBitMask;

typedef enum __attribute__ ((__packed__)) {
  CSS811_ERROR_WRITE_REG_INVALID = 1U << 0,
  CSS811_ERROR_READ_REG_INVALID = 1U << 1,
  CSS811_ERROR_MEASMODE_INVALID = 1U << 2,
  CSS811_ERROR_MAX_RESISTANCE = 1U << 3,
  CSS811_ERROR_HEATER_FAULT = 1U << 4,
  CSS811_ERROR_HEATER_SUPPLY = 1U << 5
} Css811ErrorBitMask;


static msg_t css811SwitchAppMode(const Css811Driver *cssp);
static msg_t css811SetMeasMode(const Css811Driver *cssp, uint8_t mode);
static msg_t css811GetStatus(const Css811Driver *cssp, uint8_t *status);
static Css811Status css811Error(uint8_t errorMask);
static void css811I2cResetOnError(const Css811Driver *cssp, msg_t i2cStatus);
static void css811ErrorClearError(const Css811Driver *cssp);
static msg_t css811I2CTransmit(const Css811Driver *cssp, size_t txsize, size_t rxsize);
static msg_t css811I2CTransmitFirmware(Css811Driver *cssp, const uint8_t *firmware,
				       size_t firmSize, FirmwareUploadCB onGoingCb);
static msg_t css811Erase(const Css811Driver *cssp);
static msg_t css811Verify(const Css811Driver *cssp);

void	css811Start(Css811Driver *cssp, I2CDriver *i2cp, Css811DmaBuffer *dmab,
		    ioline_t nInt, uint8_t addressPinLevel)
{
  chDbgAssert(cssp && i2cp && dmab, "pointers must be valids");
  memset(cssp, 0, sizeof(Css811Driver));
  cssp->i2cp = i2cp;
  cssp->nInt = nInt;
  cssp->dmab = dmab;
  cssp->i2cAdd = CSS811_I2C_ADDRESS |
    (addressPinLevel != PAL_LOW ? 0x1 : 0x0);
}

Css811Status css811StartContinuous(const Css811Driver *cssp,
				   Css811MeasMode mode)
{
  // read data register
  chDbgAssert(cssp && cssp->i2cp && cssp->dmab, "pointers must be valids");

  uint8_t cssStatus;
  msg_t i2cStatus = css811GetStatus(cssp, &cssStatus);
  if (i2cStatus != MSG_OK)
     return CSS811_I2C_ERROR;

  // if slave is in boot mode, switch it to app mode
  if ((cssStatus & CSS811_STATUS_FW_MODE) == 0) {
    i2cStatus = css811SwitchAppMode(cssp);
    chThdSleepMilliseconds(100);
    if (i2cStatus != MSG_OK)
      return CSS811_I2C_ERROR;
  }
  
  // start continuous sampling
  i2cStatus = css811SetMeasMode(cssp, mode);
  if (i2cStatus != MSG_OK)
    return CSS811_I2C_ERROR;
  
  return CSS811_OK;
}


static msg_t css811SwitchAppMode(const Css811Driver *cssp)
{
  cssp->dmab->tx[0] = CSS811_APP_START;
  return css811I2CTransmit(cssp, 1U, 0U);
}

static msg_t css811SetMeasMode(const Css811Driver *cssp, uint8_t mode)
{
  cssp->dmab->tx[0] = CSS811_MEAS_MODE;
  cssp->dmab->tx[1] = mode;
  
  // if ioline for interrupt is valid, enable interrupt generation
  if (cssp->nInt != 0) {
    cssp->dmab->tx[1] |= CSS811_MEASMODE_INT_DATARDY;
  }

  return css811I2CTransmit(cssp, 2U,  0U);
}

static msg_t css811GetStatus(const Css811Driver *cssp, uint8_t *status)
{
  const uint8_t savetx0 =  cssp->dmab->tx[0];
  cssp->dmab->tx[0] = CSS811_STATUS;

  const msg_t i2cStatus = css811I2CTransmit(cssp, 1U, 1U);
  *status = cssp->dmab->rx[0];
  cssp->dmab->tx[0] = savetx0;
  return i2cStatus;
}


Css811Status css811StopContinuous(const Css811Driver *cssp)
{
  return css811StartContinuous(cssp, CSS811_MEASMODE_DISABLE);
}

Css811Status css811Reset(const Css811Driver *cssp)
{
  const uint8_t txReset[5] = {CSS811_SW_RESET, 0x11, 0xE5, 0x72, 0x8A};
  memcpy(cssp->dmab->tx, txReset, sizeof(txReset));
  const msg_t i2cStatus = css811I2CTransmit(cssp, sizeof(txReset), 0U);
  
  return i2cStatus == MSG_OK ? CSS811_OK : CSS811_I2C_ERROR;
}

static msg_t css811Erase(const Css811Driver *cssp)
{
  const uint8_t txErase[5] = {CSS811_APP_ERASE, 0xE7, 0xA7, 0xE6, 0x09};
  memcpy(cssp->dmab->tx, txErase, sizeof(txErase));
  return css811I2CTransmit(cssp, sizeof(txErase), 0U);
}

static msg_t css811Verify(const Css811Driver *cssp)
{
  cssp->dmab->tx[0] = CSS811_APP_VERIFY;
  return css811I2CTransmit(cssp, 1U, 0U);
}
  
Css811Status css811FetchData(Css811Driver *cssp)
{
  struct {
    uint16_t eCo2;
    uint16_t eTvoc;
    uint8_t status;
    uint8_t errorId;
    uint8_t rawData[2];
  } __attribute__((__packed__)) rxData = {0};

  _Static_assert(sizeof(rxData) == 8, "bad structure size");
  msg_t i2cStatus;

  uint8_t readyStatus = 0;
  while ((readyStatus & CSS811_STATUS_DATA_READY) == 0) {
    i2cStatus = css811GetStatus(cssp, &readyStatus);
    if (i2cStatus != MSG_OK)
      return CSS811_I2C_ERROR;
    chThdSleepMilliseconds(100);
  }
    
  cssp->dmab->tx[0] = CSS811_ALG_RESULT_DATA;
  i2cStatus = css811I2CTransmit(cssp, 1U, sizeof(rxData));
  
  if (i2cStatus != MSG_OK)
    return CSS811_I2C_ERROR;
  memcpy(&rxData, cssp->dmab->rx, sizeof(rxData));
    
  cssp->co2 = __builtin_bswap16(rxData.eCo2);
  cssp->tvoc = __builtin_bswap16(rxData.eTvoc);
  cssp->raw.current = rxData.rawData[0] >> 2;
  cssp->raw.adc = ((rxData.rawData[0] & 0b11) << 8) | rxData.rawData[1];
  cssp->resistance = 1e6f *  1.65f * cssp->raw.adc / (1023.0f * cssp->raw.current); // see AN373

  if (rxData.status & CSS811_STATUS_ERROR) {
    css811ErrorClearError(cssp);
    return css811Error(rxData.errorId);
  } 

  return CSS811_OK;
}

Css811Status css811GetBaseline(Css811Driver *cssp, uint16_t *baseLine)
{
  cssp->dmab->tx[0] = CSS811_BASELINE;
  
  msg_t i2cStatus = css811I2CTransmit(cssp, 1U, sizeof(*baseLine));
  memcpy(baseLine, cssp->dmab->rx, sizeof(*baseLine));
  
  return i2cStatus == MSG_OK ? CSS811_OK : CSS811_I2C_ERROR;
}

Css811Status css811GetVersions(Css811Driver *cssp, Css811Version *versions)
{
  cssp->dmab->tx[0] = CSS811_HW_ID;
  msg_t i2cStatus = css811I2CTransmit(cssp, 1U, 1U);
    
  if (i2cStatus != MSG_OK)
    return CSS811_I2C_ERROR;
  versions->hw_id = cssp->dmab->rx[0];

  cssp->dmab->tx[0] = CSS811_HW_VERSION;
 i2cStatus = css811I2CTransmit(cssp, 1U, 2U);
  if (i2cStatus != MSG_OK) 
    return CSS811_I2C_ERROR;
  versions->version_major = cssp->dmab->rx[0] >> 4;
  versions->version_minor = cssp->dmab->rx[0] & 0xf;

  cssp->dmab->tx[0] = CSS811_FW_BOOT_VERSION;
 i2cStatus = css811I2CTransmit(cssp, 1U, 2U);
  if (i2cStatus != MSG_OK)
    return CSS811_I2C_ERROR;
  versions->boot_major = cssp->dmab->rx[0] >> 4;
  versions->boot_minor = cssp->dmab->rx[0] & 0xf;
  versions->boot_trivial = cssp->dmab->rx[1];
  
  cssp->dmab->tx[0] = CSS811_FW_APP_VERSION;
  i2cStatus = css811I2CTransmit(cssp, 1U, 2U);
  if (i2cStatus != MSG_OK)
    return CSS811_I2C_ERROR;
  versions->app_major = cssp->dmab->rx[0] >> 4;
  versions->app_minor = cssp->dmab->rx[0] & 0xf;
  versions->app_trivial = cssp->dmab->rx[1];
  
  return CSS811_OK;
}

Css811Status css811SetBaseline(Css811Driver *cssp, uint16_t baseLine)
{
  const struct {
    uint8_t regAddr;
    uint16_t bl;
  } __attribute__ ((__packed__)) txBl = {
    .regAddr = CSS811_BASELINE,
    .bl = baseLine
  };
  _Static_assert(sizeof(txBl) == 3, "bad structure size");
  memcpy(cssp->dmab->tx, &txBl, sizeof(txBl));
  const msg_t i2cStatus = css811I2CTransmit(cssp, sizeof(txBl), 0U);
    
  return i2cStatus == MSG_OK ? CSS811_OK : CSS811_I2C_ERROR;
}

Css811Status css811SetEnvironment(Css811Driver *cssp,
				  float humidity, float temperature)
{
  const struct {
    uint8_t regAddr;
    uint16_t h;
    uint16_t t;
  } __attribute__ ((__packed__)) txEnv = {
    .regAddr = CSS811_ENV_DATA,
    .h = __builtin_bswap16((uint16_t) (humidity * 512)),
    .t = __builtin_bswap16((uint16_t) ((temperature + 25) * 512))
  };
  _Static_assert(sizeof(txEnv) == 5, "bad structure size");
  memcpy(cssp->dmab->tx, &txEnv, sizeof(txEnv));
  const msg_t i2cStatus = css811I2CTransmit(cssp, sizeof(txEnv), 0U);

  return i2cStatus == MSG_OK ? CSS811_OK : CSS811_I2C_ERROR;
}


Css811Status css811FlashFirmware(Css811Driver *cssp, const uint8_t *firmware,
				 size_t firmSize, FirmwareUploadCB onGoingCb)
{
  chDbgAssert(cssp && cssp->i2cp && cssp->dmab, "pointers must be valids");
  chDbgAssert(firmSize > 0, "firware size must not be null");
  chDbgAssert(firmSize % 8 == 0, "firware size must be multiple of 8");

  uint8_t status;

  // RESET to boot mode
  Css811Status cssStatus = css811Reset(cssp);
  if (cssStatus != CSS811_OK)
    return cssStatus;

  if (onGoingCb)
    onGoingCb(cssp, CSS811_FIRMWARE_RESET, 0U);
  chThdSleepMilliseconds(500);

  // verify that we are in boot mode
  msg_t i2cStatus = css811GetStatus(cssp, &status);
  if (i2cStatus != MSG_OK)
    return CSS811_I2C_ERROR;
  if (status & CSS811_STATUS_FW_MODE) {
    return CSS811_RESET_FAILED;
  }

  // ERASE
  i2cStatus = css811Erase(cssp);
  if (i2cStatus != MSG_OK)
    return CSS811_I2C_ERROR;
  chThdSleepMilliseconds(500);
  
  i2cStatus = css811GetStatus(cssp, &status);
  if (i2cStatus != MSG_OK)
    return CSS811_I2C_ERROR;
  if ((status & CSS811_STATUS_APP_ERASE) == 0) {
    return CSS811_ERASE_FAILED;
  }
  if (onGoingCb)
    onGoingCb(cssp, CSS811_FIRMWARE_ERASE, 0U);

  // WRITE
  i2cStatus = css811I2CTransmitFirmware(cssp, firmware, firmSize, onGoingCb);
  if (i2cStatus != MSG_OK)
    return CSS811_I2C_ERROR;
  chThdSleepMilliseconds(500);

  // VERIFY
  i2cStatus = css811Verify(cssp);
  if (i2cStatus != MSG_OK)
    return CSS811_I2C_ERROR;
  chThdSleepMilliseconds(500);

  // verify that firmware is ok
  i2cStatus = css811GetStatus(cssp, &status);
  if (i2cStatus != MSG_OK)
    return CSS811_I2C_ERROR;
  if ((status & CSS811_STATUS_APP_VERIFY) == 0) {
    return CSS811_FWLOAD_FAILED;
  }
  if (onGoingCb)
    onGoingCb(cssp, CSS811_FIRMWARE_VERIFY, 0U);

  return CSS811_OK;
}


static void css811ErrorClearError(const Css811Driver *cssp)
{
  cssp->dmab->tx[0] = CSS811_ERROR_ID;
  css811I2CTransmit(cssp, 1U, 1U); // read to clear, don't care the answer
}

static void css811I2cResetOnError(const Css811Driver *cssp, msg_t i2cStatus)
{
  if (i2cStatus != MSG_OK) {
    const I2CConfig *cfg = cssp->i2cp->config;
    i2cStop(cssp->i2cp);
    i2cStart(cssp->i2cp, cfg);
  }
}


static Css811Status css811Error(uint8_t errorMask)
{
  errorMask &= 0b00111111;

  if (errorMask &  CSS811_ERROR_WRITE_REG_INVALID)
    return CSS811_WRITE_REG_INVALID;
  else if (errorMask & CSS811_ERROR_READ_REG_INVALID)
    return CSS811_READ_REG_INVALID;
  else if (errorMask & CSS811_ERROR_MEASMODE_INVALID)
    return CSS811_MEASMODE_INVALID;
  else if (errorMask & CSS811_ERROR_MAX_RESISTANCE)
    return CSS811_MAX_RESISTANCE;
  else if (errorMask & CSS811_ERROR_HEATER_FAULT)
    return CSS811_HEATER_FAULT;
  else if (errorMask & CSS811_ERROR_HEATER_SUPPLY)
    return CSS811_HEATER_SUPPLY;
  else
    return CSS811_OK;
}

static msg_t css811I2CTransmit(const Css811Driver *cssp, size_t txsize, size_t rxsize)
{
  i2cAcquireBus(cssp->i2cp);
  const msg_t i2cStatus = i2cMasterTransmitTimeout(cssp->i2cp,
						    cssp->i2cAdd,
						    cssp->dmab->tx,
						    txsize,
						    rxsize ? cssp->dmab->rx : NULL,
						    rxsize,
						    TIME_MS2I(100));
  css811I2cResetOnError(cssp, i2cStatus);
  i2cReleaseBus(cssp->i2cp);
  return i2cStatus;
}


#if CH_CFG_USE_MUTEXES_RECURSIVE
#define RECURSIVE_LOCK() i2cAcquireBus(cssp->i2cp)
#define RECURSIVE_RELEASE() i2cReleaseBus(cssp->i2cp)
#else
#define RECURSIVE_LOCK()
#define RECURSIVE_RELEASE()
#endif


static msg_t css811I2CTransmitFirmware(Css811Driver *cssp, const uint8_t *firmware,
				       size_t firmSize, FirmwareUploadCB onGoingCb)
{
  static const size_t chunkSize = 8U;
  _Static_assert(sizeof(cssp->dmab->tx) >= 9, "insufficient tx buffer size");
  
  RECURSIVE_LOCK();
  cssp->dmab->tx[0] = CSS811_APP_DATA;
  const uint8_t *upper = firmware + firmSize;
  const uint8_t *bottom = firmware;
  while (firmware < upper) {
    memcpy(cssp->dmab->tx+1, firmware, chunkSize);
    const msg_t i2cStatus = css811I2CTransmit(cssp, 1U + chunkSize, 0);
    chThdSleepMilliseconds(50); // see application note 371
    firmware += chunkSize;
    if (i2cStatus != MSG_OK) {
      RECURSIVE_RELEASE();
      return i2cStatus;
    }
    if (onGoingCb)
      onGoingCb(cssp, CSS811_FIRMWARE_LOAD, firmware - bottom);
  }
  RECURSIVE_RELEASE();
  return MSG_OK;
}
