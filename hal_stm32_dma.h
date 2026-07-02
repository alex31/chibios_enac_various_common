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
 * @details
 * ChibiOS-like helper driver wrapping STM32 DMA stream allocation, DMAMUX
 * selection, one-shot transfers and optional circular callbacks.
 */

#pragma once

#include <ch.h>
#include <hal.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup STM32_DMA_HELPER STM32 DMA Helper Driver
 * @brief    ChibiOS-like DMA stream wrapper used by project-local drivers.
 *
 * @details
 * This module provides a small driver object around STM32 DMA streams. A user
 * allocates a @ref DMADriver object, initializes it with @ref dmaObjectInit(),
 * starts it with a @ref DMAConfig using @ref dmaStart(), then submits transfers
 * with @ref dmaStartTransfert() or @ref dmaTransfert().
 *
 * The API is intentionally close to ChibiOS driver conventions but it is not a
 * replacement for the official ChibiOS STM32 DMA allocator. It is used by
 * low-level local drivers such as the frame DAC and FMAC FIR drivers when they
 * need an explicit one-shot or circular DMA transaction.
 *
 * Common one-shot sequence:
 * - create a static @ref DMADriver object;
 * - call @ref dmaObjectInit();
 * - fill @ref DMAConfig with stream/DMAMUX, direction, data widths and
 *   callbacks;
 * - call @ref dmaStart();
 * - call @ref dmaStartTransfert();
 * - wait for the callback or use @ref dmaTransfert() when the synchronous API
 *   is enabled;
 * - call @ref dmaStopTransfert() and @ref dmaStop() when the stream is no
 *   longer needed.
 *
 * @warning
 * Function names keep the historical spelling "Transfert" for API
 * compatibility.
 *
 * @api
 * @{
 */

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

#if !defined(STM32_DMA_USE_ASYNC_TIMOUT) || defined(__DOXYGEN__)
#define STM32_DMA_USE_ASYNC_TIMOUT                FALSE
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
 * @brief   DMA transfer operating mode.
 *
 * @details
 * @ref DMA_ONESHOT completes after one buffer. Circular modes keep the stream
 * running and invoke callbacks as buffer regions become available.
 */
typedef enum {
  DMA_ONESHOT = 1,             /**< One transfer then stop. */
  DMA_CONTINUOUS_HALF_BUFFER,  /**< Circular transfer on one buffer. */
#if  STM32_DMA_USE_DOUBLE_BUFFER
  DMA_CONTINUOUS_DOUBLE_BUFFER /**< Circular transfer over callback-provided buffers. */
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
 *
 * @note
 * The callback is called from ISR context by the low-level DMA interrupt
 * handler. Use ChibiOS ISR-safe primitives and keep the callback short.
 */
typedef void (*dmacallback_t)(DMADriver *dmap, void *buffer, const size_t n);

/**
 * @brief   DMA next buffer query callback type.
 *
 * @param[in] dmap      pointer to the @p DMADriver object triggering the
 *                      callback
 * @param[in] n         number of buffer rows needed in the returned buffer pointer
 * @return              pointer to the next to be used dma buffer 
 *
 * @note
 * Used only by @ref DMA_CONTINUOUS_DOUBLE_BUFFER.
 */
typedef void * (*dmanextcallback_t)(DMADriver *dmap, const size_t n);


/**
 * @brief   DMA error callback type.
 *
 * @param[in] dmap      pointer to the @p DMADriver object triggering the
 *                      callback
 * @param[in] err       DMA error code
 *
 * @note
 * The callback is called from ISR context.
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
 *
 * The most common project configuration for a peripheral transfer is:
 * - @c stream set to @c STM32_DMA_STREAM_ID_ANY or to a fixed stream;
 * - @c dmamux or @c channel set to the peripheral request;
 * - @c inc_peripheral_addr set to @p false;
 * - @c inc_memory_addr set to @p true;
 * - @c op_mode set to @ref DMA_ONESHOT;
 * - @c direction set to @ref DMA_DIR_M2P or @ref DMA_DIR_P2M;
 * - @c psize and @c msize set to the peripheral and memory access width in
 *   bytes.
 *
 * @note
 * This structure still exposes STM32-specific fields. Refer to the MCU
 * reference manual for stream/request availability and to the low-level driver
 * assertions for alignment constraints.
 */
typedef struct  {
  /**
   * @brief   Stream associated with transaction.
   * @note    Use @c STM32_DMA_STREAM_ID() or @c STM32_DMA_STREAM_ID_ANY.
   */
  uint32_t		stream;
#if STM32_DMA_SUPPORTS_DMAMUX
  /**
   * @brief   DMAMUX request selector.
   * @note    Four bytes wide for compatibility with MDMA-related code.
   */
  uint32_t		dmamux;
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
   * @brief   Enables peripheral address increment after each transfer item.
   */
  bool			inc_peripheral_addr;


  /**
   * @brief   Enables memory address increment after each transfer item.
   */
  bool			inc_memory_addr;


  /**
   * @brief   One-shot, circular half-buffer or circular double-buffer mode.
   */
  dmaopmode_t op_mode;


  /**
   * @brief   Callback function associated to the stream or @p NULL.
   * @details Called on transfer completion in one-shot mode, and on each
   *          half/full region in circular half-buffer mode.
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
   * @brief   DMA transaction direction.
   */
  dmadirection_t	direction;


  /**
   * @brief   DMA priority, 0 lowest to 3 highest on STM32 DMA v1/v2.
   */
  uint8_t		dma_priority;

  /**
   * @brief   DMA IRQ priority using ChibiOS/NVIC priority numbering.
   */
  uint8_t		irq_priority;

  /**
   * @brief   DMA peripheral data granularity in bytes, normally 1, 2 or 4.
   */
  uint8_t		psize;

  /**
   * @brief   DMA memory data granularity in bytes, normally 1, 2 or 4.
   */
  uint8_t		msize;
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
   * @brief   DMA peripheral burst size.
   */
  uint8_t		pburst; /**< 0 disables bursts, otherwise 4, 8 or 16. */

  /**
   * @brief   DMA memory burst size.
   */
  uint8_t		mburst; /**< 0 disables bursts, otherwise 4, 8 or 16. */

  /**
   * @brief   DMA FIFO level trigger.
   */
  uint8_t		fifo;   /**< 0 disabled, 1..4 means 25, 50, 75, 100%. */

  /**
   * @brief   Enables 4-byte peripheral increments independently from @c psize.
   */
  bool			periph_inc_size_4;

  /**
   * @brief   Enables peripheral flow controller mode.
   */
  bool			transfert_end_ctrl_by_periph;
#endif
#if STM32_DMA_DRIVER_USER_DATA_FIELD
  void *user_data;
#endif
}  DMAConfig;


/**
 * @brief   Structure representing a DMA driver.
 *
 * @details
 * Applications normally allocate this object statically and pass its address to
 * every DMA API call. Its public fields are kept visible because this helper is
 * close to the STM32 low-level layer, but application code should treat them as
 * driver-owned state after @ref dmaObjectInit().
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



/**
 * @brief   Initializes a DMA driver object.
 *
 * @param[out] dmap  Driver object.
 *
 * @post
 * The driver is in @ref DMA_STOP.
 *
 * @api
 */
void dmaObjectInit(DMADriver *dmap);

/**
 * @brief   Allocates/configures a DMA stream and puts the driver in ready state.
 *
 * @param[in,out] dmap  Driver object.
 * @param[in] cfg       DMA configuration, kept referenced by the driver.
 * @retval true         Configuration accepted.
 * @retval false        Stream allocation or configuration failed.
 *
 * @pre
 * @p dmap must be in @ref DMA_STOP or @ref DMA_READY.
 *
 * @note
 * The configuration object must remain valid until @ref dmaStop().
 *
 * @api
 */
bool dmaStart(DMADriver *dmap, const DMAConfig *cfg);

/**
 * @brief   Reloads the DMA stream configuration without reallocating it.
 *
 * @param[in,out] dmap  Driver object in @ref DMA_READY.
 * @param[in] cfg       New DMA configuration.
 * @retval true         Configuration accepted.
 * @retval false        Configuration failed.
 *
 * @api
 */
bool dmaReloadConf(DMADriver *dmap, const DMAConfig *cfg);

/**
 * @brief   Releases the DMA stream.
 *
 * @param[in,out] dmap  Driver object.
 *
 * @pre
 * No transfer must be active. Stop it first with @ref dmaStopTransfert() when
 * needed.
 *
 * @post
 * The driver is in @ref DMA_STOP.
 *
 * @api
 */
void dmaStop(DMADriver *dmap);

#if STM32_DMA_USE_WAIT == TRUE
/**
 * @brief   Starts one synchronous one-shot DMA transfer with timeout.
 *
 * @param[in,out] dmap   Driver object configured for @ref DMA_ONESHOT.
 * @param[in,out] periphp Peripheral register address.
 * @param[in,out] mem0p  Memory buffer address.
 * @param[in] size       Number of transfer items.
 * @param[in] timeout    ChibiOS timeout.
 * @retval MSG_OK        Transfer completed.
 * @retval MSG_RESET     Transfer was stopped or failed.
 * @retval MSG_TIMEOUT   Timeout expired before completion.
 *
 * @api
 */
msg_t dmaTransfertTimeout(DMADriver *dmap, volatile void *periphp, void * mem0p, const size_t size,
		   sysinterval_t timeout);

/**
 * @brief   Starts one synchronous one-shot DMA transfer without timeout.
 *
 * @param[in,out] dmap   Driver object configured for @ref DMA_ONESHOT.
 * @param[in,out] periphp Peripheral register address.
 * @param[in,out] mem0p  Memory buffer address.
 * @param[in] size       Number of transfer items.
 * @retval MSG_OK        Transfer completed.
 * @retval MSG_RESET     Transfer was stopped or failed.
 *
 * @api
 */
static inline msg_t dmaTransfert(DMADriver *dmap, volatile void *periphp, void * mem0p, const size_t size)
{
  return dmaTransfertTimeout(dmap, periphp, mem0p, size, TIME_INFINITE);
}
#endif
#if STM32_DMA_USE_MUTUAL_EXCLUSION == TRUE
/**
 * @brief   Acquires the optional DMA driver mutex.
 * @param[in,out] dmap  Driver object.
 * @api
 */
void dmaAcquireBus(DMADriver *dmap);

/**
 * @brief   Releases the optional DMA driver mutex.
 * @param[in,out] dmap  Driver object.
 * @api
 */
void dmaReleaseBus(DMADriver *dmap);
#endif

/**
 * @brief   Starts an asynchronous DMA transfer.
 *
 * @param[in,out] dmap   Driver object in @ref DMA_READY, @ref DMA_COMPLETE or
 *                       @ref DMA_ERROR.
 * @param[in,out] periphp Peripheral register address.
 * @param[in,out] mem0p  Memory buffer address.
 * @param[in] size       Number of transfer items.
 * @retval true          Transfer started.
 * @retval false         Low-level start failed.
 *
 * @note
 * Address alignment must match @c psize and @c msize. Circular half-buffer mode
 * requires an even @p size unless @p size is 1.
 *
 * @api
 */
bool dmaStartTransfert(DMADriver *dmap, volatile void *periphp, void * mem0p,
                       const size_t size);

/**
 * @brief   Stops an asynchronous DMA transfer.
 *
 * @param[in,out] dmap  Driver object.
 *
 * @post
 * The driver is back in @ref DMA_READY.
 *
 * @api
 */
void dmaStopTransfert(DMADriver *dmap);

/**
 * @brief   Starts an asynchronous DMA transfer from ISR/locked context.
 *
 * @param[in,out] dmap   Driver object.
 * @param[in,out] periphp Peripheral register address.
 * @param[in,out] mem0p  Memory buffer address.
 * @param[in] size       Number of transfer items.
 * @retval true          Transfer started.
 * @retval false         Low-level start failed.
 *
 * @iclass
 */
bool dmaStartTransfertI(DMADriver *dmap, volatile void *periphp, void *mem0p,
                        const size_t size);

/**
 * @brief   Stops an asynchronous DMA transfer from ISR/locked context.
 *
 * @param[in,out] dmap  Driver object.
 *
 * @iclass
 */
void dmaStopTransfertI(DMADriver *dmap);

/**
 * @brief   Returns the STM32 DMA stream index allocated to the driver.
 *
 * @param[in] dmap  Driver object.
 * @return          Stream index, or 0xff when no matching stream is found.
 *
 * @api
 */
uint8_t dmaGetStreamIndex(DMADriver *dmap);
#if defined DMA_request_TypeDef && defined DMA_Stream_TypeDef
/**
 * @brief   Builds a DMA register image for an already configured stream.
 *
 * @param[in] dmap        Driver object.
 * @param[in,out] periphp Peripheral register address.
 * @param[in,out] mem0p   Memory buffer address.
 * @param[in] size        Number of transfer items.
 * @param[out] registers  Destination register image.
 *
 * @iclass
 */
void  dmaGetRegisters(DMADriver *dmap, volatile void *periphp, void *mem0p,
		      const size_t size,
		      DMA_Stream_TypeDef *registers);
#endif  

/**
 * @brief   Returns the current DMA driver state.
 * @param[in] dmap  Driver object.
 * @return          Driver state.
 * @api
 */
static  inline dmastate_t dmaGetState(DMADriver *dmap) {return dmap->state;}

/**
 * @brief   Returns the hardware residual transfer counter.
 * @param[in] dmap  Driver object.
 * @return          Remaining transfer items according to the DMA stream.
 * @api
 */
static  inline size_t dmaGetTransactionCounter(DMADriver *dmap) {return dmaStreamGetTransactionSize(dmap->dmastream);}

#if STM32_DMA_USE_ASYNC_TIMOUT
/**
 * @brief   Forces a half-buffer style callback based on current DMA position.
 * @details ISR-context variant. No kernel lock required.
 * @note    Valid only when op_mode == DMA_CONTINUOUS_HALF_BUFFER.
 * @note    Must be called from ISR context with no kernel lock held.
 *
 * @xclass
 */
void dmaForceHalfBufferFromISR(DMADriver *dmap);
#endif

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

/** @} */

/* Low level driver entry points. Not application APIs. */

bool  dma_lld_start(DMADriver *dmap, bool allocate_stream);
void  dma_lld_stop(DMADriver *dmap);


bool  dma_lld_start_transfert(DMADriver *dmap, volatile void *periphp, void *mem0p, const size_t size);


void  dma_lld_stop_transfert(DMADriver *dmap);

#if STM32_DMA_USE_ASYNC_TIMOUT
void dma_lld_serve_timeout_interrupt(struct ch_virtual_timer *tim, void *arg);
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
