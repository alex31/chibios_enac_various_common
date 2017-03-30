/*
    ChibiOS - Copyright (C) 2006..2016 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @file    COMPv1/hal_comp_lld.h
 * @brief   STM32 COMP subsystem low level driver header.
 *
 * @addtogroup COMP
 * @{
 */

#ifndef HAL_COMP_LLD_H
#define HAL_COMP_LLD_H

#include "hal_community.h"

#if HAL_USE_COMP || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/



/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */

/**
 * @brief   COMPD1 driver enable switch.
 * @details If set to @p TRUE the support for COMPD1 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(STM32_COMP_USE_COMP1) || defined(__DOXYGEN__)
#define STM32_COMP_USE_COMP1                  FALSE
#endif

/**
 * @brief   COMPD2 driver enable switch.
 * @details If set to @p TRUE the support for COMPD2 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(STM32_COMP_USE_COMP2) || defined(__DOXYGEN__)
#define STM32_COMP_USE_COMP2                  FALSE
#endif

/**
 * @brief   COMPD3 driver enable switch.
 * @details If set to @p TRUE the support for COMPD3 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(STM32_COMP_USE_COMP3) || defined(__DOXYGEN__)
#define STM32_COMP_USE_COMP3                  FALSE
#endif

/**
 * @brief   COMPD4 driver enable switch.
 * @details If set to @p TRUE the support for COMPD4 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(STM32_COMP_USE_COMP4) || defined(__DOXYGEN__)
#define STM32_COMP_USE_COMP4                  FALSE
#endif

/**
 * @brief   COMPD5 driver enable switch.
 * @details If set to @p TRUE the support for COMPD5 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(STM32_COMP_USE_COMP5) || defined(__DOXYGEN__)
#define STM32_COMP_USE_COMP5                  FALSE
#endif

/**
 * @brief   COMPD8 driver enable switch.
 * @details If set to @p TRUE the support for COMPD8 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(STM32_COMP_USE_COMP6) || defined(__DOXYGEN__)
#define STM32_COMP_USE_COMP6                  FALSE
#endif

/**
 * @brief   COMPD9 driver enable switch.
 * @details If set to @p TRUE the support for COMPD9 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(STM32_COMP_USE_COMP7) || defined(__DOXYGEN__)
#define STM32_COMP_USE_COMP7                  FALSE
#endif

/** @} */

/*===========================================================================*/
/* Configuration checks.                                                     */
/*===========================================================================*/

#if STM32_COMP_USE_COMP1 && !STM32_HAS_COMP1
#error "COMP1 not present in the selected device"
#endif

#if STM32_COMP_USE_COMP2 && !STM32_HAS_COMP2
#error "COMP2 not present in the selected device"
#endif

#if STM32_COMP_USE_COMP3 && !STM32_HAS_COMP3
#error "COMP3 not present in the selected device"
#endif

#if STM32_COMP_USE_COMP4 && !STM32_HAS_COMP4
#error "COMP4 not present in the selected device"
#endif

#if STM32_COMP_USE_COMP5 && !STM32_HAS_COMP5
#error "COMP5 not present in the selected device"
#endif

#if STM32_COMP_USE_COMP6 && !STM32_HAS_COMP6
#error "COMP6 not present in the selected device"
#endif

#if STM32_COMP_USE_COMP7 && !STM32_HAS_COMP7
#error "COMP7 not present in the selected device"
#endif

#if !STM32_COMP_USE_COMP1 && !STM32_COMP_USE_COMP2 &&                           \
    !STM32_COMP_USE_COMP3 && !STM32_COMP_USE_COMP4 &&                           \
    !STM32_COMP_USE_COMP5 && !STM32_COMP_USE_COMP6 &&                           \
    !STM32_COMP_USE_COMP7
#error "COMP driver activated but no COMP peripheral assigned"
#endif



/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type of a COMP driver configuration structure.
 */
typedef struct {
  /**
   * @brief   Output polarity

   * @note    invert the comparator output if set to true
   *          
   */
  bool			output_inverted;
   /**
    * @brief COMP CSR register initialization data.
    */
  uint32_t		csr;
} COMPConfig;

/**
 * @brief   Structure representing a COMP driver.
 */
struct COMPDriver {
  /**
   * @brief Driver state.
   */
  compstate_t                state;
  /**
   * @brief Current driver configuration data.
   */
  const COMPConfig           *config;
  /**
   * @brief   Current COMP period in ticks.
   */
  COMP_TypeDef                *comp;
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/


/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if STM32_COMP_USE_COMP1 && !defined(__DOXYGEN__)
extern COMPDriver COMPD1;
#endif

#if STM32_COMP_USE_COMP2 && !defined(__DOXYGEN__)
extern COMPDriver COMPD2;
#endif

#if STM32_COMP_USE_COMP3 && !defined(__DOXYGEN__)
extern COMPDriver COMPD3;
#endif

#if STM32_COMP_USE_COMP4 && !defined(__DOXYGEN__)
extern COMPDriver COMPD4;
#endif

#if STM32_COMP_USE_COMP5 && !defined(__DOXYGEN__)
extern COMPDriver COMPD5;
#endif

#if STM32_COMP_USE_COMP6 && !defined(__DOXYGEN__)
extern COMPDriver COMPD6;
#endif

#if STM32_COMP_USE_COMP7 && !defined(__DOXYGEN__)
extern COMPDriver COMPD7;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void comp_lld_init(void);
  void comp_lld_start(COMPDriver *compp);
  void comp_lld_stop(COMPDriver *compp);
  void comp_lld_enable(COMPDriver *compp);
  void comp_lld_disable(COMPDriver *compp);
#ifdef __cplusplus
}
#endif

/**
 * @brief     return output state of the comparator
 * @pre       The COMP unit must have been activated using @p compStart().
 * @param[in] compp      pointer to a @p COMPDriver object
 * @return    output state of comparator
 * @notapi
 */
#define COMP_API_GETPOUTPUT 1
static inline uint32_t comp_lld_getOutput(const COMPDriver *compp) {
  return (compp->comp->CSR | COMP_CSR_COMPxOUT) ? COMP_MOINS_BELOW_PLUS : COMP_PLUS_BELOW_MOINS;
}

/**
 * @brief     set polarity of the comparator
 * @pre       The COMP unit must have been activated using @p compStart().
 * @param[in] compp      pointer to a @p COMPDriver object
 * @param[in] polarity   should be COMP_POLARITY_NORMAL or COMP_POLATITY_INVERTED
 * @notapi
 */
#define COMP_API_SETPOLARITY 1
static inline void comp_lld_setPolarity(const COMPDriver *compp, const uint32_t polarity) {
  if (polarity)
    compp->comp->CSR |= COMP_CSR_COMPxPOL;
  else
    compp->comp->CSR &= ~COMP_CSR_COMPxPOL;
}

/**
 * @brief     get polarity of the comparator
 * @pre       The COMP unit must have been activated using @p compStart().
 * @param[in] compp      pointer to a @p COMPDriver object
 * @return    comparator polarity : COMP_POLARITY_NORMAL or COMP_POLATITY_INVERTED
 * @notapi
 */
#define COMP_API_GETPOLARITY 1
static inline uint32_t comp_lld_getPolarity(const COMPDriver *compp) {
  return (compp->comp->CSR  & COMP_CSR_COMPxPOL) ? COMP_POLATITY_INVERTED : COMP_POLARITY_NORMAL;
}



#endif /* HAL_USE_COMP */

#endif /* HAL_COMP_LLD_H */

/** @} */
