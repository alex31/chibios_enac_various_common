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
 * @file    lfs_wspi.h
 * @brief   LittleFS bindings header.
 *
 * @addtogroup LITTLEFS_BINDINGS
 * @{
 */

#pragma once

#include <stdint.h>
#include <string.h>

#include "hal.h"

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

/*===========================================================================*/
/* Module macros.                                                            */
/*===========================================================================*/

/* Logging functions.*/
#define LFS_TRACE(...)
#define LFS_DEBUG(...)
#define LFS_WARN(...)
#define LFS_ERROR(...)

/* Runtime assertions.*/
#ifndef LFS_ASSERT
#define LFS_ASSERT(test) osalDbgAssert(test, "LittleFS")
#endif

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  uint32_t lfs_crc(uint32_t crc, const void *buffer, size_t size);
  void *lfs_malloc(size_t size);
  void lfs_free(void *p);
#ifdef __cplusplus
}
#endif

/*===========================================================================*/
/* Module inline functions.                                                  */
/*===========================================================================*/

static inline uint32_t lfs_max(uint32_t a, uint32_t b) {
  return (a > b) ? a : b;
}

static inline uint32_t lfs_min(uint32_t a, uint32_t b) {
  return (a < b) ? a : b;
}

static inline uint32_t lfs_aligndown(uint32_t a, uint32_t alignment) {
  return a - (a % alignment);
}

static inline uint32_t lfs_alignup(uint32_t a, uint32_t alignment) {
  return lfs_aligndown(a + alignment - 1, alignment);
}

static inline uint32_t lfs_npw2(uint32_t a) {
  return 32 - __builtin_clz(a - 1);
}

static inline uint32_t lfs_ctz(uint32_t a) {
  return __builtin_ctz(a);
}

static inline uint32_t lfs_popc(uint32_t a) {
  return __builtin_popcount(a);
}

static inline int lfs_scmp(uint32_t a, uint32_t b) {
  return (int)(unsigned)(a - b);
}

static inline uint32_t lfs_fromle32(uint32_t a) {
  return a;
}

static inline uint32_t lfs_tole32(uint32_t a) {
  return lfs_fromle32(a);
}

static inline uint32_t lfs_frombe32(uint32_t a) {
  return __builtin_bswap32(a);
}

static inline uint32_t lfs_tobe32(uint32_t a) {
  return lfs_frombe32(a);
}

/** @} */
