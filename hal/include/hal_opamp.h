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
 * @file    hal_opamp.h
 * @brief   OPAMP Driver macros and structures.
 *
 * @addtogroup OPAMP
 * @{
 */

#ifndef HAL_OPAMP_H
#define HAL_OPAMP_H

#if (HAL_USE_OPAMP == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/** @} */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Driver state machine possible states.
 */
typedef enum {
  OPAMP_UNINIT = 0,                   /**< Not initialized.                   */
  OPAMP_STOP = 1,                     /**< Stopped.                           */
  OPAMP_READY = 2,                    /**< Ready.                             */
  OPAMP_CALIBRATING = 3               /**< Ready.                             */
} opampstate_t;

#define OPAMP_PLUS_BELOW_MOINS (0U)
#define OPAMP_MOINS_BELOW_PLUS (1U)

/**
 * @brief   Type of a structure representing a OPAMP driver.
 */
typedef struct OPAMPDriver OPAMPDriver;


#include "hal_opamp_lld.h"

/**
 * @brief   Enables a OPAMP channel.
 * @pre     The OPAMP unit must have been activated using @p opampStart().
 *
 * @param[in] opampp      pointer to a @p OPAMPDriver object
 * @iclass
 */
#define opampEnableI(opampp) do {                        \
  opamp_lld_enable(opampp);                             \
} while (false)

/**
 * @brief   Disables a OPAMP channel.
 * @pre     The OPAMP unit must have been activated using @p opampStart().
 *
 * @param[in] opampp      pointer to a @p OPAMPDriver object
 * @iclass
 */
#define opampDisableI(opampp) do {                        \
  opamp_lld_disable(opampp);                             \
} while (false)


/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void opampInit(void);
  void opampObjectInit(OPAMPDriver *opampp);
  void opampStart(OPAMPDriver *opampp, const OPAMPConfig *config);
  void opampStop(OPAMPDriver *opampp);
  void opampEnable(OPAMPDriver *opampp);
  void opampDisable(OPAMPDriver *opampp);
  void opampCalibrate();
#ifdef __cplusplus
}
#endif


#endif /* HAL_USE_OPAMP == TRUE */

#endif /* HAL_OPAMP_H */

/** @} */
