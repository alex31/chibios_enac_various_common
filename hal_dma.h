#pragma once

#include <ch.h>
#include <hal.h>

typedef enum {
  DMA_UNINIT = 0,                           /**< Not initialized.          */
  DMA_STOP = 1,                             /**< Stopped.                  */
  DMA_READY = 2,                            /**< Ready.                    */
  DMA_ACTIVE = 3,                           /**< Transfering.              */
  DMA_COMPLETE = 4,                         /**< Transfert complete.       */
  DMA_ERROR = 5                             /**< Transfert error.          */
} dmastate_t;

typedef enum {
  DMA_ERR_TRANSFER_ERROR   = 1<<0,          /**< DMA transfer failure.         */
  DMA_ERR_DIRECTMODE_ERROR = 1<<1,          /**< DMA Direct Mode failure.      */
  DMA_ERR_FIFO_ERROR       = 1<<2           /**< DMA FIFO overrun or underrun. */
} dmaerrormask_t;

typedef enum {
  DMA_DIR_P2M = 1,           /**< PERIPHERAL to MEMORY  */
  DMA_DIR_M2P = 2,           /**< MEMORY to PERIPHERAL  */
  DMA_DIR_M2M = 3,           /**< MEMORY to MEMORY      */
  DMA_DIR_P2P = 4		 /**< PERIPHERAL to PERIPHERAL  */
} dmadirection_t;

typedef struct DMADriver DMADriver;

typedef void (*dmacallback_t)(DMADriver *dmap, void *buffer, const size_t n);
typedef void (*dmaerrorcallback_t)(DMADriver *dmap, dmaerrormask_t err);



/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @name    Low level driver helper macros
 * @{
 */

/**
 * @brief   Resumes a thread waiting for a dma transfert completion.
 *
 * @param[in] dmap      pointer to the @p DMADriver object
 *
 * @notapi
 */
#define _dma_reset_i(dmap)                                                  \
  osalThreadResumeI(&(dmap)->thread, MSG_RESET)

/**
 * @brief   Resumes a thread waiting for a dma transfert completion.
 *
 * @param[in] dmap      pointer to the @p DMADriver object
 *
 * @notapi
 */
#define _dma_reset_s(dmap)                                                  \
  osalThreadResumeS(&(dmap)->thread, MSG_RESET)

/**
 * @brief   Wakes up the waiting thread.
 *
 * @param[in] dmap      pointer to the @p DMADriver object
 *
 * @notapi
 */
#define _dma_wakeup_isr(dmap) {                                             \
  osalSysLockFromISR();                                                     \
  osalThreadResumeI(&(dmap)->thread, MSG_OK);                               \
  osalSysUnlockFromISR();                                                   \
}

/**
 * @brief   Wakes up the waiting thread with a timeout message.
 *
 * @param[in] dmap      pointer to the @p DMADriver object
 *
 * @notapi
 */
#define _dma_timeout_isr(dmap) {                                            \
  osalSysLockFromISR();                                                     \
  osalThreadResumeI(&(dmap)->thread, MSG_TIMEOUT);                          \
  osalSysUnlockFromISR();                                                   \
}

#define _dma_isr_half_code(dmap) {                                          \
  if ((dmap)->config->end_cb != NULL) {                                     \
    (dmap)->config->end_cb(dmap, (dmap)->mem0p, (dmap)->size / 2);          \
  }                                                                         \
}

