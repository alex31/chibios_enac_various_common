#include "hal_stm32_mdma.h"
#include <string.h>

/*
  1/ test : size est un nombre de bytes ou un nombre de world de taille 1..8 ?
     ° si c'est un nombre de word, y faudra revoir le calcul du cache flush/invalidate

  
 */
static void mdma_lld_serve_interrupt(MDMADriver *mdmap, uint32_t flags);

static inline uint32_t mdmaChannelGetErrorStatus(MDMADriver *mdmap);
static inline void mdmaChannelSetCBRUR(MDMADriver *mdmap, uint32_t cbrur);
static inline void mdmaChannelSetCLAR(MDMADriver *mdmap, uint32_t clar);
static inline void mdmaChannelSetCMAR(MDMADriver *mdmap, uint32_t cmar);
static inline void mdmaChannelSetCMDR(MDMADriver *mdmap, uint32_t cmdr);
static inline void mdmaChannelSelectBuses(MDMADriver *mdmap, uint32_t sourceDestBuses);

void mdmaObjectInit(MDMADriver *mdmap)
{
  osalDbgCheck(mdmap != NULL);

  mdmap->state    = MDMA_STOP;
  mdmap->config   = NULL;
  mdmap->source   = NULL;
  mdmap->destination   = NULL;
#if STM32_MDMA_USE_WAIT == TRUE
  mdmap->thread   = NULL;
#endif
#if STM32_MDMA_USE_MUTUAL_EXCLUSION == TRUE
  osalMutexObjectInit(&mdmap->mutex);
#endif
#if CH_DBG_SYSTEM_STATE_CHECK == TRUE
  mdmap->nbTransferError = 0U;
  mdmap->lastError = 0U;
#endif
}

/**
 * @brief   Configures and activates the MDMA peripheral.
 *
 * @param[in] mdmap      pointer to the @p MDMADriver object
 * @param[in] config    pointer to the @p MDMAConfig object.
 * @return              The operation result.
 * @retval true         mdma driver is OK
 * @retval false        incoherencies has been found in config
 * @api
 */
bool mdmaStart(MDMADriver *mdmap, const MDMAConfig *cfg)
{
  osalDbgCheck((mdmap != NULL) && (cfg != NULL));
  osalSysLock();
  osalDbgAssert((mdmap->state == MDMA_STOP) || (mdmap->state == MDMA_READY),
                "invalid state");
  osalDbgAssert((cfg->transfert_len > 0U) && (cfg->transfert_len <= 128U),
		"invalid transfert_len");
  
  mdmap->config = cfg;
  const bool statusOk = mdma_lld_start(mdmap);
  mdmap->state = statusOk ? MDMA_READY : MDMA_ERROR;
  
  osalSysUnlock();
  return statusOk;
}

/**
 * @brief   Deactivates the MDMA peripheral.
 *
 * @param[in] mdmap      pointer to the @p MDMADriver object
 *
 * @api
 */
void mdmaStop(MDMADriver *mdmap)
{
  osalDbgCheck(mdmap != NULL);

  osalDbgAssert((mdmap->state == MDMA_STOP) || (mdmap->state == MDMA_READY),
                "invalid state");

  mdma_lld_stop(mdmap);

  osalSysLock();
  mdmap->config = NULL;
  mdmap->state  = MDMA_STOP;
  mdmap->source = NULL;
  mdmap->destination = NULL;
  osalSysUnlock();
}





/**
 * @brief   Starts a MDMA transaction.
 * @details Starts one or many asynchronous mdma transaction(s) depending on configuration
 * @post    The callbacks associated to the MDMA config will be invoked
 *          on buffer fill and error events, and timeout events in case
 *          STM32_MDMA_USE_ASYNC_TIMOUT == TRUE
 * @note    The datas are sequentially written into the buffer
 *          with no gaps.
 *
 * @param[in]      mdmap         pointer to the @p MDMADriver object
 * @param[in,out]  source        pointer to the source address
 * @param[in,out]  dest          pointer to the destination adress
 * @param[in]      size          buffer size. 
 * @param[in]      user_data     user_data passed to callback. 
 *
 * @api
 */
bool mdmaStartTransfert(MDMADriver *mdmap, void *source, void *dest,
			const size_t size, void *user_data)
{
  osalSysLock();
  const bool statusOk = mdmaStartTransfertI(mdmap, source, dest, size, user_data);
  osalSysUnlock();
  return statusOk;
}

