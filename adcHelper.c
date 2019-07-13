#include "adcHelper.h"
#include "stdutil.h"
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
    - il faut passer la frequence : utilisation systematique de GPTD8
  

  ====
  plages de valeur pour les paramètres :

  end_callback		      : > 0x80000000
  lines                       : > 0x40000000
  internal channels           : entre 16 et 18 inclus 
  nombre de cannaux           : entre 1 et 15 inclus
  sampling cycles             : transposé entre 256 et 263 inclus
  
  frequence d'échantillonnage : valeur speciale ADC_ONE_SHOT:1000, ADC_CONTINUOUS:1001, et macro 
                                ADC_TIMER_DRIVEN(x) ->
                                transposé entre 1002 et 101_002 depuis [1 - 100_000] 
                                (utilisation de GPTD8), erreur si param défini et 
                                GPT_USE_TIM8 non défini
   
   simplifier le cours en prenant en compte la nouvelle API
   creer des TP
   - mesure de la tension de pile de sauvegarde
   - mesure du 5V venant du PC (introduire le pont de resistance)
   - idée de tp qui a du sens pour la conversion continue ?


*/
//float cbGptFrq = 0.0f;


#define CALLBACK_START_ADDR	0x8000000


static void errorcallback (ADCDriver *adcp, adcerror_t err);
static int getChannelFromLine(ioline_t line);
static void setSQR(ADCConversionGroup  *cgrp, size_t sequence, uint32_t channelMsk);
static void setSMPR(ADCConversionGroup  *cgrp, uint32_t channelMsk, uint32_t sampleCycleMsk);
static void configureGptd8(uint32_t frequency);
static uint32_t cyclesByMask(const AdcSampleCycles msk);

__attribute__ ((sentinel(1)))
ADCConversionGroup adcGetConfig(const uint8_t numberOfChannel, ...)
{
  typedef enum {ArgLine=0, ArgCycle, ArgLast} NextArgType;
  va_list ap;
  uint32_t curArg;
  uint32_t totalAdcCycles = 0U;
  size_t  sequenceIndex=0;
  int channel=-1;
  uint32_t timerFrequency = 0;
  ADCConversionGroup  cgr;

  
  memset(&cgr, 0U, sizeof(ADCConversionGroup));
  cgr.circular = false;
  cgr.end_cb = NULL;
  cgr.error_cb = errorcallback;
  cgr.cr2 = ADC_CR2_SWSTART;
  
  va_start(ap, numberOfChannel);

  // look for double NULL arguments : one null is a valid arg for
  // ADC_CHANNEL_IN0
  while ( (curArg = va_arg(ap, uint32_t)) != ADC_SENTINEL)  {
    if ((curArg > CALLBACK_START_ADDR) && (curArg <= PERIPH_BASE)) {
      cgr.end_cb = (adccallback_t) curArg;
    } else if ((curArg >= ADC_ONE_SHOT ) &&
	       (curArg <= ADC_TIMER_DRIVEN(ADC_TIMER_MAX_ALLOWED_FREQUENCY))) {
      switch (curArg) {
      case  ADC_ONE_SHOT :  cgr.circular = false; break;
      case ADC_CONTINUOUS :  cgr.circular = true; break;
      default: cgr.circular = true;
	configureGptd8((timerFrequency = curArg-ADC_TIMER_DRIVEN(0)));
	cgr.cr2 = ADC_CR2_EXTEN_RISING | ADC_CR2_EXTSEL_SRC(0b1110);
      }
    } else {
      const NextArgType nat = (curArg >= ADC_CYCLE_START) && (curArg <= ADC_CYCLE_START+7) ?
	ArgCycle : ArgLine;
      switch (nat) {
      case ArgLine : {
	if (curArg >= PERIPH_BASE) { // parameter is a line coumpound address
	  channel = getChannelFromLine((ioline_t) curArg);
	  chDbgAssert(channel >= 0, "invalid LINE");
	  setSQR(&cgr, sequenceIndex, channel);
	} else if  (curArg <= 18) { // parameter is an internal channel (ref, bat, temp)
	  channel = curArg;
	  setSQR(&cgr, sequenceIndex, channel);
	} else {
	  chDbgAssert(FALSE, "sequence parameter error : neither LINE or INTERNAL CHANNEL");
	}
	totalAdcCycles += cyclesByMask(0x7+ADC_CYCLE_START);
	sequenceIndex++;
      }
	break;
      case ArgCycle:
	chDbgAssert((curArg >= ADC_CYCLE_START ) && (curArg <= (ADC_CYCLE_START+7)),
	  "sequence sample cycle parameter error");
	setSMPR(&cgr, channel, curArg-ADC_CYCLE_START);
	totalAdcCycles -= cyclesByMask(0x7+ADC_CYCLE_START);
	totalAdcCycles += cyclesByMask(curArg);
	break;
      default:
	chDbgAssert(FALSE, "internal error");
      }
    }
  }
  va_end(ap);
  cgr.num_channels = sequenceIndex;
  chDbgAssert((numberOfChannel >= NUMBER_OF_CHANNEL_MIN) && (numberOfChannel <= NUMBER_OF_CHANNEL_MAX),
	      "number of channels not given or out of bound");
  chDbgAssert(numberOfChannel == cgr.num_channels,
	      "number of channels and sequence paramaters incoherency");
  
  if (timerFrequency) {
    chDbgAssert(totalAdcCycles < (STM32_ADCCLK / timerFrequency),
		"cannot keep sampling pace. lower frequency or/and oversampling cycles"
		" or/and number of channels");
  }

  return cgr;
}




