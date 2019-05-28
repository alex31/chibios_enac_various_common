#include "adcHelper.h"
#include "stdutil.h"
#include "globalVar.h"
#include <string.h>
#include <stdarg.h>


/*
  adcFillConversionGroup(&cgrp, FALSE, &end_cb, 
  LINE_PC01_POTAR, ADC_SAMPLE_480,
  ADC_CHANNEL_SENSOR, ADC_SAMPLE_480, 
  NULL);

 */
static void errorcallback (ADCDriver *adcp, adcerror_t err);
static int getChannelFromLine(ioline_t line);
static void setSQR(ADCConversionGroup  *cgrp, size_t sequence, uint32_t channelMsk);
static void setSMPR(ADCConversionGroup  *cgrp, uint32_t channelMsk, uint32_t sampleCycleMsk);

__attribute__ ((sentinel))
void adcFillConversionGroup(ADCConversionGroup  *cgrp,
			    const bool		circular,
			    adccallback_t	end_cb,
			    ...)
{
  typedef enum {ArgLine=0, ArgCycle, ArgLast} NextArgType;
  va_list ap;
  int32_t curArg;
  size_t  cnt=0;
  int channel=-1;
  
  memset(cgrp, 0U, sizeof(ADCConversionGroup));
  cgrp->circular = circular;
  cgrp->end_cb = end_cb;
  //cgrp->error_cb = &errorcallback;
  cgrp->cr2 = ADC_CR2_SWSTART;
  
  va_start(ap, end_cb);
  while ((curArg = va_arg(ap, int32_t)) != 0) {
    const size_t sequenceIndex = cnt / ArgLast;
    switch ((NextArgType) (cnt++ % ArgLast)) {
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
  
  if ((cnt % ArgLast) != 0)
    chSysHalt("not a set of complete sequences");
  va_end(ap);
  
  cgrp->num_channels = cnt / ArgLast;
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
  DebugTrace("setSQR sequence = %u channelMsk = %lu", sequence, channelMsk);
  if (sequence <= 5) {
    cgrp->sqr3 |= (channelMsk << (sequence*5));
  } else if (sequence <= 11) {
    cgrp->sqr2 |= (channelMsk << (sequence-6)*5);
  } else if (sequence <= 15) {
    cgrp->sqr1 |= (channelMsk << (sequence-12)*5);
  } else {
    chSysHalt("setSQR : invalid sequence");
  }
}

static void setSMPR(ADCConversionGroup  *cgrp, uint32_t channelMsk, uint32_t sampleCycleMsk)
{
  DebugTrace("setSMPR channelMsk = %lu sampleCycleMsk=%lu", channelMsk, sampleCycleMsk);
  if (channelMsk <= 9) {
    const uint32_t mask = 0b111 << (channelMsk * 3);
    if (((cgrp->smpr2 & mask) == 0) || ((cgrp->smpr2 & mask) == sampleCycleMsk)) {
      cgrp->smpr2 |= (sampleCycleMsk << (channelMsk * 3));
    } else {
      chSysHalt("try to set different sampleCycleMsk for same channel");
    }
  } else if (channelMsk <= 18) {
    const uint32_t mask = 0b111 << ((channelMsk-10) * 3);
    if (((cgrp->smpr1 & mask) == 0) || ((cgrp->smpr1 & mask) == sampleCycleMsk)) {
      cgrp->smpr1 |= (sampleCycleMsk << ((channelMsk-10) * 3));
    } else {
      chSysHalt("try to set different sampleCycleMsk for same channel");
    }
  } else {
    chSysHalt("setSQR : invalid channelMsk");
  }
}

