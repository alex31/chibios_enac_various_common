#include <ch.h>
#include <hal.h>
#include "opc_r2.h"
#include "crc16_modbus.h"

typedef enum {
  CMD_POWER = 0x03, CMD_READ_STATUS = 0x13, CMD_FANLASER = 0x42,
  CMD_BINWEIGHT = 0x05, CMD_READ_INFO = 0x3F, CMD_READ_SERIAL = 0x10,
  CMD_READ_FWVER = 0x12, CMD_READ_CONF = 0x3C, CMD_WRITE_CONF = 0x3A,
  CMD_HISTOGRAM = 0x30, CMD_READ_PM = 0x32, CMD_SAVE_CONF = 0x43,
  CMD_CHECKSTATUS = 0xCF, CMD_RESET = 0x06, CMD_BOOTLOADER = 0x41
} Command;

typedef enum {OPCR2_BUSY = 0x31, OPCR2_READY = 0xF3} OpcR2Avail;

_Static_assert(sizeof(Option) == 1);
_Static_assert(sizeof(Command) == 1);
_Static_assert(sizeof(OPCR2FwConf) == 167);
_Static_assert(sizeof(OPCR2Histogram) == 86);
_Static_assert(sizeof(OPCR2PMData) == 14);

static inline void wait1ms(void) {chThdSleepMilliseconds(1);}
static inline void wait10ms(void) {chThdSleepMilliseconds(10);}
static inline void wait100ms(void) {chThdSleepMilliseconds(100);}
static inline void wait10us(void) {chThdSleepMicroseconds(10);}
static inline void wait100us(void) {chThdSleepMicroseconds(100);}
static inline void wait2s(void) {chThdSleepSeconds(2);}
static bool sendCommandByte(OPCR2Driver *drv, uint8_t commandByte);
static bool readBuffer(OPCR2Driver *drv, Command command, uint8_t *buffer,
		       size_t len);
static bool writeBuffer(OPCR2Driver *drv, Command command, const uint8_t *buffer,
			size_t len);
static uint8_t spiTransaction(SPIDriver *spid, uint8_t w);


OPCR2Status OPCR2Start(OPCR2Driver *drv, const OPCR2Config *cfg)
{
  drv->status = OPCR2_OK;
  drv->opcr2Cfg = cfg;
  drv->spiCfg = (SPIConfig) {
    .circular = false,
    .slave = false,
    .data_cb = NULL,
    .error_cb = NULL,
    /* HW dependent part.*/
    .ssline = cfg->ssline,
    /* 650Khz, 8 bits word, CPHA=1, CPOL=0 */  // mode 1 : CPOL=0, CPHA=1
    
    .cr1 = SPI_CR1_CPHA | SPI_CR1_BR_2 | SPI_CR1_BR_1,
    .cr2 = 0
  };
  palSetLineMode(cfg->sckline,
		 PAL_MODE_ALTERNATE(cfg->sckaf) |
		 PAL_STM32_OSPEED_HIGHEST |
		 PAL_STM32_PUPDR_PULLDOWN);
  chThdSleepMilliseconds(10);
  spiStart(cfg->spid, &drv->spiCfg);

  if (OPCR2SendPower(drv, LASER_ON) != OPCR2_OK) {
    drv->status = OPCR2_NOT_RESPONDING;
    goto end;
  }

  if (OPCR2SendPower(drv, FAN_ON) != OPCR2_OK) {
    drv->status = OPCR2_NOT_RESPONDING;
    OPCR2SendPower(drv, LASER_OFF);
    goto end;
  }
   
 end:
  return drv->status;
}


static bool sendCommandByte(OPCR2Driver *drv, Command command)
{
  uint8_t status;
  size_t tries = 0;
  size_t bigLoopTries = 3;

  while (true) {
    status = spiTransaction(drv->opcr2Cfg->spid, command);
    if (status != OPCR2_READY) {
      if (status != OPCR2_BUSY) {
	return false; // status must be ready or busy otherwise there is malfunction
      }
      if (++tries < 20) {
	spiUnselect(drv->opcr2Cfg->spid);
	wait10ms();
	spiSelect(drv->opcr2Cfg->spid);
	wait1ms();
      } else {
	if (--bigLoopTries == 0)
	  return false;
	wait2s();
	tries = 0;
      }
    } else {
      break; // success
    }
  }
  return true;
}
  
