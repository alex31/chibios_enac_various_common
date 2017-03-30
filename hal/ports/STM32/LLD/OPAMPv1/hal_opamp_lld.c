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
 * @file    hal_opamp_lld.c
 * @brief   STM32 OPAMP subsystem low level driver header.
 *
 * @addtogroup OPAMP
 * @{
 */

#include "hal_community.h"

#if HAL_USE_OPAMP || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   OPAMPD1 driver identifier.
 */
#if STM32_OPAMP_USE_OPAMP1 || defined(__DOXYGEN__)
OPAMPDriver OPAMPD1;
#endif

/**
 * @brief   OPAMPD2 driver identifier.
 */
#if STM32_OPAMP_USE_OPAMP2 || defined(__DOXYGEN__)
OPAMPDriver OPAMPD2;
#endif

/**
 * @brief   OPAMPD3 driver identifier.
 */
#if STM32_OPAMP_USE_OPAMP3 || defined(__DOXYGEN__)
OPAMPDriver OPAMPD3;
#endif

/**
 * @brief   OPAMPD4 driver identifier.
 */
#if STM32_OPAMP_USE_OPAMP4 || defined(__DOXYGEN__)
OPAMPDriver OPAMPD4;
#endif


/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/
static void opamp_lld_calibrate(void);

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level OPAMP driver initialization.
 *
 * @notapi
 */
void opamp_lld_init(void) {

#if STM32_OPAMP_USE_OPAMP1
  /* Driver initialization.*/
  opampObjectInit(&OPAMPD1);
  OPAMPD1.opamp = OPAMP1;
#endif

#if STM32_OPAMP_USE_OPAMP2
  /* Driver initialization.*/
  opampObjectInit(&OPAMPD2);
  OPAMPD2.opamp = OPAMP2;
#endif

#if STM32_OPAMP_USE_OPAMP3
  /* Driver initialization.*/
  opampObjectInit(&OPAMPD3);
  OPAMPD3.opamp = OPAMP3;
#endif

#if STM32_OPAMP_USE_OPAMP4
  /* Driver initialization.*/
  opampObjectInit(&OPAMPD4);
  OPAMPD4.opamp = OPAMP4;
#endif
}

/**
 * @brief   Configures and activates the OPAMP peripheral.
 * @note    Starting a driver that is already in the @p OPAMP_READY state
 *          disables output.
 *
 * @param[in] opampp      pointer to a @p OPAMPDriver object
 *
 * @notapi
 */
void opamp_lld_start(OPAMPDriver *opampp) {
  opamp_lld_disable(opampp);
}

/**
 * @brief   Deactivates the OPAMP peripheral.
 *
 * @param[in] opampp      pointer to a @p OPAMPDriver object
 *
 * @notapi
 */
void opamp_lld_stop(OPAMPDriver *opampp) {
  opamp_lld_disable(opampp);
}

/**
 * @brief   Enables a OPAMP channel.
 * @pre     The OPAMP unit must have been activated using @p opampStart().
 * @param[in] opampp      pointer to a @p OPAMPDriver object
 *
 * @notapi
 */
void opamp_lld_enable(OPAMPDriver *opampp) {
  opampp->opamp->CSR |= OPAMP_CSR_OPAMPxEN;
}

/**
 * @brief   Disables a OPAMP channel 
 * @pre     The OPAMP unit must have been activated using @p opampStart().
 * @param[in] opampp      pointer to a @p OPAMPDriver object
 *
 * @notapi
 */
void opamp_lld_disable(OPAMPDriver *opampp) {
  opampp->opamp->CSR &= ~OPAMP_CSR_OPAMPxEN;
}


static void opamp_lld_calibrate(void)
{
}

#endif /* HAL_USE_OPAMP */

/** @} */
