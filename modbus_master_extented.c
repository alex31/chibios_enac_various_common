#include "modbus_master_extented.h"
#include "crc16_modbus.h"
#include <string.h>
#include "stdutil.h"

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
					 const ModbusFunction expectedFunction);


void modbusStart(ModbusDriver *mdp, const ModbusConfig *cfg)
{
  chDbgAssert((mdp != NULL) && (cfg != NULL), "need valid pointers");
  mdp->config = cfg;
  mdp->opTimestamp = chVTGetSystemTimeX();
  chMtxObjectInit(&mdp->mtx);
}

ModbusStatus modbusReadRegs(ModbusDriver *mdp, const uint16_t regAddr,
			    const uint16_t regNum, uint16_t *regBuffer)
{
  chDbgAssert(regNum != 0, "regNum == 0");
  chDbgAssert((regNum + regAddr) <= 32U, "0 < (regNum + regAddr) <= 32");
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

  chMtxLock(&mdp->mtx);
  sendFrameWithCrc(mdp, sizeof(RequestPDU));
  const ModbusStatus recStatus = receiveFrameWithCrc(mdp,
						     sizeof(ResponsePDU), request->funCode);
  if (recStatus == MODBUS_OK) {
    ResponsePDU *response = (ResponsePDU *) mdp->ioBuffer;
    memcpy(regBuffer, response->registers, regNum * sizeof(*regBuffer));
    if (mdp->config->endianness == MODBUS_BIG_ENDIAN)
      for (size_t i=0; i < regNum; i++) 
	regBuffer[i] =  __builtin_bswap16(regBuffer[i]);
  }
  chMtxUnlock(&mdp->mtx);
  return recStatus;
}


ModbusStatus modbusWriteRam(ModbusDriver *mdp, const uint32_t memAddr, const uint16_t bufLen,
			    const void* memBuffer)
{
  chDbgAssert((bufLen != 0) && (bufLen <= 64U), "0 < bufLen <= 64");
  chDbgAssert((memAddr + bufLen) < 1280U, "0 < memAddr < 1280");
  return writeMem(mdp,  MODBUS_WRITE_RAM, memAddr, bufLen, memBuffer);
}

ModbusStatus modbusReadRam(ModbusDriver *mdp, const uint32_t memAddr, const uint16_t bufLen,
			   void *memBuffer)
{
  chDbgAssert((bufLen != 0) && (bufLen <= 187U), "0 < bufLen <= 187");
  chDbgAssert((memAddr + bufLen) < 1280U, "0 < memAddr < 1280");
  return readMem(mdp, MODBUS_READ_RAM, memAddr, bufLen, memBuffer);
}

ModbusStatus modbusWriteEeprom(ModbusDriver *mdp, const uint32_t memAddr, const uint16_t bufLen,
			       const void* memBuffer)
{
  chDbgAssert((bufLen != 0) && (bufLen <= 64U), "0 < bufLen <= 64");
  chDbgAssert((memAddr + bufLen) < 32768, "0 < memAddr < 32768");
  return writeMem(mdp,  MODBUS_WRITE_EEPROM, memAddr, bufLen, memBuffer);
}

ModbusStatus modbusReadEeprom(ModbusDriver *mdp, const uint32_t memAddr, const uint16_t bufLen,
			      void* memBuffer)
 {
  chDbgAssert((bufLen != 0) && (bufLen <= 128U), "0 < bufLen <= 128");
  chDbgAssert((memAddr + bufLen) < 32768U, "0 < memAddr < 32768");
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

  chMtxLock(&mdp->mtx);
  sendFrameWithCrc(mdp, sizeof(RequestPDU));
  const ModbusStatus recStatus = receiveFrameWithCrc(mdp, sizeof(ResponsePDU) - 1U, funCode);
  chMtxUnlock(&mdp->mtx);
  
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

  chMtxLock(&mdp->mtx);
  sendFrameWithCrc(mdp, sizeof(RequestPDU));
  const ModbusStatus recStatus = receiveFrameWithCrc(mdp, sizeof(ResponsePDU), funCode);
  if (recStatus == MODBUS_OK) {
    ResponsePDU *response = (ResponsePDU *) mdp->ioBuffer;
    memcpy(memBuffer, response->data, bufLen * sizeof(*memBuffer));
  }
  chMtxUnlock(&mdp->mtx);
  return recStatus;
}
   

static void  sendFrameWithCrc(ModbusDriver *mdp, const uint8_t bufLen)
{
  // the header and the data are already written in mdp->ioBuffer
  uint16_t *crc = (uint16_t *) (mdp->ioBuffer + bufLen);
  *crc = modbus_crc16(mdp->ioBuffer, bufLen);
  chThdSleepUntilWindowed(mdp->opTimestamp, mdp->opTimestamp + mdp->config->timeBetweenOp);
  sdWrite(mdp->config->sd, mdp->ioBuffer, bufLen + 2U);
  mdp->opTimestamp = chVTGetSystemTimeX();
}

static ModbusStatus  receiveFrameWithCrc(ModbusDriver *mdp,
					 const uint8_t nominalBuffLen,
					 const ModbusFunction expectedFunction)

{
  const size_t blcrc = nominalBuffLen + 2U;
  const size_t readLen = sdReadTimeout(mdp->config->sd, mdp->ioBuffer, blcrc,
				       TIME_MS2I(100));
  mdp->opTimestamp = chVTGetSystemTimeX();
  if (readLen != blcrc) {
    //    DebugTrace("error readLen = %u instead of %u", readLen, blcrc);
    return readLen ? MODBUS_ARG_ERROR : MODBUS_NO_ANSWER;
  }
  if (mdp->ioBuffer[1] != expectedFunction) {
    // purge the receive queue and complete message with error codes
    sdReadTimeout(mdp->config->sd, mdp->ioBuffer + readLen, sizeof(mdp->ioBuffer) - readLen,
		  TIME_MS2I(100));
    return MODBUS_ARG_ERROR;
  }
  
  uint16_t *recCrc = (uint16_t *) (mdp->ioBuffer + nominalBuffLen);
  const uint16_t localCrc = modbus_crc16(mdp->ioBuffer, nominalBuffLen);
  if (localCrc != *recCrc) {
    //    DebugTrace("local Crc 0x%x != received Crc 0x%x", localCrc, *recCrc);
  }
  return *recCrc == localCrc ? MODBUS_OK : MODBUS_CRC_ERROR;
}
