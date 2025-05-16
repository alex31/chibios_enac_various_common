#pragma once

#include <ch.h>
#include <hal.h>
#include <string.h>
#include "hal_stm32_dma.h"

/*
  typedef struct
  {
  __IO uint32_t CCR;       *!< DMA channel x configuration register        *
__IO uint32_t CNDTR;       *!< DMA channel x number of data register       *
__IO uint32_t CPAR;        *!< DMA channel x peripheral address register   *
__IO uint32_t CMAR;        *!< DMA channel x memory address register       *
} DMA_Channel_TypeDef;


typedef struct
{
  __IO uint32_t CR;     *!< DMA stream x configuration register      *
  __IO uint32_t NDTR;   *!< DMA stream x number of data register     *
  __IO uint32_t PAR;    *!< DMA stream x peripheral address register *
  __IO uint32_t M0AR;   *!< DMA stream x memory 0 address register   *
  __IO uint32_t M1AR;   *!< DMA stream x memory 1 address register   *
  __IO uint32_t FCR;    *!< DMA stream x FIFO control register       *
} DMA_Stream_TypeDef;



*/

typedef struct {
#if STM32_DMA_ADVANCED
  DMA_Stream_TypeDef DMA_regs;
#else
  DMA_Channel_TypeDef DMA_regs;
#endif
  stm32_tim_t	     TIM_regs;
} TimerDmaCache;

void timerDmaCache_cache(TimerDmaCache *tdcp, const DMADriver *fromDma, const  stm32_tim_t *fromTim);
void timerDmaCache_restore(const TimerDmaCache *tdcp, DMADriver *toDma, stm32_tim_t *toTim);



