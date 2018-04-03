#include "hal_stm32_dma.h"


/*
TODO : 


° en mode asynchrone, proposer une option qui utilise un timer virtuel pour appeler la fonction de callback 
  même si le buffer (demi buffer) n'est pas rempli. 
  A la prochaine completude du buffer (ou demi buffer) :
  + appeler la fonction avec 
	° le pointeur de debut de buffer sur le premier nouvel element arrivé 
          depuis le dernier appel à la fonction de cb
        ° le nombre d'elements nouveaux depuis le dernier appel
  
  + armer le timer au lancement de la transaction
  + quand une ISR HTIF ou CTIF arrive, si mode continu : rearmer le timer, sinon(one shot) desarmer le timer
  
  + une simple fonction de cb qui copie n dans une variable globale 
    permet en mode synchrone d'utiliser cette fonctionnalité sans alourdir le driver



° doxygen doc

° separer en deux paires de fichier : hal_stm32_dma et hal_lld_stm32_dma

° portage H7 : bdma, dmav3, mdma
  
*/


static void dma_lld_serve_interrupt(DMADriver *dmap, uint32_t flags);


void dmaObjectInit(DMADriver *dmap)
{
  osalDbgCheck(dmap != NULL);
  
  dmap->state    = DMA_STOP;
  dmap->config   = NULL;
  dmap->mem0p    = NULL;
#if STM32_DMA_USE_WAIT == TRUE
  dmap->thread   = NULL;
#endif
#if STM32_DMA_USE_MUTUAL_EXCLUSION == TRUE
  osalMutexObjectInit(&dmap->mutex);
#endif
#if defined( STM32_DMA_DRIVER_EXT_INIT_HOOK)
   STM32_DMA_DRIVER_EXT_INIT_HOOK(dmap);
#endif
}


bool dmaStart(DMADriver *dmap, const DMAConfig *cfg)
{
  osalDbgCheck((dmap != NULL) && (cfg != NULL));

  osalSysLock();
  osalDbgAssert((dmap->state == DMA_STOP) || (dmap->state == DMA_READY),
                "invalid state");
  dmap->config = cfg;
  const bool statusOk = dma_lld_start(dmap);
  dmap->state = DMA_READY;
  osalSysUnlock();
  return statusOk;
}


void dmaStop(DMADriver *dmap)
{
  osalDbgCheck(dmap != NULL);

  osalSysLock();

  osalDbgAssert((dmap->state == DMA_STOP) || (dmap->state == DMA_READY),
                "invalid state");

  dma_lld_stop(dmap);
  dmap->config = NULL;
  dmap->state  = DMA_STOP;
  dmap->mem0p   = NULL;

  osalSysUnlock();
}



bool dmaStartTransfert(DMADriver *dmap, volatile void *periphp, void *mem0p, const size_t size)
{
  osalSysLock();
  const bool statusOk = dmaStartTransfertI(dmap, periphp, mem0p, size);
  osalSysUnlock();
  return statusOk;
}

