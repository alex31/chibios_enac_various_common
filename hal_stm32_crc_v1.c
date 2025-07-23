#include "hal_stm32_crc_v1.h"
#include "stdutil.h"
/*
  TODO : 
  COMPILE TIME OPTIONAL DMA API
 */

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/**
 * @brief   CRC default configuration for ETHERNET
 * @notes   This configuration is the same as fixed configuration of
 *          not programmable CRC peripheral of F1/F2/F4/L1 series
 */
static const CRCConfig default_config = {
  .poly_size         = 32,
  .poly              = 0x04C11DB7,
  .initial_val       = 0xFFFFFFFF,
  .final_val         = 0x00000000,
  .reflect_data      = true,
  .reflect_remainder = true
};

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/** @brief CRC1 driver identifier.*/
CRCDriver CRCD1;

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

void _crc_lld_calc_byte(uint8_t data) {
  static  __IO uint8_t * const crc8 = (__IO uint8_t*)&(CRC->DR);
  *crc8 = data;
}

/*
 * @brief   Returns calculated CRC from last reset
 *
 * @param[in] crcp      pointer to the @p CRCDriver object
 * @param[in] data      data to be added to crc
 *
 * @notapi
 */
void _crc_lld_calc_halfword(uint16_t data) {
  static  __IO uint16_t * const crc16 = (__IO uint16_t*)&(CRC->DR);
  *crc16 = data;
}

/*
 * @brief   Returns calculated CRC from last reset
 *
 * @param[in] crcp      pointer to the @p CRCDriver object
 * @param[in] data      data to be added to crc
 *
 * @notapi
 */
void _crc_lld_calc_word(uint32_t data) {
  CRC->DR = data;
}



/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level CRC driver initialization.
 *
 * @notapi
 */
void crc_lld_init(void) {
  crcObjectInit(&CRCD1);
}

/**
 * @brief   Configures and activates the CRC peripheral.
 *
 * @param[in] crcp      pointer to the @p CRCDriver object
 *
 * @notapi
 */
void crc_lld_start(CRCDriver *crcp) {
  if (crcp->config == NULL)
    crcp->config = &default_config;

  rccEnableCRC(FALSE);
  CRC->INIT = crcp->config->initial_val;
  CRC->POL = crcp->config->poly;

  CRC->CR = 0;
  switch(crcp->config->poly_size) {
    case 32:
      break;
    case 16:
      CRC->CR |= CRC_CR_POLYSIZE_0;
      break;
    case 8:
      CRC->CR |= CRC_CR_POLYSIZE_1;
      break;
    case 7:
      CRC->CR |= CRC_CR_POLYSIZE_1 | CRC_CR_POLYSIZE_0;
      break;
    default:
      osalDbgAssert(false, "hardware doesn't support polynomial size");
      break;
  };
  if (crcp->config->reflect_data) {
    CRC->CR |= CRC_CR_REV_IN_1 | CRC_CR_REV_IN_0;
  }
  if (crcp->config->reflect_remainder) {
    CRC->CR |= CRC_CR_REV_OUT;
  }

}


/**
 * @brief   Deactivates the CRC peripheral.
 *
 * @param[in] crcp      pointer to the @p CRCDriver object
 *
 * @notapi
 */
void crc_lld_stop(CRCDriver *) {
  rccDisableCRC();
}

/**
 * @brief   Resets current CRC calculation.
 *
 * @param[in] crcp      pointer to the @p CRCDriver object
 *
 * @notapi
 */
void crc_lld_reset(void) {
  CRC->CR |= CRC_CR_RESET;
}

/**
 * @brief   Returns calculated CRC from last reset
 *
 * @param[in] crcp      pointer to the @p CRCDriver object
 * @param[in] n         size of buf in bytes
 * @param[in] buf       @p buffer location
 *
 * @notapi
 */
uint32_t crc_lld_calc(CRCDriver *crcp, const void *buf, size_t n) {
  if (crcp->config->reflect_data != 0) {
    CRC->CR |= CRC_CR_REV_IN_1 | CRC_CR_REV_IN_0;

    while(n > 3) {
      _crc_lld_calc_word(*(uint32_t*)buf);
      buf = ((uint32_t*)buf) + 1;
      n -= sizeof(uint32_t);
    }
  
  /* Programmable CRC units allow variable register width accesses.
     use bit reversal done by half-word if we are going to write tailing halfword */
    CRC->CR = (CRC->CR & ~CRC_CR_REV_IN_Msk) | CRC_CR_REV_IN_1;
    while(n > 1) {
      _crc_lld_calc_halfword(*(uint16_t*)buf);
      buf = ((uint16_t*)buf) + 1;
      n -= sizeof(uint16_t);
    }
  
  /* use bit reversal done by byte if we are going to write tailing byte */
    CRC->CR = (CRC->CR & ~CRC_CR_REV_IN_Msk) | CRC_CR_REV_IN_0;
  }

  while(n > 0) {
    _crc_lld_calc_byte(*(uint8_t*)buf);
    buf = ((uint8_t*)buf) + 1;
    n--;
  }

  return CRC->DR ^ crcp->config->final_val;
}

/**
 * @brief   Initializes the standard part of a @p CRCDriver structure.
 *
 * @param[out] crcp    Pointer to the @p CRCDriver object
 *
 * @init
 */
void crcObjectInit(CRCDriver *crcp)
{
  crcp->state  = CRC_STOP;
  crcp->config = NULL;
  osalMutexObjectInit(&crcp->mutex);
}

