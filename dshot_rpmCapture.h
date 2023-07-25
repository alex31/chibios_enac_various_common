#pragma once

#include <ch.h>
#include <hal.h>
#include "inputCapture.h"
#include "hal_stm32_dma.h"
#include "timerDmaCache.h"
#include "esc_dshot_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DSHOT_DMA_DATA_LEN	16U
#define DSHOT_DMA_EXTRADATA_LEN 2U


#if STM32_DMA_SUPPORTS_DMAMUX == false
/**
 * @brief   macro helper to design DMA stream
 * @note    does not apply for DMAMUX MCU
 */
#define CONCAT_NX(pre, tim, stream, channel) pre ## tim ## stream , pre ## tim ## channel
#define DSHOTS_1STREAM(tim)  CONCAT_NX(STM32_TIM, tim, _CH1_DMA_STREAM, _CH1_DMA_CHANNEL)
#define DSHOTS_2STREAMS(tim) CONCAT_NX(STM32_TIM, tim, _CH1_DMA_STREAM, _CH1_DMA_CHANNEL), \
                             CONCAT_NX(STM32_TIM, tim, _CH2_DMA_STREAM, _CH2_DMA_CHANNEL)
#define DSHOTS_3STREAMS(tim) CONCAT_NX(STM32_TIM, tim, _CH1_DMA_STREAM, _CH1_DMA_CHANNEL), \
			     CONCAT_NX(STM32_TIM, tim, _CH2_DMA_STREAM, _CH2_DMA_CHANNEL), \
                             CONCAT_NX(STM32_TIM, tim, _CH3_DMA_STREAM, _CH3_DMA_CHANNEL)
#define DSHOTS_4STREAMS(tim) CONCAT_NX(STM32_TIM, tim, _CH1_DMA_STREAM, _CH1_DMA_CHANNEL), \
			     CONCAT_NX(STM32_TIM, tim, _CH2_DMA_STREAM, _CH2_DMA_CHANNEL), \
			     CONCAT_NX(STM32_TIM, tim, _CH3_DMA_STREAM, _CH3_DMA_CHANNEL), \
			     CONCAT_NX(STM32_TIM, tim, _CH4_DMA_STREAM, _CH4_DMA_CHANNEL)

#if DSHOT_CHANNELS == 1
#define DSHOTS_STREAMS(tim)	DSHOTS_1STREAM(tim)
#elif DSHOT_CHANNELS == 2
#define DSHOTS_STREAMS(tim)	DSHOTS_2STREAMS(tim)
#elif DSHOT_CHANNELS == 3
#define DSHOTS_STREAMS(tim)	DSHOTS_3STREAMS(tim)
#elif DSHOT_CHANNELS == 4
#define DSHOTS_STREAMS(tim)	DSHOTS_4STREAMS(tim)
#else
#error DSHOT_CHANNELS must be 1 to 4
#endif
#endif


/**
 * @brief   structure defining dma channel
 * @note    does not apply for DMAMUX MCU
 */
typedef struct  {
  uint32_t		stream;
  uint8_t		channel;
} DshotDmaStreamChan;


/**
 * @brief   DMA capture buffer
 */
typedef struct  {
  uint16_t	 	dma_buf[DSHOT_CHANNELS][DSHOT_DMA_DATA_LEN + DSHOT_DMA_EXTRADATA_LEN];
} DshotRpmCaptureDmaBuffer;

/**
 * @brief : DSHOT Rpm Capture Driver configuration structure.
 */
typedef struct  {
  /**
   * @brief : GPT Driver to manage microseconds timeout
   * @note  : timeout is too narrow to be managed by RTOS
   */
  GPTDriver		*gptd;
  /**
   * @brief : array of DMA stream for each capture channel
   */
  DshotDmaStreamChan    dma_streams[DSHOT_CHANNELS];
  /**
   * @brief : pointer to the input capture DMA buffer
   * @note  : dma_capture must point to DMA compatible memory
   *          DCACHE disabled section for F7
   */
  DshotRpmCaptureDmaBuffer *dma_capture;
} DshotRpmCaptureConfig;


typedef struct 
{
  /**
   * @brief : pointer to configuration structure
   * @note  : must be valid for the object life 
   */
  const DshotRpmCaptureConfig	*config;
  /**
   * @brief : array of erps frames
   */
  uint32_t		erps[DSHOT_CHANNELS];
  /**
   * @brief : input capture timer configuration
   */
  TimICConfig		icCfg;
  /**
   * @brief : input capture timer driver
   */
  TimICDriver	 	icd;
  /**
   * @brief : dma input capture configuration
   */
  DMAConfig	 	dmaCfgs[DSHOT_CHANNELS];
  /**
   * @brief : dma input capture drivers
   */ 
  DMADriver	 	dmads[DSHOT_CHANNELS];
  /**
   * @brief : cache for timer and dma configuration
   * @note  : cached configuration are restored via memcpy which is
   *          faster than field by field setting
   */ 
  TimerDmaCache	 	cache;
  /**
   * @brief : array of rpms
   */
  uint32_t		rpms[DSHOT_CHANNELS];
#ifdef DSHOT_STATISTICS
  /**
   * @brief : total of time taken to decode erps frame
   */
  uint64_t		accumDecodeTime;
  /**
   * @brief : total number of decoded erps frame
   */
  uint32_t		nbDecode;
#endif
} DshotRpmCapture;



void dshotRpmCaptureStart(DshotRpmCapture *drcp, const DshotRpmCaptureConfig *cfg,
			  stm32_tim_t	  *timer);
void dshotRpmCaptureStop(DshotRpmCapture *drcp);
void dshotRpmCatchErps(DshotRpmCapture *drcp);
#if DSHOT_STATISTICS
void dshotRpmTrace(DshotRpmCapture *drcp, uint8_t index);
uint32_t dshotRpmGetDmaErr(void) ;
#endif
/**
 * @brief   return last collected erps frame
 *
 * @param[in] drcp    pointer to the @p DshotRpmCapture object
 * @param[in] index   index of the channel [1 .. 4]
 * @api
 */
static inline uint32_t dshotRpmGetFrame(const DshotRpmCapture *drcp, uint8_t index) {
  return drcp->rpms[index];
}
#if DSHOT_STATISTICS
/**
 * @brief   return total time to decode erps frame in nanoseconds
 *
 * @param[in] drcp    pointer to the @p DshotRpmCapture object
 * @api
 */
static inline uint32_t  dshotRpmGetAverageDecodeTimeNs(DshotRpmCapture *drcp) {
  return (drcp->accumDecodeTime * 1e9f) / STM32_SYSCLK / drcp->nbDecode;
}
#endif


#ifdef __cplusplus
}
#endif
