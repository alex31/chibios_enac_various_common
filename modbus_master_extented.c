#include "modbus_master_extented.h"
#include "crc16_modbus.h"
#include <string.h>


typedef enum  {MODBUS_READ_INPUT_REGISTERS = 0x04,
  MODBUS_WRITE_RAM = 0x41,
  MODBUS_WRITE_EEPROM = 0x43,
  MODBUS_READ_RAM = 0x44,
  MODBUS_READ_EEPROM = 0x46
} ModbusFunction;


static ModbusStatus  writeMem(ModbusDriver *mdp, const uint8_t funCode,
			      const uint16_t memAddr, const uint8_t bufLen,
			      const uint8_t *memBuffer);

static ModbusStatus  readMem(ModbusDriver *mdp, const uint8_t funCode,
			     const uint16_t memAddr,
			     const uint8_t bufLen, uint8_t *memBuffer);

static void	     sendFrameWithCrc(ModbusDriver *mdp, const uint8_t bufLen);
static ModbusStatus  receiveFrameWithCrc(ModbusDriver *mdp,
					 const uint8_t nominalBuffLen,
					 const uint8_t errorBufflen);

static inline uint16_t modbusCrc16(ModbusEndianness me,
				   const uint8_t *buf, size_t len) {
  uint16_t crc = modbus_crc16(buf, len);
  if (me ==  MODBUS_BIG_ENDIAN)
    crc = __builtin_bswap16(me);
  return crc;
}

void modbusStart(ModbusDriver *mdp, const ModbusConfig *cfg)
{
  chDbgAssert((mdp != NULL) && (cfg != NULL), "need valid pointers");
  mdp->config = cfg;
}

ModbusStatus modbusReadRegs(ModbusDriver *mdp, const uint16_t regAddr, const uint16_t regNum, uint16_t *regBuffer)
{
  chDbgAssert(regNum != 0, "regNum == 0");
  chDbgAssert((regNum + regAddr) <= 32U, "(regNum + regAddr) > 32");
  typedef struct __attribute__((packed)) {
    uint8_t devAddr;
    uint8_t funCode;
    uint16_t addr;
    uint16_t  nb;
  } RequestPDU;
  _Static_assert(sizeof(RequestPDU) == 6, "size error");
  
  typedef struct __attribute__((packed)) {
    uint8_t devAddr;
    union {
      struct {
	uint8_t funCode;
	uint8_t count;
	uint16_t  registers[regNum];
      };
      struct {
	uint8_t errorCode;
	uint8_t exceptionCode;
      };
    };
  } ResponsePDU;

  
  RequestPDU *request = (RequestPDU *) mdp->ioBuffer;
  request->devAddr = mdp->config->slaveAddr;
  request->funCode = MODBUS_READ_INPUT_REGISTERS;

  if (mdp->config->endianness == MODBUS_BIG_ENDIAN) {
    request->addr = __builtin_bswap16(regAddr);
    request->nb = __builtin_bswap16(regNum);
  } else {
    request->addr = regAddr;
    request->nb = regNum;
  }

  sendFrameWithCrc(mdp, sizeof(RequestPDU));
  const ModbusStatus recStatus = receiveFrameWithCrc(mdp,
						     sizeof(ResponsePDU), sizeof(ResponsePDU)-2U);
  if (recStatus == MODBUS_OK) {
    ResponsePDU *response = (ResponsePDU *) mdp->ioBuffer;
    memcpy(regBuffer, response->registers, regNum * sizeof(*regBuffer));
  }
  return recStatus;
}


ModbusStatus modbusWriteRam(ModbusDriver *mdp, const uint32_t memAddr, const uint16_t bufLen, const uint8_t *memBuffer)
{
  chDbgAssert((bufLen != 0) && (bufLen < 64U), "0 < bufLen < 64");
  chDbgAssert((memAddr != 0) && ((memAddr + bufLen) < 1280U), "0 < memAddr < 1280");
  return writeMem(mdp,  MODBUS_WRITE_RAM, memAddr, bufLen, memBuffer);
}

ModbusStatus modbusReadRam(ModbusDriver *mdp, const uint32_t memAddr, const uint16_t bufLen, uint8_t *memBuffer)
{
  chDbgAssert((bufLen != 0) && (bufLen < 188U), "0 < bufLen < 188");
  chDbgAssert((memAddr != 0) && ((memAddr + bufLen) < 1280U), "0 < memAddr < 1280");
  return readMem(mdp, MODBUS_READ_RAM, memAddr, bufLen, memBuffer);
}

ModbusStatus modbusWriteEeprom(ModbusDriver *mdp, const uint32_t memAddr, const uint16_t bufLen, const uint8_t *memBuffer)
{
  chDbgAssert((bufLen != 0) && (bufLen < 64U), "0 < bufLen < 64");
  chDbgAssert((memAddr != 0) && ((memAddr + bufLen) < 32768U), "0 < memAddr < 32768");
  return writeMem(mdp,  MODBUS_WRITE_EEPROM, memAddr, bufLen, memBuffer);
}

