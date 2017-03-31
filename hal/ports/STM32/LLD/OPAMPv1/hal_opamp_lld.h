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
 * @file    OPAMPv1/hal_opamp_lld.h
 * @brief   STM32 OPAMP subsystem low level driver header.
 *
 * @addtogroup OPAMP
 * @{
 */

#ifndef HAL_OPAMP_LLD_H
#define HAL_OPAMP_LLD_H

#include "hal_community.h"

#if HAL_USE_OPAMP || defined(__DOXYGEN__)

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
 * @brief   OPAMPD1 driver enable switch.
 * @details If set to @p TRUE the support for OPAMPD1 is included.
 * @note    The default is @p FALSE
 */
#if !defined(STM32_OPAMP_USE_OPAMP1) || defined(__DOXYGEN__)
#define STM32_OPAMP_USE_OPAMP1                  FALSE
#endif

/**
 * @brief   OPAMPD2 driver enable switch.
 * @details If set to @p TRUE the support for OPAMPD2 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(STM32_OPAMP_USE_OPAMP2) || defined(__DOXYGEN__)
#define STM32_OPAMP_USE_OPAMP2                  FALSE
#endif

/**
 * @brief   OPAMPD3 driver enable switch.
 * @details If set to @p TRUE the support for OPAMPD3 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(STM32_OPAMP_USE_OPAMP3) || defined(__DOXYGEN__)
#define STM32_OPAMP_USE_OPAMP3                  FALSE
#endif

/**
 * @brief   OPAMPD4 driver enable switch.
 * @details If set to @p TRUE the support for OPAMPD4 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(STM32_OPAMP_USE_OPAMP4) || defined(__DOXYGEN__)
#define STM32_OPAMP_USE_OPAMP4                  FALSE
#endif

/**
 * @brief   OPAMPD TRIM and CALIBRATION enable switch.
 * @details If set to @p TRUE the support for USER_TRIM is included and calibration is done @init
 * @note    The default is @p TRUE.
 */
#if !defined(STM32_OPAMP_USER_TRIM_ENABLED) || defined(__DOXYGEN__)
#define STM32_OPAMP_USER_TRIM_ENABLED		TRUE
#endif


/** @} */

/*===========================================================================*/
/* Configuration checks.                                                     */
/*===========================================================================*/

#if STM32_OPAMP_USE_OPAMP1 && !STM32_HAS_OPAMP1
#error "OPAMP1 not present in the selected device"
#endif

#if STM32_OPAMP_USE_OPAMP2 && !STM32_HAS_OPAMP2
#error "OPAMP2 not present in the selected device"
#endif

#if STM32_OPAMP_USE_OPAMP3 && !STM32_HAS_OPAMP3
#error "OPAMP3 not present in the selected device"
#endif

#if STM32_OPAMP_USE_OPAMP4 && !STM32_HAS_OPAMP4
#error "OPAMP4 not present in the selected device"
#endif


#if !STM32_OPAMP_USE_OPAMP1 && !STM32_OPAMP_USE_OPAMP2 &&                   \
    !STM32_OPAMP_USE_OPAMP3 && !STM32_OPAMP_USE_OPAMP4 
#error "OPAMP driver activated but no OPAMP peripheral assigned"
#endif



/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type of a OPAMP driver configuration structure.
 */
typedef struct {
   /**
    * @brief COMP CSR register initialization data.
    */
  uint32_t		csr;
} OPAMPConfig;

/**
 * @brief   Structure representing a OPAMP driver.
 */
struct OPAMPDriver {
  /**
   * @brief Driver state.
   */
  opampstate_t			state;
  /**
   * @brief Current driver configuration data.
   */
  const OPAMPConfig		*config;
  /**
   * @brief   Current OPAMP period in ticks.
   */
  OPAMP_TypeDef			*opamp;

#if STM32_OPAMP_USER_TRIM_ENABLED
  uint16_t			trim_p;
  uint16_t			trim_n;
#endif
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/


/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if STM32_OPAMP_USE_OPAMP1 && !defined(__DOXYGEN__)
extern OPAMPDriver OPAMPD1;
#endif

#if STM32_OPAMP_USE_OPAMP2 && !defined(__DOXYGEN__)
extern OPAMPDriver OPAMPD2;
#endif

#if STM32_OPAMP_USE_OPAMP3 && !defined(__DOXYGEN__)
extern OPAMPDriver OPAMPD3;
#endif

#if STM32_OPAMP_USE_OPAMP4 && !defined(__DOXYGEN__)
extern OPAMPDriver OPAMPD4;
#endif


#ifdef __cplusplus
extern "C" {
#endif
  void opamp_lld_init(void);
  void opamp_lld_start(OPAMPDriver *opampp);
  void opamp_lld_stop(OPAMPDriver *opampp);
  void opamp_lld_enable(OPAMPDriver *opampp);
  void opamp_lld_disable(OPAMPDriver *opampp);
#ifdef __cplusplus
}
#endif




#endif /* HAL_USE_OPAMP */

#endif /* HAL_OPAMP_LLD_H */

/** @} */
