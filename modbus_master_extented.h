#pragma once

#include <ch.h>
#include <hal.h>


typedef enum  {MODBUS_LITTLE_ENDIAN, MODBUS_BIG_ENDIAN} ModbusEndianness;
typedef enum  {MODBUS_OK, MODBUS_CRC_ERROR, MODBUS_NO_ANSWER, MODBUS_ARG_ERROR} ModbusStatus;

typedef struct {
  SerialDriver    *sd;
  ModbusEndianness endianness;
  uint8_t	   slaveAddr;
} ModbusConfig ;

typedef struct {
  const ModbusConfig *config;
  uint8_t      ioBuffer[255U]; // max frame length
} ModbusDriver;


#ifdef __cplusplus
extern "C" {
#endif
  
  void modbusStart(ModbusDriver *mdp, const ModbusConfig *cfg);
  
  ModbusStatus modbusReadRegs(ModbusDriver *mdp, const uint16_t regAddr,
			      const uint16_t regNum, uint16_t *regBuffer);
  
  ModbusStatus modbusWriteRam(ModbusDriver *mdp, const uint32_t memAddr,
			      const uint16_t bufLen, const void *memBuffer);
  ModbusStatus modbusReadRam(ModbusDriver *mdp, const uint32_t memAddr,
			     const uint16_t bufLen, void *memBuffer);

  ModbusStatus modbusWriteEeprom(ModbusDriver *mdp, const uint32_t memAddr,
				 const uint16_t bufLen, const void *memBuffer);
  ModbusStatus modbusReadEeprom(ModbusDriver *mdp, const uint32_t memAddr,
				const uint16_t bufLen, void *memBuffer);
  
#ifdef __cplusplus
}
#endif
  
