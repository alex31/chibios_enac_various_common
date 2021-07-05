#include "hal_stm32_bdma.h"

/*
  TODO :



*/


#if (STM32_BDMA_USE_ASYNC_TIMOUT) && STM32_BDMA_USE_DOUBLE_BUFFER
#error "STM32_BDMA_USE_DOUBLE_BUFFER only not yet compatible with STM32_BDMA_USE_ASYNC_TIMOUT"
#endif


/**
 * @brief   BDMA ISR service routine.
 *
 * @param[in] bdmap      pointer to the @p BDMADriver object
 * @param[in] flags     pre-shifted content of the ISR register
 */
static void bdma_lld_serve_interrupt(BDMADriver *bdmap, uint32_t flags);


void bdmaObjectInit(BDMADriver *bdmap)
{
  osalDbgCheck(bdmap != NULL);

  bdmap->state    = BDMA_STOP;
  bdmap->config   = NULL;
  bdmap->mem0p    = NULL;
#if STM32_BDMA_USE_WAIT == TRUE
  bdmap->thread   = NULL;
#endif
#if STM32_BDMA_USE_MUTUAL_EXCLUSION == TRUE
  osalMutexObjectInit(&bdmap->mutex);
#endif
#if CH_DBG_SYSTEM_STATE_CHECK == TRUE
  bdmap->nbTransferError = 0U;
  bdmap->lastError = 0U;
#endif
#if STM32_BDMA_USE_ASYNC_TIMOUT
  chVTObjectInit(&bdmap->vt);
#endif
}


/**
 * @brief   Configures and activates the BDMA peripheral.
 *
 * @param[in] bdmap      pointer to the @p BDMADriver object
 * @param[in] config    pointer to the @p BDMAConfig object.
 * @return              The operation result.
 * @retval true         bdma driver is OK
 * @retval false        incoherencies has been found in config
 * @api
 */
bool bdmaStart(BDMADriver *bdmap, const BDMAConfig *cfg)
{
  osalDbgCheck((bdmap != NULL) && (cfg != NULL));
#if STM32_BDMA_USE_DOUBLE_BUFFER
  osalDbgAssert((cfg->op_mode != BDMA_CONTINUOUS_DOUBLE_BUFFER) || (!STM32_BDMA_USE_ASYNC_TIMOUT),
                "STM32_BDMA_USE_ASYNC_TIMOUT not yet implemented in BDMA_CONTINUOUS_DOUBLE_BUFFER mode");

  osalDbgAssert((cfg->op_mode != BDMA_CONTINUOUS_DOUBLE_BUFFER) || (cfg->next_cb != NULL),
                "BDMA_CONTINUOUS_DOUBLE_BUFFER mode implies next_cb not NULL");
#endif
  osalSysLock();
  osalDbgAssert((bdmap->state == BDMA_STOP) || (bdmap->state == BDMA_READY),
                "invalid state");
  bdmap->config = cfg;
  const bool statusOk = bdma_lld_start(bdmap);
  bdmap->state = BDMA_READY;
#if  STM32_BDMA_USE_DOUBLE_BUFFER
  bdmap->next_cb_errors = 0U;
#endif  
  osalSysUnlock();
  return statusOk;
}


/**
 * @brief   Deactivates the BDMA peripheral.
 *
 * @param[in] bdmap      pointer to the @p BDMADriver object
 *
 * @api
 */
void bdmaStop(BDMADriver *bdmap)
{
  osalDbgCheck(bdmap != NULL);

  osalDbgAssert((bdmap->state == BDMA_STOP) || (bdmap->state == BDMA_READY),
                "invalid state");

  bdma_lld_stop(bdmap);

  osalSysLock();
  bdmap->config = NULL;
  bdmap->state  = BDMA_STOP;
  bdmap->mem0p   = NULL;
  osalSysUnlock();
}


