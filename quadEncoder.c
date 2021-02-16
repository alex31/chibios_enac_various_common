#include "quadEncoder.h"


static void rccEnable(const stm32_tim_t * const timer);
static bool hasChanged(QEncoderDriver *qencoderp);

void qencoderObjectInit(QEncoderDriver *qencoderp)
{
  qencoderp->config = NULL;
  qencoderp->lastCnt=0U;
}

void qencoderStart(QEncoderDriver *qencoderp, const QEncoderConfig *configp)
{
  qencoderp->config = configp;
  qencoderp->lastCnt=0U;
  stm32_tim_t * const timer = qencoderp->config->timer;
  chMtxObjectInit(&qencoderp->mut);
  rccEnable(timer);

  timer->PSC = 0;	    // prescaler must be set to zero
  timer->SMCR = 1;          // Encoder mode 1 : count on TI1 only
  timer->CCER = 0;          // rising edge polarity
  timer->ARR = 0xFFFFFFFF;  // count from 0-ARR or ARR-0
  timer->CCMR1 = 0xC1C1;    // f_DTS/16, N=8, IC1->TI1, IC2->TI2
  timer->CNT = 0;           // Initialize counter
  timer->EGR = 1;           // generate an update event
  timer->CR1 = 1;           // Enable the counter
}


QEncoderCnt qencoderGetCNT(QEncoderDriver * const qencoderp)
{
  chMtxLock(&qencoderp->mut);
  const QEncoderCnt ret = {qencoderp->config->timer->CNT, hasChanged(qencoderp)};
  chMtxUnlock(&qencoderp->mut);
  return ret;
}

void qencoderSetCNT(QEncoderDriver *qencoderp, const uint32_t cnt)
{
  chMtxLock(&qencoderp->mut);
  qencoderp->config->timer->CNT = cnt;
  chMtxUnlock(&qencoderp->mut);
}



static bool hasChanged(QEncoderDriver *qencoderp)
{
  const uint32_t newV = qencoderp->config->timer->CNT;
  const bool change = (newV != qencoderp->lastCnt);
  qencoderp->lastCnt = newV;
  return change;
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
