/**
 * @file    hal_stm32_bdma.h
 * @brief   STM32 BDMA subsystem driver header.
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
#if !defined(STM32_BDMA_USE_WAIT) || defined(__DOXYGEN__)
#define STM32_BDMA_USE_WAIT                TRUE
#endif

/**
 * @brief   Enables user data in the callback
 * @note    Disabling this option saves data space.
 */
#if !defined(STM32_BDMA_DRIVER_USER_DATA_FIELD) || defined(__DOXYGEN__)
#define STM32_BDMA_DRIVER_USER_DATA_FIELD                FALSE
#endif
  
/**
 * @brief   Enables double buffer APIs.
 * @note    Disabling this option saves both code and data space.
 */
#if !defined(STM32_BDMA_USE_DOUBLE_BUFFER) || defined(__DOXYGEN__)
#define STM32_BDMA_USE_DOUBLE_BUFFER               FALSE
#endif


/**
 * @brief Enables the @p bdmaAcquireBus() and @p bdmaReleaseBus() APIs.
 * @note Disabling this option saves both code and data space.
 */
#if !defined(STM32_BDMA_USE_MUTUAL_EXCLUSION) || defined(__DOXYGEN__)
#define STM32_BDMA_USE_MUTUAL_EXCLUSION    FALSE
#endif

#if  !defined(STM32_BDMA_DUMMY_MEMORY_AREA_ADDRESS) || defined(__DOXYGEN__)
#define  STM32_BDMA_DUMMY_MEMORY_AREA_ADDRESS 0x80000000
#endif   

#if  !defined(__DCACHE_PRESENT) 
#define  __DCACHE_PRESENT FALSE
#endif   

/**
 * @brief   Driver state machine possible states.
 */
typedef enum {
  BDMA_UNINIT = 0,                           /**< Not initialized.          */
  BDMA_STOP = 1,                             /**< Stopped.                  */
  BDMA_READY = 2,                            /**< Ready.                    */
  BDMA_ACTIVE = 3,                           /**< Transfering.              */
  BDMA_COMPLETE = 4,                         /**< Transfert complete.       */
  BDMA_ERROR = 5                             /**< Transfert error.          */
} bdmastate_t;

/**
 * @brief   Possible BDMA failure causes.
 * @note    Error codes are architecture dependent and should not relied
 *          upon.
 */
typedef enum {
  BDMA_ERR_TRANSFER_ERROR   = 1U << 0U,          /**< BDMA transfer failure.         */
} bdmaerrormask_t;

/**
 * @brief   BDMA transfert direction
 */
typedef enum {
  BDMA_DIR_P2M = 1,       /**< PERIPHERAL to MEMORY  */
  BDMA_DIR_M2P,           /**< MEMORY to PERIPHERAL  */
  BDMA_DIR_M2M            /**< MEMORY to MEMORY      */
} bdmadirection_t;

  /**
 * @brief   BDMA transfert memory mode
 */
typedef enum {
  BDMA_ONESHOT = 1,			/**< One transert then stop  */
  BDMA_CONTINUOUS_HALF_BUFFER,       /**< Continuous mode to/from the same buffer */
  BDMA_CONTINUOUS_DOUBLE_BUFFER     /**< Continuous mode to/from differents buffers */
} bdmaopmode_t;

/**
 * @brief   Type of a structure representing an BDMA driver.
 */
typedef struct BDMADriver BDMADriver;

/**
 * @brief   BDMA notification callback type.
 *
 * @param[in] bdmap      pointer to the @p BDMADriver object triggering the
 *                      callback
 * @param[in] buffer    pointer to the most recent bdma data
 * @param[in] n         number of buffer rows available starting from @p buffer
 */
typedef void (*bdmacallback_t)(BDMADriver *bdmap, void *buffer, const size_t n);

/**
 * @brief   BDMA next buffer query callback type.
 *
 * @param[in] bdmap      pointer to the @p BDMADriver object triggering the
 *                      callback
 * @param[in] n         number of buffer rows needed in the returned buffer pointer
 * @return              pointer to the next to be used bdma buffer 
 */
