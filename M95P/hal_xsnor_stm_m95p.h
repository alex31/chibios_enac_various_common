
/**
 * @file        hal_xsnor_device_m95p.h
 *
 * @addtogroup  HAL_XSNOR_DEVICE_M95P
 * @brief       SNOR device m95p driver.
 * @details     implemented manually from on cloned template files.
 * @{
 */

#pragma once
#include "oop_base_object.h"
#include "hal_xsnor_base.h"
#include "eeprom_stm_m95p.h"

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
/* Module macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* Module data structures and types.                                         */
/*===========================================================================*/

/**
 * @class       hal_xsnor_stm_m95p_c
 * @extends     base_object_c, hal_xsnor_base_c.
 *
 *
 * @name        Class @p hal_xsnor_stm_m95p_c structures
 * @{
 */

/**
 * @brief       Type of a SNOR device m95p driver class.
 */
typedef struct hal_xsnor_stm_m95p hal_xsnor_stm_m95p_c;

/**
 * @brief       Class @p hal_xsnor_stm_m95p_c virtual methods table.
 */
struct hal_xsnor_stm_m95p_vmt {
  /* From base_object_c.*/
  void (*dispose)(void *ip);
  /* From hal_xsnor_base_c.*/
  flash_error_t (*init)(void *ip);
  flash_error_t (*read)(void *ip, flash_offset_t offset, size_t n, uint8_t *rp);
  flash_error_t (*program)(void *ip, flash_offset_t offset, size_t n, const uint8_t *pp);
  flash_error_t (*start_erase_all)(void *ip);
  flash_error_t (*start_erase_sector)(void *ip, flash_sector_t sector);
  flash_error_t (*query_erase)(void *ip, unsigned *msec);
  flash_error_t (*verify_erase)(void *ip, flash_sector_t sector);
  flash_error_t (*mmap_on)(void *ip, uint8_t **addrp);
  void (*mmap_off)(void *ip);
  /* From hal_xsnor_stm_m95p_c.*/
};

/**
 * @brief       Structure representing a SNOR device m95p driver class.
 */
struct hal_xsnor_stm_m95p {
  /**
   * @brief       Virtual Methods Table.
   */
  const struct hal_xsnor_stm_m95p_vmt *vmt;
  /**
   * @brief       Implemented interface @p flash_interface_i.
   */
  flash_interface_i         fls;
  /**
   * @brief       Driver state.
   */
  flash_state_t             state;
  /**
   * @brief       Driver configuration.
   */
  const xsnor_config_t      *config;
  /**
   * @brief       Flash access mutex.
   */
  mutex_t                   mutex;
  /**
   * @brief       Flash descriptor.
   * @note        This field is meant to be initialized by subclasses on memory
   *              initialization.
   */
  flash_descriptor_t        descriptor;
  M95P_instance_t	    device;
};
/** @} */

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  /* Methods of hal_xsnor_stm_m95p_c.*/
  void *__m95p_objinit_impl(void *ip, const void *vmt);
  void __m95p_dispose_impl(void *ip);
  flash_error_t __m95p_init_impl(void *ip);
  flash_error_t __m95p_read_impl(void *ip, flash_offset_t offset, size_t n,
                                 uint8_t *rp);
  flash_error_t __m95p_program_impl(void *ip, flash_offset_t offset, size_t n,
                                    const uint8_t *pp);
  flash_error_t __m95p_start_erase_all_impl(void *ip);
  flash_error_t __m95p_start_erase_sector_impl(void *ip, flash_sector_t sector);
  flash_error_t __m95p_query_erase_impl(void *ip, unsigned *msec);
  flash_error_t __m95p_verify_erase_impl(void *ip, flash_sector_t sector);
  flash_error_t __m95p_mmap_on_impl(void *ip, uint8_t **addrp);
  void __m95p_mmap_off_impl(void *ip);
  /* Regular functions.*/
#ifdef __cplusplus
}
#endif

/*===========================================================================*/
/* Module inline functions.                                                  */
/*===========================================================================*/

/**
 * @name        Default constructor of hal_xsnor_stm_m95p.h
 * @{
 */
/**
 * @memberof    hal_xsnor_stm_m95p_c
 *
 * @brief       Default initialization function of @p hal_xsnor_stm_m95p_c.
 *
 * @param[out]    self          Pointer to a @p hal_xsnor_stm_m95p_c instance
 *                              to be initialized.
 * @return                      Pointer to the initialized object.
 *
 * @objinit
 */
CC_FORCE_INLINE
static inline hal_xsnor_stm_m95p_c *m95pObjectInit(hal_xsnor_stm_m95p_c *self) {
  extern const struct hal_xsnor_stm_m95p_vmt __hal_xsnor_stm_m95p_vmt;

  return __m95p_objinit_impl(self, &__hal_xsnor_stm_m95p_vmt);
}
/** @} */


/** @} */
