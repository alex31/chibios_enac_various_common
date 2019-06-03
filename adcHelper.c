#include "adcHelper.h"
#include "stdutil.h"
#include "globalVar.h"
#include <string.h>
#include <stdarg.h>


/*
  exemple d'utilisation :

  adcFillConversionGroup(&cgrp, FALSE, &end_cb, 
  LINE_PC01_POTAR, ADC_SAMPLE_480,
  ADC_CHANNEL_SENSOR, ADC_SAMPLE_480, 
  NULL);


  TODO : 
  * passage des params sur 64 bits avec concatenation type | valeur par une MACRO par type
    et recuperation type et valeur par deux macros
  * API simple pour faire de la conversion continue pilotée par TIMER
    - il faut passer la frequence : utilisation systematique de GPTD6
  

  ====
  plages de valeur pour les paramètres :

  end_callback		      : > 0x80000000
  lines                       : > 0x40000000
  internal channels           : entre 16 et 18 inclus 
  sampling cycles             : transposé entre 256 et 263 inclus
  
  frequence d'échantillonnage : valeur speciale ADC_ONE_SHOT:1000, ADC_CONTINUOUS:1001, et macro 
                                ADC_TIMER_DRIVEN(x) ->
                                transposé entre 1002 et 101_002 depuis [1 - 100_000] 
                                (utilisation de GPTD6), erreur si param défini et 
                                GPT_USE_TIM6 non défini
   
   simplifier le cours en prenant en compte la nouvelle API
   creer des TP
   - mesure de la tension de pile de sauvegarde
   - mesure du 5V venant du PC (introduire le pont de resistance)
   - idée de tp qui a du sens pour la conversion continue ?


*/


#define CALLBACK_START_ADDR	0x8000000


static void errorcallback (ADCDriver *adcp, adcerror_t err);
static int getChannelFromLine(ioline_t line);
static void setSQR(ADCConversionGroup  *cgrp, size_t sequence, uint32_t channelMsk);
static void setSMPR(ADCConversionGroup  *cgrp, uint32_t channelMsk, uint32_t sampleCycleMsk);
static void configureGptd6(uint32_t frequency);

__attribute__ ((sentinel))
void adcFillConversionGroup(ADCConversionGroup  *cgrp, ...)
{
  typedef enum {ArgLine=0, ArgCycle, ArgLast} NextArgType;
  va_list ap;
  uint32_t curArg;
  size_t  sequenceIndex=0;
  int channel=-1;
  
  memset(cgrp, 0U, sizeof(ADCConversionGroup));
  cgrp->circular = false;
  cgrp->end_cb = NULL;
  cgrp->error_cb = errorcallback;
  cgrp->cr2 = ADC_CR2_SWSTART;
  
  va_start(ap, cgrp);
  while ((curArg = va_arg(ap, uint32_t)) != 0) {
    if ((curArg > CALLBACK_START_ADDR) && (curArg <= PERIPH_BASE)) {
      cgrp->end_cb = (adccallback_t) curArg;
    } else if ((curArg >= ADC_ONE_SHOT ) &&
	       (curArg <= ADC_TIMER_DRIVEN(ADC_TIMER_MAX_ALLOWED_FREQUENCY))) {
      switch (curArg) {
      case  ADC_ONE_SHOT :  cgrp->circular = false; break;
      case ADC_CONTINUOUS :  cgrp->circular = true; break;
      default: cgrp->circular = true;
	configureGptd6(curArg-ADC_TIMER_DRIVEN(0));
      }
    } else {
      const NextArgType nat = (curArg >= ADC_CYCLE_START) && (curArg <= ADC_CYCLE_START+7) ?
	ArgCycle : ArgLine;
      switch (nat) {
      case ArgLine : {
	if (curArg > 1024) { // parameter is a line coumpound address
	  channel = getChannelFromLine((ioline_t) curArg);
	  if (channel < 0)
	    chSysHalt("invalid LINE");
	  setSQR(cgrp, sequenceIndex, channel);
	} else if  (curArg >= 16) { // parameter is an internal channel (ref, bat, temp)
	  channel = curArg;
	  setSQR(cgrp, sequenceIndex, channel);
	} else {
	  chSysHalt("sequence parameter error : neither LINE or INTERNAL CHANNEL");
	}
	sequenceIndex++;
      }
	break;
      case ArgCycle: 
	if ((curArg < ADC_CYCLE_START ) || (curArg > (ADC_CYCLE_START+7)))
	  chSysHalt("sequencesample cycle parameter error");
	setSMPR(cgrp, channel, curArg-ADC_CYCLE_START);
	break;
      default:
	chSysHalt("internal error");
      }
    }
    
  
  va_end(ap);
  
  cgrp->num_channels = sequenceIndex;
  }
}




static void errorcallback (ADCDriver *adcp, adcerror_t err)
{
  (void) adcp;
  (void) err;
  chSysHalt("ADC error callback");
}


static int getChannelFromLine(ioline_t line)
{
  const ioportid_t port  = PAL_PORT(line);
  const ioportmask_t pad = PAL_PAD(line);
  int ret = -1;

  if (port == GPIOA) {
    ret = pad;
  } else if (port == GPIOB) {
    ret = pad + 8;
  } else if (port == GPIOC) {
    ret = pad + 10;
  }

  return ret;
}

static void setSQR(ADCConversionGroup  *cgrp, size_t sequence, uint32_t channelMsk)
{
  //  DebugTrace("setSQR sequence = %u channelMsk = %lu", sequence, channelMsk);
  if (sequence <= 5) {
    cgrp->sqr3 |= (channelMsk << (sequence*5));
  } else if (sequence <= 11) {
    cgrp->sqr2 |= (channelMsk << (sequence-6)*5);
  } else if (sequence <= 15) {
    cgrp->sqr1 |= (channelMsk << (sequence-12)*5);
  } else {
    chSysHalt("setSQR : invalid sequence");
  }
  setSMPR(cgrp, channelMsk, 7); // default value is maximum SMPR cycle for the channel
}

static void setSMPR(ADCConversionGroup  *cgrp, uint32_t channelMsk, uint32_t sampleCycleMsk)
{
  //  DebugTrace("setSMPR channelMsk = %lu sampleCycleMsk=%lu", channelMsk, sampleCycleMsk);
  if (channelMsk <= 9) {
    const uint32_t mask = 0b111 << (channelMsk * 3);
    cgrp->smpr2 &= ~mask;
    cgrp->smpr2 |= (sampleCycleMsk << (channelMsk * 3));
  } else if (channelMsk <= 18) {
    if ((channelMsk >= 16) && (sampleCycleMsk < 7))
      chSysHalt("Internal channels have to be sampled @ maximum SMPR cycles");
    const uint32_t mask = 0b111 << ((channelMsk-10) * 3);
    cgrp->smpr1 &= ~mask;
    cgrp->smpr1 |= (sampleCycleMsk << ((channelMsk-10) * 3));
  } else {
    chSysHalt("setSQR : invalid channelMsk");
  }
}

static void configureGptd6(uint32_t frequency)
{
  (void) frequency;
#if (STM32_GPT_USE_TIM6 == FALSE)
  chDbgAssert(STM32_GPT_USE_TIM6 != FALSE, "timer driver ADC need  STM32_GPT_USE_TIM6 == TRUE");
#endif

}
