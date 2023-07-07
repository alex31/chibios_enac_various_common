#include "dshot_rpmCapture.h"
#include <string.h>
#include <stdutil.h>

static const float TIM_FREQ_MHZ = (STM32_SYSCLK/1E6d);
static const float bit1t_us = TIM_FREQ_MHZ * 6.67d * 4 / 5;
static const float speed_factor = DSHOT_SPEED / 150;

static const uint32_t  ERPS_BIT1_DUTY = bit1t_us / speed_factor;
static const uint32_t  TIM_PRESCALER = 1;

static void startCapture(DshotRpmCapture *drcp);
static void stopCapture(DshotRpmCapture *drcp);
static void initCache(DshotRpmCapture *drcp);
static uint32_t processErpsDmaBuffer(const uint16_t *capture, size_t dmaLen);
static void buildDmaConfig(DshotRpmCapture *drcp);
static void dmaErrCb(DMADriver *dmad, dmaerrormask_t em);
static void gptCb(GPTDriver *gptd);

static volatile dmaerrormask_t lastErr;
static volatile uint32_t       dmaErrs = 0;



static const struct  {
  uint32_t active;
  uint32_t dier;
} activeDier[4] = {
  {CH1_BOTH_EDGES,
   TIM_DIER_CC1DE | TIM_DIER_TDE},
  {CH1_BOTH_EDGES | CH2_BOTH_EDGES,
   TIM_DIER_CC1DE | TIM_DIER_CC2DE | TIM_DIER_TDE},
  {CH1_BOTH_EDGES | CH2_BOTH_EDGES | CH3_BOTH_EDGES,
   TIM_DIER_CC1DE | TIM_DIER_CC2DE | TIM_DIER_CC3DE | TIM_DIER_TDE},
  {CH1_BOTH_EDGES | CH2_BOTH_EDGES | CH3_BOTH_EDGES | CH4_BOTH_EDGES,
   TIM_DIER_CC1DE | TIM_DIER_CC2DE | TIM_DIER_CC3DE | TIM_DIER_CC4DE | TIM_DIER_TDE}
};

static const TimICConfig timicCfgSkel = {
	  .timer = NULL,
	  .capture_cb = NULL,
	  .overflow_cb = nullptr,
	  .mode = TIMIC_INPUT_CAPTURE,
	  .active = activeDier[DSHOT_CHANNELS-1].active,
	  .dier = activeDier[DSHOT_CHANNELS-1].dier,
	  .dcr = 0, // direct access, not using DMAR
	  .prescaler = TIM_PRESCALER,
	  .arr =  (uint32_t) ((TIM_FREQ_MHZ * 25) + (ERPS_BIT1_DUTY * 20)) // silent + frame length
};

/*
this driver need additional field in gptDriver structure (in halconf.h): 
one should add following line in the GPT section of halconf.h :
#define GPT_DRIVER_EXT_FIELDS void *dshotRpmCaptured;
*/
static const GPTConfig gptCfg =  {
  .frequency    = 1'000'000, // 1MHz
  .callback     = &gptCb,
  .cr2 = 0,
  .dier = 0
};


void dshotRpmCaptureStart(DshotRpmCapture *drcp, const DshotRpmCaptureConfig *cfg)
{
  memset(drcp, 0, sizeof(*drcp));
  drcp->config = cfg;
  drcp->icCfg = timicCfgSkel;
  drcp->icCfg.timer = drcp->config->timer;
  initCache(drcp);
}

void dshotRpmCaptureStop(DshotRpmCapture *drcp)
{
  stopCapture(drcp);
}

uint32_t dshotRpmGetDmaErr(void) {
  return dmaErrs;
}

