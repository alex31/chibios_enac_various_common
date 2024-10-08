/*
    ChibiOS - Copyright (C) 2006..2016 Giovanni Di Sirio

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
 * @file    hal_stm32_dma.h
 * @brief   STM32 DMA subsystem driver header.
 *
 */

#pragma once

#include <ch.h>
#include <hal.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Enables synchronous APIs.
 * @note    Disabling this option saves both code and data space.
 */
#if !defined(STM32_DMA_USE_WAIT) || defined(__DOXYGEN__)
#define STM32_DMA_USE_WAIT                TRUE
#endif

/**
 * @brief   Enables user data in the callback
 * @note    Disabling this option saves data space.
 */
#if !defined(STM32_DMA_DRIVER_USER_DATA_FIELD) || defined(__DOXYGEN__)
#define STM32_DMA_DRIVER_USER_DATA_FIELD                FALSE
#endif
  
/**
 * @brief   Enables double buffer APIs.
 * @note    Disabling this option saves both code and data space.
 *          This option in only available on ADVANCED DMAv2 (F4, F7, H7)
 */
#if !defined(STM32_DMA_USE_DOUBLE_BUFFER) || defined(__DOXYGEN__)
#define STM32_DMA_USE_DOUBLE_BUFFER               FALSE
#endif


/**
 * @brief Enables the @p dmaAcquireBus() and @p dmaReleaseBus() APIs.
 * @note Disabling this option saves both code and data space.
 */
#if !defined(STM32_DMA_USE_MUTUAL_EXCLUSION) || defined(__DOXYGEN__)
#define STM32_DMA_USE_MUTUAL_EXCLUSION    FALSE
#endif

#if !defined(STM32_DMA_SUPPORTS_CSELR) || defined(__DOXYGEN__)
#define STM32_DMA_SUPPORTS_CSELR   FALSE
#endif

#if !defined(STM32_DMA_SUPPORTS_DMAMUX) || defined(__DOXYGEN__)
#define STM32_DMA_SUPPORTS_DMAMUX  FALSE
#endif

#if  !defined(STM32_DMA_DUMMY_MEMORY_AREA_ADDRESS) || defined(__DOXYGEN__)
#define  STM32_DMA_DUMMY_MEMORY_AREA_ADDRESS 0x80000000
#endif   

#if  !defined(__DCACHE_PRESENT) 
#define  __DCACHE_PRESENT FALSE
#endif   

/**
 * @brief   Driver state machine possible states.
 */
typedef enum {
  DMA_UNINIT = 0,                           /**< Not initialized.          */
  DMA_STOP = 1,                             /**< Stopped.                  */
  DMA_READY = 2,                            /**< Ready.                    */
  DMA_ACTIVE = 3,                           /**< Transfering.              */
  DMA_COMPLETE = 4,                         /**< Transfert complete.       */
  DMA_ERROR = 5                             /**< Transfert error.          */
} dmastate_t;

/**
 * @brief   Possible DMA failure causes.
 * @note    Error codes are architecture dependent and should not relied
 *          upon.
 */
typedef enum {
  DMA_ERR_TRANSFER_ERROR   = 1U << 0U,          /**< DMA transfer failure.         */
  DMA_ERR_DIRECTMODE_ERROR = 1U << 1U,          /**< DMA Direct Mode failure.      */
  DMA_ERR_FIFO_ERROR       = 1U << 2U,          /**< DMA FIFO error.  */
  DMA_ERR_FIFO_FULL        = 1U << 3U,          /**< DMA FIFO overrun */
  DMA_ERR_FIFO_EMPTY       = 1U << 4U           /**< DMA FIFO underrun. */
} dmaerrormask_t;

/**
 * @brief   DMA transfert direction
 */
typedef enum {
  DMA_DIR_P2M = 1,       /**< PERIPHERAL to MEMORY  */
  DMA_DIR_M2P,           /**< MEMORY to PERIPHERAL  */
  DMA_DIR_M2M            /**< MEMORY to MEMORY      */
} dmadirection_t;

  /**
 * @brief   DMA transfert memory mode
 */
