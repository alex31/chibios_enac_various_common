#include "hal_dma.h"


/*
TODO : 

° simplifier le code de test des combinaisons interdites
  + test si mode = M2M : le champ periph_addr doit être null (pas utilisé)
° appliquer les autres limitations sur les tailles données dans le chapitre dma du ref manuel

° booleen pour les masques d'interruption pour les 4 bit enable du registre CR, 
  et aussi le bit FEIE du registre FCR

° version timeout des fonctions synchrones

° transformer les macro en fonction static inline

° écrire code de test : 
  * api synchrone
  * argument des callback end et erreur
  * transfert mémoire
  * transfert vers un gpio (ou un bit bitband d'un gpio) cadencé par un timer
  * transfert mémoire vers timer (voir code driver WS2812)

 */


static void dma_lld_serve_interrupt(DMADriver *dmap, uint32_t flags);


void dmaObjectInit(DMADriver *dmap)
{
  osalDbgCheck(dmap != NULL);
  
  dmap->state    = DMA_STOP;
  dmap->config   = NULL;
  dmap->thread   = NULL;
  dmap->destp     = NULL;
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
  dmap->destp   = NULL;

  osalSysUnlock();
}



bool dmaStartMtransfert(DMADriver *dmap, void *from, void *to, const size_t size)
{
  osalSysLock();
  const bool statusOk = dmaStartMtransfertI(dmap, from, to, size);
  osalSysUnlock();
  return statusOk;
}

bool dmaStartMtransfertI(DMADriver *dmap, void *from, void *to, const size_t size)
{
  osalDbgCheckClassI();
  osalDbgCheck((dmap != NULL) && (from != NULL) && (to != NULL) &&
               (size > 0U) && ((size == 1U) || ((size & 1U) == 0U)));
  osalDbgAssert((dmap->state == DMA_READY) ||
                (dmap->state == DMA_COMPLETE) ||
                (dmap->state == DMA_ERROR),
                "not ready");

  return dma_lld_start_mtransfert(dmap, from, to, size);
}
  

bool dmaStartPtransfert(DMADriver *dmap, void *membuffer, const size_t size)
{
  osalSysLock();
  const bool statusOk = dmaStartPtransfertI(dmap, membuffer, size);
  osalSysUnlock();
  return statusOk;
}

