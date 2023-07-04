#pragma once

#include <ch.h>
#include <hal.h>
#include "inputCapture.h"
#include "esc_dshot.h"
#include "hal_stm32_dma.h"
#include "timerDmaCache.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DSHOT_DMA_DATA_LEN	16U
#define DSHOT_DMA_EXTRADATA_LEN 2U

#if STM32_DMA_SUPPORTS_DMAMUX == false
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


typedef struct  {
  uint32_t		stream;
  uint8_t		channel;
} DshotDmaStreamChan;

typedef struct  {
  uint16_t	 	dma_buf[DSHOT_CHANNELS][DSHOT_DMA_DATA_LEN + DSHOT_DMA_EXTRADATA_LEN];
} DshotRpmCaptureDmaBuffer;

typedef struct  {
  stm32_tim_t		*timer;
  GPTDriver		*gptd;
  DshotDmaStreamChan    dmaStreams[DSHOT_CHANNELS];
  DshotRpmCaptureDmaBuffer *capture;
} DshotRpmCaptureConfig;


/* DshotRpmCapture objects will need to be instancied in dma compliant
   memory on cache enabled MCU [F7, H7] for the captures field 
   another possible API is no furnish a pointer on the memory buffer in config
   structure with a mechanism to get/verify the buffer size with a xxx_DECL like chibios ?
*/
typedef struct 
{
  const DshotRpmCaptureConfig	*config;
  uint32_t		erps[DSHOT_CHANNELS];
  TimICConfig		icCfg;
  TimICDriver	 	icd;
  DMAConfig	 	dmaCfgs[DSHOT_CHANNELS];
  DMADriver	 	dmads[DSHOT_CHANNELS];
  TimerDmaCache	 	cache;
  uint32_t		rpms[DSHOT_CHANNELS];
} DshotRpmCapture;



void dshotRpmCaptureStart(DshotRpmCapture *drcp, const DshotRpmCaptureConfig *cfg);
void dshotRpmCaptureStop(DshotRpmCapture *drcp);
void dshotRpmCatchErps(DshotRpmCapture *drcp);
void dshotRpmTrace(DshotRpmCapture *drcp, uint8_t index);
static inline uint32_t dshotRpmGetFrame(const DshotRpmCapture *drcp, uint8_t index) {
  return drcp->rpms[index];
}
uint32_t dshotRpmGetDmaErr(void) ;


#ifdef __cplusplus
}
#endif
