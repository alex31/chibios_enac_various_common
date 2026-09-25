#include "outputCompare.h"

enum TimOCChannel {TIMOC_CH1=1<<0, TIMOC_CH2=1<<1, TIMOC_CH3=1<<2, TIMOC_CH4=1<<3};

static const TimOCDriver* driverByTimerIndex[6] = {NULL};

static void output_compare_lld_serve_interrupt(const TimOCDriver * const timocp) __attribute__((unused));
static void _output_compare_isr_invoke_compare_cb(const TimOCDriver * const timocp, uint32_t channel);
static void _output_compare_isr_invoke_overflow_cb(const TimOCDriver * const timocp);
static bool timOcHasIsrSupport(const stm32_tim_t *timer);
static void timOcEnableMainOutput(stm32_tim_t *timer);

void timOcObjectInit(TimOCDriver *timocp)
{
  timocp->config = NULL;
  timocp->state = TIMOC_STATE_STOP;
  timocp->dier = 0;
}

void timOcStart(TimOCDriver *timocp, const TimOCConfig *configp)
{
  osalDbgCheck((configp != NULL) && (timocp != NULL));
  osalDbgAssert((configp->prescaler >= 1) &&
        (configp->prescaler <= 65536),
        "prescaler must be 1 .. 65536");
  osalDbgAssert(timocp->state == TIMOC_STATE_STOP, "state error");
  timocp->config = configp;
  stm32_tim_t * const timer = timocp->config->timer;
  const bool use_isr = (configp->compare_cb != NULL) || (configp->overflow_cb != NULL);
  osalDbgAssert(!use_isr || timOcHasIsrSupport(timer),
                "callbacks require enabled ISR support for selected timer");
  chMtxObjectInit(&timocp->mut);
  timOcRccEnable(timocp);
  timocp->channel = 0;
  
  if (timocp->config->active & CH1_OUT_ACTIVE)
    timocp->channel |= TIMOC_CH1;
  if (timocp->config->active & CH2_OUT_ACTIVE)
    timocp->channel |= TIMOC_CH2;
  if (timocp->config->active & CH3_OUT_ACTIVE)
    timocp->channel |= TIMOC_CH3;
  if (timocp->config->active & CH4_OUT_ACTIVE)
    timocp->channel |= TIMOC_CH4;
  
  timer->CR1 = 0;       // disable timer

  // hack in case of timer with more fields
#if defined (STM32G0XX)  || defined (STM32G4XX)|| defined (STM32H7XX)
  TIM_TypeDef *cmsisTimer = (TIM_TypeDef *) timer;
  cmsisTimer->CCMR3 = cmsisTimer->AF1 = cmsisTimer->AF2 =
    cmsisTimer->TISEL = 0;
#endif
  
  timer->PSC = configp->prescaler - 1U;
  timer->ARR = configp->arr ? configp->arr : 0xffffffff;
  timer->DCR = configp->dcr;

  timer->CCMR1 = 0;
  timer->CCMR2 = 0;     
  timer->CCER  = 0; 
  timOcEnableMainOutput(timer);
  timer->SMCR  = 0; 
  
  uint32_t mode_bits = (uint32_t)timocp->config->mode;

  if (timocp->channel & TIMOC_CH1) {
    timer->CCMR1 |= (mode_bits << TIM_CCMR1_OC1M_Pos);
    /* For output compare, we could set CCxP for polarity if needed, but keeping it 0 is default */
    timer->CCER |= TIM_CCER_CC1E;
    timocp->dier |= STM32_TIM_DIER_CC1IE;
    
    /* CRITICAL SPECIFICATION: In Toggle mode, we force CCRx to 0 */
    if (timocp->config->mode == TIMOC_TOGGLE) {
      timer->CCR[0] = 0;
    }
  }
  
  if (timocp->channel & TIMOC_CH2) {
    timer->CCMR1 |= (mode_bits << TIM_CCMR1_OC2M_Pos);
    timer->CCER |= TIM_CCER_CC2E;         
    timocp->dier |= STM32_TIM_DIER_CC2IE;
    
    if (timocp->config->mode == TIMOC_TOGGLE) {
      timer->CCR[1] = 0;
    }
  }
  
  if (timocp->channel & TIMOC_CH3) {
    timer->CCMR2 |= (mode_bits << TIM_CCMR2_OC3M_Pos);
    timer->CCER |= TIM_CCER_CC3E;         
    timocp->dier |= STM32_TIM_DIER_CC3IE;
    
    if (timocp->config->mode == TIMOC_TOGGLE) {
      timer->CCR[2] = 0;
    }
  }
  
  if (timocp->channel & TIMOC_CH4) {
    timer->CCMR2 |= (mode_bits << TIM_CCMR2_OC4M_Pos);
    timer->CCER |= TIM_CCER_CC4E;
    timocp->dier |= STM32_TIM_DIER_CC4IE;
    
    if (timocp->config->mode == TIMOC_TOGGLE) {
      timer->CCR[3] = 0;
    }
  }
  
  // keep only DMA bits, not ISR bits that are handle by driver
  if (timocp->config->compare_cb == NULL)
    timocp->dier = 0;
  if (timocp->config->overflow_cb)
    timocp->dier |= STM32_TIM_DIER_UIE;
  
  timocp->dier = timocp->dier | (timocp->config->dier & (~ STM32_TIM_DIER_IRQ_MASK)); 
  timocp->state = TIMOC_STATE_READY;
}

