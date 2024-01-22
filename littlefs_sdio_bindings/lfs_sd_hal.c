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
#include "stdutil.h"
#include <string.h>


/*
  TODO : remove all static variables in the driver to make it reentrant 
  : should be able to operate multiple lfs mount in //
  need a config struct
  need a driver struct
  need a function that return lfs from driver to directlu use lfs API
  
  * optionnaly use hardware assisted CRC(ethernet) calculation
    ° H7 : DMA+CRC

 */

/*===========================================================================*/
/* Module local definitions.                                                 */
/*===========================================================================*/
#define LFS_LOOKAHEAD_BUFFER_SIZE 16U
/*===========================================================================*/
/* Module exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Module local types.                                                       */
/*===========================================================================*/
typedef struct  {
  SDCDriver *sdcd;
  size_t blk_num;
  mutex_t mtx;
  uint8_t lfs_read_buffer[MMCSD_BLOCK_SIZE];
  uint8_t lfs_prog_buffer[MMCSD_BLOCK_SIZE];
  uint8_t lfs_lookahead_buffer[LFS_LOOKAHEAD_BUFFER_SIZE];
} lfs_sd_context;

/*===========================================================================*/
/* Module local variables.                                                   */
/*===========================================================================*/
static lfs_sd_context  IN_DMA_SECTION_NOINIT(context);

static struct lfs_config lfscfg = {
    /* Link to the flash device driver.*/
  .context            = &context,
  
  /* Block device operations.*/
  .read               = __lfs_sd_read,
  .prog               = __lfs_sd_prog,
  .erase              = __lfs_sd_erase,
  .sync               = __lfs_sd_sync,
  .lock               = __lfs_sd_lock,
  .unlock             = __lfs_sd_unlock,
  
  /* Block device configuration.*/
  .read_size          = MMCSD_BLOCK_SIZE,
  .prog_size          = MMCSD_BLOCK_SIZE,
  .block_size         = MMCSD_BLOCK_SIZE,
  .block_count        = 0, // dynamically set from SD capacity
  .block_cycles       = -1, // block device take care of wear leveling
  .cache_size         = MMCSD_BLOCK_SIZE,
  .lookahead_size     = LFS_LOOKAHEAD_BUFFER_SIZE,
  .read_buffer        = context.lfs_read_buffer,
  .prog_buffer        = context.lfs_prog_buffer,
  .lookahead_buffer   = context.lfs_lookahead_buffer,
  .name_max           = 0,
  .file_max           = 0,
  .attr_max           = 0,
  .metadata_max       = 0
};

static lfs_t lfs;

/*===========================================================================*/
/* Module local functions.                                                   */
/*===========================================================================*/
static bool sdio_start(lfs_sd_context  *ctx);
/*===========================================================================*/
/* Module exported functions.                                                */
/*===========================================================================*/

lfs_t* lfs_sd_init(SDCDriver* sdcd, lfs_sd_init_t flags)  
{
  context.sdcd = sdcd;
  chMtxObjectInit(&context.mtx);
  int err;
  
  if (!sdio_start(&context))
    return nullptr;

  lfscfg.block_count = context.blk_num;
  if (flags & LFS_SD_FORMAT) {
    err = lfs_format(&lfs, &lfscfg);
    if (err < 0) {
      return nullptr;
    }
  }

  err = lfs_mount(&lfs, &lfscfg);
  if (err < 0) {
    return nullptr;
  }
  
  return &lfs;
}

void   lfs_sd_file_cache_init(lfs_sd_file_cache_t * const cache,
			      struct lfs_file_config *lfs_file_cfg)
{
  lfs_file_cfg->buffer = cache->lfs_file_buffer;
  lfs_file_cfg->attrs = nullptr;
  lfs_file_cfg->attr_count = 0;
}

uint32_t lfs_crc(uint32_t crc, const void *buffer, size_t size)
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
  chDbgAssert(size == MMCSD_BLOCK_SIZE, "size should be MMCSD_BLOCK_SIZE");
  lfs_sd_context *ctx = (lfs_sd_context *) c->context;
  SDCDriver *sdcd = ctx->sdcd;

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
  chDbgAssert(size == MMCSD_BLOCK_SIZE, "size should be MMCSD_BLOCK_SIZE");

  lfs_sd_context *ctx = (lfs_sd_context *) c->context;
  SDCDriver *sdcd = ctx->sdcd;

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
  lfs_sd_context *ctx = (lfs_sd_context *) c->context;
  SDCDriver * const sdcd = ctx->sdcd;
  const bool sdcStatus = sdcSync(sdcd);
  return sdcStatus == HAL_SUCCESS ? LFS_ERR_OK : LFS_ERR_IO;
}

int __lfs_sd_lock(const struct lfs_config *c)
{
  lfs_sd_context *ctx = (lfs_sd_context *) c->context;

  chMtxLock(&ctx->mtx);

  return 0;
}

int __lfs_sd_unlock(const struct lfs_config *c)
{
  lfs_sd_context *ctx = (lfs_sd_context *) c->context;

  chMtxUnlock(&ctx->mtx);

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

static bool sdio_start(lfs_sd_context *ctx)
{
  if  (!sdc_lld_is_card_inserted (nullptr))
    return  false;
  
  if (!sdioIsConnected())
    if (sdioConnect() == FALSE)
      return  false;
  
  BlockDeviceInfo bdi = {};
  if (blkGetInfo(ctx->sdcd, &bdi)) {
    return false;
  }
  ctx->blk_num = bdi.blk_num;

  chDbgAssert(bdi.blk_size == MMCSD_BLOCK_SIZE,
	      "device reported non compatible block size");
  return true;
}

/** @} */
