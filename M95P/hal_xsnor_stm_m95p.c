/*
    ChibiOS - Copyright (C) 2006..2024 Giovanni Di Sirio

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
 * @file        hal_xsnor_stm_m95p.c
 * @brief       implemented by filling cloned template files
 *
 * @addtogroup  HAL_XSNOR_DEVICE_M95P
 * @{
 */

#include "hal.h"
#include "hal_xsnor_stm_m95p.h"

/*===========================================================================*/
/* Module local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Module local macros.                                                      */
/*===========================================================================*/

/*===========================================================================*/
/* Module exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Module local types.                                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Module local variables.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Module local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Module exported functions.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Module class "hal_xsnor_stm_m95p_c" methods.                             */
/*===========================================================================*/

/**
 * @name        Methods implementations of hal_xsnor_stm_m95p_c
 * @{
 */
/**
 * @memberof    hal_xsnor_stm_m95p_c
 * @protected
 *
 * @brief       Implementation of object creation.
 * @note        This function is meant to be used by derived classes.
 *
 * @param[out]    ip            Pointer to a @p hal_xsnor_stm_m95p_c instance
 *                              to be initialized.
 * @param[in]     vmt           VMT pointer for the new object.
 * @return                      A new reference to the object.
 */
void *__m95p_objinit_impl(void *ip, const void *vmt) {
  hal_xsnor_stm_m95p_c *self = (hal_xsnor_stm_m95p_c *)ip;

  /* Initialization of the ancestors-defined parts.*/
  __xsnor_objinit_impl(self, vmt);
 /* Initialization code.*/
  self->descriptor.attributes    = FLASH_ATTR_ERASED_IS_ONE |
                                   FLASH_ATTR_REWRITABLE;
  self->descriptor.page_size     = 512U;
  self->descriptor.sectors_count = 0U; /* Overwritten.*/
  self->descriptor.sectors       = NULL;
  self->descriptor.sectors_size  = self->descriptor.page_size;
  self->descriptor.address       = 0U;
  self->descriptor.size          = 0U; /* Overwritten.*/

  /* Initialization code.*/
  /* Implementation.*/

  return self;
}

/**
 * @memberof    hal_xsnor_stm_m95p_c
 * @protected
 *
 * @brief       Implementation of object finalization.
 * @note        This function is meant to be used by derived classes.
 *
 * @param[in,out] ip            Pointer to a @p hal_xsnor_stm_m95p_c instance
 *                              to be disposed.
 */
void __m95p_dispose_impl(void *ip) {
  hal_xsnor_stm_m95p_c *self = (hal_xsnor_stm_m95p_c *)ip;

  /* Finalization code.*/
  /* Implementation.*/

  /* Finalization of the ancestors-defined parts.*/
  __xsnor_dispose_impl(self);
}

/**
 * @memberof    hal_xsnor_stm_m95p_c
 * @protected
 *
 * @brief       Override of method @p xsnor_device_init().
 *
 * @param[in,out] ip            Pointer to a @p hal_xsnor_stm_m95p_c instance.
 * @return                      An error code.
 */
flash_error_t __m95p_init_impl(void *ip) {
  hal_xsnor_stm_m95p_c *self = (hal_xsnor_stm_m95p_c *)ip;
  MP95_Identification ident;
  /* Implementation.*/
  self->device = M95P_new(self->config->bus.spi.drv, self->config->bus.spi.cfg);
  M95P_softwareReset(self->device);
  M95P_readIdentification(self->device, &ident);
  if ((ident.manufacturer != 0x20) || (ident.family != 0x0)) {
    return FLASH_ERROR_HW_FAILURE;
  }
  //  ident.density = 0x11;
  self->descriptor.size =  1 << ident.density;
  self->descriptor.sectors_count = self->descriptor.size / self->descriptor.sectors_size;
  return FLASH_NO_ERROR;
}

/**
 * @memberof    hal_xsnor_stm_m95p_c
 * @protected
 *
 * @brief       Override of method @p xsnor_device_read().
 *
 * @param[in,out] ip            Pointer to a @p hal_xsnor_stm_m95p_c instance.
 * @param[in]     offset        Flash offset.
 * @param[in]     n             Number of bytes to be read.
 * @param[out]    rp            Pointer to the data buffer.
 * @return                      An error code.
 */
flash_error_t __m95p_read_impl(void *ip, flash_offset_t offset, size_t n,
                               uint8_t *rp) {
  hal_xsnor_stm_m95p_c *self = (hal_xsnor_stm_m95p_c *)ip;

  /* Implementation.*/
  const msg_t status = M95P_read(self->device, offset, n, rp);
  chDbgAssert(status == MSG_OK, "status failed");
  return status == MSG_OK ? FLASH_NO_ERROR : FLASH_ERROR_HW_FAILURE;
}

/**
 * @memberof    hal_xsnor_stm_m95p_c
 * @protected
 *
 * @brief       Override of method @p xsnor_device_program().
 *
 * @param[in,out] ip            Pointer to a @p hal_xsnor_stm_m95p_c instance.
 * @param[in]     offset        Flash offset.
 * @param[in]     n             Number of bytes to be programmed.
 * @param[in]     pp            Pointer to the data buffer.
 * @return                      An error code.
 */
flash_error_t __m95p_program_impl(void *ip, flash_offset_t offset, size_t n,
                                  const uint8_t *pp) {
  hal_xsnor_stm_m95p_c *self = (hal_xsnor_stm_m95p_c *)ip;

  /* Implementation.*/
  const msg_t status = M95P_write(self->device, offset, n, pp);
  chDbgAssert(status == MSG_OK, "status failed");
  return status == MSG_OK ? FLASH_NO_ERROR : FLASH_ERROR_HW_FAILURE;
}