void timOcStartCompare(TimOCDriver *timocp)
{
  osalDbgCheck(timocp != NULL);
  osalDbgAssert(timocp->state == TIMOC_STATE_READY, "state error");
  stm32_tim_t * const timer = timocp->config->timer;
  osalDbgCheck(timer != NULL);
  timer->CR1 = STM32_TIM_CR1_URS;
  timer->EGR |= STM32_TIM_EGR_UG;
  timer->SR = 0;
  timer->DIER = timocp->dier;
  timer->CR1 = STM32_TIM_CR1_URS | STM32_TIM_CR1_CEN;
  timocp->state = TIMOC_STATE_ACTIVE;
}

void timOcStartCompareDma(TimOCDriver *timocp)
{
  osalDbgCheck(timocp != NULL);
  osalDbgAssert(timocp->state == TIMOC_STATE_READY, "state error");
  stm32_tim_t * const timer = timocp->config->timer;
  osalDbgCheck(timer != NULL);
  
  uint32_t dma_dier = 0;
  if (timocp->channel & TIMOC_CH1) dma_dier |= STM32_TIM_DIER_CC1DE;
  if (timocp->channel & TIMOC_CH2) dma_dier |= STM32_TIM_DIER_CC2DE;
  if (timocp->channel & TIMOC_CH3) dma_dier |= STM32_TIM_DIER_CC3DE;
  if (timocp->channel & TIMOC_CH4) dma_dier |= STM32_TIM_DIER_CC4DE;

  timer->CR1 = STM32_TIM_CR1_URS;
  timer->EGR |= STM32_TIM_EGR_UG;
  timer->SR = 0;
  /* Configure timer to trigger DMA on CCx match instead of/in addition to IRQ */
  timer->DIER = timocp->dier | dma_dier;
  timer->CR1 = STM32_TIM_CR1_URS | STM32_TIM_CR1_CEN;
  timocp->state = TIMOC_STATE_ACTIVE;
}

void timOcStopCompare(TimOCDriver *timocp)
{
  osalDbgCheck(timocp != NULL);
  stm32_tim_t * const timer = timocp->config->timer;
  osalDbgCheck(timer != NULL);
  timer->CR1 &= ~TIM_CR1_CEN;
  timer->DIER = 0;
  timocp->state = TIMOC_STATE_READY;
}

void timOcStop(TimOCDriver *timocp)
{
  chMtxLock(&timocp->mut);
  timOcRccDisable(timocp);
  timOcObjectInit(timocp);
  chMtxUnlock(&timocp->mut);
  timocp->state = TIMOC_STATE_STOP;
  timocp->dier = 0;
}

volatile uint32_t* timOcGetArrAddress(const TimOCDriver *timocp)
{
  osalDbgCheck(timocp != NULL);
  stm32_tim_t * const timer = timocp->config->timer;
  /* Utlity function to easily link DMA to the ARR register */
  return &(timer->ARR);
}


