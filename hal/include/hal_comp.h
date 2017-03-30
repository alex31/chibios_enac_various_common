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
 * @file    hal_comp.h
 * @brief   COMP Driver macros and structures.
 *
 * @addtogroup COMP
 * @{
 */

#ifndef HAL_COMP_H
#define HAL_COMP_H

#if (HAL_USE_COMP == TRUE) || defined(__DOXYGEN__)

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
  COMP_UNINIT = 0,                   /**< Not initialized.                   */
  COMP_STOP = 1,                     /**< Stopped.                           */
  COMP_READY = 2                     /**< Ready.                             */
} compstate_t;

#define COMP_PLUS_BELOW_MOINS (0U)
#define COMP_MOINS_BELOW_PLUS (1U)

/**
 * @brief   Type of a structure representing a COMP driver.
 */
typedef struct COMPDriver COMPDriver;


#include "hal_comp_lld.h"

/**
 * @brief   Enables a COMP channel.
 * @pre     The COMP unit must have been activated using @p compStart().
 *
 * @param[in] compp      pointer to a @p COMPDriver object
 * @iclass
 */
#define compEnableI(compp) do {                        \
  comp_lld_enable(compp);                             \
} while (false)

/**
 * @brief   Disables a COMP channel.
 * @pre     The COMP unit must have been activated using @p compStart().
 *
 * @param[in] compp      pointer to a @p COMPDriver object
 * @iclass
 */
#define compDisableI(compp) do {                        \
  comp_lld_disable(compp);                             \
} while (false)


/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void compInit(void);
  void compObjectInit(COMPDriver *compp);
  void compStart(COMPDriver *compp, const COMPConfig *config);
  void compStop(COMPDriver *compp);
  void compEnable(COMPDriver *compp);
  void compDisable(COMPDriver *compp);
#ifdef __cplusplus
}
#endif

#define compGetOutputX(compp) comp_lld_getOutput(compp)

#endif /* HAL_USE_COMP == TRUE */

#endif /* HAL_COMP_H */

/** @} */
