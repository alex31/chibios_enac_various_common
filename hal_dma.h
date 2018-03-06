#pragma once

#include <ch.h>
#include <hal.h>


typedef struct {
  // 32 bits width Public FIELD
  stm32_dmaisr_t	serve_dma_isr;
  void *		serve_dma_isr_arg;
  volatile void *	periph_addr;
  uint32_t		direction; // STM32_DMA_CR_DIR_P2M, STM32_DMA_CR_DIR_M2P, STM32_DMA_CR_DIR_M2M
  bool			inc_peripheral_addr;
  bool			inc_memory_addr;
  bool			circular;
  uint32_t		isr_flags; // combination of STM32_DMA_CR_[TCIE, HTIE, DMEIE, TEIE]
				  // transfert complete, half transfert, direct mode error,
				  // transfert error


  // 32 bit width PRIVATE FIELD, DOT NOT USE
  const stm32_dma_stream_t  *dmastream;
  uint32_t		     dmamode;
  // End of 32 bits width PRIVATE FIELD
  
  //  8 bits width Public FIELD
  uint8_t		controller;
  uint8_t		stream;
  uint8_t		channel;
  uint8_t		priority;
  uint8_t		psize; // 1,2,4
  uint8_t		msize; // 1,2,4
  
#if STM32_DMA_ADVANCED
#define DMA_FIFO_SIZE 4 // hardware specification for dma V2
  uint8_t		pburst; // 0(burst disabled), 4, 8, 16  
  uint8_t		mburst; // 0(burst disabled), 4, 8, 16 
  uint8_t		fifo; // 0(fifo disabled), 1, 2, 3, 4 : 25, 50, 75, 100% 
#endif

} DMAConfig ;


bool dma_start(DMAConfig *cfg);
void dma_stop(DMAConfig *cfg);

void dma_mtransfert(DMAConfig *cfg, void *membuffer, const size_t size);
bool dma_start_mtransfert(DMAConfig *cfg, void *from, void *to, const size_t size);

bool dma_ptransfert(DMAConfig *cfg, void *membuffer, const size_t size);
bool dma_start_ptransfert(DMAConfig *cfg, void *membuffer, const size_t size);
void dma_stop_transfert(DMAConfig *cfg);