/**
 * @brief   Starts a MDMA transaction.
 * @details Starts one or many asynchronous mdma transaction(s) depending on configuration
 * @post    The callbacks associated to the MDMA config will be invoked
 *          on buffer fill and error events, and timeout events in case
 *          STM32_MDMA_USE_ASYNC_TIMOUT == TRUE
 * @note    The datas are sequentially written into the buffer
 *          with no gaps.
 *
 * @param[in]      mdmap         pointer to the @p MDMADriver object
 * @param[in,out]  source        pointer to the source address
 * @param[in,out]  dest          pointer to the destination adress
 * @param[in]      size          buffer size. 
 * @param[in]      user_data     user_data passed to callback. 
 *
 * @api
 */
bool  mdmaStartTransfertI(MDMADriver *mdmap, void *source, void *dest,
			  const size_t size, void *user_data)
{
  osalDbgCheckClassI();
  
#if (CH_DBG_ENABLE_ASSERTS != FALSE)
  if (size != mdmap->size) {
    osalDbgCheck((mdmap != NULL) && (dest != NULL) && (source != NULL) &&
		 (size > 0U));

    const MDMAConfig	    *cfg = mdmap->config;
    const size_t ssize = 1U << cfg->swidth;
    const size_t dsize = 1U << cfg->dwidth;
    const size_t sburst = 1U << cfg->sburst;
    const size_t dburst = 1U << cfg->dwidth;
    const size_t dincos = cfg->dest_incr > 0 ? cfg->dest_incr : -cfg->dest_incr;
    const size_t sincos = cfg->source_incr > 0 ? cfg->source_incr : -cfg->source_incr;
    osalDbgAssert((mdmap->state == MDMA_READY) ||
		  (mdmap->state == MDMA_COMPLETE) ||
		  (mdmap->state == MDMA_ERROR),
		  "not ready");

    osalDbgAssert((uint32_t) source % ssize == 0, "source address not aligned");
    osalDbgAssert(size % ssize == 0, "size must me a multiple of source data size");
    osalDbgAssert(size % dsize == 0, "size must me a multiple of destination data size");
    osalDbgAssert(cfg->transfert_len % ssize == 0, "transfert_len must me a multiple of source data size");
    osalDbgAssert(cfg->transfert_len % dsize == 0, "transfert_len must me a multiple of destination data size");
    osalDbgAssert(sburst < cfg->transfert_len, "source burst must be less than transfert_len");
    osalDbgAssert(dburst < cfg->transfert_len, "destination burst must be less than transfert_len");
    if (cfg->bus_selection | MDMA_DESTBUS_TCM) {
      if ((dincos == 8) ||
	  (dincos == 0) ||
	  (dincos != dsize)) {
	osalDbgAssert(dburst == MDMA_BURST_1, "several conditions implies destination "
		      "burst must be single transfert");
      }
    } else { // destination bus is AXI
      if (dincos == 0) {
	osalDbgAssert(dburst <= 16, "several conditions implies destination burst must not exceed 16");
      }
    }
    if (cfg->bus_selection | MDMA_SOURCEBUS_TCM) {
      if ((sincos == 8) ||
	  (sincos == 0) ||
	  (sincos != ssize)) {
	osalDbgAssert(sburst == MDMA_BURST_1, "several conditions implies source "
		      "burst must be single transfert");
      }
    } else { // source bus is AXI
      if (sincos == 0) {
	osalDbgAssert(sburst <= 16, "several conditions implies source burst must not exceed 16");
      }
    }

    if (dincos != 0) {
      osalDbgAssert(dincos >= dsize, "destination increment must be greater or equal to destination size");
      if (cfg->bus_selection | MDMA_DESTBUS_TCM) {
	if (dburst != 1) {
	  osalDbgAssert(((uint32_t) dest % dincos) == 0, "when dest is AHB and burst is enabled, "
			"destination address must be aligned with destination increment");
	}
      }
    }
    
    if (sincos != 0) {
      osalDbgAssert(sincos >= ssize, "source increment must be greater or equal to source size");
     if (cfg->bus_selection | MDMA_SOURCEBUS_TCM) {
	if (sburst != 1) {
	  osalDbgAssert(((uint32_t) source % sincos) == 0, "when source is AHB and burst is enabled, "
			"source address must be aligned with source increment");
	}
      }
    }

    if (cfg->bus_selection | MDMA_SOURCEBUS_TCM) {
      osalDbgAssert(ssize <= 4, "when source is TCM/AHB source, source width should be inferior or equal to 4");
      osalDbgAssert(sincos != 0, "when source is TCM/AHB source, fixed source address is forbidden");
    }
   if (cfg->bus_selection | MDMA_DESTBUS_TCM) {
      osalDbgAssert(dsize <= 4, "when destination is TCM/AHB, destination width should be inferior or equal to 4"); 
      osalDbgAssert(dincos != 0, "when destination is TCM/AHB destination, fixed destination address is forbidden");
    }
    
  }
#endif // CH_DBG_ENABLE_ASSERTS != FALSE
  mdmap->state    = MDMA_ACTIVE;
  mdmap->user_data = user_data;
  return mdma_lld_start_transfert(mdmap, source, dest, size);
}