void crcInit(void) {
  crc_lld_init();
}
/**
 * @brief   Configures and activates the CRC peripheral.
 *
 * @param[in] crcp      Pointer to the @p CRCDriver object
 * @param[in] config    Pointer to the @p CRCConfig object
 *                      @p NULL if the low level driver implementation
 *                      supports a default configuration
 *
 * @api
 */
void crcStart(CRCDriver *crcp, const CRCConfig *config) {
  osalDbgCheck(crcp != NULL);

  osalSysLock();
  osalDbgAssert((crcp->state == CRC_STOP) || (crcp->state == CRC_READY),
                "invalid state");
  crcp->config = config;
  crc_lld_start(crcp);
  crcp->state = CRC_READY;
  osalSysUnlock();
}

/**
 * @brief   Deactivates the CRC peripheral.
 *
 * @param[in] crcp     Pointer to the @p CRCDriver object
 *
 * @api
 */
void crcStop(CRCDriver *crcp) {
  osalDbgCheck(crcp != NULL);

  osalSysLock();
  osalDbgAssert((crcp->state == CRC_STOP) || (crcp->state == CRC_READY),
                "invalid state");
  crc_lld_stop(crcp);
  crcp->state = CRC_STOP;
  osalSysUnlock();
}

/**
 * @brief   Resets the CRC calculation
 *
 * @param[in] crcp     Pointer to the @p CRCDriver object
 * @note	       thread context
 * @api
 */
void crcReset(CRCDriver *crcp)
{
  osalSysLock();
  crcResetI(crcp);
  osalSysUnlock();
}
/**
 * @brief   Resets the current CRC calculation
 *
 * @param[in] crcp      pointer to the @p CRCDriver object
 *
 * @iclass
 */
void crcResetI(CRCDriver *crcp)
{
  osalDbgCheck(crcp != NULL);
  osalDbgAssert(crcp->state == CRC_READY, "Not ready");
  crc_lld_reset();
}

/**
 * @brief   Set CRC initial value
 *
 * @param[in] crcp     Pointer to the @p CRCDriver object
 * @note	       thread context
 * @api
 */
void crcSetInitialValue(CRCDriver *crcp, uint32_t init)
{
  osalSysLock();
  crcSetInitialValueI(crcp, init);
  osalSysUnlock();
}

/**
 * @brief   Set CRC initial value
 *
 * @param[in] crcp     Pointer to the @p CRCDriver object
 * @note	       Lock(I) context
 * @api
 */
void crcSetInitialValueI(CRCDriver *crcp, uint32_t init)
{
  osalDbgCheck(crcp != NULL);
  osalDbgAssert(crcp->state == CRC_READY, "Not ready");
  CRC->INIT = crcp->config->reflect_data ? revbit(init) : init;
}

uint32_t crcGetFinalValue(CRCDriver *crcp)
{
  osalSysLock();
  const int32_t crc = crcGetFinalValueI(crcp);
  osalSysUnlock();
  return crc;
}

uint32_t crcGetFinalValueI(CRCDriver *crcp)
{
  osalDbgCheck(crcp != NULL);
  osalDbgAssert(crcp->state == CRC_READY, "Not ready");
  return CRC->DR ^ crcp->config->final_val;
}


/**
 * @brief   Performs a CRC calculation.
 * @details This synchronous function performs a crc calculation operation.
 *
 * @param[in] crcp      pointer to the @p CRCDriver object
 * @param[in] n         number of bytes to send
 * @param[in] buf       the pointer to the buffer
 *
 * @api
 */
uint32_t crcCalc(CRCDriver *crcp, const void *buf, size_t n)
{
  osalSysLock();
  const int32_t crc = crcCalcI(crcp, buf, n);
  osalSysUnlock();
  return crc;
}

/**
 * @brief   Performs a CRC calculation.
 * @details This synchronous function performs a crc calcuation operation.
 * @pre     In order to use this function the driver must have been configured
 *          without callbacks (@p end_cb = @p NULL).
 * @post    At the end of the operation the configured callback is invoked.
 *
 * @param[in] crcp      pointer to the @p CRCDriver object
 * @param[in] n         number of bytes to send
 * @param[in] buf       the pointer to the buffer
 *
 * @iclass
 */
uint32_t crcCalcI(CRCDriver *crcp, const void *buf, size_t n) {
  osalDbgCheck((crcp != NULL) && (n > 0U) && (buf != NULL));
  osalDbgAssert(crcp->state == CRC_READY, "not ready");
  return crc_lld_calc(crcp, buf, n);
}

/**
 * @brief   Gains exclusive access to the CRC unit.
 * @details This function tries to gain ownership to the CRC, if the CRC is
 *          already being used then the invoking thread is queued.
 * @pre     In order to use this function the option @p CRC_USE_MUTUAL_EXCLUSION
 *          must be enabled.
 *
 * @param[in] crcp      pointer to the @p CRCDriver object
 *
 * @api
 */
void crcAcquireUnit(CRCDriver *crcp) {
  osalDbgCheck(crcp != NULL);

  osalMutexLock(&crcp->mutex);
}

/**
 * @brief   Releases exclusive access to the CRC unit.
 * @pre     In order to use this function the option @p CRC_USE_MUTUAL_EXCLUSION
 *          must be enabled.
 *
 * @param[in] crcp      pointer to the @p CRCDriver object
 *
 * @api
 */
void crcReleaseUnit(CRCDriver *crcp) {
  osalDbgCheck(crcp != NULL);

  osalMutexUnlock(&crcp->mutex);
}

/** @} */