/**
 * @brief   Starts a BDMA transaction.
 * @details Starts one or many asynchronous bdma transaction(s) depending on continuous field
 * @post    The callbacks associated to the BDMA config will be invoked
 *          on buffer fill and error events, and timeout events in case
 *          STM32_BDMA_USE_ASYNC_TIMOUT == TRUE
 * @note    The datas are sequentially written into the buffer
 *          with no gaps.
 *
 * @param[in]      bdmap      pointer to the @p BDMADriver object
 * @param[in,out]  periphp   pointer to a @p peripheral register address
 * @param[in,out]  mem0p    pointer to the data buffer
 * @param[in]      size     buffer size. The buffer size
 *                          must be one or an even number.
 *
 * @api
 */
bool bdmaStartTransfert(BDMADriver *bdmap, volatile void *periphp,  void * mem0p, const size_t size)
{
  osalSysLock();
  const bool statusOk = bdmaStartTransfertI(bdmap, periphp, mem0p, size);
  osalSysUnlock();
  return statusOk;
}

/**
 * @brief   Starts a BDMA transaction.
 * @details Starts one or many asynchronous bdma transaction(s) depending on continuous field
 * @post    The callbacks associated to the BDMA config will be invoked
 *          on buffer fill and error events, and timeout events in case
 *          STM32_BDMA_USE_ASYNC_TIMOUT == TRUE
 * @note    The datas are sequentially written into the buffer
 *          with no gaps.
 *
 * @param[in]      bdmap      pointer to the @p BDMADriver object
 * @param[in,out]  periphp   pointer to a @p peripheral register address
 * @param[in,out]  mem0p    pointer to the data buffer
 * @param[in]      size     buffer size. The buffer size
 *                          must be one or an even number.
 *
 * @iclass
 */
bool bdmaStartTransfertI(BDMADriver *bdmap, volatile void *periphp,  void *  mem0p, const size_t size)
{
  osalDbgCheckClassI();

#if STM32_BDMA_USE_DOUBLE_BUFFER
  if (bdmap->config->op_mode == BDMA_CONTINUOUS_DOUBLE_BUFFER) {
    osalDbgAssert(mem0p == NULL,
		  "in double buffer mode memory pointer is dynamically completed by next_cb callback");
    mem0p = bdmap->config->next_cb(bdmap, size);    
  }
#endif
  
#if (CH_DBG_ENABLE_ASSERTS != FALSE)
  if (size != bdmap->size) {
    osalDbgCheck((bdmap != NULL) && (mem0p != NULL) && (periphp != NULL) &&
		 (size > 0U) && ((size == 1U) || ((size & 1U) == 0U)));

    const BDMAConfig	    *cfg = bdmap->config;
    osalDbgAssert((bdmap->state == BDMA_READY) ||
		  (bdmap->state == BDMA_COMPLETE) ||
		  (bdmap->state == BDMA_ERROR),
		  "not ready");
    /* if (cfg->pburst) */
    /*   osalDbgAssert((uint32_t) periphp % (cfg->pburst * cfg->psize) == 0, "peripheral address not aligned"); */
    /* else */
      osalDbgAssert((uint32_t) periphp % cfg->psize == 0, "peripheral address not aligned");

    /* if (cfg->mburst) */
    /*   osalDbgAssert((uint32_t) mem0p % (cfg->mburst * cfg->msize) == 0, "memory address not aligned"); */
    /* else */
      osalDbgAssert((uint32_t) mem0p % cfg->msize == 0, "memory address not aligned");
  }
#endif // CH_DBG_ENABLE_ASSERTS != FALSE
  bdmap->state    = BDMA_ACTIVE;

#if STM32_BDMA_USE_ASYNC_TIMOUT
  bdmap->currPtr = mem0p;
  if (bdmap->config->timeout != TIME_INFINITE) {
    chVTSetI(&bdmap->vt, bdmap->config->timeout,
	     &bdma_lld_serve_timeout_interrupt, (void *) bdmap);
  }
#endif

  return bdma_lld_start_transfert(bdmap, periphp, mem0p, size);
}