bool dmaStartPtransfertI(DMADriver *dmap, void *membuffer, const size_t size)
{
  osalDbgCheckClassI();
  osalDbgCheck((dmap != NULL) && (membuffer != NULL) && 
               (size > 0U) && ((size == 1U) || ((size & 1U) == 0U)));
  osalDbgAssert((dmap->state == DMA_READY) ||
                (dmap->state == DMA_COMPLETE) ||
                (dmap->state == DMA_ERROR),
                "not ready");

  return dma_lld_start_ptransfert(dmap, membuffer, size);
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



void dmaStoptransfertI(DMADriver *dmap)
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


msg_t dmaPtransfert(DMADriver *dmap, void *membuffer, const size_t size)
{
  msg_t msg;

  osalSysLock();
  osalDbgAssert(dmap->thread == NULL, "already waiting");
  dmaStartPtransfertI(dmap, membuffer, size);
  msg = osalThreadSuspendS(&dmap->thread);
  osalSysUnlock();
  return msg;
}

msg_t dmaMtransfert(DMADriver *dmap, void *from, void *to, const size_t size)
{
  msg_t msg;
  
  osalSysLock();
  osalDbgAssert(dmap->thread == NULL, "already waiting");
  dmaStartMtransfertI(dmap, from, to, size);
  msg = osalThreadSuspendS(&dmap->thread);
  osalSysUnlock();
  return msg;

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
  
  dmap->dmastream =  STM32_DMA_STREAM(STM32_DMA_STREAM_ID(cfg->controller, cfg->stream));

  dmap->dmamode = STM32_DMA_CR_CHSEL(cfg->channel) |
		 STM32_DMA_CR_PL(cfg->dma_priority) |
		 cfg->direction |
		 psize_msk | msize_msk |
                 cfg->isr_flags |
                 (cfg->circular ? STM32_DMA_CR_CIRC : 0UL) |
                 (cfg->inc_peripheral_addr ? STM32_DMA_CR_PINC : 0UL) |
	         (cfg->inc_memory_addr ? STM32_DMA_CR_MINC : 0UL);
                 

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

#if STM32_DMA_ADVANCED
  dmap->dmamode |= (pburst_msk | mburst_msk);
#endif
  
  /*
    When burst transfers are requested on the peripheral AHB port and the FIFO is used
    (DMDIS = 1 in the DMA_SxCR register), it is mandatory to respect the following rule to
    avoid permanent underrun or overrun conditions, depending on the DMA stream direction:
    If (PBURST × PSIZE) = FIFO_SIZE (4 words), FIFO_Threshold = 3/4 is forbidden
  */
  if ( ((cfg->pburst * cfg->psize) == DMA_FIFO_SIZE) && (cfg->fifo == 3))
    goto forbiddenCombination;
  
#endif
    
  const bool error = dmaStreamAllocate( dmap->dmastream,
					cfg->irq_priority,
					(stm32_dmaisr_t) &dma_lld_serve_interrupt,
					(void *) dmap );
  if (error) {
    osalDbgAssert(false, "stream already allocated");
    return false;
  }
  
  if (cfg->fifo) {
    dmaStreamSetFIFO(dmap->dmastream, STM32_DMA_FCR_DMDIS | fifo_msk);
  }
  
  dmaStreamSetPeripheral(dmap->dmastream, cfg->periph_addr);
  
  return true;
  
  
 forbiddenCombination:
  chSysHalt("forbidden combination of msize, mburst, fifo, see FIFO threshold "
	    "configuration in reference manuel");
  return false;
}


bool dma_lld_start_ptransfert(DMADriver *dmap, void *membuffer, const size_t size)
{
  if (dmap->config->direction == STM32_DMA_CR_DIR_M2M) {
    osalDbgAssert(false, "dma_lld_start_ptransfert not compatible with config direction field");
    return false;
  }
  
  dmap->destp = membuffer;
  dmap->size = size;
  dmaStreamSetMemory0(dmap->dmastream, membuffer);
  dmaStreamSetTransactionSize(dmap->dmastream, size);
  dmaStreamSetMode(dmap->dmastream, dmap->dmamode);
  dmaStreamEnable(dmap->dmastream);
  
  return true;
}

bool dma_lld_start_mtransfert(DMADriver *dmap, void *from, void *to, const size_t size)
{
  if (dmap->config->direction != STM32_DMA_CR_DIR_M2M) {
    osalDbgAssert(false, "dma_lld_start_mtransfert not compatible with config direction field");
    return false;
  }
  
  dmap->destp = to;
  dmap->size = size;
  dmaStreamSetPeripheral(dmap->dmastream, from);
  dmaStreamSetMemory0(dmap->dmastream, to);
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
  if ((flags & (STM32_DMA_ISR_TEIF | STM32_DMA_ISR_DMEIF)) != 0) {
    /* DMA, this could help only if the DMA tries to access an unmapped
       address space or violates alignment rules.*/
    const dmaerrormask_t err =
      ( (flags & STM32_DMA_ISR_TEIF) ? DMA_ERR_TRANSFER_ERROR : 0UL) |
      ( (flags & STM32_DMA_ISR_DMEIF) ? DMA_ERR_DIRECTMODE_ERROR : 0UL);
      
    _dma_isr_error_code(dmap, err);
  }
  else {
    /* It is possible that the conversion group has already be reset by the
       DMA error handler, in this case this interrupt is spurious.*/
    if (dmap->config != NULL) {

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