static void errorcallback (ADCDriver *adcp, adcerror_t err)
{
  (void) adcp;
  (void) err;
  chDbgAssert(FALSE, "ADC error callback");
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
    chDbgAssert(FALSE, "setSQR : invalid sequence");
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
    chDbgAssert((channelMsk < 16) || (sampleCycleMsk == 7),
		"Internal channels have to be sampled @ maximum SMPR cycles");
    const uint32_t mask = 0b111 << ((channelMsk-10) * 3);
    cgrp->smpr1 &= ~mask;
    cgrp->smpr1 |= (sampleCycleMsk << ((channelMsk-10) * 3));
  } else {
    chDbgAssert(FALSE, "setSQR : invalid channelMsk");
  }
}

/* static void gptcb (GPTDriver *gptp) */
/* { */
/*   (void) gptp; */
  
/*   static rtcnt_t ts = 0; */
/*   const rtcnt_t now = chSysGetRealtimeCounterX(); */
/*   cbGptFrq = ((float) STM32_SYSCLK / (now - ts)); */
/*   ts = now; */
/* } */


static void configureGptd8(uint32_t frequency)
{
  (void) frequency;
#if (STM32_GPT_USE_TIM8 == FALSE)
  chDbgAssert(STM32_GPT_USE_TIM8 != FALSE, "timer driver ADC need  STM32_GPT_USE_TIM8 == TRUE");
#else
  /*
    #                 _      _                                     _             _     __ _  
    #                | |    (_)                                   | |           (_)   / _` | 
    #                | |_    _    _ __ ___     ___   _ __         | |_    _ __   _   | (_| | 
    #                | __|  | |  | '_ ` _ \   / _ \ | '__|        | __|  | '__| | |   \__, | 
    #                \ |_   | |  | | | | | | |  __/ | |           \ |_   | |    | |    __/ | 
    #                 \__|  |_|  |_| |_| |_|  \___| |_|            \__|  |_|    |_|   |___/  
    #                  __ _                            _          
    #                 / _` |                          | |         
    #                | (_| |   ___   _ __    ___    __| |         
    #                 \__, |  / _ \ | '__|  / _ \  / _` |         
    #                  __/ | |  __/ | |    |  __/ | (_| |         
    #                 |___/   \___| |_|     \___|  \__,_|         
  */

  static GPTConfig gpt8cfg1 = {
  .frequency =  1e4,
  .callback  =  NULL,//&gptcb,
  .cr2       =  TIM_CR2_MMS_1,  /* MMS = 010 = TRGO on Update Event.        */
  .dier      =  0U
  };

  gptStart(&GPTD8, &gpt8cfg1);
  const uint16_t psc = (GPTD8.clock / frequency) / 60000U;
  const uint16_t   timcnt = (GPTD8.clock / (psc+1)) / frequency;

  //  DebugTrace ("DBG>> psc=%u timcnt=%u", psc, timcnt);
  
  GPTD8.tim->PSC = psc;
  gptStartContinuous(&GPTD8, timcnt);
#endif
}



static uint32_t cyclesByMask(const AdcSampleCycles msk)
{
#if defined ADC_SAMPLE_480 // ADCV2: F4 F7
  switch (msk) {
  case ADC_CYCLES_3   : return 3U+15U;
  case ADC_CYCLES_15  : return 15U+15U;
  case ADC_CYCLES_28  : return 28U+15U;
  case ADC_CYCLES_56  : return 56U+15U;
  case ADC_CYCLES_84  : return 84U+15U;
  case ADC_CYCLES_112 : return 112U+15U;
  case ADC_CYCLES_144 : return 144U+15U;
  case ADC_CYCLES_480 : return 480U+15U;
  }

#elif defined ADC_SMPR_SMP_601P5 // ADCV3: L4

#elif defined ADC_SMPR_SMP_810P5 // ADCV4: H7

#endif
  return 0;
}
