#pragma once

#include "ch.h"
#include "hal.h"

#include <concepts>
#include <expected>
#include <utility>
#include <cstring>
#include <array>
#include <numeric>

#if defined STM32F4XX or defined STM32F7XX
#define ADC_IS_ADCV2
#elif (defined (STM32L431xx) || defined (STM32L432xx) || defined (STM32L433xx) || defined (STM32L442xx) || defined (STM32L443xx))
#define ADC_IS_ADCV3
#error "stm32L4 (ADCv3) not yet supported"
//
#elif defined(STM32H723xx) || defined(STM32H725xx)  ||	\
      defined(STM32H730xx) || defined(STM32H730xxQ) ||	\
    defined(STM32H733xx) || defined(STM32H735xx)  ||	\
    defined(STM32H742xx) || defined(STM32H743xx)  ||	\
    defined(STM32H745xx) || defined(STM32H745xG)  ||	\
    defined(STM32H747xx) || defined(STM32H747xG)  ||	\
    defined(STM32H750xx) || defined(STM32H753xx)  ||	\
    defined(STM32H755xx) || defined(STM32H757xx)  ||	\
    defined(STM32H7A3xx) || defined(STM32H7A3xxQ) ||	\
    defined(STM32H7B0xx) || defined(STM32H7B0xxQ) ||	\
  defined(STM32H7B3xx) || defined(STM32H7B3xxQ)
#error "stm32H7 (ADCv4) not yet supported"
#define ADC_IS_ADCV4
#else
#error "unknow stm32 family not yet supported"
#endif

/*
  TODO:
  ° impl L4 et H7

 */

/*
 Example of use :

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
  static constexpr auto adccfgExp =  AdcCGroup::get(AdcCGroup::Modes::Continuous,
					  AdcCGroup::Channels::C01,  AdcCGroup::Cycles::C480,
					  AdcCGroup::Channels::C02,  AdcCGroup::Cycles::C480,
					  adcEndCb,
					  (ADCConversionGroup) {
					       .htr = 1000,
					       .ltr = 2000},
					   AdcCGroup::SampleFrequency(10'000),
					   AdcCGroup::ExtSels::TIM8_TRGO);
 #pragma GCC diagnostic pop
 ADCHELPER_STATIC_CHECK(adccfgExp);
 static constexpr ADCConversionGroup adccfg = *adccfgExp;
 static constexpr size_t ADC_GRP1_NUM_CHANNELS = adccfg.num_channels;


 */
namespace AdcCGroup {

  constexpr std::array<uint16_t, 8> nbCycles = {3,15,28,56,84,112,144,480};
  constexpr uint16_t cyclesToStart = 15;
  
  enum class Status {Ok=0, InvalidChannel=1<<0, InvalidSequence=1<<1,
    InvalidInternalCycles=1<<2, sampleCycleOverload=1<<3,
    ExtTriggerImposeContinuous=1<<4, EmptySequence=1<<5};

  enum class Cycles {C3, C15, C28, C56, C84, C112,
		     C144, C480};

  enum class Channels {IN0=ADC_CHANNEL_IN0, IN1, IN2, IN3, IN4, IN5, IN6, IN7, IN8, IN9,
    IN10, IN11, IN12, IN13, IN14, IN15, IN16, IN17, IN18,
    A00=IN0, A01=IN1, A02=IN2, A03=IN3, A04=IN4, A05=IN5, A06=IN6, A07=IN7,
    B00=IN8, B01=IN9,
    C00=IN10, C01=IN11, C02=IN12, C03=IN13, C04=IN14, C05=IN15,
    SENSOR=IN16, VREFINT=IN17, VBAT=IN18};

  enum class ExtSels {TIM1_CH1, TIM1_CH2, TIM1_CH3, TIM2_CH2, TIM2_CH3, TIM2_CH4,
    TIM2_TRGO, TIM3_CH1, TIM3_TRGO, TIM4_CH4, TIM5_CH1, TIM5_CH2,
    TIM5_CH3, TIM8_CH1, TIM8_TRGO, EXTIL11};
  
  enum class Modes {OneShot, Continuous};

  struct SampleFrequency {
    constexpr SampleFrequency(int _freq) : freq(_freq) {};
    constexpr uint32_t operator* ()  const {return freq;};
    const uint32_t freq;
  };