/**
 * @brief   Stops an ongoing transaction.
 * @details This function stops the currently ongoing transaction and returns
 *          the driver in the @p MDMA_READY state. If there was no transaction
 *          being processed then the function does nothing.
 *
 * @param[in] mdmap      pointer to the @p MDMADriver object
 *
 * @api
 */
void mdmaStopTransfert(MDMADriver *mdmap)
{

  osalDbgCheck(mdmap != NULL);

  osalSysLock();
  osalDbgAssert((mdmap->state == MDMA_READY) || (mdmap->state == MDMA_ACTIVE),
                "invalid state");
  if (mdmap->state != MDMA_READY) {
    mdma_lld_stop_transfert(mdmap);
    mdmap->state = MDMA_READY;
    _mdma_reset_s(mdmap);
  }
  osalSysUnlock();
}

/**
 * @brief   Stops an ongoing transaction.
 * @details This function stops the currently ongoing transaction and returns
 *          the driver in the @p MDMA_READY state. If there was no transaction
 *          being processed then the function does nothing.
 *
 * @param[in] mdmap      pointer to the @p MDMADriver object
 *
 * @iclass
 */
void mdmaStopTransfertI(MDMADriver *mdmap)
{
  osalDbgCheckClassI();
  osalDbgCheck(mdmap != NULL);
  osalDbgAssert((mdmap->state == MDMA_READY) ||
                (mdmap->state == MDMA_ACTIVE) ||
                (mdmap->state == MDMA_COMPLETE),
                "invalid state");


  if (mdmap->state != MDMA_READY) {
    mdma_lld_stop_transfert(mdmap);
    mdmap->state = MDMA_READY;
    _mdma_reset_i(mdmap);
  }
}

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
 * @brief   Configures and activates the MDMA peripheral.
 *
 * @param[in] mdmap      pointer to the @p MDMADriver object
 *
 * @notapi
 */