/**
 * @memberof    hal_xsnor_stm_m95p_c
 * @protected
 *
 * @brief       Override of method @p xsnor_device_start_erase_all().
 *
 * @param[in,out] ip            Pointer to a @p hal_xsnor_stm_m95p_c instance.
 * @return                      An error code.
 */
flash_error_t __m95p_start_erase_all_impl(void *ip) {
  hal_xsnor_stm_m95p_c *self = (hal_xsnor_stm_m95p_c *)ip;

  /* Implementation.*/
  const msg_t status = M95P_erase_chip(self->device);
  chDbgAssert(status == MSG_OK, "status failed");
  return status == MSG_OK ? FLASH_NO_ERROR : FLASH_ERROR_HW_FAILURE;
}

/**
 * @memberof    hal_xsnor_stm_m95p_c
 * @protected
 *
 * @brief       Override of method @p xsnor_device_start_erase_sector().
 *
 * @param[in,out] ip            Pointer to a @p hal_xsnor_stm_m95p_c instance.
 * @param[in]     sector        Sector to be erased.
 * @return                      An error code.
 */
flash_error_t __m95p_start_erase_sector_impl(void *ip, flash_sector_t sector) {
  hal_xsnor_stm_m95p_c *self = (hal_xsnor_stm_m95p_c *)ip;

  /* Implementation.*/
  const msg_t status = M95P_erase_page(self->device, sector *
				       self->descriptor.sectors_size);
  chDbgAssert(status == MSG_OK, "status failed");
  chDbgAssert (__m95p_verify_erase_impl(ip, sector) ==  FLASH_NO_ERROR, "verify after erase failed");
  return status == MSG_OK ? FLASH_NO_ERROR : FLASH_ERROR_HW_FAILURE;
}

/**
 * @memberof    hal_xsnor_stm_m95p_c
 * @protected
 *
 * @brief       Override of method @p xsnor_device_query_erase().
 *
 * @param[in,out] ip            Pointer to a @p hal_xsnor_stm_m95p_c instance.
 * @param[out]    msec          Recommended time, in milliseconds, that should
 *                              be spent before calling this function again,
 *                              can be @p NULL
 * @return                      An error code.
 */
flash_error_t __m95p_query_erase_impl(void *, unsigned *msec) {
  // no need : underlying driver take care of buzy flag
  *msec = 0;
  return FLASH_NO_ERROR;
}

/**
 * @memberof    hal_xsnor_stm_m95p_c
 * @protected
 *
 * @brief       Override of method @p xsnor_device_verify_erase().
 *
 * @param[in,out] ip            Pointer to a @p hal_xsnor_stm_m95p_c instance.
 * @param[in]     sector        Sector to be verified.
 * @return                      An error code.
 */
flash_error_t __m95p_verify_erase_impl(void *ip, flash_sector_t sector) {
  hal_xsnor_stm_m95p_c *self = (hal_xsnor_stm_m95p_c *)ip;
  
  /* Implementation.*/
  /* Read command.*/
  size_t n = self->descriptor.sectors_size;
  flash_offset_t offset = (flash_offset_t)(sector * n);
  
  while (n > 0U) {
    uint8_t *p;
    
    __m95p_read_impl(self, offset,
		     XSNOR_BUFFER_SIZE, &self->config->buffers->databuf[0]);

    /* Checking for erased state of current buffer.*/
    for (p = &self->config->buffers->databuf[0];
         p < &self->config->buffers->databuf[XSNOR_BUFFER_SIZE];
         p++) {

      if (*p != 0xFFU) {
        return FLASH_ERROR_VERIFY;
      }
    }

    offset += XSNOR_BUFFER_SIZE;
    n -= XSNOR_BUFFER_SIZE;
  }

  return FLASH_NO_ERROR;
}

/**
 * @memberof    hal_xsnor_stm_m95p_c
 * @protected
 *
 * @brief       Override of method @p xsnor_device_mmap_on().
 *
 * @param[in,out] ip            Pointer to a @p hal_xsnor_stm_m95p_c instance.
 * @param[out]    addrp         Pointer to the memory mapped memory or @p NULL
 * @return                      An error code.
 */
flash_error_t __m95p_mmap_on_impl(void *ip, uint8_t **addrp) {
  hal_xsnor_stm_m95p_c *self = (hal_xsnor_stm_m95p_c *)ip;

  /* Implementation.*/
  (void)self;
  (void)addrp;

   return FLASH_ERROR_UNIMPLEMENTED;
}

/**
 * @memberof    hal_xsnor_stm_m95p_c
 * @protected
 *
 * @brief       Override of method @p xsnor_device_mmap_off().
 *
 * @param[in,out] ip            Pointer to a @p hal_xsnor_stm_m95p_c instance.
 */
void __m95p_mmap_off_impl(void *ip) {
  hal_xsnor_stm_m95p_c *self = (hal_xsnor_stm_m95p_c *)ip;

  /* Implementation.*/
  (void)self;
}
/** @} */

/**
 * @brief       VMT structure of SNOR device M95P driver class.
 * @note        It is public because accessed by the inlined constructor.
 */
const struct hal_xsnor_stm_m95p_vmt __hal_xsnor_stm_m95p_vmt = {
  .dispose                  = __m95p_dispose_impl,
  .init                     = __m95p_init_impl,
  .read                     = __m95p_read_impl,
  .program                  = __m95p_program_impl,
  .start_erase_all          = __m95p_start_erase_all_impl,
  .start_erase_sector       = __m95p_start_erase_sector_impl,
  .query_erase              = __m95p_query_erase_impl,
  .verify_erase             = __m95p_verify_erase_impl,
  .mmap_on                  = __m95p_mmap_on_impl,
  .mmap_off                 = __m95p_mmap_off_impl
};

/** @} */