void dshotRpmCatchErps(DshotRpmCapture *drcp)
{
  startCapture(drcp);
  static systime_t ts = 0;
  if (ts == 0)
    ts = chVTGetSystemTimeX();

  memset(drcp->config->dma_capture->dma_buf, 0, sizeof(drcp->config->dma_capture->dma_buf));
  _Static_assert(sizeof(drcp->config->dma_capture->dma_buf) == sizeof(uint16_t) *
		 DSHOT_CHANNELS * (DSHOT_DMA_DATA_LEN + DSHOT_DMA_EXTRADATA_LEN));

  for (size_t i = 0; i < DSHOT_CHANNELS; i++) {
    dmaStartTransfert(&drcp->dmads[i], &drcp->icd.config->timer->CCR[i],
		      drcp->config->dma_capture->dma_buf[i], DSHOT_DMA_DATA_LEN + DSHOT_DMA_EXTRADATA_LEN);
  }

  osalSysLock();
  // dma end callback will resume the thread upon completion of ALL dma transaction
  // else, the timeout will take care of thread resume
  static const sysinterval_t timeoutUs = 25U + (120U * 150U / DSHOT_SPEED);
  palSetLine(LINE_LA_DBG_1);
  gptStartOneShotI(drcp->config->gptd, timeoutUs);
  palClearLine(LINE_LA_DBG_1);
  chThdSuspendS(&drcp->dmads[0].thread);
  palSetLine(LINE_LA_DBG_1);
  chSysPolledDelayX(1);
  palClearLine(LINE_LA_DBG_1);

  for (size_t i = 0; i < DSHOT_CHANNELS; i++) 
    dmaStopTransfertI(&drcp->dmads[i]);
  
  osalSysUnlock();
  stopCapture(drcp);

  for (size_t i = 0; i < DSHOT_CHANNELS; i++) {
    drcp->rpms[i] = processErpsDmaBuffer(drcp->config->dma_capture->dma_buf[i],
					 DSHOT_DMA_DATA_LEN + DSHOT_DMA_EXTRADATA_LEN -
					 dmaGetTransactionCounter(&drcp->dmads[i]));
  }

#if defined(DFREQ) && (DFREQ < 10) && (DFREQ != 0)
  DebugTrace("dma out on %s", msg == MSG_OK ? "completion" : "timeout");
#endif

}

void dshotRpmTrace(DshotRpmCapture *drcp, uint8_t index)
{
  for (size_t i = 0; i < DSHOT_CHANNELS; i++) {
    if ((index != 0xff) && (index != i))
      continue;
    uint16_t cur = drcp->config->dma_capture->dma_buf[i][0];
    for (size_t j = 1; j < DSHOT_DMA_DATA_LEN; j++) {
      const uint16_t  dur = drcp->config->dma_capture->dma_buf[i][j] - cur;
      cur = drcp->config->dma_capture->dma_buf[i][j];
      chprintf(chp, "[%u] %u, ", i, dur);
    }
    DebugTrace("");
  }
  DebugTrace("");
}



/*
#                 _ __           _                    _                   
#                | '_ \         (_)                  | |                  
#                | |_) |  _ __   _   __   __   __ _  | |_     ___         
#                | .__/  | '__| | |  \ \ / /  / _` | | __|   / _ \        
#                | |     | |    | |   \ V /  | (_| | \ |_   |  __/        
#                |_|     |_|    |_|    \_/    \__,_|  \__|   \___|        
*/

static void buildDmaConfig(DshotRpmCapture *drcp)
{
  static const DMAConfig skel = (DMAConfig) {
    .stream = 0,
    .channel = 0,
    .inc_peripheral_addr = false,
    .inc_memory_addr = true,
    .op_mode = DMA_ONESHOT,
    .end_cb = nullptr,
    .error_cb = &dmaErrCb,
    //.error_cb = nullptr,
#if STM32_DMA_USE_ASYNC_TIMOUT
    .timeout = TIME_MS2I(100),
#endif
    .direction = DMA_DIR_P2M,
    .dma_priority = 1,
    .irq_priority = 7,
    .psize = 2,
    .msize = 2,
    .pburst = 0,
    .mburst = 0,
    .fifo = 4,
    .periph_inc_size_4 = false,
    .transfert_end_ctrl_by_periph = false
  };
  
  for (size_t i = 0; i < DSHOT_CHANNELS; i++) {
    drcp->dmaCfgs[i] = skel;
    drcp->dmaCfgs[i].stream = drcp->config->dma_streams[i].stream;
    drcp->dmaCfgs[i].channel = drcp->config->dma_streams[i].channel;
  }
}

static void initCache(DshotRpmCapture *drcp)
{
  const DshotRpmCaptureConfig *cfg = drcp->config;
  timIcObjectInit(&drcp->icd);
  timIcStart(&drcp->icd, &drcp->icCfg);
  gptStart(cfg->gptd, &gptCfg);
  cfg->gptd->dshotRpmCaptured = drcp; // user data in GPT driver

  buildDmaConfig(drcp);
  for (size_t i=0; i < DSHOT_CHANNELS; i++) {
    dmaObjectInit(&drcp->dmads[i]);
    dmaStart(&drcp->dmads[i], &drcp->dmaCfgs[i]);
  }

  timerDmaCache_cache(&drcp->cache, &drcp->dmads[0], cfg->timer);
  dshotRpmCaptureStop(drcp);
} 