bool mdma_lld_start(MDMADriver *mdmap)
{
  const MDMAConfig *cfg = mdmap->config;
  uint32_t sinc, sincval, dinc, dincval;
  memset(&mdmap->cache, 0U, sizeof(mdmap->cache));

  
  
  if (cfg->source_incr > 0) {
    sinc = STM32_MDMA_CTCR_SINC_INC;
    sincval = cfg->source_incr;
  } else if (cfg->source_incr < 0) {
    sinc = STM32_MDMA_CTCR_SINC_DEC;
    sincval = -cfg->source_incr;
  } else {
    sinc = STM32_MDMA_CTCR_SINC_FIXED;
    sincval = 0;
  }
  if (cfg->dest_incr > 0) {
    dinc = STM32_MDMA_CTCR_DINC_INC;
    dincval = cfg->dest_incr;
  } else if (cfg->dest_incr < 0) {
    dinc = STM32_MDMA_CTCR_DINC_DEC;
    dincval = -cfg->dest_incr;
  } else {
    dinc = STM32_MDMA_CTCR_DINC_FIXED;
    dincval = 0;
  }
  
  mdmap->cache.ccr = STM32_MDMA_CCR_PL(cfg->mdma_priority) |
    STM32_MDMA_CCR_CTCIE |
    (cfg->error_cb != NULL) ? STM32_MDMA_CCR_TEIE : 0 |
    (cfg->buffer_transfert_cb != NULL) ? STM32_MDMA_CCR_TCIE : 0 |
    (cfg->block_transfert_cb != NULL) ? STM32_MDMA_CCR_BTIE : 0 |
    (cfg->block_transfert_repeat_cb != NULL) ? STM32_MDMA_CCR_BRTIE : 0 |
    (cfg->endianness_swap == true) ? STM32_MDMA_CCR_WEX : 0;

  mdmap->cache.ctcr = STM32_MDMA_CTCR_BWM_NON_BUFF  |
    cfg->op_mode |
    STM32_MDMA_CTCR_TLEN(cfg->transfert_len - 1U) |
    STM32_MDMA_CTCR_SBURST(cfg->sburst) |
    STM32_MDMA_CTCR_DBURST(cfg->dburst) |
    STM32_MDMA_CTCR_SINCOS(sincval) |
    STM32_MDMA_CTCR_DINCOS(dincval) |
    STM32_MDMA_CTCR_SSIZE(cfg->swidth) |
    STM32_MDMA_CTCR_DSIZE(cfg->dwidth) |
    sinc |
    dinc |
    cfg->ctcr;

  if (cfg->op_mode == MDMA_REPEATED_BLOCK) {
    uint32_t src_update=0, dest_update=0;
    if (cfg->block.src_addr_update < 0) {
      mdmap->cache.opt = MDMA_CBNDTR_BRSUM;
      src_update = -cfg->block.src_addr_update;
    } else {
      src_update = cfg->block.src_addr_update;
    }
    
    if (cfg->block.dest_addr_update < 0) {
      mdmap->cache.opt |= MDMA_CBNDTR_BRDUM;
      dest_update = -cfg->block.dest_addr_update;
    } else {
      dest_update = cfg->block.dest_addr_update;
    }
    mdmap->cache.cbrur = src_update | (dest_update << 16U);
    mdmap->cache.brc =  mdmap->config->block.repeat;
  } else if (cfg->op_mode ==  MDMA_LINKED_LIST) {
    mdmap->cache.clar = (uint32_t) mdmap->config->link.address;
  }
    
  
  mdmap->mdma = mdmaChannelAllocI(mdmap->config->channel,
				  (stm32_mdmaisr_t)mdma_lld_serve_interrupt,
				  (void *)mdmap);

  osalDbgAssert(mdmap->mdma != NULL, "unable to allocate MDMA channel");
  return mdmap->mdma != NULL;
}


/**
 * @brief   Deactivates the MDMA peripheral.
 *
 * @param[in] mdmap      pointer to the @p MDMADriver object
 *
 * @notapi
 */
void mdma_lld_stop(MDMADriver *mdmap)
{
  /* Releasing the DMA.*/
  mdmaChannelFreeI(mdmap->mdma);
  mdmap->mdma = NULL;
}

/**
 * @brief   Starts a MDMA transaction.
 *
 * @param[in] mdmap      pointer to the @p MDMADriver object
 *
 * @notapi
 */
bool  mdma_lld_start_transfert(MDMADriver *mdmap, void *source, void *dest, const size_t size)
{
#if __DCACHE_PRESENT
  if (mdmap->config->activate_dcache_sync) {
    const size_t cacheSize = getCrossCacheBoundaryAwareSize(source, size);
    cacheBufferFlush(source, cacheSize);
  }
#endif
  mdmap->destination = dest;
  mdmap->source = source;
  mdmap->size = size;

    /* MDMA initializations.*/
  mdmaChannelSetSourceX(mdmap->mdma, source);
  mdmaChannelSetDestinationX(mdmap->mdma, dest);
  mdmaChannelSetTransactionSizeX(mdmap->mdma, size, mdmap->cache.brc,
				 mdmap->cache.opt);
  mdmaChannelSetModeX(mdmap->mdma, mdmap->cache.ctcr, mdmap->cache.ccr);
  mdmaChannelSetTrigModeX(mdmap->mdma, mdmap->config->trigger_src);
  mdmaChannelSelectBuses(mdmap, mdmap->config->bus_selection);
  mdmaChannelSetCBRUR(mdmap, mdmap->cache.cbrur);
  mdmaChannelSetCLAR(mdmap, mdmap->cache.clar);
  mdmaChannelSetCMDR(mdmap, mdmap->config->mask_data_register);
  mdmaChannelSetCMAR(mdmap, (uint32_t) mdmap->config->mask_address_register);


  mdmaChannelEnableX(mdmap->mdma);
  
  return true;
}


