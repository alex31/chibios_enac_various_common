/**
 * @file    lfs_sd_hal.h
 * @brief   LittleFS-HAL over SD bindings header.
 *
 * @addtogroup LITTLEFS_BINDINGS
 * @{
 */

#pragma once

#include "hal.h"
#include "lfs.h"

/*===========================================================================*/
/* Module constants.                                                         */
/*===========================================================================*/
#ifndef  LFS_LOOKAHEAD_BUFFER_SIZE
#define LFS_LOOKAHEAD_BUFFER_SIZE 16U
#endif

// if redefined, must me a multiple of MMCSD_BLOCK_SIZE
#ifndef  LFS_CACHES_SIZE
#define  LFS_CACHES_SIZE MMCSD_BLOCK_SIZE
#endif

#if (LFS_CACHES_SIZE % MMCSD_BLOCK_SIZE)
#error "LFS_CACHES_SIZE must be a multiple of MMCSD_BLOCK_SIZE"
#endif

/*===========================================================================*/
/* Module pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Module data structures and types.                                         */
/*===========================================================================*/
typedef enum {LFS_ATTR_CTIME = 1U,
  LFS_ATTR_MTIME} LfsSdAttribType;

typedef struct {
  SDCDriver *sdcd;
  bool	     formatDevice;
} LfsSdConfig;

typedef struct {
  const LfsSdConfig *cfg;
  lfs_t lfs;
  size_t blk_num;
  mutex_t mtx;
  struct lfs_config lfs_cfg;
  uint8_t lfs_read_buffer[LFS_CACHES_SIZE];
  uint8_t lfs_prog_buffer[LFS_CACHES_SIZE];
  uint8_t lfs_lookahead_buffer[LFS_LOOKAHEAD_BUFFER_SIZE];
} LfsSdDriver;

typedef struct {
  //  time_t creationTime;
  //  struct lfs_attr lfsAttr;
  uint8_t lfs_file_buffer[LFS_CACHES_SIZE];
} LfsSdFileCache;
/*===========================================================================*/
/* Module macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  lfs_t* lfsSdStart(LfsSdDriver *lfsSdd, const LfsSdConfig *cfg);
  void   lfsSdFileCacheInit(LfsSdFileCache *cache,
			    struct lfs_file_config *lfs_file_cfg);
  int   lfsSdSetTime(lfs_t*lfs, const char *path,
		      LfsSdAttribType timeAttrType);
		      
#ifdef __cplusplus
}
#endif

/*===========================================================================*/
/* Module inline functions.                                                  */
/*===========================================================================*/

/** @} */