#define _dma_isr_full_code(dmap) {                                          \
  if ((dmap)->config->circular) {                                           \
    /* Callback handling.*/                                                 \
    if ((dmap)->config->end_cb != NULL) {                                   \
      if ((dmap)->size > 1) {                                               \
        /* Invokes the callback passing the 2nd half of the buffer.*/       \
        const size_t half_index = (dmap)->size / 2;                         \
	const uint8_t *byte_array_p = ((uint8_t *) (dmap)->mem0p) +         \
	  dmap->config->msize * half_index;				    \
        (dmap)->config->end_cb(dmap, (void *) byte_array_p, half_index);    \
      }                                                                     \
      else {                                                                \
        /* Invokes the callback passing the whole buffer.*/                 \
        (dmap)->config->end_cb(dmap, (dmap)->mem0p, (dmap)->size);          \
      }                                                                     \
    }                                                                       \
  }                                                                         \
  else {                                                                    \
    /* End transfert.*/                                                     \
    dma_lld_stop_transfert(dmap);                                           \
    if ((dmap)->config->end_cb != NULL) {                                   \
      (dmap)->state = DMA_COMPLETE;                                         \
      /* Invoke the callback passing the whole buffer.*/                    \
      (dmap)->config->end_cb(dmap, (dmap)->mem0p, (dmap)->size);            \
      if ((dmap)->state == DMA_COMPLETE) {                                  \
        (dmap)->state = DMA_READY;                                          \
      }                                                                     \
    }                                                                       \
    else {                                                                  \
      (dmap)->state = DMA_READY;                                            \
    }                                                                       \
    _dma_wakeup_isr(dmap);                                                  \
  }                                                                         \
}


#define _dma_isr_error_code(dmap, err) {                                    \
  dma_lld_stop_transfert(dmap);                                             \
  if ((dmap)->config->error_cb != NULL) {                                   \
    (dmap)->state = DMA_ERROR;                                              \
    (dmap)->config->error_cb(dmap, err);                                    \
    if ((dmap)->state == DMA_ERROR)                                         \
      (dmap)->state = DMA_READY;                                            \
  }                                                                         \
  else {                                                                    \
    (dmap)->state = DMA_READY;                                              \
  }                                                                         \
  _dma_timeout_isr(dmap);                                                   \
}





typedef struct  {
  dmadirection_t	direction; 
  bool			inc_peripheral_addr;
  bool			inc_memory_addr;
  bool			circular;
  bool			periph_inc_size_4; // PINCOS bit
  bool			transfert_end_ctrl_by_periph; // PFCTRL bit
  //uint32_t		isr_flags; // combination of STM32_DMA_CR_[TCIE, HTIE, DMEIE, TEIE]
				   // transfert complete, half transfert, direct mode error,
				   // transfert error

  /**
   * @brief   Callback function associated to the group or @p NULL.
   */
  dmacallback_t         end_cb;
  /**
   * @brief   Error callback or @p NULL.
   */
  dmaerrorcallback_t    error_cb;
  
  uint8_t		controller;
  uint8_t		stream;
  uint8_t		channel;
  uint8_t		dma_priority;
  uint8_t		irq_priority;
  uint8_t		psize; // 1,2,4
  uint8_t		msize; // 1,2,4
  
#if STM32_DMA_ADVANCED
#define DMA_FIFO_SIZE 4 // hardware specification for dma V2
  uint8_t		pburst; // 0(burst disabled), 4, 8, 16  
  uint8_t		mburst; // 0(burst disabled), 4, 8, 16 
  uint8_t		fifo;   // 0(fifo disabled), 1, 2, 3, 4 : 25, 50, 75, 100% 
#endif
}  DMAConfig ;

struct DMADriver {
  const stm32_dma_stream_t  *dmastream;
  uint32_t		     dmamode;
  dmastate_t		     state;
  thread_reference_t         thread;
  void			     *mem0p;
  size_t		     size;
  const DMAConfig	    *config;
}  ;



void  dmaObjectInit(DMADriver *dmap);
bool  dmaStart(DMADriver *dmap, const DMAConfig *cfg);
void  dmaStop(DMADriver *dmap);

msg_t dmaTransfert(DMADriver *dmap, volatile void *periphp, void *mem0p, const size_t size);
bool  dmaStartTransfert(DMADriver *dmap, volatile void *periphp, void *mem0p, const size_t size);
void  dmaStoptransfert(DMADriver *dmap);

bool dmaStartTransfertI(DMADriver *dmap, volatile void *periphp, void *mem0p, const size_t size);
void dmaStoptransfertI(DMADriver *dmap);


// low level driver
			
bool dma_lld_start(DMADriver *dmap);
void dma_lld_stop(DMADriver *dmap);


bool dma_lld_start_transfert(DMADriver *dmap, volatile void *periphp, void *mem0p, const size_t size);


void dma_lld_stop_transfert(DMADriver *dmap);