static bool readBuffer(OPCR2Driver *drv, Command command, uint8_t *buffer, size_t len)
{
  spiSelect(drv->opcr2Cfg->spid);
  const bool statusOk = sendCommandByte(drv, command);
  if (statusOk) {
    tprio_t p = chThdSetPriority(HIGHPRIO);
    for (size_t i=0; i < len; i++) {
      wait10us();
      buffer[i] = spiTransaction(drv->opcr2Cfg->spid, command);
    }
    chThdSetPriority(p);
  }
  spiUnselect(drv->opcr2Cfg->spid);
  
  return statusOk;
}
 
static bool writeBuffer(OPCR2Driver *drv, Command command, const uint8_t *buffer, size_t len)
{
  spiSelect(drv->opcr2Cfg->spid);
  const bool statusOk = sendCommandByte(drv, command);
  if (statusOk) {
    tprio_t p = chThdSetPriority(HIGHPRIO);
    for (size_t i=0; i < len; i++) {
      wait10us();
      spiTransaction(drv->opcr2Cfg->spid,  buffer[i]);
    }
    chThdSetPriority(p);
  }
  spiUnselect(drv->opcr2Cfg->spid);
  
  return statusOk;
}
 

OPCR2Status OPCR2SendPower(OPCR2Driver *drv, Option option)
{
  spiSelect(drv->opcr2Cfg->spid);
  const bool statusOk = sendCommandByte(drv, CMD_POWER);
  if (statusOk) {
    wait10ms();
    spiTransaction(drv->opcr2Cfg->spid, option);
  }
  spiUnselect(drv->opcr2Cfg->spid);
  switch (option) {
  case FAN_OFF : wait10ms(); break;
  case FAN_ON : wait2s(); break;
  case LASER_OFF : wait10ms(); break;
  case LASER_ON : wait100us(); break;
  }
  return statusOk ? OPCR2_OK :  OPCR2_NOT_RESPONDING;
}


// we dont offer to modify laser power as it necessitate calibration
// fan power modulation is useful in very dusty environment
OPCR2Status OPCR2SetFanPower(OPCR2Driver *drv, uint8_t power)
{
  spiSelect(drv->opcr2Cfg->spid);
  const bool statusOk = sendCommandByte(drv, CMD_FANLASER);

  if (statusOk) {
    wait10us();
    spiTransaction(drv->opcr2Cfg->spid, 0);
    wait10us();
    spiTransaction(drv->opcr2Cfg->spid, power);
  }
  
  spiUnselect(drv->opcr2Cfg->spid);
  return statusOk;
}

OPCR2Status OPCR2SetBinWeighting(OPCR2Driver *drv, uint8_t weight)
{
  spiSelect(drv->opcr2Cfg->spid);
  const bool statusOk = sendCommandByte(drv, CMD_BINWEIGHT);
  if (statusOk) {
    wait10us();
    spiTransaction(drv->opcr2Cfg->spid, weight);
  }
  spiUnselect(drv->opcr2Cfg->spid);
  return statusOk;
}

OPCR2Status OPCR2ReadInfo(OPCR2Driver *drv, char info[61])
{
  const bool statusOk = readBuffer(drv, CMD_READ_INFO, (uint8_t *) info,
				   60);
  info[60] = 0;
  return statusOk ? OPCR2_OK :  OPCR2_NOT_RESPONDING;
}

OPCR2Status OPCR2ReadSerial(OPCR2Driver *drv, char serial[61])
{
  const bool statusOk = readBuffer(drv, CMD_READ_SERIAL, (uint8_t *) serial,
				   60);
  serial[60] = 0;
  return statusOk ? OPCR2_OK :  OPCR2_NOT_RESPONDING;
}

