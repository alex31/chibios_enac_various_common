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

/*===========================================================================*/
/* Module local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Module exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Module local types.                                                       */
/*===========================================================================*/
typedef struct  {
  SDCDriver *sdcd;
  mutex_t mtx;
  uint8_t lfs_read_buffer[MMCSD_BLOCK_SIZE];
  uint8_t lfs_prog_buffer[MMCSD_BLOCK_SIZE];
  uint8_t lfs_lookahead_buffer[MMCSD_BLOCK_SIZE];
} lfs_sd_context;

/*===========================================================================*/
/* Module local variables.                                                   */
/*===========================================================================*/
static lfs_sd_context context;

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
  .block_count        = 4096,//4 * (2 * 1024 * 1024),
  .block_cycles       = -1,
  .cache_size         = MMCSD_BLOCK_SIZE,
  .lookahead_size     = MMCSD_BLOCK_SIZE,
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
static bool sdio_start();
/*===========================================================================*/
/* Module exported functions.                                                */
/*===========================================================================*/

lfs_t* lfs_sd_init(SDCDriver* sdcd)  
{
  context.sdcd = sdcd;
  chMtxObjectInit(&context.mtx);
  if (!sdio_start())
    return nullptr;
  int err = lfs_mount(&lfs, &lfscfg);
  if (err < 0) {
    err = lfs_format(&lfs, &lfscfg);
    if (err < 0) {
      return nullptr;
    }
    err = lfs_mount(&lfs, &lfscfg);
    if (err < 0) {
      return nullptr;
    }
  }
  
  return &lfs;
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

void *lfs_malloc(size_t s)
{
  return malloc_m(s);
}

void lfs_free(void *p)
{
  free_m(p);
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
static uint8_t wpBuffer[MMCSD_BLOCK_SIZE];

int __lfs_sd_read(const struct lfs_config *c, lfs_block_t block,
               lfs_off_t offset, void *buffer, lfs_size_t size)
{
  chDbgAssert(offset < MMCSD_BLOCK_SIZE, "offset should be < MMCSD_BLOCK_SIZE");
  lfs_sd_context *ctx = (lfs_sd_context *) c->context;
  SDCDriver *sdcd = ctx->sdcd;

  if (!sdioIsConnected())
      return  LFS_ERR_IO;
  /*
    block : block index
    offset : offset in the begining block
    size : size in byte
   */

  // simple case : all is aligned to blocks
  if ((offset == 0) && ((size % MMCSD_BLOCK_SIZE)) == 0) {
    const bool sdcStatus = sdcRead(sdcd, block, buffer, size / MMCSD_BLOCK_SIZE);
    return sdcStatus == HAL_SUCCESS ? LFS_ERR_OK : LFS_ERR_IO;
  }

  // other case, either offset and/or size is not multiple of block size
  // we will keep size and curBuffer moving along
  uint8_t *curBuffer = buffer;

  // first block is partial : copy block to working buffer
  // then copy what is ask to return buffer
  bool sdcStatus = sdcRead(sdcd, block, wpBuffer, 1);
  if (sdcStatus != HAL_SUCCESS) return  LFS_ERR_IO;
  memcpy(curBuffer, &wpBuffer[offset], MMCSD_BLOCK_SIZE - offset);
  curBuffer += offset;
  size -= (MMCSD_BLOCK_SIZE-offset);
  block++;

  // all middle complete blocks if there is
  const int32_t nbFullBlocsLeft = size / MMCSD_BLOCK_SIZE;
  if (nbFullBlocsLeft) {
     sdcStatus = sdcRead(sdcd, block, curBuffer, nbFullBlocsLeft);
     if (sdcStatus != HAL_SUCCESS) return  LFS_ERR_IO;
     curBuffer += (nbFullBlocsLeft * MMCSD_BLOCK_SIZE);
     size -= (nbFullBlocsLeft * MMCSD_BLOCK_SIZE);
     block += nbFullBlocsLeft;
  }
  
  // last block is partial : copy block to working buffer
  // then copy what is ask to return buffer
   sdcStatus = sdcRead(sdcd, block, wpBuffer, 1);
   if (sdcStatus != HAL_SUCCESS) return  LFS_ERR_IO;
   memcpy(curBuffer, wpBuffer, size);
  return LFS_ERR_OK;
}

int __lfs_sd_prog(const struct lfs_config *c, lfs_block_t block,
               lfs_off_t offset, const void *buffer, lfs_size_t size)
{
  chDbgAssert(offset < MMCSD_BLOCK_SIZE, "offset should be < MMCSD_BLOCK_SIZE");
  lfs_sd_context *ctx = (lfs_sd_context *) c->context;
  SDCDriver * const sdcd = ctx->sdcd;

  if (!sdioIsConnected())
      return  LFS_ERR_IO;
  /*
    block : block index
    offset : offset in the begining block
    size : size in byte
   */

  // simple case : all is aligned to blocks
  if ((offset == 0) && ((size % MMCSD_BLOCK_SIZE)) == 0) {
    const bool sdcStatus = sdcWrite(sdcd, block, buffer, size / MMCSD_BLOCK_SIZE);
    return sdcStatus == HAL_SUCCESS ? LFS_ERR_OK : LFS_ERR_IO;
  }

  // other case, either offset and/or size is not multiple of block size
  // we will keep size and curBuffer moving along
  const uint8_t *curBuffer = buffer;

  // first block is partial : read block in working buffer
  // then modify working buffer with beginning of buffer
  // then write block
  bool sdcStatus = sdcRead(sdcd, block, wpBuffer, 1);
  if (sdcStatus != HAL_SUCCESS) return  LFS_ERR_IO;
  memcpy(&wpBuffer[offset], curBuffer, MMCSD_BLOCK_SIZE - offset);
  sdcStatus = sdcWrite(sdcd, block, wpBuffer, 1);
  if (sdcStatus != HAL_SUCCESS) return  LFS_ERR_IO;
  curBuffer += offset;
  size -= (MMCSD_BLOCK_SIZE-offset);
  block++;

  // all middle complete blocks if there is
  const int32_t nbFullBlocsLeft = size / MMCSD_BLOCK_SIZE;
  if (nbFullBlocsLeft) {
     sdcStatus = sdcWrite(sdcd, block, curBuffer, nbFullBlocsLeft);
     if (sdcStatus != HAL_SUCCESS) return  LFS_ERR_IO;
     curBuffer += (nbFullBlocsLeft * MMCSD_BLOCK_SIZE);
     size -= (nbFullBlocsLeft * MMCSD_BLOCK_SIZE);
     block += nbFullBlocsLeft;
  }
  
  // last block is partial : read block in working buffer
  // then modify working buffer with end of buffer
  // then write block
  sdcStatus = sdcRead(sdcd, block, wpBuffer, 1);
  if (sdcStatus != HAL_SUCCESS) return  LFS_ERR_IO;
  memcpy(wpBuffer, curBuffer, size);
  sdcStatus = sdcWrite(sdcd, block, wpBuffer, 1);
  if (sdcStatus != HAL_SUCCESS) return  LFS_ERR_IO;
  
  return LFS_ERR_OK;
}

int __lfs_sd_erase(const struct lfs_config *c, lfs_block_t block)
{
  lfs_sd_context *ctx = (lfs_sd_context *) c->context;
  SDCDriver * const sdcd = ctx->sdcd;
  const bool sdcStatus = sdcErase(sdcd, block, 1);
  return sdcStatus == HAL_SUCCESS ? LFS_ERR_OK : LFS_ERR_IO;
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

static bool sdio_start()
{
  if  (!sdc_lld_is_card_inserted (nullptr))
    return  false;
  
  if (!sdioIsConnected())
    if (sdioConnect() == FALSE)
      return  false;
  
  
  return true;
}

/** @} */