typedef void * (*bdmanextcallback_t)(BDMADriver *bdmap, const size_t n);


/**
 * @brief   BDMA error callback type.
 *
 * @param[in] bdmap      pointer to the @p BDMADriver object triggering the
 *                      callback
 * @param[in] err       BDMA error code
 */
typedef void (*bdmaerrorcallback_t)(BDMADriver *bdmap, bdmaerrormask_t err);



/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/
#if (STM32_BDMA_USE_WAIT == TRUE) || defined(__DOXYGEN__)
/**
 * @name    Low level driver helper macros
 * @{
 */

/**
 * @brief   Resumes a thread waiting for a bdma transfert completion.
 *
 * @param[in] bdmap      pointer to the @p BDMADriver object
 *
 * @notapi
 */
#define _bdma_reset_i(bdmap)                                                  \
  osalThreadResumeI(&(bdmap)->thread, MSG_RESET)

/**
 * @brief   Resumes a thread waiting for a bdma transfert completion.
 *
 * @param[in] bdmap      pointer to the @p BDMADriver object
 *
 * @notapi
 */
#define _bdma_reset_s(bdmap)                                                  \
  osalThreadResumeS(&(bdmap)->thread, MSG_RESET)

/**
 * @brief   Wakes up the waiting thread.
 *
 * @param[in] bdmap      pointer to the @p BDMADriver object
 *
 * @notapi
 */
#define _bdma_wakeup_isr(bdmap) {                                             \
  osalSysLockFromISR();                                                     \
  osalThreadResumeI(&(bdmap)->thread, MSG_OK);                               \
  osalSysUnlockFromISR();                                                   \
}

/**
 * @brief   Wakes up the waiting thread with a timeout message.
 *
 * @param[in] bdmap      pointer to the @p BDMADriver object
 *
 * @notapi
 */
#define _bdma_timeout_isr(bdmap) {                                            \
  osalSysLockFromISR();                                                     \
  osalThreadResumeI(&(bdmap)->thread, MSG_TIMEOUT);                          \
  osalSysUnlockFromISR();                                                   \
}
#else /* !STM32_BDMA_USE_WAIT */
#define _bdma_reset_i(bdmap)
#define _bdma_reset_s(bdmap)
#define _bdma_wakeup_isr(bdmap)
#define _bdma_timeout_isr(bdmap)
#endif /* !STM32_BDMA_USE_WAIT */

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
static inline void _bdma_isr_half_code(BDMADriver *bdmap);


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
static inline void _bdma_isr_full_code(BDMADriver *bdmap);


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
static inline void _bdma_isr_error_code(BDMADriver *bdmap, bdmaerrormask_t err);




/**
 * @brief   BDMA stream configuration structure.
 * @details This implementation-dependent structure describes a BDMA
 *          operation.
 * @note    The use of this configuration structure requires knowledge of
 *          STM32 BDMA registers interface, please refer to the STM32
 *          reference manual for details.
 */
typedef struct  {
  /**
   * @brief   stream associated with transaction
   * @note    use STM32_BDMA_STREAM_ID macro
   */
  uint32_t		stream;
  uint32_t		dmamux2; // 4 bytes wide for mdma use

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
  bdmaopmode_t op_mode;


  /**
   * @brief   Callback function associated to the stream or @p NULL.
   */
  bdmacallback_t         end_cb;

#if STM32_BDMA_USE_DOUBLE_BUFFER
  /**
   * @brief   Next data buffer callback function associated to the stream or @p NULL.
   * @note    Mandatory in the BDMA_CONTINUOUS_DOUBLE_BUFFER mode
   */
  bdmanextcallback_t     next_cb;
#endif
  
  /**
   * @brief   Error callback or @p NULL.
   */
  bdmaerrorcallback_t    error_cb;
#if STM32_BDMA_USE_ASYNC_TIMOUT
  /**
   * @brief   callback function will be called after timeout if data is available
   * @note    experimental feature
   */
  sysinterval_t	timeout;
#endif


  /**
   * @brief   BDMA transaction direction
   */
  bdmadirection_t	direction;


  /**
   * @brief   BDMA priority (1 .. 4)
   */
  uint8_t		bdma_priority;

  /**
   * @brief   BDMA IRQ priority (2 .. 7)
   */
  uint8_t		irq_priority;

  /**
   * @brief   BDMA peripheral data granurality in bytes (1,2,4)
   */
  uint8_t		psize; // 1,2,4

  /**
   * @brief   BDMA memory data granurality in bytes (1,2,4)
   */
  uint8_t		msize; // 1,2,4
#if __DCACHE_PRESENT
  /**
   * @brief   BDMA memory is in a cached section and need to be flushed
   */
  union {
    bool		dcache_memory_in_use; // this name was hardly meaningfull
    bool		activate_dcache_sync;
  };
#endif
#if STM32_BDMA_DRIVER_USER_DATA_FIELD
  void *user_data;
#endif
}  BDMAConfig;


