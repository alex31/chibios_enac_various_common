#include "timerDmaCache.h"



static void rccEnableAndReset(const stm32_tim_t * const timer);




#define RESTORE_REG(r) toTim->r = tdcp->TIM_regs.r
static inline __attribute__((always_inline))
void register_restoreSelectedTimerReg(const TimerDmaCache *tdcp, stm32_tim_t *toTim)
{
  // just restore the needed registers to save few clock cycles
  RESTORE_REG(SMCR);	   
  RESTORE_REG(DIER);	   
  RESTORE_REG(CCMR1);   
  RESTORE_REG(CCMR2);   
  RESTORE_REG(CCER);	   
  RESTORE_REG(PSC);	   
  RESTORE_REG(ARR);	   
  RESTORE_REG(CCMR3);   
  RESTORE_REG(DCR);     
  RESTORE_REG(DMAR);    
}

#define SAVE_REG(r) tdcp->TIM_regs.r = toTim->r
static inline __attribute__((always_inline))
void register_cacheSelectedTimerReg(TimerDmaCache *tdcp, const stm32_tim_t *toTim)
{
  // just restore the needed registers to save few clock cycles
  SAVE_REG(SMCR);	   
  SAVE_REG(DIER);	   
  SAVE_REG(CCMR1);   
  SAVE_REG(CCMR2);   
  SAVE_REG(CCER);	   
  SAVE_REG(PSC);	   
  SAVE_REG(ARR);	   
  SAVE_REG(CCMR3);   
  SAVE_REG(DCR);     
  SAVE_REG(DMAR);    
}

void timerDmaCache_cache(TimerDmaCache *tdcp, const DMADriver *fromDma, const  stm32_tim_t *fromTim)
{
#if STM32_DMA_ADVANCED
  static_assert(__builtin_types_compatible_p(typeof(tdcp->DMA_regs), typeof(*fromDma->dmastream->stream)),
              "Incompatible types");
  memcpy(&tdcp->DMA_regs, fromDma->dmastream->stream, sizeof(tdcp->DMA_regs));
#else
  static_assert(__builtin_types_compatible_p(typeof(tdcp->DMA_regs), typeof(*fromDma->dmastream->channel)),
              "Incompatible types");
  memcpy(&tdcp->DMA_regs, fromDma->dmastream->channel, sizeof(tdcp->DMA_regs));
#endif

  register_cacheSelectedTimerReg(tdcp, fromTim);
#if STM32_DMA_ADVANCED
  tdcp->DMA_regs.CR &= ~STM32_DMA_CR_EN;
#else
  tdcp->DMA_regs.CCR &= ~STM32_DMA_CR_EN;
#endif
  tdcp->TIM_regs.CR1 &=  ~STM32_TIM_CR1_CEN;
}


void timerDmaCache_restore(const TimerDmaCache *tdcp, DMADriver *toDma, stm32_tim_t *toTim)
{
  rccEnableAndReset(toTim);
  register_restoreSelectedTimerReg(tdcp, toTim);
#if STM32_DMA_ADVANCED
  memcpy(toDma->dmastream->stream, &tdcp->DMA_regs, sizeof(tdcp->DMA_regs));
#else
  memcpy(toDma->dmastream->channel, &tdcp->DMA_regs, sizeof(tdcp->DMA_regs));
#endif

}

static void rccEnableAndReset(const stm32_tim_t * const timer)
{
#ifdef TIM1
  if (timer == STM32_TIM1) {
    rccDisableTIM1();
    rccEnableTIM1(true);
    rccResetTIM1();
  }
#endif
#ifdef TIM2
  else  if (timer == STM32_TIM2) {
    rccEnableTIM2(true);
    rccResetTIM2();
  }
#endif
#ifdef TIM3
  else  if (timer == STM32_TIM3) {
    rccEnableTIM3(true);
    rccResetTIM3();
  }
#endif
#ifdef TIM4
  else  if (timer == STM32_TIM4) {
    rccEnableTIM4(true);
    rccResetTIM4();
  }
#endif
#ifdef TIM5
  else  if (timer == STM32_TIM5) {
    rccEnableTIM5(true);
    rccResetTIM5();
  }
#endif
#ifdef TIM8
  else  if (timer == STM32_TIM8) {
    rccEnableTIM8(true);
    rccResetTIM8();
  }
#endif
#ifdef TIM9
  else  if (timer == STM32_TIM9) {
    rccEnableTIM9(true);
    rccResetTIM9();
  }
#endif
#ifdef TIM10
  else  if (timer == STM32_TIM10) {
    rccEnableTIM10(true);
    rccResetTIM10();
  }
#endif
#ifdef TIM11
  else  if (timer == STM32_TIM11) {
    rccEnableTIM11(true);
    rccResetTIM11();
  }
#endif
#ifdef TIM12
  else  if (timer == STM32_TIM12) {
    rccEnableTIM12(true);
    rccResetTIM12();
  }
#endif
#ifdef TIM13
  else  if (timer == STM32_TIM13) {
    rccEnableTIM13(true);
    rccResetTIM13();
  }
#endif
#ifdef TIM14
  else  if (timer == STM32_TIM14) {
    rccEnableTIM14(true);
    rccResetTIM14();
  }
#endif
#ifdef TIM15
  else  if (timer == STM32_TIM15) {
    rccEnableTIM15(true);
    rccResetTIM15();
  }
#endif
#ifdef TIM16
  else  if (timer == STM32_TIM16) {
    rccEnableTIM16(true);
    rccResetTIM16();
  }
#endif
#ifdef TIM17
  else  if (timer == STM32_TIM17) {
    rccEnableTIM17(true);
    rccResetTIM17();
  }
#endif
#ifdef TIM18
  else  if (timer == STM32_TIM18) {
    rccEnableTIM18(true);
    rccResetTIM18();
  }
#endif
#ifdef TIM19
  else  if (timer == STM32_TIM19) {
    rccEnableTIM19(true);
    rccResetTIM19();
  }
#endif
  else {
    chSysHalt("not a valid timer");
  }
}