typedef enum {
  DMA_ONESHOT = 1,			/**< One transert then stop  */
  DMA_CONTINUOUS_HALF_BUFFER,       /**< Continuous mode to/from the same buffer */
#if  STM32_DMA_USE_DOUBLE_BUFFER
  DMA_CONTINUOUS_DOUBLE_BUFFER     /**< Continuous mode to/from differents buffers */
#endif
} dmaopmode_t;

/**
 * @brief   Type of a structure representing an DMA driver.
 */
typedef struct DMADriver DMADriver;

/**
 * @brief   DMA notification callback type.
 *
 * @param[in] dmap      pointer to the @p DMADriver object triggering the
 *                      callback
 * @param[in] buffer    pointer to the most recent dma data
 * @param[in] n         number of buffer rows available starting from @p buffer
 */
typedef void (*dmacallback_t)(DMADriver *dmap, void *buffer, const size_t n);

/**
 * @brief   DMA next buffer query callback type.
 *
 * @param[in] dmap      pointer to the @p DMADriver object triggering the
 *                      callback
 * @param[in] n         number of buffer rows needed in the returned buffer pointer
 * @return              pointer to the next to be used dma buffer 
 */
typedef void * (*dmanextcallback_t)(DMADriver *dmap, const size_t n);


/**
 * @brief   DMA error callback type.
 *
 * @param[in] dmap      pointer to the @p DMADriver object triggering the
 *                      callback
 * @param[in] err       DMA error code
 */
typedef void (*dmaerrorcallback_t)(DMADriver *dmap, dmaerrormask_t err);



/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/
#if (STM32_DMA_USE_WAIT == TRUE) || defined(__DOXYGEN__)
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
#else /* !STM32_DMA_USE_WAIT */
#define _dma_reset_i(dmap)
#define _dma_reset_s(dmap)
#define _dma_wakeup_isr(dmap)
#define _dma_timeout_isr(dmap)
#endif /* !STM32_DMA_USE_WAIT */

/**
 * @brief   Common ISR code, half buffer event.
 * @details This code handles the portable part of the ISR code:
 *          - Callback invocation.
 *          .
 * @note    This macro is meant to be used in the low level drivers
 *          implementation only.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @notapi
 */
static inline void _dma_isr_half_code(DMADriver *dmap);


/**
 * @brief   Common ISR code, full buffer event.
 * @details This code handles the portable part of the ISR code:
 *          - Callback invocation.
 *          - Waiting thread wakeup, if any.
 *          - Driver state transitions.
 *          .
 * @note    This macro is meant to be used in the low level drivers
 *          implementation only.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @notapi
 */
static inline void _dma_isr_full_code(DMADriver *dmap);


/**
 * @brief   Common ISR code, error event.
 * @details This code handles the portable part of the ISR code:
 *          - Callback invocation.
 *          - Waiting thread timeout signaling, if any.
 *          - Driver state transitions.
 *          .
 * @note    This macro is meant to be used in the low level drivers
 *          implementation only.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 * @param[in] err       platform dependent error code
 *
 * @notapi
 */
static inline void _dma_isr_error_code(DMADriver *dmap, dmaerrormask_t err);




/**
 * @brief   DMA stream configuration structure.
 * @details This implementation-dependent structure describes a DMA
 *          operation.
 * @note    The use of this configuration structure requires knowledge of
 *          STM32 DMA registers interface, please refer to the STM32
 *          reference manual for details.
 */
