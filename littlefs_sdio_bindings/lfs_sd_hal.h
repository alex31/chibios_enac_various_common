/*
 ChibiOS - Copyright (C) 2006..2022 Giovanni Di Sirio

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
/*
 * Parts of this file are:
 *
 * Copyright (c) 2017, Arm Limited. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @file    lfs_hal.h
 * @brief   LittleFS-HAL bindings header.
 *
 * @addtogroup LITTLEFS_BINDINGS
 * @{
 */

#ifndef LFS_WSPI_H
#define LFS_WSPI_H

#include "hal.h"
#include "lfs.h"

/*===========================================================================*/
/* Module constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Module pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Module data structures and types.                                         */
/*===========================================================================*/
typedef enum {LFS_SD_MOUNT=0, LFS_SD_FORMAT=1}  lfs_sd_init_t;
typedef struct {
   uint8_t lfs_file_buffer[MMCSD_BLOCK_SIZE];
} lfs_sd_file_cache_t;
/*===========================================================================*/
/* Module macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  lfs_t* lfs_sd_init(SDCDriver* sdcd, lfs_sd_init_t flags);
  void   lfs_sd_file_cache_init(lfs_sd_file_cache_t * const cache,
				struct lfs_file_config *lfs_file_cfg);
  
  int __lfs_sd_read(const struct lfs_config *c, lfs_block_t block,
		    lfs_off_t off, void *buffer, lfs_size_t size);
  int __lfs_sd_prog(const struct lfs_config *c, lfs_block_t block,
		    lfs_off_t off, const void *buffer, lfs_size_t size);
  int __lfs_sd_erase(const struct lfs_config *c, lfs_block_t block);
  int __lfs_sd_sync(const struct lfs_config *c);
  int __lfs_sd_lock(const struct lfs_config *c);
  int __lfs_sd_unlock(const struct lfs_config *c);
#ifdef __cplusplus
}
#endif

/*===========================================================================*/
/* Module inline functions.                                                  */
/*===========================================================================*/

#endif /* LFS_WSPI_H */

/** @} */
