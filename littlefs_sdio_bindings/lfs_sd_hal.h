/**
 * @file    lfs_sd_hal.h
 * @brief   LittleFS-HAL over SDIO bindings header.
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
  uint8_t lfs_read_buffer[MMCSD_BLOCK_SIZE];
  uint8_t lfs_prog_buffer[MMCSD_BLOCK_SIZE];
  uint8_t lfs_lookahead_buffer[LFS_LOOKAHEAD_BUFFER_SIZE];
} LfsSdDriver;

typedef struct {
  uint8_t lfs_file_buffer[MMCSD_BLOCK_SIZE];
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
  int   lfsSdSetTime(lfs_t *lfs, const char *path,
		      LfsSdAttribType timeAttrType);
		      
#ifdef __cplusplus
}
#endif

/*===========================================================================*/
/* Module inline functions.                                                  */
/*===========================================================================*/

/** @} */
