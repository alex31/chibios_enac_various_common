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
 * @file    hal.h
 * @brief   HAL subsystem header.
 *
 * @addtogroup HAL
 * @{
 */

#ifndef HAL_COMMUNITY_H
#define HAL_COMMUNITY_H

#include "osal.h"
#include "board.h"
#include "halconf.h"

/* Error checks on the configuration header file.*/
#if !defined(HAL_USE_COMP)
#define HAL_USE_COMP                         FALSE
#endif

/* Low Level HAL support.*/
#include "hal_community_lld.h"

/* Normal drivers.*/
#include "hal_comp.h"
#include "hal_opamp.h"

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void halCommunityInit(void);
#ifdef __cplusplus
}
#endif

#endif /* HAL_COMMUNITY_H */

/** @} */
