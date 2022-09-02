#include <ch.h>
#include <hal.h>
#include "opc_n3.h"
#include "crc16_modbus.h"

typedef enum {
  CMD_POWER = 0x03, CMD_READ_STATUS = 0x13, CMD_FANLASER = 0x42,
  CMD_BINWEIGHT = 0x05, CMD_READ_INFO = 0x3F, CMD_READ_SERIAL = 0x10,
  CMD_READ_FWVER = 0x12, CMD_READ_CONF = 0x3C, CMD_WRITE_CONF = 0x3A,
  CMD_HISTOGRAM = 0x30, CMD_READ_PM = 0x32, CMD_SAVE_CONF = 0x43,
  CMD_CHECKSTATUS = 0xCF, CMD_RESET = 0x06, CMD_BOOTLOADER = 0x41
} Command;

typedef enum {OPCN3_BUSY = 0x31, OPCN3_READY = 0xF3} OpcN3Avail;

_Static_assert(sizeof(Option) == 1);
_Static_assert(sizeof(Command) == 1);
_Static_assert(sizeof(OPCN3FwConf) == 167);
_Static_assert(sizeof(OPCN3Histogram) == 86);
_Static_assert(sizeof(OPCN3PMData) == 14);

static inline void wait1ms(void) {chThdSleepMilliseconds(1);}
static inline void wait10ms(void) {chThdSleepMilliseconds(10);}
static inline void wait100ms(void) {chThdSleepMilliseconds(100);}
static inline void wait10us(void) {chThdSleepMicroseconds(10);}
static inline void wait100us(void) {chThdSleepMicroseconds(100);}
static inline void wait2s(void) {chThdSleepSeconds(2);}
static bool sendCommandByte(OPCN3Driver *drv, uint8_t commandByte);
static bool readBuffer(OPCN3Driver *drv, Command command, uint8_t *buffer,
		       size_t len);
static bool writeBuffer(OPCN3Driver *drv, Command command, const uint8_t *buffer,
			size_t len);
static uint8_t spiTransaction(SPIDriver *spid, uint8_t w);


OPCN3Status OPCN3Start(OPCN3Driver *drv, const OPCN3Config *cfg)
{
  drv->status = OPCN3_OK;
  drv->opcn3Cfg = cfg;
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

  if (OPCN3SendPower(drv, LASER_ON) != OPCN3_OK) {
    drv->status = OPCN3_NOT_RESPONDING;
    goto end;
  }

  if (OPCN3SendPower(drv, FAN_ON) != OPCN3_OK) {
    drv->status = OPCN3_NOT_RESPONDING;
    OPCN3SendPower(drv, LASER_OFF);
    goto end;
  }
   
 end:
  return drv->status;
}


