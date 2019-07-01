#pragma once

#include "ch.h"
#include "hal.h"

#define NUMBER_OF_CHANNEL_MIN	1
#define NUMBER_OF_CHANNEL_MAX	16

#if defined ADC_SAMPLE_480 // ADCV2: F4 F7
typedef enum {ADC_CYCLE_START=256, ADC_CYCLES_3=ADC_CYCLE_START, ADC_CYCLES_15,
	      ADC_CYCLES_28, ADC_CYCLES_56, ADC_CYCLES_84, ADC_CYCLES_112,
	      ADC_CYCLES_144, ADC_CYCLES_480} AdcSampleCycles;
#elif defined ADC_SMPR_SMP_601P5 // ADCV3: L4
typedef enum {ADC_CYCLE_START=256, ADC_CYCLES_1P5=ADC_CYCLE_START, ADC_CYCLES_2P5,
	      ADC_CYCLES_4P5, ADC_CYCLES_7P5, ADC_CYCLES_19P5,
	      ADC_CYCLES_61P5, ADC_CYCLES_181P5, ADC_CYCLES_601P5} AdcSampleCycles;
#elif defined ADC_SMPR_SMP_810P5 // ADCV4: H7
typedef enum {ADC_CYCLE_START=256, ADC_CYCLES_1P5=ADC_CYCLE_START, ADC_CYCLES_2P5,
	      ADC_CYCLES_8P5, ADC_CYCLES_16P5, ADC_CYCLES_32P5,
	      ADC_CYCLES_64P5, ADC_CYCLES_384P5, ADC_CYCLES_810P5} AdcSampleCycles;
#endif

#define ADC_ONE_SHOT	1000U
#define ADC_CONTINUOUS	1001U
#define ADC_TIMER_MAX_ALLOWED_FREQUENCY	100000U
#define ADC_TIMER_DRIVEN(f) (f+ADC_CONTINUOUS+1)


/*
  mandatory argument :
  ° address of ADCConversionGroup structure
  ° number of channels

  optionnal arguments
  ° mode of operation : ADC_ONE_SHOT [default], ADC_CONTINUOUS, ADC_TIMER_DRIVEN(frequency)
  ° address of callback function

  channels arguments :
  
  channels or line, {optionnal oversampling cycles [maximum if not given]},
  channels or line, {optionnal oversampling cycles [maximum if not given]},
  ...,
  NULL : mandatory last argument of variable number of argument function


  behaviour : the function with chSysHalt if any error is detected

  example of use :

  // one channel, one shot synchronous convertion => use adcConvert
  adcFillConversionGroup(&adcgrpcfg1,
			 1U,
			 LINE_C01_POTAR, // or ADC_CHANNEL_IN11
			 NULL);

  // three channels, one shot synchronous convertion => use adcConvert
  adcFillConversionGroup(&adcgrpcfg1,
			 3U,
			 LINE_C01_POTAR, // default to ADC_CYCLES_480
			 ADC_CHANNEL_SENSOR, // default to ADC_CYCLES_480
			 ADC_CHANNEL_VBAT, // default to ADC_CYCLES_480
			 NULL);

  // two channels, continuous convertion => use adcStartConvertion
  // callback called every half buffers
  // oversampling explicitely given for every channels
  adcFillConversionGroup(&adcgrpcfg1,
			 2U,
                         ADC_CONTINUOUS,
			 &adc_cb,
			 LINE_C01_POTAR, ADC_CYCLES_3, 
			 ADC_CHANNEL_SENSOR, ADC_CYCLES_480,
			 NULL);

  // two channels, timer driven continuous convertion @44100Hz => use adcStartConvertion
  // callback called every half buffers
  // oversampling explicitely given for every channels
  adcFillConversionGroup(&adcgrpcfg1,
			 2U,
                         ADC_TIMER_DRIVEN(44100),
			 &adc_cb,
			 LINE_C01_LEFT,  ADC_CYCLES_56,
			 LINE_C02_RIGHT, ADC_CYCLES_56, 
			 NULL);



 */


__attribute__ ((sentinel))
void adcFillConversionGroup(ADCConversionGroup  *cgrp, const uint8_t numberOfChannel, ...);





static inline adcsample_t adcGetSample(const ADCConversionGroup *cgrp,
				    const adcsample_t  * const buf,
				    const size_t depth, const size_t channel) {
  return buf[(depth * cgrp->num_channels) + channel];
}

