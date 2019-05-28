#pragma once

#include "ch.h"
#include "hal.h"


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

__attribute__ ((sentinel))
void adcFillConversionGroup(ADCConversionGroup  *cgrp,
			    const bool		circular,
			    adccallback_t	end_cb,
			    ...);





static inline adcsample_t adcGetSample(const ADCConversionGroup *cgrp,
				    const adcsample_t  * const buf,
				    const size_t depth, const size_t channel) {
  return buf[(depth * cgrp->num_channels) + channel];
}

