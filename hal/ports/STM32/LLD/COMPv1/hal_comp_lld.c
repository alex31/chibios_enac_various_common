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
 * @file    hal_comp_lld.c
 * @brief   STM32 COMP subsystem low level driver header.
 *
 * @addtogroup COMP
 * @{
 */

#include "hal_community.h"

#if HAL_USE_COMP || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   COMPD1 driver identifier.
 */
#if STM32_COMP_USE_COMP1 || defined(__DOXYGEN__)
COMPDriver COMPD1;
#endif

/**
 * @brief   COMPD2 driver identifier.
 */
#if STM32_COMP_USE_COMP2 || defined(__DOXYGEN__)
COMPDriver COMPD2;
#endif

/**
 * @brief   COMPD3 driver identifier.
 */
#if STM32_COMP_USE_COMP3 || defined(__DOXYGEN__)
COMPDriver COMPD3;
#endif

/**
 * @brief   COMPD4 driver identifier.
 */
#if STM32_COMP_USE_COMP4 || defined(__DOXYGEN__)
COMPDriver COMPD4;
#endif

/**
 * @brief   COMPD5 driver identifier.
 */
#if STM32_COMP_USE_COMP5 || defined(__DOXYGEN__)
COMPDriver COMPD5;
#endif

/**
 * @brief   COMPD6 driver identifier.
 */
#if STM32_COMP_USE_COMP6 || defined(__DOXYGEN__)
COMPDriver COMPD6;
#endif

/**
 * @brief   COMPD9 driver identifier.
 */
#if STM32_COMP_USE_COMP7 || defined(__DOXYGEN__)
COMPDriver COMPD7;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/


/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level COMP driver initialization.
 *
 * @notapi
 */
void comp_lld_init(void) {

#if STM32_COMP_USE_COMP1
  /* Driver initialization.*/
  compObjectInit(&COMPD1);
  COMPD1.comp = COMP1;
#endif

#if STM32_COMP_USE_COMP2
  /* Driver initialization.*/
  compObjectInit(&COMPD2);
  COMPD2.comp = COMP2;
#endif

#if STM32_COMP_USE_COMP3
  /* Driver initialization.*/
  compObjectInit(&COMPD3);
  COMPD3.comp = COMP3;
#endif

#if STM32_COMP_USE_COMP4
  /* Driver initialization.*/
  compObjectInit(&COMPD4);
  COMPD4.comp = COMP4;
#endif

#if STM32_COMP_USE_COMP5
  /* Driver initialization.*/
  compObjectInit(&COMPD5);
  COMPD5.comp = COMP5;
#endif

#if STM32_COMP_USE_COMP6
  /* Driver initialization.*/
  compObjectInit(&COMPD6);
  COMPD6.comp = COMP6;
#endif

#if STM32_COMP_USE_COMP7
  /* Driver initialization.*/
  compObjectInit(&COMPD7);
  COMPD7.comp = COMP7;
#endif
}

/**
 * @brief   Configures and activates the COMP peripheral.
 * @note    Starting a driver that is already in the @p COMP_READY state
 *          disables output.
 *
 * @param[in] compp      pointer to a @p COMPDriver object
 *
 * @notapi
 */
void comp_lld_start(COMPDriver *compp) {
  (void) compp;

  if (compp->config->output_inverted) {
    compp->comp->CSR = (compp->config->csr & ~COMP_CSR_COMPxEN) | COMP_CSR_COMPxPOL;
  } else {
    compp->comp->CSR = compp->config->csr & ~COMP_CSR_COMPxEN;
  }
}

/**
 * @brief   Deactivates the COMP peripheral.
 *
 * @param[in] compp      pointer to a @p COMPDriver object
 *
 * @notapi
 */
void comp_lld_stop(COMPDriver *compp) {
  compp->comp->CSR = 0U;
}

/**
 * @brief   Enables a COMP channel.
 * @pre     The COMP unit must have been activated using @p compStart().
 * @param[in] compp      pointer to a @p COMPDriver object
 *
 * @notapi
 */
void comp_lld_enable(COMPDriver *compp) {
  compp->comp->CSR |= COMP_CSR_COMPxEN;
}

/**
 * @brief   Disables a COMP channel 
 * @pre     The COMP unit must have been activated using @p compStart().
 * @param[in] compp      pointer to a @p COMPDriver object
 *
 * @notapi
 */
void comp_lld_disable(COMPDriver *compp) {
  compp->comp->CSR &= ~COMP_CSR_COMPxEN;
}



#endif /* HAL_USE_COMP */

/** @} */
