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

#ifdef __cplusplus
extern "C" {
#endif

  void k96Start(SenseairK96Driver *k96d, const SenseairK96Config *cfg);
  ModbusStatus k96Fetch(SenseairK96Driver *k96d);
  inline K96Status getStatus(const SenseairK96Driver *k96d) {return k96d->status;}
  inline int16_t getLpl(const SenseairK96Driver *k96d) {return k96d->lpl;}
  inline int16_t getSpl(const SenseairK96Driver *k96d) {return k96d->spl;}
  inline int16_t getMpl(const SenseairK96Driver *k96d) {return k96d->mpl;}
  inline int16_t getPressure(const SenseairK96Driver *k96d) {return k96d->pressure;}
  inline int16_t getTempNtc0(const SenseairK96Driver *k96d) {return k96d->tempNtc0;}
  inline int16_t getTempNtc1(const SenseairK96Driver *k96d) {return k96d->tempNtc1;}
  inline int16_t getTempAduc(const SenseairK96Driver *k96d) {return k96d->tempAduc;}
  inline int16_t getHumidity(const SenseairK96Driver *k96d) {return k96d->humidity;}
  inline int16_t getTempBme280(const SenseairK96Driver *k96d) {return k96d->tempBme280;}
  
#ifdef __cplusplus
}
#endif
  