OPCR2Status OPCR2WriteFwConf(OPCR2Driver *drv, const OPCR2FwConf *conf)
{
  const bool statusOk = writeBuffer(drv, CMD_WRITE_CONF, (const uint8_t *) conf,
				   sizeof(OPCR2FwConf));
  return statusOk ? OPCR2_OK :  OPCR2_NOT_RESPONDING;
}

OPCR2Status OPCR2SaveConf(OPCR2Driver *drv)
{
  const uint8_t unlock[5] = {0x3f, 0x3c,0x3f, 0x3c, 0x43}; 
  const bool statusOk = writeBuffer(drv, CMD_SAVE_CONF, unlock,
				   sizeof(unlock));
  return statusOk ? OPCR2_OK :  OPCR2_NOT_RESPONDING;
}

OPCR2Status OPCR2CheckStatus(OPCR2Driver *drv)
{
  spiSelect(drv->opcr2Cfg->spid);
  const bool statusOk = sendCommandByte(drv, CMD_CHECKSTATUS);
  spiUnselect(drv->opcr2Cfg->spid);
  return statusOk ? OPCR2_OK :  OPCR2_NOT_RESPONDING;
}

OPCR2Status OPCR2EnterBootLoaderMode(OPCR2Driver *drv)
{
  spiSelect(drv->opcr2Cfg->spid);
  const bool statusOk = sendCommandByte(drv, CMD_BOOTLOADER);
  spiUnselect(drv->opcr2Cfg->spid);
  return statusOk ? OPCR2_OK :  OPCR2_NOT_RESPONDING;
}

OPCR2Status OPCR2Reset(OPCR2Driver *drv)
{
  spiSelect(drv->opcr2Cfg->spid);
  const bool statusOk = sendCommandByte(drv, CMD_RESET);
  return statusOk ? OPCR2_OK :  OPCR2_NOT_RESPONDING;
}

#define OPCR2_DECL_READ(fun, cmd, type) \
  OPCR2Status  fun(OPCR2Driver *drv, type *var) \
  { \
    const bool statusOk = readBuffer(drv, cmd, (uint8_t *) var, \
				   sizeof(type)); \
  return statusOk ? OPCR2_OK :  OPCR2_NOT_RESPONDING; \
}

OPCR2_DECL_READ(OPCR2ReadDacAndPowerStatus, CMD_READ_STATUS, OPCR2DacAndPowerStatus);
OPCR2_DECL_READ(OPCR2ReadFwVersion, CMD_READ_FWVER, OPCR2FwVersion);
OPCR2_DECL_READ(OPCR2ReadFwConf, CMD_READ_CONF, OPCR2FwConf);
static OPCR2_DECL_READ(OPCR2ReadHistogram_r, CMD_HISTOGRAM, OPCR2Histogram);
static OPCR2_DECL_READ(OPCR2ReadPMData_r, CMD_READ_PM, OPCR2PMData);

OPCR2Status OPCR2ReadHistogram(OPCR2Driver *drv, OPCR2Histogram *data)
{
  OPCR2Status status = OPCR2ReadHistogram_r(drv, data);
  if (status == OPCR2_OK) {
    if (data->crc != modbus_crc16((uint8_t *) data, sizeof(*data) - sizeof(data->crc))) {
      status = OPCR2_CRC_ERROR;
    }
  }
  return status;
}
    
OPCR2Status OPCR2ReadPMData(OPCR2Driver *drv, OPCR2PMData *data)
{
  OPCR2Status status = OPCR2ReadPMData_r(drv, data);
  if (status == OPCR2_OK) {
    if (data->crc != modbus_crc16((uint8_t *) data, sizeof(*data) - sizeof(data->crc))) {
      status = OPCR2_CRC_ERROR;
    }
  }
  return status;
}


static uint8_t spiTransaction(SPIDriver *spid, uint8_t tx)
{
#if ((! defined(SPI_OPC_POLLED) || SPI_OPC_POLLED == 0))
  uint8_t rx;
  spiExchange(spid, sizeof(tx), &tx, &rx);
  return rx;
#else
  return spiPolledExchange(spid, tx);
#endif
}


