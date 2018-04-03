#pragma once

#include "ch.h"
#include "hal.h"

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   ICUD1 driver enable switch.
 * @details If set to @p TRUE the support for ICUD1 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(STM32_ICU_USE_TIM15) || defined(__DOXYGEN__)
#define STM32_ICU_USE_TIM15                  FALSE
#endif

/**
 * @brief   ICUD2 interrupt priority level setting.
 */
#if !defined(STM32_ICU_TIM15_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define STM32_ICU_TIM15_IRQ_PRIORITY         7
#endif

#if STM32_ICU_USE_TIM15 && !STM32_HAS_TIM15
#error "TIM15 not present in the selected device"
#endif


#if STM32_ICU_USE_TIM15
#if defined(STM32_TIM15_IS_USED)
#error "ICUD2 requires TIM15 but the timer is already used"
#else
#define STM32_TIM15_IS_USED
#endif
#endif


#if STM32_ICU_USE_TIM15 && !defined(STM32_TIM15_SUPPRESS_ISR) &&              \
    !OSAL_IRQ_IS_VALID_PRIORITY(STM32_ICU_TIM15_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to TIM15"
#endif


#if STM32_ICU_USE_TIM15 && !defined(__DOXYGEN__)
extern ICUDriver ICUD15;
#endif

#ifdef __cplusplus
extern "C" {
#endif
void icuOptStart(ICUDriver *icup, const ICUConfig *config);
void icu_opt_lld_init(void);
void icu_opt_lld_start(ICUDriver *icup);
void icu_opt_lld_stop(ICUDriver *icup);
#ifdef __cplusplus
}
#endif