void timOcRccEnable(const TimOCDriver * const timocp)
{
  const stm32_tim_t * const timer = timocp->config->timer;
  const bool use_isr = (timocp->config->compare_cb || timocp->config->overflow_cb) &&
                       timOcHasIsrSupport(timer);
#ifdef TIM1
  if (timer == STM32_TIM1) {
    driverByTimerIndex[0] = timocp;
    rccEnableTIM1(true);
    rccResetTIM1();
    if (use_isr) {
#ifdef STM32_TIM1_UP_TIM10_NUMBER 
      nvicEnableVector(STM32_TIM1_UP_TIM10_NUMBER, STM32_IRQ_TIM1_UP_TIM10_PRIORITY);
#warning "STM32_TIM1_UP_TIM10_NUMBER case"
#elifdef STM32_TIM1_CC_NUMBER 
      nvicEnableVector(STM32_TIM1_CC_NUMBER, STM32_IRQ_TIM1_CC_PRIORITY);
#else
#error "nvicEnableVector not handled"  
#endif
    }
  }
#endif
#ifdef TIM2
  else  if (timer == STM32_TIM2) {
    driverByTimerIndex[1] = timocp;
    rccEnableTIM2(true);
    rccResetTIM2();
    if (use_isr) {
      nvicEnableVector(STM32_TIM2_NUMBER, STM32_IRQ_TIM2_PRIORITY);
    }
  }
#endif
#ifdef TIM3
  else  if (timer == STM32_TIM3) {
    driverByTimerIndex[2] = timocp;
    rccEnableTIM3(true);
    rccResetTIM3();
    if (use_isr) {
      nvicEnableVector(STM32_TIM3_NUMBER, STM32_IRQ_TIM3_PRIORITY);
    }
  }
#endif
#ifdef TIM4
  else  if (timer == STM32_TIM4) {
    driverByTimerIndex[3] = timocp;
    rccEnableTIM4(true);
    rccResetTIM4();
    if (use_isr) {
      nvicEnableVector(STM32_TIM4_NUMBER, STM32_IRQ_TIM4_PRIORITY);
    }
  }
#endif
#ifdef TIM5
  else  if (timer == STM32_TIM5) {
    driverByTimerIndex[4] = timocp;
    rccEnableTIM5(true);
    rccResetTIM5();
    if (use_isr) {
      nvicEnableVector(STM32_TIM5_NUMBER, STM32_IRQ_TIM5_PRIORITY);
    }
  }
#endif
#ifdef TIM8
  else  if (timer == STM32_TIM8) {
    driverByTimerIndex[5] = timocp;
    rccEnableTIM8(true);
    rccResetTIM8();
    if (use_isr) {
#ifdef STM32_TIM8_UP_TIM13_NUMBER 
      nvicEnableVector(STM32_TIM8_UP_TIM13_NUMBER, STM32_IRQ_TIM8_UP_TIM13_PRIORITY);
#endif
#ifdef STM32_TIM8_CC_NUMBER 
      nvicEnableVector(STM32_TIM8_CC_NUMBER, STM32_IRQ_TIM8_CC_PRIORITY);
#endif
    }
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
};

void timOcRccDisable(const TimOCDriver * const timocp)
{
  const stm32_tim_t * const timer = timocp->config->timer;
#ifdef TIM1
  if (timer == STM32_TIM1) {
    rccResetTIM1();
    rccDisableTIM1();
  }
#endif
#ifdef TIM2
  else  if (timer == STM32_TIM2) {
    rccResetTIM2();
    rccDisableTIM2();
  }
#endif
#ifdef TIM3
  else  if (timer == STM32_TIM3) {
    rccResetTIM3();
    rccDisableTIM3();
  }
#endif
#ifdef TIM4
  else  if (timer == STM32_TIM4) {
    rccResetTIM4();
    rccDisableTIM4();
  }
#endif
#ifdef TIM5
  else  if (timer == STM32_TIM5) {
    rccResetTIM5();
    rccDisableTIM5();
  }
#endif
#ifdef TIM8
  else  if (timer == STM32_TIM8) {
    rccResetTIM8();
    rccDisableTIM8();
  }
#endif
#ifdef TIM9
  else  if (timer == STM32_TIM9) {
    rccResetTIM9();
    rccDisableTIM9();
  }
#endif
#ifdef TIM10
  else  if (timer == STM32_TIM10) {
    rccResetTIM10();
    rccDisableTIM10();
  }
#endif
#ifdef TIM11
  else  if (timer == STM32_TIM11) {
    rccResetTIM11();
    rccDisableTIM11();
  }
#endif
#ifdef TIM12
  else  if (timer == STM32_TIM12) {
    rccResetTIM12();
    rccDisableTIM12();
  }
#endif
#ifdef TIM13
  else  if (timer == STM32_TIM13) {
    rccResetTIM13();
    rccDisableTIM13();
  }
#endif
#ifdef TIM14
  else  if (timer == STM32_TIM14) {
    rccResetTIM14();
    rccDisableTIM14();
  }
#endif
#ifdef TIM15
  else  if (timer == STM32_TIM15) {
    rccResetTIM15();
    rccDisableTIM15();
  }
#endif
#ifdef TIM16
  else  if (timer == STM32_TIM16) {
    rccResetTIM16();
    rccDisableTIM16();
  }
#endif
#ifdef TIM17
  else  if (timer == STM32_TIM17) {
    rccResetTIM17();
    rccDisableTIM17();
  }
#endif
#ifdef TIM18
  else  if (timer == STM32_TIM18) {
    rccResetTIM18();
    rccDisableTIM18();
  }
#endif
#ifdef TIM19
  else  if (timer == STM32_TIM19) {
    rccResetTIM19();
    rccDisableTIM19();
  }
#endif
  else {
    chSysHalt("not a valid timer");
  }
};


/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/
#ifndef STM32_OUTPUT_COMPARE_USE_TIM1
#define STM32_OUTPUT_COMPARE_USE_TIM1 false
#endif

#ifndef STM32_OUTPUT_COMPARE_USE_TIM2
#define STM32_OUTPUT_COMPARE_USE_TIM2 false
#endif

#ifndef STM32_OUTPUT_COMPARE_USE_TIM3
#define STM32_OUTPUT_COMPARE_USE_TIM3 false
#endif

#ifndef STM32_OUTPUT_COMPARE_USE_TIM4
#define STM32_OUTPUT_COMPARE_USE_TIM4 false
#endif

#ifndef STM32_OUTPUT_COMPARE_USE_TIM5
#define STM32_OUTPUT_COMPARE_USE_TIM5 false
#endif

#ifndef STM32_OUTPUT_COMPARE_USE_TIM8
#define STM32_OUTPUT_COMPARE_USE_TIM8 false
#endif

#ifndef STM32_OUTPUT_COMPARE_SHARE_TIM1
#define STM32_OUTPUT_COMPARE_SHARE_TIM1 false
#endif

#ifndef STM32_OUTPUT_COMPARE_SHARE_TIM2
#define STM32_OUTPUT_COMPARE_SHARE_TIM2 false
#endif

#ifndef STM32_OUTPUT_COMPARE_SHARE_TIM3
#define STM32_OUTPUT_COMPARE_SHARE_TIM3 false
#endif

#ifndef STM32_OUTPUT_COMPARE_SHARE_TIM4
#define STM32_OUTPUT_COMPARE_SHARE_TIM4 false
#endif

#ifndef STM32_OUTPUT_COMPARE_SHARE_TIM5
#define STM32_OUTPUT_COMPARE_SHARE_TIM5 false
#endif

#ifndef STM32_OUTPUT_COMPARE_SHARE_TIM8
#define STM32_OUTPUT_COMPARE_SHARE_TIM8 false
#endif

#ifndef STM32_OUTPUT_COMPARE_ENABLE_TIM1_ISR
#define STM32_OUTPUT_COMPARE_ENABLE_TIM1_ISR false
#endif

#ifndef STM32_OUTPUT_COMPARE_ENABLE_TIM2_ISR
#define STM32_OUTPUT_COMPARE_ENABLE_TIM2_ISR false
#endif


#ifndef STM32_OUTPUT_COMPARE_ENABLE_TIM3_ISR
#define STM32_OUTPUT_COMPARE_ENABLE_TIM3_ISR false
#endif


#ifndef STM32_OUTPUT_COMPARE_ENABLE_TIM4_ISR
#define STM32_OUTPUT_COMPARE_ENABLE_TIM4_ISR false
#endif


#ifndef STM32_OUTPUT_COMPARE_ENABLE_TIM5_ISR
#define STM32_OUTPUT_COMPARE_ENABLE_TIM5_ISR false
#endif


#ifndef STM32_OUTPUT_COMPARE_ENABLE_TIM8_ISR
#define STM32_OUTPUT_COMPARE_ENABLE_TIM8_ISR false
#endif

#ifndef STM32_INPUT_CAPTURE_USE_TIM1
#define STM32_INPUT_CAPTURE_USE_TIM1 false
#endif

#ifndef STM32_INPUT_CAPTURE_USE_TIM2
#define STM32_INPUT_CAPTURE_USE_TIM2 false
#endif

#ifndef STM32_INPUT_CAPTURE_USE_TIM3
#define STM32_INPUT_CAPTURE_USE_TIM3 false
#endif

#ifndef STM32_INPUT_CAPTURE_USE_TIM4
#define STM32_INPUT_CAPTURE_USE_TIM4 false
#endif

#ifndef STM32_INPUT_CAPTURE_USE_TIM5
#define STM32_INPUT_CAPTURE_USE_TIM5 false
#endif

#ifndef STM32_INPUT_CAPTURE_USE_TIM8
#define STM32_INPUT_CAPTURE_USE_TIM8 false
#endif



#if STM32_OUTPUT_COMPARE_USE_TIM1 && (!STM32_OUTPUT_COMPARE_SHARE_TIM1) && \
(STM32_GPT_USE_TIM1 || STM32_ICU_USE_TIM1 || STM32_PWM_USE_TIM1 || STM32_INPUT_CAPTURE_USE_TIM1)
#error "STM32 OUTPUT_COMPARE USE TIM1 but already used by GPT or ICU or PWM or INPUT CAPTURE"
#endif

#if STM32_OUTPUT_COMPARE_USE_TIM2 && (!STM32_OUTPUT_COMPARE_SHARE_TIM2) &&					\
(STM32_GPT_USE_TIM2 || STM32_ICU_USE_TIM2 || STM32_PWM_USE_TIM2 || STM32_INPUT_CAPTURE_USE_TIM2)
#error "STM32 OUTPUT_COMPARE USE TIM2 but already used by GPT or ICU or PWM or INPUT CAPTURE"
#endif

#if STM32_OUTPUT_COMPARE_USE_TIM3 && (!STM32_OUTPUT_COMPARE_SHARE_TIM3) &&				\
(STM32_GPT_USE_TIM3 || STM32_ICU_USE_TIM3 || STM32_PWM_USE_TIM3 || STM32_INPUT_CAPTURE_USE_TIM3)
#error "STM32 OUTPUT_COMPARE USE TIM3 but already used by GPT or ICU or PWM or INPUT CAPTURE"
#endif

#if STM32_OUTPUT_COMPARE_USE_TIM4 && (!STM32_OUTPUT_COMPARE_SHARE_TIM4) &&				\
(STM32_GPT_USE_TIM4 || STM32_ICU_USE_TIM4 || STM32_PWM_USE_TIM4 || STM32_INPUT_CAPTURE_USE_TIM4)
#error "STM32 OUTPUT_COMPARE USE TIM4 but already used by GPT or ICU or PWM or INPUT CAPTURE"
#endif

#if STM32_OUTPUT_COMPARE_USE_TIM5 && (!STM32_OUTPUT_COMPARE_SHARE_TIM5) &&				\
(STM32_GPT_USE_TIM5 || STM32_ICU_USE_TIM5 || STM32_PWM_USE_TIM5 || STM32_INPUT_CAPTURE_USE_TIM5)
#error "STM32 OUTPUT_COMPARE USE TIM5 but already used by GPT or ICU or PWM or INPUT CAPTURE"
#endif

#if STM32_OUTPUT_COMPARE_USE_TIM8 && (!STM32_OUTPUT_COMPARE_SHARE_TIM8) &&				\
(STM32_GPT_USE_TIM8 || STM32_ICU_USE_TIM8 || STM32_PWM_USE_TIM8 || STM32_INPUT_CAPTURE_USE_TIM8)
#error "STM32 OUTPUT_COMPARE USE TIM8 but already used by GPT or ICU or PWM or INPUT CAPTURE"
#endif


#if STM32_OUTPUT_COMPARE_USE_TIM1 || defined(__DOXYGEN__)
#if STM32_OUTPUT_COMPARE_ENABLE_TIM1_ISR
#if defined(STM32_TIM1_UP_TIM10_HANDLER)
/**
 * @brief   TIM1 compare interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_TIM1_UP_TIM10_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  output_compare_lld_serve_interrupt(driverByTimerIndex[0]);

  OSAL_IRQ_EPILOGUE();
}
#elifdef STM32_TIM1_UP_HANDLER
OSAL_IRQ_HANDLER(STM32_TIM1_UP_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  output_compare_lld_serve_interrupt(driverByTimerIndex[0]);

  OSAL_IRQ_EPILOGUE();
}
#elifdef STM32_TIM1_UP_TIM16_HANDLER
OSAL_IRQ_HANDLER(STM32_TIM1_UP_TIM16_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  output_compare_lld_serve_interrupt(driverByTimerIndex[0]);

  OSAL_IRQ_EPILOGUE();
}
#else
#error "no handler defined for TIM1"
#endif

#if !defined(STM32_TIM1_CC_HANDLER)
#error "STM32_TIM1_CC_HANDLER not defined"
#endif
/**
 * @brief   TIM1 compare interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_TIM1_CC_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  output_compare_lld_serve_interrupt(driverByTimerIndex[0]);

  OSAL_IRQ_EPILOGUE();
}
#endif /* STM32_OUTPUT_COMPARE_ENABLE_TIM1_ISR */
#endif /* STM32_OUTPUT_COMPARE_USE_TIM1 */

#if STM32_OUTPUT_COMPARE_USE_TIM2 || defined(__DOXYGEN__)
#if STM32_OUTPUT_COMPARE_ENABLE_TIM2_ISR
#if !defined(STM32_TIM2_HANDLER)
#error "STM32_TIM2_HANDLER not defined"
#endif
/**
 * @brief   TIM2 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_TIM2_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  output_compare_lld_serve_interrupt(driverByTimerIndex[1]);

  OSAL_IRQ_EPILOGUE();
}
#endif /* STM32_OUTPUT_COMPARE_ENABLE_TIM2_ISR */
#endif /* STM32_OUTPUT_COMPARE_USE_TIM2 */

#if STM32_OUTPUT_COMPARE_USE_TIM3 || defined(__DOXYGEN__)
#if STM32_OUTPUT_COMPARE_ENABLE_TIM3_ISR
#if !defined(STM32_TIM3_HANDLER)
#error "STM32_TIM3_HANDLER not defined"
#endif
/**
 * @brief   TIM3 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_TIM3_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  output_compare_lld_serve_interrupt(driverByTimerIndex[2]);

  OSAL_IRQ_EPILOGUE();
}
#endif /* STM32_OUTPUT_COMPARE_ENABLE_TIM3_ISR */
#endif /* STM32_OUTPUT_COMPARE_USE_TIM3 */

#if STM32_OUTPUT_COMPARE_USE_TIM4 || defined(__DOXYGEN__)
#if STM32_OUTPUT_COMPARE_ENABLE_TIM4_ISR
#if !defined(STM32_TIM4_HANDLER)
#error "STM32_TIM4_HANDLER not defined"
#endif
/**
 * @brief   TIM4 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_TIM4_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  output_compare_lld_serve_interrupt(driverByTimerIndex[3]);

  OSAL_IRQ_EPILOGUE();
}
#endif /* STM32_OUTPUT_COMPARE_ENABLE_TIM4_ISR */
#endif /* STM32_OUTPUT_COMPARE_USE_TIM4 */

#if STM32_OUTPUT_COMPARE_USE_TIM5 || defined(__DOXYGEN__)
#if STM32_OUTPUT_COMPARE_ENABLE_TIM5_ISR
#if !defined(STM32_TIM5_HANDLER)
#error "STM32_TIM5_HANDLER not defined"
#endif
/**
 * @brief   TIM5 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_TIM5_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  output_compare_lld_serve_interrupt(driverByTimerIndex[4]);

  OSAL_IRQ_EPILOGUE();
}
#endif /* STM32_OUTPUT_COMPARE_ENABLE_TIM5_ISR */
#endif /* STM32_OUTPUT_COMPARE_USE_TIM5 */

#if STM32_OUTPUT_COMPARE_USE_TIM8 || defined(__DOXYGEN__)
#if STM32_OUTPUT_COMPARE_ENABLE_TIM8_ISR
#if defined(STM32_TIM8_UP_TIM13_HANDLER)
/**
 * @brief   TIM8 compare interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_TIM8_UP_TIM13_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  output_compare_lld_serve_interrupt(driverByTimerIndex[5]);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if !defined(STM32_TIM8_CC_HANDLER)
#error "STM32_TIM8_CC_HANDLER not defined"
#endif
/**
 * @brief   TIM8 compare interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_TIM8_CC_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  output_compare_lld_serve_interrupt(driverByTimerIndex[5]);

  OSAL_IRQ_EPILOGUE();
}
#endif /* STM32_OUTPUT_COMPARE_ENABLE_TIM8_ISR */
#endif /* STM32_OUTPUT_COMPARE_USE_TIM8 */

static void output_compare_lld_serve_interrupt(const TimOCDriver * const timocp)
{
  uint32_t sr;
  stm32_tim_t * const timer = timocp->config->timer;
  
  sr  = timer->SR;
  sr &= (timer->DIER & STM32_TIM_DIER_IRQ_MASK);
  timer->SR = ~sr;

  if (timocp->channel & TIMOC_CH1) {
    if ((sr & STM32_TIM_SR_CC1IF) != 0)
      _output_compare_isr_invoke_compare_cb(timocp, 0);
  }
  if (timocp->channel & TIMOC_CH2) {
    if ((sr & STM32_TIM_SR_CC2IF) != 0)
      _output_compare_isr_invoke_compare_cb(timocp, 1);
  }
  if (timocp->channel & TIMOC_CH3) {
    if ((sr & STM32_TIM_SR_CC3IF) != 0)
      _output_compare_isr_invoke_compare_cb(timocp, 2);
  }
  if (timocp->channel & TIMOC_CH4) {
    if ((sr & STM32_TIM_SR_CC4IF) != 0)
      _output_compare_isr_invoke_compare_cb(timocp, 3);
  }
  
  if ((sr & STM32_TIM_SR_UIF) != 0)
    _output_compare_isr_invoke_overflow_cb(timocp);
}

static void _output_compare_isr_invoke_compare_cb(const TimOCDriver * const timocp, uint32_t channel)
{
  if (timocp->config->compare_cb) {
    timocp->config->compare_cb(timocp, channel);
  }
}

static void _output_compare_isr_invoke_overflow_cb(const TimOCDriver * const timocp)
{
  if (timocp->config->overflow_cb)
    timocp->config->overflow_cb(timocp);
}

static bool timOcHasIsrSupport(const stm32_tim_t *timer)
{
  (void) timer;
#if STM32_OUTPUT_COMPARE_ENABLE_TIM1_ISR
#ifdef TIM1
  if (timer == STM32_TIM1) {
    return true;
  }
#endif
#endif
#if STM32_OUTPUT_COMPARE_ENABLE_TIM2_ISR
#ifdef TIM2
  if (timer == STM32_TIM2) {
    return true;
  }
#endif
#endif
#if STM32_OUTPUT_COMPARE_ENABLE_TIM3_ISR
#ifdef TIM3
  if (timer == STM32_TIM3) {
    return true;
  }
#endif
#endif
#if STM32_OUTPUT_COMPARE_ENABLE_TIM4_ISR
#ifdef TIM4
  if (timer == STM32_TIM4) {
    return true;
  }
#endif
#endif
#if STM32_OUTPUT_COMPARE_ENABLE_TIM5_ISR
#ifdef TIM5
  if (timer == STM32_TIM5) {
    return true;
  }
#endif
#endif
#if STM32_OUTPUT_COMPARE_ENABLE_TIM8_ISR
#ifdef TIM8
  if (timer == STM32_TIM8) {
    return true;
  }
#endif
#endif
  return false;
}

static void timOcEnableMainOutput(stm32_tim_t *timer)
{
#ifdef TIM1
  if (timer == STM32_TIM1) {
    timer->BDTR |= TIM_BDTR_MOE;
    return;
  }
#endif
#ifdef TIM8
  if (timer == STM32_TIM8) {
    timer->BDTR |= TIM_BDTR_MOE;
  }
#endif
}
