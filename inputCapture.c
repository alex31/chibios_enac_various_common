#include "inputCapture.h"

static void rccEnable(const stm32_tim_t * const timer);
static void rccDisable(const stm32_tim_t * const timer);

void timIcObjectInit(TimICDriver *pwmInp)
{
  pwmInp->config = NULL;
}

void timIcStart(TimICDriver *pwmInp, const TimICConfig *configp)
{
  osalDbgCheck((configp != NULL) && (pwmInp != NULL));
  osalDbgAssert((configp->prescaler >= 1) &&
		(configp->prescaler <= 65536),
		"prescaler must be 1 .. 65536");
  pwmInp->config = configp;
  stm32_tim_t * const timer = pwmInp->config->timer;
  const uint32_t channel = pwmInp->config->channel;
  chMtxObjectInit(&pwmInp->mut);
  rccEnable(timer);

  timer->CR1 = 0;	    // disable timer

  // hack in case of timer with more fields
#if defined (STM32G0XX)  || defined (STM32G4XX)|| defined (STM32H7XX)

  TIM_TypeDef *cmsisTimer = (TIM_TypeDef *) timer;
  cmsisTimer->CCMR3 = cmsisTimer->AF1 = cmsisTimer->AF2 =
    cmsisTimer->TISEL = 0;
#endif
  
  timer->PSC = configp->prescaler - 1U;	 // prescaler
  timer->CNT = 0;

  if (pwmInp->config->mode == TIMIC_PWM_IN) {
  chDbgAssert(__builtin_popcount(channel) == 1, "In pwm mode, only one channel must be set");
  switch (channel) {
  case TIMIC_CH1:
    timer->CCMR1 = (0b01 << TIM_CCMR1_CC1S_Pos) | (0b10 << TIM_CCMR1_CC2S_Pos);
    timer->CCMR2 = 0U;
    timer->CCER = TIM_CCER_CC2P; /* CC1P et CC1NP = 0 */
    timer->SMCR = (0b101 << TIM_SMCR_TS_Pos) | (0b100 << TIM_SMCR_SMS_Pos);
    timer->CCER |= (TIM_CCER_CC1E | TIM_CCER_CC2E);         
    break;
  case TIMIC_CH2:
    timer->CCMR1 = (0b10 << TIM_CCMR1_CC1S_Pos) | (0b01 << TIM_CCMR1_CC2S_Pos);
    timer->CCMR2 = 0U;
    timer->CCER = TIM_CCER_CC1P;   /* CC2P et CC2NP = 0 */      
    timer->SMCR = (0b110 << TIM_SMCR_TS_Pos) | (0b100 << TIM_SMCR_SMS_Pos);
    timer->CCER |= (TIM_CCER_CC1E | TIM_CCER_CC2E); 
    break;
  case TIMIC_CH3:
    timer->CCMR2 = (0b01 << TIM_CCMR2_CC3S_Pos) | (0b10 << TIM_CCMR2_CC4S_Pos);
    timer->CCMR1 = 0U;
    timer->CCER =  TIM_CCER_CC4P;    /* CC3P et CC3NP = 0 */ 
    timer->SMCR = (0b101 << TIM_SMCR_TS_Pos) | (0b100 << TIM_SMCR_SMS_Pos);
    timer->CCER |= (TIM_CCER_CC3E | TIM_CCER_CC4E); 
    break;
  case TIMIC_CH4:
    timer->CCMR2 = (0b10 << TIM_CCMR2_CC3S_Pos) | (0b01 << TIM_CCMR2_CC4S_Pos);
    timer->CCMR1 = 0U;
    timer->CCER = TIM_CCER_CC3P; /* CC4P et CC4NP = 0 */ 
    timer->SMCR = (0b110 << TIM_SMCR_TS_Pos) | (0b100 << TIM_SMCR_SMS_Pos);
    timer->CCER |= (TIM_CCER_CC3E | TIM_CCER_CC4E);
    break;
  default:
    chSysHalt("channel must be TIMIC_CH1 .. TIMIC_CH4");
  }
  } else if (pwmInp->config->mode == TIMIC_INPUT_CAPTURE) {
    /*
      Select the active input: TIMx_CCR1 must be linked to the TI1 input, so write the CC1S
      bits to 01 in the TIMx_CCMR1 register. As soon as CC1S becomes different from 00,
      the channel is configured in input and the TIMx_CCR1 register becomes read-only.
      •
      Select the edge of the active transition on the TI1 channel by writing CC1P and CC1NP
      bits to 0 in the TIMx_CCER register (rising edge in this case).
      •
      •
      Enable capture from the counter into the capture register by setting the CC1E bit in the
      TIMx_CCER register.
      •
      If needed, enable the related interrupt request by setting the CC1IE bit in the
      TIMx_DIER register, and/or the DMA request by setting the CC1DE bit in the
      TIMx_DIER register.
     */


    timer->CCMR1 = 0;
    timer->CCMR2 = 0;     
    timer->CCER  = 0; 
    timer->SMCR  = 0; 
    timer->CCER = 0;
    
    if (channel & TIMIC_CH1) {
      switch (pwmInp->config->active & (CH1_RISING_EDGE | CH1_FALLING_EDGE | CH1_BOTH_EDGES)) {
      case CH1_RISING_EDGE:
	timer->CCER |= 0;
	break;
      case CH1_FALLING_EDGE:
	timer->CCER |= TIM_CCER_CC1P;
	break;
      case CH1_BOTH_EDGES: 
	timer->CCER |= (TIM_CCER_CC1P | TIM_CCER_CC1NP);
	break;
      default:
      chSysHalt("No configuration given for CH1");
      }
      timer->CCMR1 |= (0b01 << TIM_CCMR1_CC1S_Pos); 
      timer->CCER |= TIM_CCER_CC1E;         
    }
    if (channel & TIMIC_CH2) {
      switch (pwmInp->config->active & (CH2_RISING_EDGE | CH2_FALLING_EDGE | CH2_BOTH_EDGES)) {
      case CH2_RISING_EDGE:
	timer->CCER |= 0;
	break;
      case CH2_FALLING_EDGE:
	timer->CCER |= TIM_CCER_CC2P;
	break;
      case CH2_BOTH_EDGES: 
	timer->CCER |= (TIM_CCER_CC2P | TIM_CCER_CC2NP);
	break;
       default:
	chSysHalt("No configuration given for CH2");
      }
      timer->CCMR1 |= (0b01 << TIM_CCMR1_CC2S_Pos);
      timer->CCER |= TIM_CCER_CC2E;         
    }
    if (channel & TIMIC_CH3) {
      switch (pwmInp->config->active & (CH3_RISING_EDGE | CH3_FALLING_EDGE | CH3_BOTH_EDGES)) {
      case CH3_RISING_EDGE:
	timer->CCER |= 0;
	break;
      case CH3_FALLING_EDGE:
	timer->CCER |= TIM_CCER_CC3P;
	break;
      case CH3_BOTH_EDGES: 
	timer->CCER |= (TIM_CCER_CC3P | TIM_CCER_CC3NP);
	break;
       default:
	chSysHalt("No configuration given for CH3");
      }
      timer->CCMR2 |= (0b01 << TIM_CCMR2_CC3S_Pos);
      timer->CCER |= TIM_CCER_CC3E;         
    }
    if (channel & TIMIC_CH4) {
      switch (pwmInp->config->active & (CH4_RISING_EDGE | CH4_FALLING_EDGE | CH4_BOTH_EDGES)) {
      case CH4_RISING_EDGE:
	timer->CCER |= 0;
	break;
      case CH4_FALLING_EDGE:
	timer->CCER |= TIM_CCER_CC4P;
	break;
      case CH4_BOTH_EDGES: 
	timer->CCER |= (TIM_CCER_CC4P | TIM_CCER_CC4NP);
	break;
    default:
	chSysHalt("No configuration given for CH4");
      }
      timer->CCMR2 |= (0b01 << TIM_CCMR2_CC4S_Pos);
      timer->CCER |= TIM_CCER_CC4E;         
    }
  } else { 
    chSysHalt("invalid mode");
  }
  timer->DIER = pwmInp->config->dier;
  timer->SR = 0;
}

void timIcStartCapture(TimICDriver *pwmInp)
{
  osalDbgCheck(pwmInp != NULL);
  stm32_tim_t * const timer = pwmInp->config->timer;
  osalDbgCheck(timer != NULL);
  timer->CNT = 0;
  timer->CR1 |= TIM_CR1_CEN;
}

void timIcStopCapture(TimICDriver *pwmInp)
{
  osalDbgCheck(pwmInp != NULL);
  stm32_tim_t * const timer = pwmInp->config->timer;
  osalDbgCheck(timer != NULL);
  timer->CR1 &= ~TIM_CR1_CEN;
}

void timIcStop(TimICDriver *pwmInp)
{
  chMtxLock(&pwmInp->mut);
  rccDisable(pwmInp->config->timer);
  timIcObjectInit(pwmInp);
  chMtxUnlock(&pwmInp->mut);
}




static void rccEnable(const stm32_tim_t * const timer)
{
#ifdef TIM2
  if (timer == STM32_TIM1) {
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
};

static void rccDisable(const stm32_tim_t * const timer)
{
#ifdef TIM2
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