/**
 * @brief   Stops an ongoing transaction.
 * @details This function stops the currently ongoing transaction and returns
 *          the driver in the @p BDMA_READY state. If there was no transaction
 *          being processed then the function does nothing.
 *
 * @param[in] bdmap      pointer to the @p BDMADriver object
 *
 * @api
 */
void bdmaStopTransfert(BDMADriver *bdmap)
{

  osalDbgCheck(bdmap != NULL);

  osalSysLock();
  osalDbgAssert((bdmap->state == BDMA_READY) || (bdmap->state == BDMA_ACTIVE),
                "invalid state");
  if (bdmap->state != BDMA_READY) {
    bdma_lld_stop_transfert(bdmap);
    bdmap->state = BDMA_READY;
    _bdma_reset_s(bdmap);
  }
  osalSysUnlock();
}



/**
 * @brief   Stops an ongoing transaction.
 * @details This function stops the currently ongoing transaction and returns
 *          the driver in the @p BDMA_READY state. If there was no transaction
 *          being processed then the function does nothing.
 *
 * @param[in] bdmap      pointer to the @p BDMADriver object
 *
 * @iclass
 */
void bdmaStopTransfertI(BDMADriver *bdmap)
{
  osalDbgCheckClassI();
  osalDbgCheck(bdmap != NULL);
  osalDbgAssert((bdmap->state == BDMA_READY) ||
                (bdmap->state == BDMA_ACTIVE) ||
                (bdmap->state == BDMA_COMPLETE),
                "invalid state");


  if (bdmap->state != BDMA_READY) {
    bdma_lld_stop_transfert(bdmap);
    bdmap->state = BDMA_READY;
    _bdma_reset_i(bdmap);
  }

}

#if (STM32_BDMA_USE_WAIT == TRUE) || defined(__DOXYGEN__)

/**
 * @brief   Performs  a BDMA transaction.
 * @details Performs one synchronous bdma transaction
 * @note    The datas are sequentially written into the buffer
 *          with no gaps.
 *
 * @param[in]      bdmap      pointer to the @p BDMADriver object
 * @param[in,out]  periphp   pointer to a @p peripheral register address
 * @param[in,out]  mem0p    pointer to the data buffer
 * @param[in]      size     buffer size. The buffer size
 *                          must be one or an even number.
 * @param[in]      timeout  function will exit after timeout is transaction is not done
 *                          can be TIME_INFINITE (but not TIME_IMMEDIATE)
 * @return              The operation result.
 * @retval MSG_OK       Transaction finished.
 * @retval MSG_RESET    The transaction has been stopped using
 *                      @p bdmaStopTransaction() or @p bdmaStopTransactionI(),
 *                      the result buffer may contain incorrect data.
 * @retval MSG_TIMEOUT  The transaction has been stopped because of hardware
 *                      error or timeout limit reach
 *
 * @api
 */
msg_t bdmaTransfertTimeout(BDMADriver *bdmap, volatile void *periphp, void *mem0p, const size_t size,
			  sysinterval_t timeout)
{
  msg_t msg;

  osalSysLock();
  osalDbgAssert(bdmap->thread == NULL, "already waiting");
  osalDbgAssert(bdmap->config->op_mode == BDMA_ONESHOT, "blocking API is incompatible with circular modes");
  bdmaStartTransfertI(bdmap, periphp, mem0p, size);
  msg = osalThreadSuspendTimeoutS(&bdmap->thread, timeout);
  if (msg != MSG_OK) {
    bdmaStopTransfertI(bdmap);
  }
  osalSysUnlock();
  return msg;
}
#endif

#if (STM32_BDMA_USE_MUTUAL_EXCLUSION == TRUE) || defined(__DOXYGEN__)
/**
 * @brief   Gains exclusive access to the BDMA peripheral.
 * @details This function tries to gain ownership to the BDMA bus, if the bus
 *          is already being used then the invoking thread is queued.
 * @pre     In order to use this function the option
 *          @p BDMA_USE_MUTUAL_EXCLUSION must be enabled.
 *
 * @param[in] bdmap      pointer to the @p BDMADriver object
 *
 * @api
 */