typedef struct  {
  /**
   * @brief   stream associated with transaction
   * @note    use STM32_DMA_STREAM_ID macro
   */
  uint32_t		stream;
#if STM32_DMA_SUPPORTS_DMAMUX
  uint32_t		dmamux; // 4 bytes wide for mdma use
#else
#if    STM32_DMA_SUPPORTS_CSELR
  /**
   * @brief   DMA request or DMA channel
   * @note    terminology depend on DMA version
   */
  union {
    uint8_t		request; // STM terminology for dmaV1
    uint8_t		channel; // ChibiOS terminology for both dmaV1 and dmaV2 (portability)
  };
#  else
  uint8_t		channel;
#  endif
#endif
  /**
   * @brief   Enable increment of peripheral address after each transfert
   */
  bool			inc_peripheral_addr;


  /**
   * @brief   Enable increment of memory address after each transfert
   */
  bool			inc_memory_addr;


  /**
   * @brief   one shot, or circular half buffer, or circular double buffers
   */
  dmaopmode_t op_mode;


  /**
   * @brief   Callback function associated to the stream or @p NULL.
   */
  dmacallback_t         end_cb;

#if STM32_DMA_USE_DOUBLE_BUFFER
  /**
   * @brief   Next data buffer callback function associated to the stream or @p NULL.
   * @note    Mandatory in the DMA_CONTINUOUS_DOUBLE_BUFFER mode
   */
  dmanextcallback_t     next_cb;
#endif
  
  /**
   * @brief   Error callback or @p NULL.
   */
  dmaerrorcallback_t    error_cb;
#if STM32_DMA_USE_ASYNC_TIMOUT
  /**
   * @brief   callback function will be called after timeout if data is available
   * @note    experimental feature
   */
  sysinterval_t	timeout;
#endif


  /**
   * @brief   DMA transaction direction
   */
  dmadirection_t	direction;


  /**
   * @brief   DMA priority (0 .. 3) lowest to highest
   */
  uint8_t		dma_priority;

  /**
   * @brief   DMA IRQ priority (3 .. 15) highest to lowest
   */
  uint8_t		irq_priority;

  /**
   * @brief   DMA peripheral data granurality in bytes (1,2,4)
   */
  uint8_t		psize; // 1,2,4

  /**
   * @brief   DMA memory data granurality in bytes (1,2,4)
   */
  uint8_t		msize; // 1,2,4
#if __DCACHE_PRESENT
  /**
   * @brief   DMA memory is in a cached section and need to be flushed
   */
  union {
    bool		dcache_memory_in_use; // this name was hardly meaningfull
    bool		activate_dcache_sync;
  };
#endif
#if STM32_DMA_ADVANCED
#define STM32_DMA_FIFO_SIZE 16 // hardware specification for dma V2

  /**
   * @brief   DMA peripheral burst size
   */
  uint8_t		pburst; // 0(burst disabled), 4, 8, 16

  /**
   * @brief   DMA memory burst size
   */
  uint8_t		mburst; // 0(burst disabled), 4, 8, 16

  /**
   * @brief   DMA fifo level trigger
   */
  uint8_t		fifo;   // 0(fifo disabled), 1, 2, 3, 4 : 25, 50, 75, 100%

  /**
   * @brief   DMA enable 4 bytes increment independantly of psize
   */
  bool			periph_inc_size_4; // PINCOS bit

  /**
   * @brief   DMA enable peripheral as flow controller
   */
  bool			transfert_end_ctrl_by_periph; // PFCTRL bit
#endif
#if STM32_DMA_DRIVER_USER_DATA_FIELD
  void *user_data;
#endif
}  DMAConfig;


/**
 * @brief   Structure representing a DMA driver.
 */
struct DMADriver {
  /**
   * @brief   DMA stream associated with peripheral or memory
   */
  const stm32_dma_stream_t  *dmastream;

  /**
   * @brief Current configuration data.
   */
  const DMAConfig	    *config;

#if STM32_DMA_USE_WAIT || defined(__DOXYGEN__)
  /**
   * @brief Waiting thread.
   */
  thread_reference_t        thread;
#endif
#if STM32_DMA_USE_MUTUAL_EXCLUSION || defined(__DOXYGEN__)
  /**
   * @brief Mutex protecting the peripheral.
   */
  mutex_t                   mutex;
#endif /* STM32_DMA_USE_MUTUAL_EXCLUSION */
#if STM32_DMA_USE_ASYNC_TIMOUT
  /**
   * @brief manage double buffer as a circular buffer
   */
  uint8_t	     * volatile currPtr;