  struct Context {
    ADCConversionGroup cgroup = {};
    int sequenceIndex = 0;
    Channels lastChannel = Channels::IN0;
    Cycles lastCycles = Cycles::C480;
    Status status = Status::Ok;
    Modes mode = Modes::OneShot;
    std::array<uint16_t, 28> cycleBySeq = {};
    uint32_t sampleFrequency = 0;
  };

  template<typename T> 
  concept AuthParamType = std::same_as<Status, T> ||
    std::same_as<Status, T> ||
    std::same_as<Cycles, T> ||
    std::same_as<Channels, T> ||
    std::same_as<ExtSels, T> ||
    std::same_as<Modes, T> ||
    std::same_as<SampleFrequency, T> ||
    std::same_as<adccallback_t, T> ||
    std::same_as<adcerrorcallback_t, T> ||
    std::same_as<ADCConversionGroup, T>;
  
  constexpr Status operator|(Status lhs, Status rhs) {
    return static_cast<Status>(std::to_underlying(lhs) | std::to_underlying(rhs));
  }
  constexpr void operator|=(Status &lhs, Status rhs) {
    lhs = lhs | rhs;
  }
  constexpr Status operator&(Status lhs, Status rhs) {
    return static_cast<Status>(std::to_underlying(lhs) & std::to_underlying(rhs));
  }
  constexpr void operator&=(Status &lhs, Status rhs) {
    lhs = lhs & rhs;
  }
  //  constexpr Channels
  //  getChannelFromLine(ioline_t line);

  constexpr Status
  setSQR(ADCConversionGroup  &cgrp, size_t sequence, uint32_t channelMsk, Cycles cycles);
  constexpr Status
  setSMPR(ADCConversionGroup &cgrp, uint32_t channelMsk, uint32_t sampleCycleMsk);


 
  constexpr Context
  modifyConversionGroup(Cycles c, Context context) {
    context.lastCycles = c;
    context.status |= setSMPR(context.cgroup, std::to_underlying(context.lastChannel),
			      std::to_underlying(c));
    context.cycleBySeq[context.sequenceIndex] = nbCycles[std::to_underlying(c)] + cyclesToStart;
    return context;
  }

  constexpr Context
  modifyConversionGroup(Channels ch, Context context) {
    // internal cycles are by default 480, not the last value used
    const Cycles defaultsCycles = std::to_underlying(ch) >= 16 ?
                                  Cycles::C480 : context.lastCycles;
    context.status |= setSQR(context.cgroup, context.sequenceIndex,
			     std::to_underlying(ch), defaultsCycles);
    context.cycleBySeq[context.sequenceIndex++] =
      nbCycles[std::to_underlying(context.lastCycles)] + cyclesToStart;
    context.lastChannel = ch;
    return context;
  }

  constexpr Context
  modifyConversionGroup(adccallback_t end_cb, Context context) {
    context.cgroup.end_cb = end_cb;
    return context;
  }

  constexpr Context
  modifyConversionGroup(adcerrorcallback_t err_cb, Context context) {
    context.cgroup.error_cb = err_cb;
    return context;
  }

  constexpr Context
  modifyConversionGroup(Modes mode, Context context) {
    context.mode = mode;
    context.cgroup.circular = mode == Modes::Continuous;
    return context;
  }

  constexpr Context
  modifyConversionGroup(ExtSels extSel, Context context) {
    context.cgroup.cr2 = ADC_CR2_EXTEN_RISING |
      ADC_CR2_EXTSEL_SRC(std::to_underlying(extSel));
    if (context.mode != Modes::Continuous) {
      context.status |= Status::ExtTriggerImposeContinuous;
    }
    return context;
  }

  constexpr Context
  modifyConversionGroup(const ADCConversionGroup grpp, Context context) {
    ADCConversionGroup &cgr = context.cgroup;
    cgr.htr |=    grpp.htr;
    cgr.ltr |=    grpp.ltr;
    cgr.cr1 |=    grpp.cr1; 
    cgr.cr2 |=    grpp.cr2;             
    
    return context;
  }

  constexpr Context
  modifyConversionGroup(SampleFrequency sf, Context context) {
    context.sampleFrequency = *sf;
    return context;
  }
  