bool dmaStartTransfertI(DMADriver *dmap, volatile void *periphp, void *mem0p, const size_t size)
{
  osalDbgCheckClassI();
  osalDbgCheck((dmap != NULL) && (mem0p != NULL) && (periphp != NULL) &&
               (size > 0U) && ((size == 1U) || ((size & 1U) == 0U)));

#if (CH_DBG_ENABLE_ASSERTS != FALSE)
  const DMAConfig	    *cfg = dmap->config;
  osalDbgAssert((dmap->state == DMA_READY) ||
                (dmap->state == DMA_COMPLETE) ||
                (dmap->state == DMA_ERROR),
                "not ready");

  osalDbgAssert((uint32_t) periphp % cfg->psize == 0, "peripheral address not aligned");
  osalDbgAssert((uint32_t) mem0p % cfg->msize == 0, "memory address not aligned");

  /*
    In the circular mode, it is mandatory to respect the following rule in case of a burst mode
    configured for memory:
    DMA_SxNDTR = Multiple of ((Mburst beat) × (Msize)/(Psize)), where:
    – (Mburst beat) = 4, 8 or 16 (depending on the MBURST bits in the DMA_SxCR
    register)
    – ((Msize)/(Psize)) = 1, 2, 4, 1/2 or 1/4 (Msize and Psize represent the MSIZE and
    PSIZE bits in the DMA_SxCR register. They are byte dependent)
    – DMA_SxNDTR = Number of data items to transfer on the AHB peripheral port
   */
# if  STM32_DMA_ADVANCED
  if (cfg->pburst) {
    osalDbgAssert((size % (cfg->pburst * cfg->psize)) == 0,
		  "pburst alignment rule not respected");
    osalDbgAssert((((uint32_t) periphp) % (cfg->pburst * cfg->psize)) == 0,
		  "peripheral address alignment rule not respected");
  }
  if (cfg->mburst) {
    osalDbgAssert((size % (cfg->mburst * cfg->msize)) == 0,
		  "mburst alignment rule not respected");
    osalDbgAssert((((uint32_t) mem0p) % (cfg->mburst * cfg->msize)) == 0,
		  "memory address alignment rule not respected");
  }
  
# endif
#endif
  dmap->state    = DMA_ACTIVE;
  return dma_lld_start_transfert(dmap, periphp, mem0p, size);
}
  

  
void dmaStopTransfert(DMADriver *dmap)
{

  osalDbgCheck(dmap != NULL);

  osalSysLock();
  osalDbgAssert((dmap->state == DMA_READY) || (dmap->state == DMA_ACTIVE),
                "invalid state");
  if (dmap->state != DMA_READY) {
    dma_lld_stop_transfert(dmap);
    dmap->state = DMA_READY;
    _dma_reset_s(dmap);
  }
  osalSysUnlock();
}



void dmaStopTransfertI(DMADriver *dmap)
{
  osalDbgCheckClassI();
  osalDbgCheck(dmap != NULL);
  osalDbgAssert((dmap->state == DMA_READY) ||
                (dmap->state == DMA_ACTIVE) ||
                (dmap->state == DMA_COMPLETE),
                "invalid state");
  

  if (dmap->state != DMA_READY) {
    dma_lld_stop_transfert(dmap);
    dmap->state = DMA_READY;
    _dma_reset_i(dmap);
  }

}

#if (STM32_DMA_USE_WAIT == TRUE) || defined(__DOXYGEN__)

msg_t dmaTransfertTimeout(DMADriver *dmap, volatile void *periphp, void *mem0p, const size_t size,
			  sysinterval_t timeout)
{
  msg_t msg;
  
  osalSysLock();
  osalDbgAssert(dmap->thread == NULL, "already waiting");
  dmaStartTransfertI(dmap, periphp, mem0p, size);
  msg = osalThreadSuspendTimeoutS(&dmap->thread, timeout);
  if (msg != MSG_OK) {
    dmaStopTransfertI(dmap);
  }
  osalSysUnlock();
  return msg;
}
#endif

#if (STM32_DMA_USE_MUTUAL_EXCLUSION == TRUE) || defined(__DOXYGEN__)
/**
 * @brief   Gains exclusive access to the DMA peripheral.
 * @details This function tries to gain ownership to the DMA bus, if the bus
 *          is already being used then the invoking thread is queued.
 * @pre     In order to use this function the option
 *          @p DMA_USE_MUTUAL_EXCLUSION must be enabled.
 *
 * @param[in] dmap      pointer to the @p DMADriver object
 *
 * @api
 */
void dmaAcquireBus(DMADriver *dmap) {

  osalDbgCheck(dmap != NULL);

  osalMutexLock(&dmap->mutex);
}

/**
 * @brief   Releases exclusive access to the DMA peripheral.
 * @pre     In order to use this function the option
 *          @p DMA_USE_MUTUAL_EXCLUSION must be enabled.
 *
 * @param[in] dmap      pointer to the @p DMADriver object
 *
 * @api
 */
void dmaReleaseBus(DMADriver *dmap) {

  osalDbgCheck(dmap != NULL);

  osalMutexUnlock(&dmap->mutex);
}
#endif /* DMA_USE_MUTUAL_EXCLUSION == TRUE */



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


