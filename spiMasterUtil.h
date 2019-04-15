#pragma once

#include "hal.h"
#include "ch.h"
#include "string.h"
#include "stdutil.h"

//void 	spiExchange (SPIDriver *spip, size_t n, const void *txbuf, void *rxbuf)
//void 	spiSend (SPIDriver *spip, size_t n, const void *txbuf)
//void 	spiReceive (SPIDriver *spip, size_t n, void *rxbuf)
//uint16_t spiPolledExchange(spip, frame) 


#define SPI_WRITE_REGISTERS(spid, regAdr,...)   {			\
    spiSelect(spid);							\
    const uint8_t w_array[] __attribute__((aligned(CACHE_ALIGN))) = {regAdr, __VA_ARGS__}; \
    cacheBufferFlush(w_array, sizeof(w_array));				\
    spiSend(spid, sizeof(w_array), w_array);				\
    spiUnselect(spid);							\
  }



//    spiPolledExchange(spid, regAdr | 0x80);				



static inline void spiReadRegisters(SPIDriver *spid, const uint8_t regAddr,
				    uint8_t *r_array, const size_t r_arrayLen)
{
  spiSelect(spid);
  const size_t w_len = r_arrayLen + 1;
  uint8_t w_array[w_len] __attribute__((aligned(CACHE_ALIGN)));
  uint8_t lr_array[w_len] __attribute__((aligned(CACHE_ALIGN)));
  w_array[0] = regAddr | 0x80;
  cacheBufferFlush( w_array, 2);
  spiExchange(spid, w_len, w_array, lr_array);
  cacheBufferInvalidate(lr_array, w_len);
  spiUnselect(spid);
  
  memcpy(r_array, lr_array+1, r_arrayLen);
}

static inline uint8_t spiReadOneRegister(SPIDriver *spid, const uint8_t regAddr)
{
  spiSelect(spid);
  uint8_t w_array[2] __attribute__((aligned(CACHE_ALIGN))) = {regAddr | 0x80, 0};
  uint8_t r_array[sizeof(w_array)] __attribute__((aligned(CACHE_ALIGN)));
  cacheBufferFlush( w_array, sizeof(w_array));
  spiExchange(spid, sizeof(w_array), w_array, r_array);
  cacheBufferInvalidate(r_array, sizeof(r_array));
  spiUnselect(spid);
  return r_array[1];
}


#define SPI_READ_REGISTERS(spid, regAdr, r_array)   {		\
    spiReadRegisters(spid, regAdr, r_array, sizeof(r_array));	\
}

