#include "hal_stm32_lld_icu_opt.h"

/**
 * @brief   ICUD15 driver identifier.
 * @note    The driver ICUD1 allocates the timer TIM15 when enabled.
 */
/**
 * @brief   ICUD15 driver identifier.
 * @note    The driver ICUD1 allocates the timer TIM15 when enabled.
 */
#if STM32_ICU_USE_TIM15 || defined(__DOXYGEN__)
ICUDriver ICUD15;
#endif


#if STM32_ICU_USE_TIM15 || defined(__DOXYGEN__)
#if !defined(STM32_TIM15_SUPPRESS_ISR)
#if !defined(STM32_TIM15_HANDLER)
#error "STM32_TIM15_HANDLER not defined"
#endif
/**
 * @brief   TIM15 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_TIM15_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  icu_lld_serve_interrupt(&ICUD15);

  OSAL_IRQ_EPILOGUE();
}
#endif /* !defined(STM32_TIM15_SUPPRESS_ISR) */
#endif /* STM32_ICU_USE_TIM15 */

void icu_opt_lld_init(void)
{
#if STM32_ICU_USE_TIM15
  /* Driver initialization.*/
  icuObjectInit(&ICUD15);
  ICUD15.tim = STM32_TIM15;
#endif
}



void icuOptStart(ICUDriver *icup, const ICUConfig *config) {

  osalDbgCheck((icup != NULL) && (config != NULL));

  osalSysLock();
  osalDbgAssert((icup->state == ICU_STOP) || (icup->state == ICU_READY),
                "invalid state");
  icup->config = config;
  icu_opt_lld_start(icup);
  icup->state = ICU_READY;
  osalSysUnlock();
}




void icu_opt_lld_start(ICUDriver *icup)
{
  osalDbgAssert((icup->config->channel == ICU_CHANNEL_1) ||
                (icup->config->channel == ICU_CHANNEL_2),
                "invalid input");
  
  if (icup->state == ICU_STOP) {
    /* Clock activation and timer reset.*/
#if STM32_ICU_USE_TIM15
    if (&ICUD15 == icup) {
      rccEnableTIM15(true);
      rccResetTIM15();
#if !defined(STM32_TIM15_SUPPRESS_ISR)
      nvicEnableVector(STM32_TIM15_NUMBER, STM32_ICU_TIM15_IRQ_PRIORITY);
#endif
#if defined(STM32_TIM15CLK)
      icup->clock = STM32_TIM15CLK;
#else
      icup->clock = STM32_TIMCLK1;
#endif
    }
#endif
  }
  icu_lld_start(icup);
}


void icu_opt_lld_stop(ICUDriver *icup)
{
  
  if (icup->state == ICU_READY) {
    /* Clock deactivation.*/
    icup->tim->CR1  = 0;                    /* Timer disabled.              */
    icup->tim->DIER = 0;                    /* All IRQs disabled.           */
    icup->tim->SR   = 0;                    /* Clear eventual pending IRQs. */
    
    
#if STM32_ICU_USE_TIM15
    if (&ICUD15 == icup) {
#if !defined(STM32_TIM15_SUPPRESS_ISR)
      nvicDisableVector(STM32_TIM15_NUMBER);
#endif
      rccDisableTIM15();
    }
#endif
  }
}