bool dma_lld_start(DMADriver *dmap)
{
  uint32_t psize_msk, msize_msk;

  const DMAConfig *cfg = dmap->config;
				      
  switch (cfg->psize) {
  case 1 : psize_msk = STM32_DMA_CR_PSIZE_BYTE; break;
  case 2 : psize_msk = STM32_DMA_CR_PSIZE_HWORD; break;
  case 4 : psize_msk = STM32_DMA_CR_PSIZE_WORD; break;
  default: osalSysHalt("psize should be 1 or 2 or 4");
    return false;
  }
  switch (cfg->msize) {
  case 1 : msize_msk = STM32_DMA_CR_MSIZE_BYTE; break;
  case 2 : msize_msk = STM32_DMA_CR_MSIZE_HWORD; break;
  case 4 : msize_msk = STM32_DMA_CR_MSIZE_WORD; break;
  default: osalDbgAssert(false, "msize should be 1 or 2 or 4");
    return false;
  }

  uint32_t dir_msk=0UL;
  switch (cfg->direction) {
  case DMA_DIR_P2M: dir_msk=STM32_DMA_CR_DIR_P2M; break;
  case DMA_DIR_M2P: dir_msk=STM32_DMA_CR_DIR_M2P; break;
  case DMA_DIR_M2M: dir_msk=STM32_DMA_CR_DIR_M2M; break;
  default: osalDbgAssert(false, "direction not set or incorrect"); 
  }

  uint32_t isr_flags = cfg->circular ? 0UL: STM32_DMA_CR_TCIE;

  if (cfg->direction != DMA_DIR_M2M) {
    if (cfg->end_cb) {
      isr_flags |=STM32_DMA_CR_TCIE;
      if (cfg->circular) {
	isr_flags |= STM32_DMA_CR_HTIE;
      } 
    }
  }
  
  if (cfg->error_cb) {
    isr_flags |= STM32_DMA_CR_DMEIE | STM32_DMA_CR_TCIE;
  }


  dmap->dmastream =  STM32_DMA_STREAM(cfg->stream);

  // portable way (V1, V2) to retreive controler number
#if STM32_DMA_ADVANCED
  dmap->controller = 1 + (cfg->stream / STM32_DMA_STREAM_ID(2,0));
#else
  dmap->controller = 1 + (cfg->stream / STM32_DMA_STREAM_ID(2,1));
#endif

  dmap->dmamode = STM32_DMA_CR_PL(cfg->dma_priority) |
		 dir_msk | psize_msk | msize_msk | isr_flags |
                 (cfg->circular ? STM32_DMA_CR_CIRC : 0UL) |
                 (cfg->inc_peripheral_addr ? STM32_DMA_CR_PINC : 0UL) |
	         (cfg->inc_memory_addr ? STM32_DMA_CR_MINC : 0UL)

#if STM32_DMA_SUPPORTS_CSELR
                  | STM32_DMA_CR_CHSEL(cfg->request)    
#elif STM32_DMA_ADVANCED
                 | STM32_DMA_CR_CHSEL(cfg->channel)    
		 | (cfg->periph_inc_size_4 ? STM32_DMA_CR_PINCOS : 0UL) |
		   (cfg->transfert_end_ctrl_by_periph? STM32_DMA_CR_PFCTRL : 0UL)
#   endif
		 ;
                 

#if STM32_DMA_ADVANCED
  uint32_t  pburst_msk, mburst_msk, fifo_msk; // STM32_DMA_CR_PBURST_INCRx, STM32_DMA_CR_MBURST_INCRx
  switch (cfg->pburst) {
  case 0 : pburst_msk = 0UL; break;
  case 4 : pburst_msk  = STM32_DMA_CR_PBURST_INCR4; break;
  case 8 : pburst_msk  = STM32_DMA_CR_PBURST_INCR8; break;
  case 16 : pburst_msk = STM32_DMA_CR_PBURST_INCR16; break;
  default: osalDbgAssert(false, "pburst size should be 0 or 4 or 8 or 16");
    return false;
  }
  switch (cfg->mburst) {
  case 0 : mburst_msk = 0UL; break;
  case 4 : mburst_msk  = STM32_DMA_CR_MBURST_INCR4; break;
  case 8 : mburst_msk  = STM32_DMA_CR_MBURST_INCR8; break;
  case 16 : mburst_msk = STM32_DMA_CR_MBURST_INCR16; break;
  default: osalDbgAssert(false, "mburst size should be 0 or 4 or 8 or 16");
    return false;
  }
  switch (cfg->fifo) {
  case 0 : fifo_msk = 0UL; break;
  case 1 : fifo_msk = STM32_DMA_FCR_FTH_1Q; break;
  case 2 : fifo_msk  = STM32_DMA_FCR_FTH_HALF; break;
  case 3 : fifo_msk  = STM32_DMA_FCR_FTH_3Q;  break;
  case 4 : fifo_msk =  STM32_DMA_FCR_FTH_FULL; ; break;
  default: osalDbgAssert(false, "fifo threshold should be 1(/4) or 2(/4) or 3(/4) or 4(/4)");
    return false;
  }


# if (CH_DBG_ENABLE_ASSERTS != FALSE)
  // lot of combination of parameters are forbiden, and some conditions must be meet
  if (!cfg->msize !=  !cfg->psize) {
     osalDbgAssert(false, "psize and msize should be enabled or disabled together");
     return false;
  }

  if (cfg->fifo) {
    switch (cfg->msize) {
    case 1:
      switch (cfg->mburst) {
      case 4 :
	switch (cfg->fifo) {
	case 1: break;
	case 2: break;
	case 3: break;
	case 4: break;
	}
	break;
      case 8 :
	switch (cfg->fifo) {
	case 1: goto forbiddenCombination;
	case 2: break;
	case 3: goto forbiddenCombination;
	case 4: break;
	}
	break;
      case 16 :
	switch (cfg->fifo) {
	case 1: goto forbiddenCombination;
	case 2: goto forbiddenCombination;
	case 3: goto forbiddenCombination;
	case 4: break;
	}
	break;
      }
      break;
    case 2:
      switch (cfg->mburst) {
      case 4 :
	switch (cfg->fifo) {
	case 1: goto forbiddenCombination;
	case 2: break;
	case 3: goto forbiddenCombination;
	case 4: break;
	}
	break;
      case 8 :
	switch (cfg->fifo) {
	case 1: goto forbiddenCombination;
	case 2: goto forbiddenCombination;
	case 3: goto forbiddenCombination;
	case 4: break;
	}
	break;
      case 16 :
	switch (cfg->fifo) {
	case 1: goto forbiddenCombination;
	case 2: goto forbiddenCombination;
	case 3: goto forbiddenCombination;
	case 4: goto forbiddenCombination;
	}
      }
      break;
    case 4:
      switch (cfg->mburst) {
      case 4 :
	switch (cfg->fifo) {
	case 1: goto forbiddenCombination;
	case 2: goto forbiddenCombination;
	case 3: goto forbiddenCombination;
	case 4: break;
	}
	break;
      case 8 :
	switch (cfg->fifo) {
	case 1: goto forbiddenCombination;
	case 2: goto forbiddenCombination;
	case 3: goto forbiddenCombination;
	case 4: goto forbiddenCombination;
	}
	break;
      case 16 :
	switch (cfg->fifo) {
	case 1: goto forbiddenCombination;
	case 2: goto forbiddenCombination;
	case 3: goto forbiddenCombination;
	case 4: goto forbiddenCombination;
	}
      }
    }
  }
# endif

  dmap->dmamode |= (pburst_msk | mburst_msk);

#  if (CH_DBG_ENABLE_ASSERTS != FALSE)

  
  /*
    When burst transfers are requested on the peripheral AHB port and the FIFO is used
    (DMDIS = 1 in the DMA_SxCR register), it is mandatory to respect the following rule to
    avoid permanent underrun or overrun conditions, depending on the DMA stream direction:
    If (PBURST × PSIZE) = FIFO_SIZE (4 words), FIFO_Threshold = 3/4 is forbidden
  */

  if ( ((cfg->pburst * cfg->psize) == STM32_DMA_FIFO_SIZE) && (cfg->fifo == 3))
    goto forbiddenCombination;

  /*
    When memory-to-memory mode is used, the Circular and direct modes are not allowed.
    Only the DMA2 controller is able to perform memory-to-memory transfers.
  */
  
  if (cfg->direction == DMA_DIR_M2M) {
    osalDbgAssert(dmap->controller == 2, "M2M not available on DMA1");
    osalDbgAssert(cfg->circular == false, "M2M not available in circular mode");
  }


#  endif
#endif
    
  const bool error = dmaStreamAllocate( dmap->dmastream,
					cfg->irq_priority,
					(stm32_dmaisr_t) &dma_lld_serve_interrupt,
					(void *) dmap );
  if (error) {
    osalDbgAssert(false, "stream already allocated");
    return false;
  }

#if STM32_DMA_ADVANCED
  if (cfg->fifo) {
    dmaStreamSetFIFO(dmap->dmastream, STM32_DMA_FCR_DMDIS | STM32_DMA_FCR_FEIE | fifo_msk);
  } else {
    osalDbgAssert(cfg->direction != DMA_DIR_M2M, "fifo mode mandatory for M2M");
    osalDbgAssert(cfg->psize == cfg->msize, "msize == psize is mandatory when fifo is disabled");
  }
#endif

  
  
  return true;
  
#if (CH_DBG_ENABLE_ASSERTS != FALSE)
#if STM32_DMA_ADVANCED
 forbiddenCombination:
  chSysHalt("forbidden combination of msize, mburst, fifo, see FIFO threshold "
	    "configuration in reference manuel");
  return false;
# endif
#endif
}