/**
 * @brief   Structure representing a BDMA driver.
 */
struct BDMADriver {
  /**
   * @brief   BDMA stream associated with peripheral or memory
   */
  const stm32_bdma_stream_t  *bdmastream;

  /**
   * @brief Current configuration data.
   */
  const BDMAConfig	    *config;

#if STM32_BDMA_USE_WAIT || defined(__DOXYGEN__)
  /**
   * @brief Waiting thread.
   */
  thread_reference_t        thread;
#endif
#if STM32_BDMA_USE_MUTUAL_EXCLUSION || defined(__DOXYGEN__)
  /**
   * @brief Mutex protecting the peripheral.
   */
  mutex_t                   mutex;
#endif /* STM32_BDMA_USE_MUTUAL_EXCLUSION */
#if STM32_BDMA_USE_ASYNC_TIMOUT
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
   * @brief	hold BDMA CR register for the stream
   */
  uint32_t		     bdmamode;

#if __DCACHE_PRESENT
  /**
   * @brief	periph address (or destination memory in case of M2M)
   */
  volatile void			     * periphp;
#endif


  /**
   * @brief	hold size of current transaction
   */
  size_t		     size;

#if STM32_BDMA_USE_DOUBLE_BUFFER
  volatile uint32_t		     next_cb_errors;
#endif
  
#if CH_DBG_SYSTEM_STATE_CHECK
  volatile size_t		     nbTransferError;
  volatile bdmaerrormask_t	     lastError;
#endif
  /**
   * @brief	Driver state
   */
  volatile bdmastate_t		     state;
};



void  bdmaObjectInit(BDMADriver *bdmap);
bool  bdmaStart(BDMADriver *bdmap, const BDMAConfig *cfg);
void  bdmaStop(BDMADriver *bdmap);

#if STM32_BDMA_USE_WAIT == TRUE
msg_t bdmaTransfertTimeout(BDMADriver *bdmap, volatile void *periphp, void * mem0p, const size_t size,
		   sysinterval_t timeout);
// helper
static inline msg_t bdmaTransfert(BDMADriver *bdmap, volatile void *periphp, void * mem0p, const size_t size)
{
  return bdmaTransfertTimeout(bdmap, periphp, mem0p, size, TIME_INFINITE);
}
#endif
#if STM32_BDMA_USE_MUTUAL_EXCLUSION == TRUE
void bdmaAcquireBus(BDMADriver *bdmap);
void bdmaReleaseBus(BDMADriver *bdmap);
#endif
bool  bdmaStartTransfert(BDMADriver *bdmap, volatile void *periphp, void * mem0p, const size_t size);
void  bdmaStopTransfert(BDMADriver *bdmap);

bool  bdmaStartTransfertI(BDMADriver *bdmap, volatile void *periphp, void *mem0p, const size_t size);
void  bdmaStopTransfertI(BDMADriver *bdmap);

static  inline bdmastate_t bdmaGetState(BDMADriver *bdmap) {return bdmap->state;}

#if  STM32_BDMA_USE_DOUBLE_BUFFER
/**
 * @brief   get double buffer allocation errors counter
 *
 * @param[in] bdmap      pointer to the @p BDMADriver object triggering the
 *                      callback
 * @return		the number of allocation error since the last call to
 *			bdmaClearNextErrors
 * @note		there is allocation error when nect_cb callback return NULL pointer
 */
