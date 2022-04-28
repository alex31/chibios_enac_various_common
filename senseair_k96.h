#pragma once

#include <ch.h>
#include <hal.h>
#include "modbus_master_extented.h"


typedef enum {
  K96_OK=MODBUS_OK, K96_CRC_ERROR=MODBUS_CRC_ERROR, K96_ARG_ERROR=MODBUS_ARG_ERROR,
  K96_INTERNAL = 0xF0,
  K96_FATAL_ERROR = 0xF0 | (1 << 0),
  K96_CONF_ERROR = 0xF0 | (1 << 2),
  K96_CALIB_ERROR = 0xF0 | (1 << 3),
  K96_SELFDIAG_ERROR = 0xF0 | (1 << 4),
  K96_MEMORY_ERROR = 0xF0 | (1 << 6),
  K96_WARMUP = 0xF0 | (1 << 7),
  K96_LOGGER_ERROR = 0xF0 | (1 << 8),
  K96_SPI_ERROR = 0xF0 | (1 << 9),
  K96_NTC_ERROR = 0xF0 | (1 << 10),
  K96_ADUCDIE_ERROR = 0xF0 | (1 << 11),
  K96_MPL_ERROR = 0xF0 | (1 << 13),
  K96_SPL_ERROR = 0xF0 | (1 << 14),
  K96_LPL_ERROR = 0xF0 | (1 << 15)
} K96Status;
_Static_assert(sizeof(K96Status) == sizeof(uint16_t),
	       "enum K96Status size must be uint16_t size");

typedef struct {
  SerialDriver *sd;
} SenseairK96Config;

typedef struct {
  ModbusDriver	mdrv;
  union {
    struct {
      int16_t	lpl;
      int16_t	spl;
      int16_t	mpl;
      int16_t	pressure;
      int16_t	tempNtc0;
      int16_t	tempNtc1;
      int16_t	reserved;
      int16_t	tempAduc;
      int16_t	humidity;
      int16_t	tempBme280;
    };
    uint16_t data[10];
  };
  K96Status status;
} SenseairK96Driver;

typedef struct {
  uint8_t address;
  uint8_t pgaGain;
  uint32_t meterId;
  uint32_t mapVersion;
  uint8_t fwType;
  uint8_t revMain;
  uint8_t revSub;
  uint8_t revAux;
  uint8_t frac;
} SenseairK96Info;


typedef struct {
  int32_t irLow;
  int32_t irHigh;
  int32_t irSignal;
  double irSignalFiltered;
} SenseairK96IrChannel;

typedef struct {
  SenseairK96IrChannel lpl;
  SenseairK96IrChannel spl;
  SenseairK96IrChannel mpl;
} SenseairK96IrRaw;



#ifdef __cplusplus
extern "C" {
#endif

  void k96Start(SenseairK96Driver *k96d, const SenseairK96Config *cfg);
  ModbusStatus k96Fetch(SenseairK96Driver *k96d);
  ModbusStatus k96FetchInfo(SenseairK96Driver *k96d, SenseairK96Info *k96i);
  ModbusStatus k96FetchIrRaw(SenseairK96Driver *k96d, SenseairK96IrRaw *k96ir);
  ModbusStatus k96ReadRam(SenseairK96Driver *k96d, const uint32_t k96RamAddr,
			  void *buffer, const size_t bufferSize);
  ModbusStatus k96WriteRam(SenseairK96Driver *k96d, const uint32_t k96RamAddr,
			  const void *buffer, size_t bufferSize);
  ModbusStatus k96ReadEeprom(SenseairK96Driver *k96d, const uint32_t k96EepromAddr,
			  void *buffer, const size_t bufferSize);
  ModbusStatus k96WriteEeprom(SenseairK96Driver *k96d, const uint32_t k96EepromAddr,
			  const void *buffer, const size_t bufferSize);
  static inline K96Status k96GetStatus(const SenseairK96Driver *k96d) {return k96d->status;}
  static inline int16_t k96GetLpl(const SenseairK96Driver *k96d) {return k96d->lpl;}
  static inline int16_t k96GetSpl(const SenseairK96Driver *k96d) {return k96d->spl;}
  static inline int16_t k96GetMpl(const SenseairK96Driver *k96d) {return k96d->mpl;}
  static inline int16_t k96GetPressure(const SenseairK96Driver *k96d) {return k96d->pressure;}
  static inline int16_t k96GetTempNtc0(const SenseairK96Driver *k96d) {return k96d->tempNtc0;}
  static inline int16_t k96GetTempNtc1(const SenseairK96Driver *k96d) {return k96d->tempNtc1;}
  static inline int16_t k96GetTempAduc(const SenseairK96Driver *k96d) {return k96d->tempAduc;}
  static inline int16_t k96GetHumidity(const SenseairK96Driver *k96d) {return k96d->humidity;}
  static inline int16_t k96GetTempBme280(const SenseairK96Driver *k96d) {return k96d->tempBme280;}
  
#ifdef __cplusplus
}
#endif
  