ModbusStatus modbusReadEeprom(ModbusDriver *mdp, const uint32_t memAddr, const uint16_t bufLen, uint8_t *memBuffer)
 {
  chDbgAssert((bufLen != 0) && (bufLen <= 128U), "0 < bufLen < 129");
  chDbgAssert((memAddr != 0) && ((memAddr + bufLen) < 32768U), "0 < memAddr < 32768");
  return readMem(mdp,  MODBUS_READ_EEPROM, memAddr, bufLen, memBuffer);
}
  
static ModbusStatus  writeMem(ModbusDriver *mdp, const uint8_t funCode,
			      const uint16_t memAddr,
			      const uint8_t bufLen, const uint8_t *memBuffer)
{
  chDbgAssert(bufLen != 0, "bufLen == 0");
  typedef struct __attribute__((packed)) {
    uint8_t devAddr;
    uint8_t funCode;
    uint16_t addr;
    uint8_t  nb;
    uint8_t  data[bufLen];
  } RequestPDU;


  typedef struct __attribute__((packed)) {
    uint8_t devAddr;
    union {
      struct {
	uint8_t funCode;
      };
      struct {
	uint8_t errorCode;
	uint8_t exceptionCode;
      };
    };
  } ResponsePDU;
  _Static_assert(sizeof(ResponsePDU) == 3, "size error");
  
  RequestPDU *request = (RequestPDU *) mdp->ioBuffer;
  request->devAddr = mdp->config->slaveAddr;
  request->funCode = funCode;
  request->nb = bufLen;
  memcpy(request->data, memBuffer, bufLen);
  if (mdp->config->endianness == MODBUS_BIG_ENDIAN) {
    request->addr = __builtin_bswap16(memAddr);
  } else {
    request->addr = memAddr;
  }

  sendFrameWithCrc(mdp, sizeof(RequestPDU));
  const ModbusStatus recStatus = receiveFrameWithCrc(mdp, sizeof(ResponsePDU)-1U, sizeof(ResponsePDU));
  
  return recStatus;
}
   
static ModbusStatus  readMem(ModbusDriver *mdp, const uint8_t funCode,
			      const uint16_t memAddr,
			      const uint8_t bufLen, uint8_t *memBuffer)
{
  chDbgAssert(bufLen != 0, "bufLen == 0");
  typedef struct __attribute__((packed)) {
    uint8_t devAddr;
    uint8_t funCode;
    uint16_t addr;
    uint8_t  nb;
  } RequestPDU;
  _Static_assert(sizeof(RequestPDU) == 5, "size error");


  typedef struct __attribute__((packed)) {
    uint8_t devAddr;
    union {
      struct {
	uint8_t funCode;
	uint8_t  nb;
	uint8_t  data[bufLen];
      };
      struct {
	uint8_t errorCode;
	uint8_t exceptionCode;
      };
    };
  } ResponsePDU;
  
  RequestPDU *request = (RequestPDU *) mdp->ioBuffer;
  request->devAddr = mdp->config->slaveAddr;
  request->funCode = funCode;
  request->nb = bufLen;
  if (mdp->config->endianness == MODBUS_BIG_ENDIAN) {
    request->addr = __builtin_bswap16(memAddr);
  } else {
    request->addr = memAddr;
  }

  sendFrameWithCrc(mdp, sizeof(RequestPDU));
  const ModbusStatus recStatus = receiveFrameWithCrc(mdp, 3U, sizeof(ResponsePDU));
  if (recStatus == MODBUS_OK) {
    ResponsePDU *response = (ResponsePDU *) mdp->ioBuffer;
    memcpy(memBuffer, response->data, bufLen * sizeof(*memBuffer));
  }
  
  return recStatus;
}
   

static void  sendFrameWithCrc(ModbusDriver *mdp, const uint8_t bufLen)
{
  // the header and the data are already written in mdp->ioBuffer
  uint16_t *crc = (uint16_t *) (mdp->ioBuffer + bufLen);
  *crc = modbusCrc16(mdp->config->endianness, mdp->ioBuffer, bufLen);
  sdWrite(mdp->config->sd, mdp->ioBuffer, bufLen + 2U);
}

static ModbusStatus  receiveFrameWithCrc(ModbusDriver *mdp,
					 const uint8_t nominalBuffLen,
					 const uint8_t errorBufflen)
{
  const size_t bufLen = nominalBuffLen > errorBufflen ? nominalBuffLen : errorBufflen;
  const size_t blcrc = bufLen + 2U;
  const size_t readLen = sdReadTimeout(mdp->config->sd, mdp->ioBuffer, blcrc,
				       TIME_MS2I(100));
  if (readLen != nominalBuffLen) 
    return MODBUS_ARG_ERROR;

  uint16_t *recCrc = (uint16_t *) (mdp->ioBuffer + nominalBuffLen);
  const uint16_t localCrc = modbusCrc16(mdp->config->endianness,
					mdp->ioBuffer, nominalBuffLen);
  return *recCrc == localCrc ? MODBUS_OK : MODBUS_CRC_ERROR;
}