void bdmaAcquireBus(BDMADriver *bdmap) {

  osalDbgCheck(bdmap != NULL);

  osalMutexLock(&bdmap->mutex);
}

/**
 * @brief   Releases exclusive access to the BDMA peripheral.
 * @pre     In order to use this function the option
 *          @p BDMA_USE_MUTUAL_EXCLUSION must be enabled.
 *
 * @param[in] bdmap      pointer to the @p BDMADriver object
 *
 * @api
 */
void bdmaReleaseBus(BDMADriver *bdmap) {

  osalDbgCheck(bdmap != NULL);

  osalMutexUnlock(&bdmap->mutex);
}
#endif /* BDMA_USE_MUTUAL_EXCLUSION == TRUE */



/*
  #                 _                                  _                              _
  #                | |                                | |                            | |
  #                | |        ___   __      __        | |        ___  __   __   ___  | |
  #                | |       / _ \  \ \ /\ / /        | |       / _ \ \ \ / /  / _ \ | |
  #                | |____  | (_) |  \ V  V /         | |____  |  __/  \ V /  |  __/ | |
  #                |______|  \___/    \_/\_/          |______|  \___|   \_/    \___| |_|
  #                 _____            _
  #                |  __ \          (_)
  #                | |  | |   _ __   _   __   __   ___   _ __
  #                | |  | |  | '__| | |  \ \ / /  / _ \ | '__|
  #                | |__| |  | |    | |   \ V /  |  __/ | |
  #                |_____/   |_|    |_|    \_/    \___| |_|
*/


/**
 * @brief   Configures and activates the BDMA peripheral.
 *
 * @param[in] bdmap      pointer to the @p BDMADriver object
 *
 * @notapi
 */
bool bdma_lld_start(BDMADriver *bdmap)
{
  uint32_t psize_msk, msize_msk;

  const BDMAConfig *cfg = bdmap->config;

  switch (cfg->psize) {
  case 1 : psize_msk = STM32_BDMA_CR_PSIZE_BYTE; break;
  case 2 : psize_msk = STM32_BDMA_CR_PSIZE_HWORD; break;
  case 4 : psize_msk = STM32_BDMA_CR_PSIZE_WORD; break;
  default: osalSysHalt("psize should be 1 or 2 or 4");
    return false;
  }
  switch (cfg->msize) {
  case 1 : msize_msk = STM32_BDMA_CR_MSIZE_BYTE; break;
  case 2 : msize_msk = STM32_BDMA_CR_MSIZE_HWORD; break;
  case 4 : msize_msk = STM32_BDMA_CR_MSIZE_WORD; break;
  default: osalDbgAssert(false, "msize should be 1 or 2 or 4");
    return false;
  }

  uint32_t dir_msk=0UL;
  switch (cfg->direction) {
  case BDMA_DIR_P2M: dir_msk=STM32_BDMA_CR_DIR_P2M; break;
  case BDMA_DIR_M2P: dir_msk=STM32_BDMA_CR_DIR_M2P; break;
  case BDMA_DIR_M2M: dir_msk=STM32_BDMA_CR_DIR_M2M; break;
  default: osalDbgAssert(false, "direction not set or incorrect");
  }

  uint32_t isr_flags = cfg->op_mode == BDMA_ONESHOT ? STM32_BDMA_CR_TCIE : 0UL;

  // in M2M mode, half buffer transfert ISR is disabled, but
  // full buffer transfert complete ISR is enabled
  if (cfg->end_cb) {
    isr_flags |= STM32_BDMA_CR_TCIE;
    if ((cfg->direction != BDMA_DIR_M2M) &&
	(cfg->op_mode == BDMA_CONTINUOUS_HALF_BUFFER)) {
      isr_flags |= STM32_BDMA_CR_HTIE;
    }
  }

  if (cfg->error_cb) {
    isr_flags |= STM32_BDMA_CR_TCIE;
  }
  
  bdmap->bdmamode = STM32_BDMA_CR_PL(cfg->bdma_priority) |
    dir_msk | psize_msk | msize_msk | isr_flags |
    (cfg->op_mode == BDMA_CONTINUOUS_HALF_BUFFER ? STM32_BDMA_CR_CIRC : 0UL) |
#if  STM32_BDMA_USE_DOUBLE_BUFFER
    (cfg->op_mode == BDMA_CONTINUOUS_DOUBLE_BUFFER ? STM32_BDMA_CR_DBM : 0UL) |
#endif
    (cfg->inc_peripheral_addr ? STM32_BDMA_CR_PINC : 0UL) |
    (cfg->inc_memory_addr ? STM32_BDMA_CR_MINC : 0UL);


  bdmap->bdmastream = bdmaStreamAllocI(bdmap->config->stream,
				    cfg->irq_priority,
				    (stm32_bdmaisr_t) &bdma_lld_serve_interrupt,
				    (void *) bdmap );
  bool error = bdmap->bdmastream == NULL;

  if (error) {
    osalDbgAssert(false, "stream already allocated");
    return false;
  }

  return true;
}