  /**
   * @brief virtual timer for calling end_cb between half and full ISR
   */
  virtual_timer_t	     vt;
#endif
  /**
   * @brief	memory address
   * @note	for now, only half buffer with one memory pointer is managed
   *            mem1p not yet interfaced
   */
  void			     * mem0p;

  /**
   * @brief	hold DMA CR register for the stream
   */
  uint32_t		     dmamode;

#if __DCACHE_PRESENT
  /**
   * @brief	periph address (or destination memory in case of M2M)
   */
  volatile void			     * periphp;
#endif

#if STM32_DMA_ADVANCED
  /**
   * @brief	hold DMA Fifo FCR register for the stream
   */
  uint32_t		     fifomode;
#endif

  /**
   * @brief	hold size of current transaction
   */
  size_t		     size;

#if STM32_DMA_USE_DOUBLE_BUFFER
  volatile uint32_t		     next_cb_errors;
#endif
  
#if CH_DBG_SYSTEM_STATE_CHECK
  volatile size_t		     nbTransferError;
  volatile size_t		     nbDirectModeError;
  volatile size_t		     nbFifoError;
  volatile size_t		     nbFifoFull;
  volatile size_t		     nbFifoEmpty;
  volatile dmaerrormask_t	     lastError;
#endif
  /**
   * @brief	Driver state
   */
  volatile dmastate_t		     state;


  /**
   * @brief	controller associated with stream
   */
#if STM32_DMA_SUPPORTS_DMAMUX == 0
  uint8_t		     controller;
#endif
};



void  dmaObjectInit(DMADriver *dmap);
bool  dmaStart(DMADriver *dmap, const DMAConfig *cfg);
bool  dmaReloadConf(DMADriver *dmap, const DMAConfig *cfg);
void  dmaStop(DMADriver *dmap);

#if STM32_DMA_USE_WAIT == TRUE
msg_t dmaTransfertTimeout(DMADriver *dmap, volatile void *periphp, void * mem0p, const size_t size,
		   sysinterval_t timeout);
// helper
static inline msg_t dmaTransfert(DMADriver *dmap, volatile void *periphp, void * mem0p, const size_t size)
{
  return dmaTransfertTimeout(dmap, periphp, mem0p, size, TIME_INFINITE);
}
#endif
#if STM32_DMA_USE_MUTUAL_EXCLUSION == TRUE
void dmaAcquireBus(DMADriver *dmap);
void dmaReleaseBus(DMADriver *dmap);
#endif
bool  dmaStartTransfert(DMADriver *dmap, volatile void *periphp, void * mem0p,
			const size_t size);
void  dmaStopTransfert(DMADriver *dmap);

bool  dmaStartTransfertI(DMADriver *dmap, volatile void *periphp, void *mem0p,
			 const size_t size);
void  dmaStopTransfertI(DMADriver *dmap);
uint8_t dmaGetStreamIndex(DMADriver *dmap);
#if defined DMA_request_TypeDef && defined DMA_Stream_TypeDef
void  dmaGetRegisters(DMADriver *dmap, volatile void *periphp, void *mem0p,
		      const size_t size,
		      DMA_Stream_TypeDef *registers);
#endif  
static  inline dmastate_t dmaGetState(DMADriver *dmap) {return dmap->state;}
static  inline size_t dmaGetTransactionCounter(DMADriver *dmap) {return dmaStreamGetTransactionSize(dmap->dmastream);}

#if  STM32_DMA_USE_DOUBLE_BUFFER
/**
 * @brief   get double buffer allocation errors counter
 *
 * @param[in] dmap      pointer to the @p DMADriver object triggering the
 *                      callback
 * @return		the number of allocation error since the last call to
 *			dmaClearNextErrors
 * @note		there is allocation error when nect_cb callback return NULL pointer
 */
static  inline dmastate_t dmaGetNextErrors(DMADriver *dmap) {return dmap->next_cb_errors;}
/**
 * @brief   clear double buffer allocation errors counter
 *
 * @param[in] dmap      pointer to the @p DMADriver object triggering the
 *                      callback
 */
static  inline void dmaClearNextErrors(DMADriver *dmap) {dmap->next_cb_errors = 0U;}
#endif

// low level driver

