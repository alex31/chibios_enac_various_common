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
 * @param[in] dmap      pointer to the @p ADCDriver object
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





typedef struct {
  stm32_dmaisr_t	serve_dma_isr;
  void *		serve_dma_isr_arg;
  volatile void *	periph_addr;
  uint32_t		direction; // STM32_DMA_CR_DIR_P2M, STM32_DMA_CR_DIR_M2P, STM32_DMA_CR_DIR_M2M
  bool			inc_peripheral_addr;
  bool			inc_memory_addr;
  bool			circular;
  uint32_t		isr_flags; // combination of STM32_DMA_CR_[TCIE, HTIE, DMEIE, TEIE]
				   // transfert complete, half transfert, direct mode error,
				   // transfert error

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
} DMAConfig ;

typedef struct {
  const stm32_dma_stream_t  *dmastream;
  uint32_t		     dmamode;
  dmastate_t		     state;
  thread_reference_t         thread;
  const DMAConfig	    *config;
} DMADriver ;


bool dma_start(DMADriver *dmad, const DMAConfig *cfg);
void dma_stop(DMADriver *dmad);

void dma_mtransfert(DMADriver *dmad, void *membuffer, const size_t size);
bool dma_start_mtransfert(DMADriver *dmad, void *from, void *to, const size_t size);

bool dma_ptransfert(DMADriver *dmad, void *membuffer, const size_t size);
bool dma_start_ptransfert(DMADriver *dmad, void *membuffer, const size_t size);

void dma_stop_transfert(DMADriver *dmad);