static  inline bdmastate_t bdmaGetNextErrors(BDMADriver *bdmap) {return bdmap->next_cb_errors;}
/**
 * @brief   clear double buffer allocation errors counter
 *
 * @param[in] bdmap      pointer to the @p BDMADriver object triggering the
 *                      callback
 */
static  inline void bdmaClearNextErrors(BDMADriver *bdmap) {bdmap->next_cb_errors = 0U;}
#endif

// low level driver

bool  bdma_lld_start(BDMADriver *bdmap);
void  bdma_lld_stop(BDMADriver *bdmap);


bool  bdma_lld_start_transfert(BDMADriver *bdmap, volatile void *periphp, void *mem0p, const size_t size);


void  bdma_lld_stop_transfert(BDMADriver *bdmap);

#if STM32_BDMA_USE_ASYNC_TIMOUT
void bdma_lld_serve_timeout_interrupt(void *arg);
#endif

void* bdma_lld_set_next_double_buffer(BDMADriver *bdmap, void *nextBuffer);

#if STM32_BDMA_USE_ASYNC_TIMOUT
typedef enum {FROM_TIMOUT_CODE, FROM_HALF_CODE, FROM_FULL_CODE, FROM_NON_CIRCULAR_CODE} CbCallContext;
static inline void async_timout_enabled_call_end_cb(BDMADriver *bdmap, const CbCallContext context)
{
  uint8_t * const baseAddr = bdmap->currPtr;
  const size_t fullSize = bdmap->size;
  const size_t halfSize = fullSize / 2;
  size_t rem = 0;
  uint8_t * const basePtr = (uint8_t *) bdmap->mem0p;
  uint8_t * const midPtr = ((uint8_t *) bdmap->mem0p) + (bdmap->config->msize * halfSize);
  uint8_t * const endPtr = ((uint8_t *) bdmap->mem0p) + (bdmap->config->msize * fullSize);


  switch (context) {
  case (FROM_HALF_CODE) :
    if (midPtr > baseAddr) {
      rem = (midPtr - baseAddr) / bdmap->config->msize;
      bdmap->currPtr = midPtr;
    }
    break;

  case (FROM_FULL_CODE) :
  case (FROM_NON_CIRCULAR_CODE) :
    rem = (endPtr - baseAddr) / bdmap->config->msize;
    bdmap->currPtr = basePtr;
    break;

  case (FROM_TIMOUT_CODE) : {
    const size_t bdmaCNT = bdmaStreamGetTransactionSize(bdmap->bdmastream);
    const size_t index = (baseAddr - basePtr) / bdmap->config->msize;

    // if following test fail, it's because BDMACNT has rollover during the ISR,
    // so that we can safely ignore this TIMOUT event since a fullcode ISR will follow
    // briefly
    if (fullSize >= (bdmaCNT + index)) {
      rem = (fullSize - bdmaCNT - index);
      bdmap->currPtr = baseAddr + (rem * bdmap->config->msize);
    }
  }
    break;
  }

  if (bdmap->config->end_cb != NULL  && (rem > 0)) {
    bdmap->config->end_cb(bdmap, baseAddr, rem);
  }
}
#endif

static inline void _bdma_isr_half_code(BDMADriver *bdmap) {
#if STM32_BDMA_USE_ASYNC_TIMOUT
  if (bdmap->config->timeout != TIME_INFINITE) {
    chSysLockFromISR();
    chVTSetI(&bdmap->vt, bdmap->config->timeout,
	     &bdma_lld_serve_timeout_interrupt, (void *) bdmap);
    chSysUnlockFromISR();
  }
  async_timout_enabled_call_end_cb(bdmap, FROM_HALF_CODE);
#else
  if (bdmap->config->end_cb != NULL) {
    bdmap->config->end_cb(bdmap, bdmap->mem0p, bdmap->size / 2);
  }
#endif
}

