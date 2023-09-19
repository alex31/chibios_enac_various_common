#pragma once

#include "ch.h"
#include "hal.h"

#include <concepts>
#include <expected>
#include <utility>
#include <cstring>
#include <array>
#include <numeric>

#if not( defined(STM32H723xx) || defined(STM32H725xx)  ||	\
      defined(STM32H730xx) || defined(STM32H730xxQ) ||	\
    defined(STM32H733xx) || defined(STM32H735xx)  ||	\
    defined(STM32H742xx) || defined(STM32H743xx)  ||	\
    defined(STM32H745xx) || defined(STM32H745xG)  ||	\
    defined(STM32H747xx) || defined(STM32H747xG)  ||	\
    defined(STM32H750xx) || defined(STM32H753xx)  ||	\
    defined(STM32H755xx) || defined(STM32H757xx)  ||	\
    defined(STM32H7A3xx) || defined(STM32H7A3xxQ) ||	\
    defined(STM32H7B0xx) || defined(STM32H7B0xxQ) ||	\
  defined(STM32H7B3xx) || defined(STM32H7B3xxQ))
#error #error "this ADCV4 driver is specific to STM32H7"
#endif

/*
  TODO:
  ° test
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

  constexpr std::array<uint16_t, 8> nbCycles = {1,2,8,16,32,64,384,810};
  constexpr uint16_t cyclesToStart = 8;
  
  enum class Status {Ok=0, InvalidChannel=1<<0, InvalidSequence=1<<1,
    InvalidInternalCycles=1<<2, sampleCycleOverload=1<<3,
    ExtTriggerImposeContinuous=1<<4, EmptySequence=1<<5};

  enum class Cycles {C1P5, C2P5, C8P5, C16P5, C32P5, C64P5,
		     C384P5, C810P5};

  enum class Channels {IN0=ADC_CHANNEL_IN0, IN1, IN2, IN3, IN4, IN5, IN6, IN7, IN8, IN9,
    IN10, IN11, IN12, IN13, IN14, IN15, IN16, IN17, IN18, IN19,
    VBAT=IN17, VSENSE=IN18, VREFINT=IN19};

  enum class ExtSels {TIM1_OC1, TIM1_OC2, TIM1_OC3, TIM2_OC2, TIM3_TRGO, TIM4_OC4,
    EXTI11, TIM8_TRGO, TIM8_TRGO2, TIM1_TRGO, TIM1_TRGO2, TIM2_TRGO, TIM4_TRGO,
    TIM6_TRGO, TIM15_TRGO, TIM3_OC4, HRTIM1_ADCTRG1, HRTIM1_ADCTRG3,
    LPTIM1_OUT, LPTIM2_OUT, LPTIM3_OUT};


  
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
    Cycles lastCycles = Cycles::C810P5;
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
    const auto ch_ulg = std::to_underlying(ch);
    const Cycles defaultsCycles = ch_ulg >= 17 ?
                                  Cycles::C810P5 : context.lastCycles;
    context.status |= setSQR(context.cgroup, context.sequenceIndex,
			     ch_ulg, defaultsCycles);
    context.cgroup.pcsel |= (1U << ch_ulg);
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
    if (mode == Modes::Continuous) {
      context.cgroup.circular =  true;
      context.cgroup.cfgr |= ADC_CFGR_CONT_ENABLED;
    }
    return context;
  }

  constexpr Context
  modifyConversionGroup(ExtSels extSel, Context context) {
    context.cgroup.cfgr |= (ADC_CFGR_EXTEN_RISING |
			    ADC_CFGR_EXTSEL_SRC(std::to_underlying(extSel)));
    if (context.mode != Modes::Continuous) {
      context.status |= Status::ExtTriggerImposeContinuous;
    }
    return context;
  }

  constexpr Context
  modifyConversionGroup(const ADCConversionGroup grpp, Context context) {
    ADCConversionGroup &cgr = context.cgroup;
    cgr.cfgr    |=  grpp.cfgr;
    cgr.cfgr2   |=  grpp.cfgr2; 
    cgr.ccr     |=  grpp.ccr;   
    cgr.ltr1	|=  grpp.ltr1;	 
    cgr.htr1	|=  grpp.htr1;	 
    cgr.ltr2	|=  grpp.ltr2;	 
    cgr.htr2	|=  grpp.htr2;	 
    cgr.ltr3	|=  grpp.ltr3;	 
    cgr.htr3	|=  grpp.htr3;	 
    cgr.awd2cr	|= grpp.awd2cr;
    cgr.awd3cr	|=  grpp.awd3cr;

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
    if (sequence <= 3U) { // 0..3
      cgrp.sqr[0] |= (channelMsk << ((sequence+1U)*6U));
    } else if (sequence <= 8U) { // 4..8
      cgrp.sqr[1] |= (channelMsk << (sequence-4U)*6U);
    } else if (sequence <= 13U) { // 9..13
      cgrp.sqr[2] |= (channelMsk << (sequence-9U)*6U);
    } else if (sequence <= 15U) { // 14..15
      cgrp.sqr[3] |= (channelMsk << (sequence-14U)*6U);
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
      cgrp.smpr[0] &= ~mask;
      cgrp.smpr[0] |= (sampleCycleMsk << (channelMsk * 3));
    } else if (channelMsk <= 18) {
      // Internal channels have to be sampled @ maximum SMPR cycles;
      if ((channelMsk >= 16) && (sampleCycleMsk != 7))
	return Status::InvalidInternalCycles;

      const uint32_t mask = 0b111 << ((channelMsk-10) * 3);
      cgrp.smpr[1] &= ~mask;
      cgrp.smpr[1] |= (sampleCycleMsk << ((channelMsk-10) * 3));
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