static inline size_t getCrossCacheBoundaryAwareSize(const void *memp,
						    const size_t dsize)
{
  // L1 cache on F7 and H7 is organised of line of 32 bytes
  // returned size is not 32 bytes aligned by a mask operation
  // because cache management does internal mask and this operation
  // would be useless

  const uint32_t endp = ((uint32_t) memp % CACHE_LINE_SIZE  +
			 dsize % CACHE_LINE_SIZE );
  return endp < CACHE_LINE_SIZE  ? dsize + CACHE_LINE_SIZE  :
    dsize + CACHE_LINE_SIZE *2U;
}

/**
 * @brief   Starts a BDMA transaction.
 *
 * @param[in] bdmap      pointer to the @p BDMADriver object
 *
 * @notapi
 */
bool bdma_lld_start_transfert(BDMADriver *bdmap, volatile void *periphp, void *mem0p, const size_t size)
{
#if __DCACHE_PRESENT
  if (bdmap->config->activate_dcache_sync &&
      (bdmap->config->direction != BDMA_DIR_P2M)) {
    const size_t cacheSize = getCrossCacheBoundaryAwareSize(mem0p,
						    size * bdmap->config->msize);
    cacheBufferFlush(mem0p, cacheSize);
  }
#endif
  bdmap->mem0p = mem0p;
#if __DCACHE_PRESENT
  bdmap->periphp = periphp;
#endif
  bdmap->size = size;
  bdmaStreamSetPeripheral(bdmap->bdmastream, periphp);
  bdmaSetRequestSource(bdmap->bdmastream, bdmap->config->dmamux2);


  bdmaStreamSetMemory0(bdmap->bdmastream, mem0p);
#if  STM32_BDMA_USE_DOUBLE_BUFFER
  if (bdmap->config->op_mode == BDMA_CONTINUOUS_DOUBLE_BUFFER) {
    bdmaStreamSetMemory1(bdmap->bdmastream, bdmap->config->next_cb(bdmap, size));
  }
#endif
  bdmaStreamSetTransactionSize(bdmap->bdmastream, size);
  bdmaStreamSetMode(bdmap->bdmastream, bdmap->bdmamode);
  bdmaStreamEnable(bdmap->bdmastream);

  return true;
}

/**
 * @brief   Stops an ongoing transaction.
 *
 * @param[in] bdmap      pointer to the @p BDMADriver object
 *
 * @notapi
 */
void bdma_lld_stop_transfert(BDMADriver *bdmap)
{
  bdmaStreamDisable(bdmap->bdmastream);
}

/**
 * @brief   Deactivates the BDMA peripheral.
 *
 * @param[in] bdmap      pointer to the @p BDMADriver object
 *
 * @notapi
 */
void bdma_lld_stop(BDMADriver *bdmap)
{
  bdmaStreamFree(bdmap->bdmastream);
}


/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   BDMA BDMA ISR service routine.
 *
 * @param[in] bdmap      pointer to the @p BDMADriver object
 * @param[in] flags     pre-shifted content of the ISR register
 */
