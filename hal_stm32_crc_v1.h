#pragma once
#include "ch.h"
#include "hal.h"

#if !STM32_HAS_CRC
#error "Hardware CRC not present in the selected device"
#endif

#if !STM32_CRC_PROGRAMMABLE
#error "Hardware CRC in the selected device is not programmable"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

typedef enum {
  CRC_UNINIT,                /* Not initialized.                           */
  CRC_STOP,                  /* Stopped.                                   */
  CRC_READY,                 /* Ready.                                     */
  CRC_ACTIVE,                /* Calculating CRC.                           */
  CRC_COMPLETE               /* Asynchronous operation complete.           */
} crcstate_t;

/**
 * @brief   Type of a structure representing an CRC driver.
 */
typedef struct CRCDriver CRCDriver;

/**
 * @brief   Driver configuration structure.
 */
typedef struct {
  /**
   * @brief The size of polynomial to be used for CRC.
   */
  uint32_t                 poly_size;
  /**
   * @brief The coefficients of the polynomial to be used for CRC.
   */
  uint32_t                 poly;
  /**
   * @brief The inital value
   */
  uint32_t                 initial_val;
  /**
   * @brief The final XOR value
   */
  uint32_t                 final_val;
  /**
   * @brief Reflect bit order data going into CRC
   */
  bool                     reflect_data;
  /**
   * @brief Reflect bit order of final remainder
   */
  bool                     reflect_remainder;
  /* End of the mandatory fields.*/
} CRCConfig;


/**
 * @brief   Structure representing an CRC driver.
 */
struct CRCDriver {
  /**
   * @brief Driver state.
   */
  crcstate_t                state;
  /**
   * @brief Current configuration data.
   */
  const CRCConfig           *config;
  /**
   * @brief   Mutex protecting the peripheral.
   */
  mutex_t                   mutex;

  /**
   * @brief Pointer to the CRCx registers block.
   */
};


/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

extern CRCDriver CRCD1;

#ifdef __cplusplus
extern "C" {
#endif

  void crcInit(void);
  void crcObjectInit(CRCDriver *crcp);
  void crcAcquireUnit(CRCDriver *crcp);
  void crcReleaseUnit(CRCDriver *crcp);
  void crcStart(CRCDriver *crcp, const CRCConfig *config);
  void crcStop(CRCDriver *crcp);
  void crcReset(CRCDriver *crcp);
  void crcResetI(CRCDriver *crcp);
  void crcSetInitialValue(CRCDriver *crcp, uint32_t init);
  void crcSetInitialValueI(CRCDriver *crcp, uint32_t init);
  uint32_t crcGetFinalValue(CRCDriver *crcp);
  uint32_t crcGetFinalValueI(CRCDriver *crcp);
  uint32_t crcCalc(CRCDriver *crcp, const void *buf, size_t n);
  uint32_t crcCalcI(CRCDriver *crcp, const void *buf, size_t n);
  
  void crc_lld_init(void);
  void crc_lld_start(CRCDriver *crcp);
  void crc_lld_stop(CRCDriver *crcp);
  void crc_lld_reset(void);
  uint32_t crc_lld_calc(CRCDriver *crcp, const void *buf, size_t n);
#ifdef __cplusplus
}
#endif


/** @} */
