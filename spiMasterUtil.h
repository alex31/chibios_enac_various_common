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
    uint8_t CACHE_ALIGNED(w_array[]) = {regAdr, __VA_ARGS__};		\
    cacheBufferFlush(w_array, 1);				\
    spiSend(spid, sizeof(w_array), w_array);				\
    spiUnselect(spid);							\
  }



//    spiPolledExchange(spid, regAdr | 0x80);				

static inline void spiWriteRegister(SPIDriver *spid, const uint8_t regAddr,
				    const uint8_t regVal);

static inline void spiReadRegisters(SPIDriver *spid, const uint8_t regAddr,
				    uint8_t *r_array, const size_t r_arrayLen)
{
  chDbgAssert(r_arrayLen <= 128, "use spiDirectReadRegisters for large read");
  spiSelect(spid);
  const size_t w_len = r_arrayLen + 1;
  uint8_t CACHE_ALIGNED(w_array[w_len]);
  uint8_t CACHE_ALIGNED(lr_array[w_len]);
  w_array[0] = regAddr | 0x80;
  cacheBufferFlush( w_array, w_len);
  spiExchange(spid, w_len, w_array, lr_array);
  cacheBufferInvalidate(lr_array, w_len);
  spiUnselect(spid);
  
  memcpy(r_array, lr_array+1, r_arrayLen);
}

static inline void spiDirectReadRegisters(SPIDriver *spid, const uint8_t regAddr,
					  uint8_t *r_array, const size_t r_arrayLen)
{
  spiSelect(spid);
  uint8_t CACHE_ALIGNED(w_array[1]);
  w_array[0] = regAddr | 0x80;
  cacheBufferFlush(w_array, sizeof(w_array));
  spiSend(spid, sizeof(w_array), w_array);
  spiReceive(spid, r_arrayLen, r_array);
  cacheBufferInvalidate(r_array, r_arrayLen);
  spiUnselect(spid);
}

static inline uint8_t spiReadOneRegister(SPIDriver *spid, const uint8_t regAddr)
{
  spiSelect(spid);
  uint8_t CACHE_ALIGNED(w_array[2]) = {regAddr | 0x80, 0};
  uint8_t CACHE_ALIGNED(r_array[2]);

  cacheBufferFlush( w_array, sizeof(w_array));
  spiExchange(spid, sizeof(w_array), w_array, r_array);
  cacheBufferInvalidate(r_array, sizeof(r_array));
  spiUnselect(spid);
  return r_array[1];
}

static inline void spiModifyRegister(SPIDriver *spid, const uint8_t regAddr,
				     const uint8_t clearMask, const uint8_t setMask)
{
  uint8_t reg = spiReadOneRegister(spid, regAddr);
  MODIFY_REG(reg, clearMask, setMask);
  spiWriteRegister(spid, regAddr, reg);
}

static inline void spiSetBitsRegister(SPIDriver *spid, const uint8_t regAddr, const uint8_t setMask)
{
  uint8_t reg = spiReadOneRegister(spid, regAddr);
  reg |= setMask;
  spiWriteRegister(spid, regAddr, reg);
}

static inline void spiWriteRegister(SPIDriver *spid, const uint8_t regAddr,
				    const uint8_t regVal)
{
    spiSelect(spid);							
    uint8_t CACHE_ALIGNED(w_array[2]) = {regAddr, regVal};		
    cacheBufferFlush(w_array, 1);				
    spiSend(spid, sizeof(w_array), w_array);				
    spiUnselect(spid);							
}

#define SPI_READ_REGISTERS(spid, regAdr, r_array)   {		\
    spiReadRegisters(spid, regAdr, r_array, sizeof(r_array));	\
}

