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
 * @file    hal_opamp.c
 * @brief   OPAMP Driver code.
 *
 * @addtogroup OPAMP
 * @{
 */

#include "hal_community.h"

#if (HAL_USE_OPAMP == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

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
 * @brief   OPAMP Driver initialization.
 * @note    This function is implicitly invoked by @p halInit(), there is
 *          no need to explicitly initialize the driver.
 *
 * @init
 */
void opampInit(void) {

  opamp_lld_init();
}

/**
 * @brief   Initializes the standard part of a @p OPAMPDriver structure.
 *
 * @param[out] opampp     pointer to a @p OPAMPDriver object
 *
 * @init
 */
void opampObjectInit(OPAMPDriver *opampp) {

  opampp->state    = OPAMP_STOP;
  opampp->config   = NULL;
#if defined(OPAMP_DRIVER_EXT_INIT_HOOK)
  OPAMP_DRIVER_EXT_INIT_HOOK(opampp);
#endif
}

/**
 * @brief   Configures and activates the OPAMP peripheral.
 * @note    Starting a driver that is already in the @p OPAMP_READY state
 *          disables all the active channels.
 *
 * @param[in] opampp      pointer to a @p OPAMPDriver object
 * @param[in] config    pointer to a @p OPAMPConfig object
 *
 * @api
 */
void opampStart(OPAMPDriver *opampp, const OPAMPConfig *config) {

  osalDbgCheck((opampp != NULL) && (config != NULL)); 
  
  osalSysLock();
  opampp->state = OPAMP_STOP;
  osalDbgAssert((opampp->state == OPAMP_STOP) || (opampp->state == OPAMP_READY),
  		"invalid state");
  opampp->config = config; 
  opamp_lld_start(opampp); 
  opampp->state = OPAMP_READY; 
  osalSysUnlock(); 
}

/**
 * @brief   Deactivates the OPAMP peripheral.
 *
 * @param[in] opampp      pointer to a @p OPAMPDriver object
 *
 * @api
 */
void opampStop(OPAMPDriver *opampp) {

  osalDbgCheck(opampp != NULL);

  osalSysLock();

  osalDbgAssert((opampp->state == OPAMP_STOP) || (opampp->state == OPAMP_READY),
                "invalid state");

  opamp_lld_stop(opampp);
  opampp->config  = NULL;
  opampp->state   = OPAMP_STOP;

  osalSysUnlock();
}

/**
 * @brief   Enables a OPAMP opamparator
 * @pre     The OPAMP unit must have been activated using @p opampStart().
 * @post    The channel is active using the specified configuration.
 * @note    Depending on the hardware implementation this function has
 *          effect starting on the next cycle (recommended implementation)
 *          or immediately (fallback implementation).
 *
 * @param[in] opampp      pointer to a @p OPAMPDriver object
 *
 * @api
 */
void opampEnable(OPAMPDriver *opampp) {

  osalDbgCheck(opampp != NULL);

  osalSysLock();

  osalDbgAssert(opampp->state == OPAMP_READY, "not ready");

  opampEnableI(opampp);

  osalSysUnlock();
}

/**
 * @brief   Disables a OPAMP opamparator
 * @pre     The OPAMP unit must have been activated using @p opampStart().
 * @post    The channel is active using the specified configuration.
 * @note    Depending on the hardware implementation this function has
 *          effect starting on the next cycle (recommended implementation)
 *          or immediately (fallback implementation).
 *
 * @param[in] opampp      pointer to a @p OPAMPDriver object
 *
 * @api
 */
void opampDisable(OPAMPDriver *opampp) {

  osalDbgCheck(opampp != NULL);

  osalSysLock();

  osalDbgAssert(opampp->state == OPAMP_READY, "not ready");

  opampDisableI(opampp);

  osalSysUnlock();
}

#endif /* HAL_USE_OPAMP == TRUE */

/** @} */