  template<typename ...T>
  constexpr std::expected<ADCConversionGroup, Status>
  get(AuthParamType auto ...c)
  {
    Context context = {};
    ADCConversionGroup &cgr = context.cgroup;
    if (not std::is_constant_evaluated()) {
      memset(&cgr, 0U, sizeof(ADCConversionGroup));
    }
    cgr.circular = false;
    cgr.end_cb = nullptr;
    cgr.error_cb = nullptr;
    cgr.cr2 = ADC_CR2_SWSTART;
    
    ( (context = modifyConversionGroup(c, context)), ... );
    cgr.num_channels = context.sequenceIndex;
    if (context.sequenceIndex == 0)
      context.status |= Status::EmptySequence;

    const uint32_t totalCycles = std::accumulate(context.cycleBySeq.begin(),
						 context.cycleBySeq.end(), 0);
    if (context.sampleFrequency and (totalCycles >=  (STM32_ADCCLK / context.sampleFrequency)))
      context.status |= Status::sampleCycleOverload;

    if (context.status != Status::Ok)
      return std::unexpected(context.status);
    else
      return context.cgroup;
  }


  // constexpr Channels
  // getChannelFromLine(ioline_t line)
  // {
  //   const uint32_t port  =  line & 0xFFFFFFF0U;
  //   const ioportmask_t pad = PAL_PAD(line);
  //   int ret = -1;
    
  //   if (port == GPIOA_BASE) {
  //     ret = pad;
  //   } else if (port == GPIOB_BASE) {
  //     ret = pad + 8;
  //   } else if (port == GPIOC_BASE) {
  //     ret = pad + 10;
  //   }
    
  //   return static_cast<Channels>(ret);
  // }

  constexpr Status
  setSQR(ADCConversionGroup  &cgrp, size_t sequence, uint32_t channelMsk, Cycles cycles)
  {
    if (sequence <= 5) {
      cgrp.sqr3 |= (channelMsk << (sequence*5));
    } else if (sequence <= 11) {
      cgrp.sqr2 |= (channelMsk << (sequence-6)*5);
    } else if (sequence <= 15) {
      cgrp.sqr1 |= (channelMsk << (sequence-12)*5);
    } else {
      // error to much sequences
      return Status::InvalidSequence;
    }
    // default value is maximum SMPR cycle for the channel
    return setSMPR(cgrp, channelMsk, std::to_underlying(cycles));
  }



  
  constexpr Status
  setSMPR(ADCConversionGroup  &cgrp, uint32_t channelMsk, uint32_t sampleCycleMsk)
  {
    if (channelMsk <= 9) {
      const uint32_t mask = 0b111 << (channelMsk * 3);
      cgrp.smpr2 &= ~mask;
      cgrp.smpr2 |= (sampleCycleMsk << (channelMsk * 3));
    } else if (channelMsk <= 18) {
      // Internal channels have to be sampled @ maximum SMPR cycles;
      if ((channelMsk >= 16) && (sampleCycleMsk != 7))
	return Status::InvalidInternalCycles;

      const uint32_t mask = 0b111 << ((channelMsk-10) * 3);
      cgrp.smpr1 &= ~mask;
      cgrp.smpr1 |= (sampleCycleMsk << ((channelMsk-10) * 3));
    } else {
      return Status::InvalidChannel;
    }
    return Status::Ok;
  }

#define __INTERNAL_ADCHELPER_STATIC_CHECK(val,cond)	\
  constexpr auto meet##cond##val = [] -> bool { \
    return  val.has_value() || \
      (val.error() & AdcCGroup::Status::cond) == AdcCGroup::Status::Ok;}; \
  static_assert(meet##cond##val(), "AdcCGroup initialization error : " #cond)

#define  ADCHELPER_STATIC_CHECK(val) \
  __INTERNAL_ADCHELPER_STATIC_CHECK(val, InvalidChannel); \
  __INTERNAL_ADCHELPER_STATIC_CHECK(val, InvalidSequence);\
  __INTERNAL_ADCHELPER_STATIC_CHECK(val, InvalidInternalCycles); \
  __INTERNAL_ADCHELPER_STATIC_CHECK(val, ExtTriggerImposeContinuous); \
  __INTERNAL_ADCHELPER_STATIC_CHECK(val, EmptySequence); \
  __INTERNAL_ADCHELPER_STATIC_CHECK(val, sampleCycleOverload)

}