bool dma_lld_start_transfert(DMADriver *dmap, volatile void *periphp, void *mem0p, const size_t size)
{
  dmap->mem0p = mem0p;
  dmap->size = size;
  dmaStreamSetPeripheral(dmap->dmastream, periphp);
  dmaStreamSetMemory0(dmap->dmastream, mem0p);
  dmaStreamSetTransactionSize(dmap->dmastream, size);
  dmaStreamSetMode(dmap->dmastream, dmap->dmamode);
  dmaStreamEnable(dmap->dmastream);
  
  return true;
}

void dma_lld_stop_transfert(DMADriver *dmap)
{
  dmaStreamDisable(dmap->dmastream);
}

void dma_lld_stop(DMADriver *dmap)
{
  dmaStreamRelease(dmap->dmastream);
}


/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   DMA DMA ISR service routine.
 *
 * @param[in] dmap      pointer to the @p DMADriver object
 * @param[in] flags     pre-shifted content of the ISR register
 */
static void dma_lld_serve_interrupt(DMADriver *dmap, uint32_t flags)
{

  /* DMA errors handling.*/
  if ((flags & (STM32_DMA_ISR_TEIF | STM32_DMA_ISR_DMEIF | STM32_DMA_ISR_FEIF)) != 0) {
    /* DMA, this could help only if the DMA tries to access an unmapped
       address space or violates alignment rules.*/
    const dmaerrormask_t err =
      ( (flags & STM32_DMA_ISR_TEIF)  ? DMA_ERR_TRANSFER_ERROR : 0UL) |
      ( (flags & STM32_DMA_ISR_DMEIF) ? DMA_ERR_DIRECTMODE_ERROR : 0UL) |
      ( (flags & STM32_DMA_ISR_FEIF)  ? DMA_ERR_FIFO_ERROR : 0UL);
    
    _dma_isr_error_code(dmap, err);
  }
  else {
    /* It is possible that the conversion group has already be reset by the
       DMA error handler, in this case this interrupt is spurious.*/
    if (dmap->state == DMA_ACTIVE) {

      if ((flags & STM32_DMA_ISR_TCIF) != 0) {
        /* Transfer complete processing.*/
        _dma_isr_full_code(dmap);
      }
      else if ((flags & STM32_DMA_ISR_HTIF) != 0) {
        /* Half transfer processing.*/
        _dma_isr_half_code(dmap);
      }
    }
  }
}