bool  dma_lld_start(DMADriver *dmap, bool allocate_stream);
void  dma_lld_stop(DMADriver *dmap);


bool  dma_lld_start_transfert(DMADriver *dmap, volatile void *periphp, void *mem0p, const size_t size);


void  dma_lld_stop_transfert(DMADriver *dmap);

#if STM32_DMA_USE_ASYNC_TIMOUT
void dma_lld_serve_timeout_interrupt(void *arg);
#endif

void* dma_lld_set_next_double_buffer(DMADriver *dmap, void *nextBuffer);

#if STM32_DMA_USE_ASYNC_TIMOUT
typedef enum {FROM_TIMOUT_CODE, FROM_HALF_CODE, FROM_FULL_CODE, FROM_NON_CIRCULAR_CODE} CbCallContext;
static inline void async_timout_enabled_call_end_cb(DMADriver *dmap, const CbCallContext context)
{
  uint8_t * const baseAddr = dmap->currPtr;
  const size_t fullSize = dmap->size;
  const size_t halfSize = fullSize / 2;
  size_t rem = 0;
  uint8_t * const basePtr = (uint8_t *) dmap->mem0p;
  uint8_t * const midPtr = ((uint8_t *) dmap->mem0p) + (dmap->config->msize * halfSize);
  uint8_t * const endPtr = ((uint8_t *) dmap->mem0p) + (dmap->config->msize * fullSize);


  switch (context) {
  case (FROM_HALF_CODE) :
    if (midPtr > baseAddr) {
      rem = (midPtr - baseAddr) / dmap->config->msize;
      dmap->currPtr = midPtr;
    }
    break;

  case (FROM_FULL_CODE) :
  case (FROM_NON_CIRCULAR_CODE) :
    rem = (endPtr - baseAddr) / dmap->config->msize;
    dmap->currPtr = basePtr;
    break;

  case (FROM_TIMOUT_CODE) : {
    const size_t dmaCNT = dmaStreamGetTransactionSize(dmap->dmastream);
    const size_t index = (baseAddr - basePtr) / dmap->config->msize;

    // if following test fail, it's because DMACNT has rollover during the ISR,
    // so that we can safely ignore this TIMOUT event since a fullcode ISR will follow
    // briefly
    if (fullSize >= (dmaCNT + index)) {
      rem = (fullSize - dmaCNT - index);
      dmap->currPtr = baseAddr + (rem * dmap->config->msize);
    }
  }
    break;
  }

  if (dmap->config->end_cb != NULL  && (rem > 0)) {
    dmap->config->end_cb(dmap, baseAddr, rem);
  }
}
#endif

static inline void _dma_isr_half_code(DMADriver *dmap) {
#if STM32_DMA_USE_ASYNC_TIMOUT
  if (dmap->config->timeout != TIME_INFINITE) {
    chSysLockFromISR();
    chVTSetI(&dmap->vt, dmap->config->timeout,
	     &dma_lld_serve_timeout_interrupt, (void *) dmap);
    chSysUnlockFromISR();
  }
  async_timout_enabled_call_end_cb(dmap, FROM_HALF_CODE);
#else
  if (dmap->config->end_cb != NULL) {
    dmap->config->end_cb(dmap, dmap->mem0p, dmap->size / 2);
  }
#endif
}