static inline void _bdma_isr_full_code(BDMADriver *bdmap) {
  if (bdmap->config->op_mode == BDMA_CONTINUOUS_HALF_BUFFER) {
#if STM32_BDMA_USE_ASYNC_TIMOUT
    if (bdmap->config->timeout != TIME_INFINITE) {
      chSysLockFromISR();
      chVTSetI(&bdmap->vt, bdmap->config->timeout,
	       &bdma_lld_serve_timeout_interrupt, (void *) bdmap);
      chSysUnlockFromISR();
    }
    async_timout_enabled_call_end_cb(bdmap, FROM_FULL_CODE);
#else
    /* Callback handling.*/
    if (bdmap->config->end_cb != NULL) {
      if (bdmap->size > 1) {
        /* Invokes the callback passing the 2nd half of the buffer.*/
        const size_t half_index = bdmap->size / 2;
	const uint8_t *byte_array_p = ((uint8_t *) bdmap->mem0p) +
	  bdmap->config->msize * half_index;
        bdmap->config->end_cb(bdmap, (void *) byte_array_p, half_index);
      } else {
        /* Invokes the callback passing the whole buffer.*/
        bdmap->config->end_cb(bdmap, bdmap->mem0p, bdmap->size);
      }
    }
#endif
  }
  else if (bdmap->config->op_mode == BDMA_ONESHOT) {  // not circular
    /* End transfert.*/
#if STM32_BDMA_USE_ASYNC_TIMOUT
    if (bdmap->config->timeout != TIME_INFINITE) {
      chSysLockFromISR();
      chVTResetI(&bdmap->vt);
      chSysUnlockFromISR();
    }
#endif
    bdma_lld_stop_transfert(bdmap);
    if (bdmap->config->end_cb != NULL) {
      bdmap->state = BDMA_COMPLETE;
      /* Invoke the callback passing the whole buffer.*/
#if STM32_BDMA_USE_ASYNC_TIMOUT
      async_timout_enabled_call_end_cb(bdmap, FROM_NON_CIRCULAR_CODE);
#else
      bdmap->config->end_cb(bdmap, bdmap->mem0p, bdmap->size);
#endif
      if (bdmap->state == BDMA_COMPLETE) {
        bdmap->state = BDMA_READY;
      }
    } else {
      bdmap->state = BDMA_READY;
    }
    _bdma_wakeup_isr(bdmap);
  }
#if  STM32_BDMA_USE_DOUBLE_BUFFER
  else { // CONTINUOUS_DOUBLE_BUFFER
    /* Next buffer handling */
    void* const rawNextBuff =  bdmap->config->next_cb(bdmap, bdmap->size);
    if (rawNextBuff == NULL) 
      bdmap->next_cb_errors++;
    void* const nextBuff = rawNextBuff ? rawNextBuff : (void *) STM32_BDMA_DUMMY_MEMORY_AREA_ADDRESS;
    void* const memXp = bdma_lld_set_next_double_buffer(bdmap, nextBuff);
    /* Callback handling.*/
    if ((bdmap->config->end_cb != NULL) &&
	(memXp != (void *) STM32_BDMA_DUMMY_MEMORY_AREA_ADDRESS)){
      bdmap->config->end_cb(bdmap, memXp, bdmap->size);
    }
  }
#endif
}

static inline void _bdma_isr_error_code(BDMADriver *bdmap, bdmaerrormask_t err) {
#if CH_DBG_SYSTEM_STATE_CHECK == TRUE
  if (err & BDMA_ERR_TRANSFER_ERROR)
    bdmap->nbTransferError++;
  bdmap->lastError = err;
#endif
  if (err & (BDMA_ERR_TRANSFER_ERROR))
    bdma_lld_stop_transfert(bdmap);
  else
    return;

  if (bdmap->config->error_cb != NULL) {
    bdmap->state = BDMA_ERROR;
    bdmap->config->error_cb(bdmap, err);
    if (bdmap->state == BDMA_ERROR)
      bdmap->state = BDMA_READY;
  } else {
    bdmap->state = BDMA_READY;
  }
  _bdma_timeout_isr(bdmap);
}


#ifdef __cplusplus
}
#endif