void  mdma_lld_get_link_block(MDMADriver *mdmap, void *source, void *dest,
			      const size_t size, mdmalinkblock_t *link_block)
{
  mdmaChannelSetSourceX(mdmap->mdma, source);
  mdmaChannelSetDestinationX(mdmap->mdma, dest);
  mdmaChannelSetTransactionSizeX(mdmap->mdma, size, mdmap->cache.brc,
				 mdmap->cache.opt);
  mdmaChannelSetModeX(mdmap->mdma, mdmap->cache.ctcr, mdmap->cache.ccr);
  mdmaChannelSetTrigModeX(mdmap->mdma, mdmap->config->trigger_src);
  mdmaChannelSelectBuses(mdmap, mdmap->config->bus_selection);
  mdmaChannelSetCBRUR(mdmap, mdmap->cache.cbrur);
  mdmaChannelSetCLAR(mdmap, mdmap->cache.clar);
  mdmaChannelSetCMDR(mdmap, mdmap->config->mask_data_register);
  mdmaChannelSetCMAR(mdmap, (uint32_t) mdmap->config->mask_address_register);
  memcpy(link_block, (void *)&mdmap->mdma->channel->CTCR, sizeof(mdmalinkblock_t));
}				

/**
 * @brief   Stops an ongoing transaction.
 *
 * @param[in] mdmap      pointer to the @p MDMADriver object
 *
 * @notapi
 */
void mdma_lld_stop_transfert(MDMADriver *mdmap)
{
  mdmaChannelDisableX(mdmap->mdma);
}


/**
 * @brief   Shared service routine.
 *
 * @param[in] mdmap     pointer to the @p MDMADriver object
 * @param[in] flags     content of the CISR register
 */
static void mdma_lld_serve_interrupt(MDMADriver *mdmap, uint32_t flags) {

  (void)mdmap;
  (void)flags;
#if __DCACHE_PRESENT
  if (mdmap->config->activate_dcache_sync) {
    const size_t cacheSize =
      getCrossCacheBoundaryAwareSize(mdmap->destination, mdmap->size);
    cacheBufferInvalidate(mdmap->destination, cacheSize);
  }
#endif

  /* DMA errors handling.*/
  if ((flags & STM32_MDMA_CISR_TEIF) != 0) {
    if(mdmap->config->error_cb != NULL) {
      // TODO : different kind of errors ?
      _mdma_isr_error_code(mdmap, mdmaChannelGetErrorStatus(mdmap));
    }
  }
  
  if ((flags & STM32_MDMA_CISR_CTCIF) != 0) {
    /* Transfer complete processing.*/
    _mdma_isr_transaction_complete(mdmap);
   }
  
  if ((flags & STM32_MDMA_CISR_TCIF) != 0) {
    /*Buffer complete processing.*/
    mdmap->config->buffer_transfert_cb(mdmap);
   }
  
  if ((flags & STM32_MDMA_CISR_BTIF) != 0) {
    /* Block complete processing.*/
    mdmap->config->block_transfert_cb(mdmap);
   }
  
  if ((flags & STM32_MDMA_CISR_BRTIF) != 0) {
    /* Block Repeat complete processing.*/
    mdmap->config->block_transfert_repeat_cb(mdmap);
   }
}



static inline uint32_t mdmaChannelGetErrorStatus(MDMADriver *mdmap)
{
  return mdmap->mdma->channel->CESR;
}

static inline void mdmaChannelSetCBRUR(MDMADriver *mdmap, uint32_t cbrur)
{
  mdmap->mdma->channel->CBRUR =  cbrur;
}

static inline void mdmaChannelSetCLAR(MDMADriver *mdmap, uint32_t clar)
{
  mdmap->mdma->channel->CLAR =  clar;
}

static inline void mdmaChannelSetCMAR(MDMADriver *mdmap, uint32_t cmar)
{
  mdmap->mdma->channel->CMAR =  cmar;
}

static inline void mdmaChannelSetCMDR(MDMADriver *mdmap, uint32_t cmdr)
{
  mdmap->mdma->channel->CMDR =  cmdr;
}

static inline void mdmaChannelSelectBuses(MDMADriver *mdmap, uint32_t sourceDestBuses)
{
  mdmap->mdma->channel->CTBR |=  sourceDestBuses;
}