static inline void _dma_isr_full_code(DMADriver *dmap) {
  if (dmap->config->op_mode == DMA_CONTINUOUS_HALF_BUFFER) {
#if STM32_DMA_USE_ASYNC_TIMOUT
    if (dmap->config->timeout != TIME_INFINITE) {
      chSysLockFromISR();
      chVTSetI(&dmap->vt, dmap->config->timeout,
	       &dma_lld_serve_timeout_interrupt, (void *) dmap);
      chSysUnlockFromISR();
    }
    async_timout_enabled_call_end_cb(dmap, FROM_FULL_CODE);
#else
    /* Callback handling.*/
    if (dmap->config->end_cb != NULL) {
      if (dmap->size > 1) {
        /* Invokes the callback passing the 2nd half of the buffer.*/
        const size_t half_index = dmap->size / 2;
	const uint8_t *byte_array_p = ((uint8_t *) dmap->mem0p) +
	  dmap->config->msize * half_index;
        dmap->config->end_cb(dmap, (void *) byte_array_p, half_index);
      } else {
        /* Invokes the callback passing the whole buffer.*/
        dmap->config->end_cb(dmap, dmap->mem0p, dmap->size);
      }
    }
#endif
  }
  else if (dmap->config->op_mode == DMA_ONESHOT) {  // not circular
    /* End transfert.*/
#if STM32_DMA_USE_ASYNC_TIMOUT
    if (dmap->config->timeout != TIME_INFINITE) {
      chSysLockFromISR();
      chVTResetI(&dmap->vt);
      chSysUnlockFromISR();
    }
#endif
    dma_lld_stop_transfert(dmap);
    if (dmap->config->end_cb != NULL) {
      dmap->state = DMA_COMPLETE;
      /* Invoke the callback passing the whole buffer.*/
#if STM32_DMA_USE_ASYNC_TIMOUT
      async_timout_enabled_call_end_cb(dmap, FROM_NON_CIRCULAR_CODE);
#else
      dmap->config->end_cb(dmap, dmap->mem0p, dmap->size);
#endif
      if (dmap->state == DMA_COMPLETE) {
        dmap->state = DMA_READY;
      }
    } else {
      dmap->state = DMA_READY;
    }
    _dma_wakeup_isr(dmap);
  }
#if  STM32_DMA_USE_DOUBLE_BUFFER
  else { // CONTINUOUS_DOUBLE_BUFFER
    /* Next buffer handling */
    void* const rawNextBuff =  dmap->config->next_cb(dmap, dmap->size);
    if (rawNextBuff == NULL) 
      dmap->next_cb_errors++;
    void* const nextBuff = rawNextBuff ? rawNextBuff : (void *) STM32_DMA_DUMMY_MEMORY_AREA_ADDRESS;
    void* const memXp = dma_lld_set_next_double_buffer(dmap, nextBuff);
    /* Callback handling.*/
    if ((dmap->config->end_cb != NULL) &&
	(memXp != (void *) STM32_DMA_DUMMY_MEMORY_AREA_ADDRESS)){
      dmap->config->end_cb(dmap, memXp, dmap->size);
    }
  }
#endif
}

static inline void _dma_isr_error_code(DMADriver *dmap, dmaerrormask_t err) {
#if CH_DBG_SYSTEM_STATE_CHECK == TRUE
  if (err & DMA_ERR_TRANSFER_ERROR)
    dmap->nbTransferError = dmap->nbTransferError + 1;
  if (err & DMA_ERR_DIRECTMODE_ERROR)
    dmap->nbDirectModeError = dmap->nbDirectModeError + 1;
  if (err & DMA_ERR_FIFO_ERROR) {
    dmap->nbFifoError= dmap->nbFifoError + 1;
    if (err & DMA_ERR_FIFO_FULL)
       dmap->nbFifoFull = dmap->nbFifoFull + 1;
    if (err & DMA_ERR_FIFO_EMPTY)
       dmap->nbFifoEmpty = dmap->nbFifoEmpty + 1;
  }
  dmap->lastError = err;
#endif
  if (err & (DMA_ERR_TRANSFER_ERROR | DMA_ERR_DIRECTMODE_ERROR))
    dma_lld_stop_transfert(dmap);
  else
    return;

  if (dmap->config->error_cb != NULL) {
    dmap->state = DMA_ERROR;
    dmap->config->error_cb(dmap, err);
    if (dmap->state == DMA_ERROR)
      dmap->state = DMA_READY;
  } else {
    dmap->state = DMA_READY;
  }
  _dma_timeout_isr(dmap);
}
#if defined DMA_request_TypeDef && defined DMA_Stream_TypeDef
void  dma_lld_get_registers(DMADriver *dmap, volatile void *periphp,
			    void *mem0p, const size_t size,
			    DMA_Stream_TypeDef *registers);
#endif
#ifdef __cplusplus
}
#endif