static bool sendCommandByte(OPCN3Driver *drv, Command command)
{
  uint8_t status;
  size_t tries = 0;
  size_t bigLoopTries = 3;

  while (true) {
    status = spiTransaction(drv->opcn3Cfg->spid, command);
    if (status != OPCN3_READY) {
      if (status != OPCN3_BUSY) {
	return false; // status must be ready or busy otherwise there is malfunction
      }
      if (++tries < 20) {
	spiUnselect(drv->opcn3Cfg->spid);
	wait10ms();
	spiSelect(drv->opcn3Cfg->spid);
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
  
static bool readBuffer(OPCN3Driver *drv, Command command, uint8_t *buffer, size_t len)
{
  spiSelect(drv->opcn3Cfg->spid);
  const bool statusOk = sendCommandByte(drv, command);
  if (statusOk) {
    tprio_t p = chThdSetPriority(HIGHPRIO);
    for (size_t i=0; i < len; i++) {
      wait10us();
      buffer[i] = spiTransaction(drv->opcn3Cfg->spid, command);
    }
    chThdSetPriority(p);
  }
  spiUnselect(drv->opcn3Cfg->spid);
  
  return statusOk;
}
 
static bool writeBuffer(OPCN3Driver *drv, Command command, const uint8_t *buffer, size_t len)
{
  spiSelect(drv->opcn3Cfg->spid);
  const bool statusOk = sendCommandByte(drv, command);
  if (statusOk) {
    tprio_t p = chThdSetPriority(HIGHPRIO);
    for (size_t i=0; i < len; i++) {
      wait10us();
      spiTransaction(drv->opcn3Cfg->spid,  buffer[i]);
    }
    chThdSetPriority(p);
  }
  spiUnselect(drv->opcn3Cfg->spid);
  
  return statusOk;
}
 

OPCN3Status OPCN3SendPower(OPCN3Driver *drv, Option option)
{
  spiSelect(drv->opcn3Cfg->spid);
  const bool statusOk = sendCommandByte(drv, CMD_POWER);
  if (statusOk) {
    wait10ms();
    spiTransaction(drv->opcn3Cfg->spid, option);
  }
  spiUnselect(drv->opcn3Cfg->spid);
  switch (option) {
  case FAN_OFF : wait10ms(); break;
  case FAN_ON : wait2s(); break;
  case LASER_OFF : wait10ms(); break;
  case LASER_ON : wait100us(); break;
  }
  return statusOk ? OPCN3_OK :  OPCN3_NOT_RESPONDING;
}


// we dont offer to modify laser power as it necessitate calibration
// fan power modulation is useful in very dusty environment
OPCN3Status OPCN3SetFanPower(OPCN3Driver *drv, uint8_t power)
{
  spiSelect(drv->opcn3Cfg->spid);
  const bool statusOk = sendCommandByte(drv, CMD_FANLASER);

  if (statusOk) {
    wait10us();
    spiTransaction(drv->opcn3Cfg->spid, 0);
    wait10us();
    spiTransaction(drv->opcn3Cfg->spid, power);
  }
  
  spiUnselect(drv->opcn3Cfg->spid);
  return statusOk;
}

OPCN3Status OPCN3SetBinWeighting(OPCN3Driver *drv, uint8_t weight)
{
  spiSelect(drv->opcn3Cfg->spid);
  const bool statusOk = sendCommandByte(drv, CMD_BINWEIGHT);
  if (statusOk) {
    wait10us();
    spiTransaction(drv->opcn3Cfg->spid, weight);
  }
  spiUnselect(drv->opcn3Cfg->spid);
  return statusOk;
}

OPCN3Status OPCN3ReadInfo(OPCN3Driver *drv, char info[61])
{
  const bool statusOk = readBuffer(drv, CMD_READ_INFO, (uint8_t *) info,
				   60);
  info[60] = 0;
  return statusOk ? OPCN3_OK :  OPCN3_NOT_RESPONDING;
}

OPCN3Status OPCN3ReadSerial(OPCN3Driver *drv, char serial[61])
{
  const bool statusOk = readBuffer(drv, CMD_READ_SERIAL, (uint8_t *) serial,
				   60);
  serial[60] = 0;
  return statusOk ? OPCN3_OK :  OPCN3_NOT_RESPONDING;
}

OPCN3Status OPCN3WriteFwConf(OPCN3Driver *drv, const OPCN3FwConf *conf)
{
  const bool statusOk = writeBuffer(drv, CMD_WRITE_CONF, (const uint8_t *) conf,
				   sizeof(OPCN3FwConf));
  return statusOk ? OPCN3_OK :  OPCN3_NOT_RESPONDING;
}

OPCN3Status OPCN3SaveConf(OPCN3Driver *drv)
{
  const uint8_t unlock[5] = {0x3f, 0x3c,0x3f, 0x3c, 0x43}; 
  const bool statusOk = writeBuffer(drv, CMD_SAVE_CONF, unlock,
				   sizeof(unlock));
  return statusOk ? OPCN3_OK :  OPCN3_NOT_RESPONDING;
}

OPCN3Status OPCN3CheckStatus(OPCN3Driver *drv)
{
  spiSelect(drv->opcn3Cfg->spid);
  const bool statusOk = sendCommandByte(drv, CMD_CHECKSTATUS);
  spiUnselect(drv->opcn3Cfg->spid);
  return statusOk ? OPCN3_OK :  OPCN3_NOT_RESPONDING;
}

OPCN3Status OPCN3EnterBootLoaderMode(OPCN3Driver *drv)
{
  spiSelect(drv->opcn3Cfg->spid);
  const bool statusOk = sendCommandByte(drv, CMD_BOOTLOADER);
  spiUnselect(drv->opcn3Cfg->spid);
  return statusOk ? OPCN3_OK :  OPCN3_NOT_RESPONDING;
}

OPCN3Status OPCN3Reset(OPCN3Driver *drv)
{
  spiSelect(drv->opcn3Cfg->spid);
  const bool statusOk = sendCommandByte(drv, CMD_RESET);
  return statusOk ? OPCN3_OK :  OPCN3_NOT_RESPONDING;
}

#define OPCN3_DECL_READ(fun, cmd, type) \
  OPCN3Status  fun(OPCN3Driver *drv, type *var) \
  { \
    const bool statusOk = readBuffer(drv, cmd, (uint8_t *) var, \
				   sizeof(type)); \
  return statusOk ? OPCN3_OK :  OPCN3_NOT_RESPONDING; \
}

OPCN3_DECL_READ(OPCN3ReadDacAndPowerStatus, CMD_READ_STATUS, OPCN3DacAndPowerStatus);
OPCN3_DECL_READ(OPCN3ReadFwVersion, CMD_READ_FWVER, OPCN3FwVersion);
OPCN3_DECL_READ(OPCN3ReadFwConf, CMD_READ_CONF, OPCN3FwConf);
static OPCN3_DECL_READ(OPCN3ReadHistogram_r, CMD_HISTOGRAM, OPCN3Histogram);
static OPCN3_DECL_READ(OPCN3ReadPMData_r, CMD_READ_PM, OPCN3PMData);

OPCN3Status OPCN3ReadHistogram(OPCN3Driver *drv, OPCN3Histogram *data)
{
  OPCN3Status status = OPCN3ReadHistogram_r(drv, data);
  if (status == OPCN3_OK) {
    if (data->crc != modbus_crc16((uint8_t *) data, sizeof(*data) - sizeof(data->crc))) {
      status = OPCN3_CRC_ERROR;
    }
  }
  return status;
}
    
OPCN3Status OPCN3ReadPMData(OPCN3Driver *drv, OPCN3PMData *data)
{
  OPCN3Status status = OPCN3ReadPMData_r(drv, data);
  if (status == OPCN3_OK) {
    if (data->crc != modbus_crc16((uint8_t *) data, sizeof(*data) - sizeof(data->crc))) {
      status = OPCN3_CRC_ERROR;
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