static void bdma_lld_serve_interrupt(BDMADriver *bdmap, uint32_t flags)
{

  /* BDMA errors handling.*/
  static const uint32_t feif_msk = 0U;
  if ((flags & (STM32_BDMA_ISR_TEIF | feif_msk)) != 0U) {
    /* BDMA, this could help only if the BDMA tries to access an unmapped
       address space or violates alignment rules.*/
    const bdmaerrormask_t err =
      (flags & STM32_BDMA_ISR_TEIF)  ? BDMA_ERR_TRANSFER_ERROR : 0UL;
    _bdma_isr_error_code(bdmap, err);
  } else {
    /* It is possible that the transaction has already be reset by the
       BDMA error handler, in this case this interrupt is spurious.*/
    if (bdmap->state == BDMA_ACTIVE) {
#if __DCACHE_PRESENT
      if (bdmap->config->activate_dcache_sync)
	  switch (bdmap->config->direction) {
	  case BDMA_DIR_M2P : break;
	  case BDMA_DIR_P2M : if (bdmap->mem0p >= (void *) 0x20000000) {
	      const size_t cacheSize =
		getCrossCacheBoundaryAwareSize(bdmap->mem0p, bdmap->size *
					       bdmap->config->msize);
	      cacheBufferInvalidate(bdmap->mem0p, cacheSize);
	    }
	    break;

	  case BDMA_DIR_M2M :  if (bdmap->periphp >=  (void *) 0x20000000) {
	      const size_t cacheSize =
		getCrossCacheBoundaryAwareSize((void *) bdmap->periphp,
					       bdmap->size * bdmap->config->psize);
	      cacheBufferInvalidate(bdmap->periphp, cacheSize);
	    }
	    break;
	  }
#endif

      if ((flags & STM32_BDMA_ISR_TCIF) != 0) {
        /* Transfer complete processing.*/
        _bdma_isr_full_code(bdmap);
      } else if ((flags & STM32_BDMA_ISR_HTIF) != 0) {
        /* Half transfer processing.*/
        _bdma_isr_half_code(bdmap);
      }
    }
  }
}

#if STM32_BDMA_USE_ASYNC_TIMOUT
/**
 * @brief   BDMA ISR service routine.
 *
 * @param[in] bdmap      pointer to the @p BDMADriver object
 */
void bdma_lld_serve_timeout_interrupt(void *arg)
{
  BDMADriver *bdmap = (BDMADriver *) arg;
  if (bdmap->config->op_mode != BDMA_ONESHOT) {
    chSysLockFromISR();
    chVTSetI(&bdmap->vt, bdmap->config->timeout,
	     &bdma_lld_serve_timeout_interrupt, (void *) bdmap);
    chSysUnlockFromISR();
  }
  async_timout_enabled_call_end_cb(bdmap, FROM_TIMOUT_CODE);
}
#endif


#if  STM32_BDMA_USE_DOUBLE_BUFFER
/**
 * @brief   Common ISR code, switch memory pointer in double buffer mode
 * @note    This macro is meant to be used in the low level drivers
 *          implementation only. This function must be called as soon as
 *          BDMA has switched buffer.
 *
 * @param[in] bdmap        pointer to the @p BDMADriver object
 * @param[in] nextBuffer  pointer to a buffer that is set in MEM0xP or MEM1xP
 *			  the one which is not occupied beeing filled
 *                        by BDMA.
 * @notapi
 */
void* bdma_lld_set_next_double_buffer(BDMADriver *bdmap, void *nextBuffer)
{
  void *lastBuffer;

  if (bdmaStreamGetCurrentTarget(bdmap->bdmastream)) {
    lastBuffer = (void *) bdmap->bdmastream->stream->M0AR;
    bdmap->bdmastream->stream->M0AR = (uint32_t) nextBuffer;
  } else {
    lastBuffer = (void *) bdmap->bdmastream->stream->M1AR;
    bdmap->bdmastream->stream->M1AR = (uint32_t) nextBuffer;
  }
  return lastBuffer;
}
#endif
