/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

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
 * @file    lfs_hal.c
 * @brief   LittleFS-HAL bindings code.
 *
 * @addtogroup LITTLEFS_BINDINGS
 * @{
 */



#include "lfs_sd_hal.h"
#include "sdio.h"
//#include "stdutil.h"
#include <string.h>
#if STM32_CRC_PROGRAMMABLE
#include "hal_stm32_crc_v1.h"
#endif
/*
  TODO :

  * compiletime option to use hardware assisted CRC(ethernet) calculation
    ° pb si il y a 2 FS : partage du CRCDMA

  * tester la taille des caches != MMCSD_BLOCK_SIZE

  * M2M DMA with DMAMUX enabled MCU ?

  */

/*===========================================================================*/
/* Module local definitions.                                                 */
/*===========================================================================*/

int __lfs_sd_read(const struct lfs_config *c, lfs_block_t block,
		  lfs_off_t off, void *buffer, lfs_size_t size);
int __lfs_sd_prog(const struct lfs_config *c, lfs_block_t block,
		  lfs_off_t off, const void *buffer, lfs_size_t size);
int __lfs_sd_erase(const struct lfs_config *c, lfs_block_t block);
int __lfs_sd_sync(const struct lfs_config *c);
int __lfs_sd_lock(const struct lfs_config *c);
int __lfs_sd_unlock(const struct lfs_config *c);

/*===========================================================================*/
/* Module exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Module local types.                                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Module local variables.                                                   */
/*===========================================================================*/

static const struct lfs_config lfs_cfg_skl = {
    /* Link to the flash device driver.*/
  .context            = nullptr,
  
  /* Block device operations.*/
  .read               = __lfs_sd_read,
  .prog               = __lfs_sd_prog,
  .erase              = __lfs_sd_erase,
  .sync               = __lfs_sd_sync,
  .lock               = __lfs_sd_lock,
  .unlock             = __lfs_sd_unlock,
  
  /* Block device configuration.*/
  .read_size          = LFS_CACHES_SIZE,
  .prog_size          = LFS_CACHES_SIZE,
  .block_size         = MMCSD_BLOCK_SIZE,
  .block_count        = 0,  // dynamically set from SD capacity
  .block_cycles       = -1, // block device take care of wear leveling
  .cache_size         = LFS_CACHES_SIZE,
  .lookahead_size     = LFS_LOOKAHEAD_BUFFER_SIZE,
  .read_buffer        = nullptr, // dynamically set from LfsSdDriver
  .prog_buffer        = nullptr, // dynamically set from LfsSdDriver
  .lookahead_buffer   = nullptr, // dynamically set from LfsSdDriver
  .name_max           = 0,
  .file_max           = 0,
  .attr_max           = 0,
  .metadata_max       = 0
};



/*===========================================================================*/
/* Module local functions.                                                   */
/*===========================================================================*/
static bool sdio_start(LfsSdDriver  *lfsSdd);
#if STM32_CRC_PROGRAMMABLE
static void init_crc(void);
#endif
/*===========================================================================*/
/* Module exported functions.                                                */
/*===========================================================================*/

lfs_t* lfsSdStart(LfsSdDriver *lfsSdd, const LfsSdConfig *cfg)
{
  lfsSdd->cfg = cfg;
  lfsSdd->lfs_cfg = lfs_cfg_skl;
  chMtxObjectInit(&lfsSdd->mtx);
  int err;
  
#if STM32_CRC_PROGRAMMABLE
  init_crc();
#endif
  
  if (!sdio_start(lfsSdd))
    return nullptr;
  lfsSdd->lfs_cfg.block_count = lfsSdd->blk_num;
  lfsSdd->lfs_cfg.context = lfsSdd;
  lfsSdd->lfs_cfg.read_buffer = lfsSdd->lfs_read_buffer;
  lfsSdd->lfs_cfg.prog_buffer = lfsSdd->lfs_prog_buffer;
  lfsSdd->lfs_cfg.lookahead_buffer = lfsSdd->lfs_lookahead_buffer;
  
  if (cfg->formatDevice == true) {
    err = lfs_format(&lfsSdd->lfs, &lfsSdd->lfs_cfg);
    if (err < 0) {
      return nullptr;
    }
  }

  err = lfs_mount(&lfsSdd->lfs, &lfsSdd->lfs_cfg);
  if (err < 0) {
    return nullptr;
  }
  
  return &lfsSdd->lfs;
}

void   lfsSdFileCacheInit(LfsSdFileCache *cache,
			  struct lfs_file_config *lfs_file_cfg)
{
  lfs_file_cfg->buffer = cache->lfs_file_buffer;
  lfs_file_cfg->attrs = nullptr;
  lfs_file_cfg->attr_count = 0;
}

int   lfsSdSetTime(lfs_t*lfs, const char *path,
		      LfsSdAttribType timeAttrType)
{
  struct tm tim;
  RTCDateTime timespec;
  
  rtcGetTime(&RTCD1, &timespec);
  rtcConvertDateTimeToStructTm(&timespec, &tim, NULL);
  const time_t creationTime = mktime(&tim);
  return lfs_setattr(lfs, path, timeAttrType, &creationTime,
		     sizeof(creationTime));
}

#if STM32_CRC_PROGRAMMABLE
uint32_t lfs_crc(uint32_t crc, const void *buffer, size_t size)
{
  if (size == 0)
    return crc;

  //  if (size > 8)
  //    DebugTrace("crc size = %u", size);
  crcAcquireUnit(&CRCD1);
  crcSetInitialValue(&CRCD1, crc);
  crcReset(&CRCD1);
  
  const uint32_t rcrc = crcCalc(&CRCD1, buffer, size);
  crcReleaseUnit(&CRCD1);
  return rcrc;
}
#else
uint32_t lfs_crc(uint32_t crc, const void * const buffer, const size_t size)
{
  static const uint32_t rtable[16] = {0x00000000, 0x1db71064, 0x3b6e20c8,
                                      0x26d930ac, 0x76dc4190, 0x6b6b51f4,
                                      0x4db26158, 0x5005713c, 0xedb88320,
                                      0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
                                      0x9b64c2b0, 0x86d3d2d4, 0xa00ae278,
                                      0xbdbdf21c, };
  const uint8_t *data = buffer;
  
  for (size_t i = 0; i < size; i++) {
    crc = (crc >> 4) ^ rtable[(crc ^ (data[i] >> 0)) & 0xf];
    crc = (crc >> 4) ^ rtable[(crc ^ (data[i] >> 4)) & 0xf];
  }

  return crc;
}
#endif

void *lfs_malloc(size_t)
{ 
  chSysHalt("malloc");
  return nullptr;
}

void lfs_free(void *)
{
  chSysHalt("free");
}
/*
  POSSIBLE STATUS VALUE returned to LITTLEFS:
    LFS_ERR_OK    
    LFS_ERR_IO    
    LFS_ERR_NOMEM 

  POSSIBLE RETURN VALUE from SDC BLK API
   HAL_SUCCESS
   HAL_FAILED 
======
int __lfs_read_flash(const struct lfs_config *c, lfs_block_t block,
               lfs_off_t off, void *buffer, lfs_size_t size) {
  BaseFlash *flp = (BaseFlash *)c->context;
  flash_error_t err;

  err = flashRead(flp,
                  (flash_offset_t)(block * c->block_size) + (flash_offset_t)off,
                  (size_t)size,
                  (uint8_t *)buffer);
  if (err != FLASH_NO_ERROR) {
    return LFS_ERR_IO;
  }

  return 0;
}
======
*/

int __lfs_sd_read(const struct lfs_config *c, lfs_block_t block,
               lfs_off_t offset, void *buffer, lfs_size_t size)
{
  chDbgAssert(offset == 0, "offset should be 0");
  chDbgAssert(size % MMCSD_BLOCK_SIZE == 0, "size should be multiple of MMCSD_BLOCK_SIZE");
  LfsSdDriver *lfsSdd = (LfsSdDriver *) c->context;
  SDCDriver *sdcd = lfsSdd->cfg->sdcd;

  if (!sdioIsConnected())
      return  LFS_ERR_IO;
  /*
    block : block index
    offset : offset in the begining block
    size : size in byte
   */

  const bool sdcStatus = sdcRead(sdcd, block, buffer, size / MMCSD_BLOCK_SIZE);
  return sdcStatus == HAL_SUCCESS ? LFS_ERR_OK : LFS_ERR_IO;
}

int __lfs_sd_prog(const struct lfs_config *c, lfs_block_t block,
               lfs_off_t offset, const void *buffer, lfs_size_t size)
{
  chDbgAssert(offset == 0, "offset should be 0");
  chDbgAssert(size % MMCSD_BLOCK_SIZE == 0, "size should be multiple of MMCSD_BLOCK_SIZE");
  LfsSdDriver *lfsSdd = (LfsSdDriver *) c->context;
  SDCDriver *sdcd = lfsSdd->cfg->sdcd;

  if (!sdioIsConnected())
      return  LFS_ERR_IO;
  /*
    block : block index
    offset : offset in the begining block
    size : size in byte
   */

  const bool sdcStatus = sdcWrite(sdcd, block, buffer, size / MMCSD_BLOCK_SIZE);
  return sdcStatus == HAL_SUCCESS ? LFS_ERR_OK : LFS_ERR_IO;
}

/* int __lfs_sd_erase(const struct lfs_config *c, lfs_block_t block) */
/* { */
/*   lfs_sd_context *ctx = (lfs_sd_context *) c->context; */
/*   SDCDriver * const sdcd = ctx->sdcd; */
/*   const bool sdcStatus = sdcErase(sdcd, block, block); */
/*   return sdcStatus == HAL_SUCCESS ? LFS_ERR_OK : LFS_ERR_IO; */
/* } */

/*
  block device should take care of erasing flash
 */
int __lfs_sd_erase(const struct lfs_config *, lfs_block_t )
{
  return LFS_ERR_OK;
}

int __lfs_sd_sync(const struct lfs_config *c)
{
  LfsSdDriver *lfsSdd = (LfsSdDriver *) c->context;
  SDCDriver *sdcd = lfsSdd->cfg->sdcd;
  const bool sdcStatus = sdcSync(sdcd);
  return sdcStatus == HAL_SUCCESS ? LFS_ERR_OK : LFS_ERR_IO;
}

int __lfs_sd_lock(const struct lfs_config *c)
{
  LfsSdDriver *lfsSdd = (LfsSdDriver *) c->context;
  chMtxLock(&lfsSdd->mtx);
  return 0;
}

int __lfs_sd_unlock(const struct lfs_config *c)
{
  LfsSdDriver *lfsSdd = (LfsSdDriver *) c->context;
  chMtxUnlock(&lfsSdd->mtx);
  return 0;
}
/*
#                 _ __           _                    _                   
#                | '_ \         (_)                  | |                  
#                | |_) |  _ __   _   __   __   __ _  | |_     ___         
#                | .__/  | '__| | |  \ \ / /  / _` | | __|   / _ \        
#                | |     | |    | |   \ V /  | (_| | \ |_   |  __/        
#                |_|     |_|    |_|    \_/    \__,_|  \__|   \___|        
*/

static bool sdio_start(LfsSdDriver  *lfsSdd)
{
  if  (!sdc_lld_is_card_inserted (nullptr))
    return  false;
  
  if (!sdioIsConnected())
    if (sdioConnect() == FALSE)
      return  false;
  
  BlockDeviceInfo bdi = {};
  if (blkGetInfo(lfsSdd->cfg->sdcd, &bdi)) {
    return false;
  }
  lfsSdd->blk_num = bdi.blk_num;

  chDbgAssert(bdi.blk_size == MMCSD_BLOCK_SIZE,
	      "device reported non compatible block size");
  return true;
}

#if STM32_CRC_PROGRAMMABLE
static void init_crc(void)
{
  if (CRCD1.state != CRC_READY) {
    crcInit();
    crcObjectInit(&CRCD1);
    crcStart(&CRCD1, nullptr);
  }
}
#endif

/** @} */