static void startCapture(DshotRpmCapture *drcp)
{
  timerDmaCache_restore(&drcp->cache, &drcp->dmads[0], drcp->config->timer);
  timIcStartCapture(&drcp->icd);
}

static void stopCapture(DshotRpmCapture *drcp)
{
   timIcStopCapture(&drcp->icd);
   timIcRccDisable(&drcp->icd);
}

// naive implementation, there is a need for a more inspired one
static uint32_t processErpsDmaBuffer(const uint16_t *capture, size_t dmaLen)
{
  static const size_t frameLen = 20U;
  uint32_t erpsVal = 0;
  uint_fast8_t bit = 0x0;
  uint_fast8_t bitIndex = 0;
  uint_fast16_t prec = capture[0];
  
  for (size_t i = 1; i < dmaLen; i++) {
    const uint_fast16_t len = capture[i] - prec;
    prec = capture[i];
    // GRC encoding garanties that there can be no more than 3 consecutives bits at the same level
    const uint_fast8_t nbConsecutives =  ((len + (ERPS_BIT1_DUTY / 2)) /  ERPS_BIT1_DUTY) % 4;
    
    if (bit) {
      for (size_t j = bitIndex; j < bitIndex + nbConsecutives; j++) 
	erpsVal |= (1U << (frameLen - j));
      // On F4 : 2.0 ns pour above coden 2.3ns for commented code 
      // for (size_t j = frameLen - bitIndex; j > frameLen - bitIndex - nbConsecutives; j--) 
      // 	erpsVal |= (1U << j);
      
      //            DebugTrace("bitIndex = %u; nbConsecutives 1 = %u, [%lx]", bitIndex, nbConsecutives, erpsVal);
    } else {
      //            DebugTrace("bitIndex = %u; nbConsecutives 0 = %u, [%lx]", bitIndex, nbConsecutives, erpsVal);
    }
    bit = bit ^ 0x1U; // flip bit
    bitIndex += nbConsecutives;
  }
  // there can be several high bits hidden in the trailing high level signal
  for (size_t j=bitIndex; j <= frameLen; j++) 
    erpsVal |= (1U << (frameLen - j));
  //DebugTrace("bit index = %u; erpsVal = 0x%lx", bitIndex, erpsVal);
  return erpsVal;
}

// this version must be benchmarked with above version, should be faster
// on cortexM7 superscalar unit (F7, H7) but slower on cortexM4 (all but F7, H7)
// static uint32_t processErpsDmaBuffer(const uint16_t *capture, size_t dmaLen)
// {
//   static const size_t frameLen = 20U;
//   uint32_t erpsVal = 0;
//   uint_fast32_t bit = 0x0;
//   uint_fast8_t bitIndex = 0;
//   uint_fast16_t prec = capture[0];
  
//   for (size_t i = 1U; i < dmaLen; i++) {
//     const uint_fast16_t len = capture[i] - prec;
//     prec = capture[i];
//     // GRC encoding garanties that there can be no more than 3 consecutives bits at the same level
//     const uint_fast8_t nbConsecutives =  (len + (ERPS_BIT1_DUTY / 2)) /  ERPS_BIT1_DUTY;
    
//     for (size_t j=bitIndex; j < bitIndex + nbConsecutives; j++) {
// 	     erpsVal |= ((1U << (frameLen - j)) & bit);
//     }  
//     bit ^= 0xffffffffU; 
//     bitIndex += nbConsecutives;
//   }
//   // there can be several high bits hidden in the trailing high level signal
//   for (size_t j=bitIndex; j <= frameLen; j++) 
//     erpsVal |= (1U << (frameLen - j));
//   //DebugTrace("bit index = %u; erpsVal = 0x%lx", bitIndex, erpsVal);
//   return erpsVal;
// }


static void dmaErrCb(DMADriver *, dmaerrormask_t em)
{
  lastErr = em;
  dmaErrs++;
}

static void gptCb(GPTDriver *gptd)
{
  chSysLockFromISR();
  DshotRpmCapture *drcd = (DshotRpmCapture *) gptd->dshotRpmCaptured;
  chThdResumeI(&drcd->dmads[0].thread, MSG_TIMEOUT);
  chSysUnlockFromISR();
}
