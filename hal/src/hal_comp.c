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
 * @file    hal_comp.c
 * @brief   COMP Driver code.
 *
 * @addtogroup COMP
 * @{
 */

#include "hal_community.h"

#if (HAL_USE_COMP == TRUE) || defined(__DOXYGEN__)

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
 * @brief   COMP Driver initialization.
 * @note    This function is implicitly invoked by @p halInit(), there is
 *          no need to explicitly initialize the driver.
 *
 * @init
 */
void compInit(void) {

  comp_lld_init();
}

/**
 * @brief   Initializes the standard part of a @p COMPDriver structure.
 *
 * @param[out] compp     pointer to a @p COMPDriver object
 *
 * @init
 */
void compObjectInit(COMPDriver *compp) {

  compp->state    = COMP_STOP;
  compp->config   = NULL;
#if defined(COMP_DRIVER_EXT_INIT_HOOK)
  COMP_DRIVER_EXT_INIT_HOOK(compp);
#endif
}

/**
 * @brief   Configures and activates the COMP peripheral.
 * @note    Starting a driver that is already in the @p COMP_READY state
 *          disables all the active channels.
 *
 * @param[in] compp      pointer to a @p COMPDriver object
 * @param[in] config    pointer to a @p COMPConfig object
 *
 * @api
 */
void compStart(COMPDriver *compp, const COMPConfig *config) {

  osalDbgCheck((compp != NULL) && (config != NULL));

  osalSysLock();
  osalDbgAssert((compp->state == COMP_STOP) || (compp->state == COMP_READY),
                "invalid state");
  compp->config = config;
  comp_lld_start(compp);
  compp->state = COMP_READY;
  osalSysUnlock();
}

/**
 * @brief   Deactivates the COMP peripheral.
 *
 * @param[in] compp      pointer to a @p COMPDriver object
 *
 * @api
 */
void compStop(COMPDriver *compp) {

  osalDbgCheck(compp != NULL);

  osalSysLock();

  osalDbgAssert((compp->state == COMP_STOP) || (compp->state == COMP_READY),
                "invalid state");

  comp_lld_stop(compp);
  compp->config  = NULL;
  compp->state   = COMP_STOP;

  osalSysUnlock();
}

/**
 * @brief   Enables a COMP comparator
 * @pre     The COMP unit must have been activated using @p compStart().
 * @post    The channel is active using the specified configuration.
 * @note    Depending on the hardware implementation this function has
 *          effect starting on the next cycle (recommended implementation)
 *          or immediately (fallback implementation).
 *
 * @param[in] compp      pointer to a @p COMPDriver object
 *
 * @api
 */
void compEnable(COMPDriver *compp) {

  osalDbgCheck(compp != NULL);

  osalSysLock();

  osalDbgAssert(compp->state == COMP_READY, "not ready");

  compEnableI(compp);

  osalSysUnlock();
}

/**
 * @brief   Disables a COMP comparator
 * @pre     The COMP unit must have been activated using @p compStart().
 * @post    The channel is active using the specified configuration.
 * @note    Depending on the hardware implementation this function has
 *          effect starting on the next cycle (recommended implementation)
 *          or immediately (fallback implementation).
 *
 * @param[in] compp      pointer to a @p COMPDriver object
 *
 * @api
 */
void compDisable(COMPDriver *compp) {

  osalDbgCheck(compp != NULL);

  osalSysLock();

  osalDbgAssert(compp->state == COMP_READY, "not ready");

  compDisableI(compp);

  osalSysUnlock();
}


#endif /* HAL_USE_COMP == TRUE */

/** @} */
