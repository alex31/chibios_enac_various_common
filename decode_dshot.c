#include "decode_dshot.h"
#include <string.h>
#include "stdutil.h"



#define DCR_DBL (1U << 8U) // 2 transfert
#define ICU_FREQUENCY_32B  (STM32_SYSCLK / 4ULL) /* 42 Mhz ICU clock frequency.   */
#define ICU_FREQUENCY_16B  (3000000U)  /* 3 Mhz ICU clock frequency.   */

typedef struct {
  uint32_t min;
  uint32_t max;
} MinMax;

static void error_cb(DMADriver *dmap, dmaerrormask_t err);
static void initIcu(DecodeDSHOTDriver *driver);
static void initDma(DecodeDSHOTDriver *driver); // must be called after initIcu
static EscCmdMode guessMode(DecodeDSHOTDriver *driver);
static uint8_t guessPWM(DecodeDSHOTDriver *driver);
static DshotPacket guessDshot(DecodeDSHOTDriver *driver);
static MinMax getMinMaxPeriod(DecodeDSHOTDriver *driver);
static MinMax getMinMaxWidth(DecodeDSHOTDriver *driver);
static inline uint32_t getPeriod(DecodeDSHOTDriver *driver, size_t index);
static inline uint32_t getWidth(DecodeDSHOTDriver *driver, size_t index);

void             decodeDshotStart(DecodeDSHOTDriver *driver,
				  const DecodeDSHOTConfig *config)
{
  driver->config = config;
  initIcu(driver);
  initDma(driver);
}

DecodeESCFrame decodeDshotCapture(DecodeDSHOTDriver *driver)
{
  DecodeESCFrame df;
  df.mode = guessMode(driver);
  if ((df.mode == ESC_PWM50) || (df.mode == ESC_PWM400)) {
    df.percent = guessPWM(driver);
  } else {
    df.dshotp = guessDshot(driver);
  }
  
  return df ;
}

void             decodeDshotStop(DecodeDSHOTDriver *driver)
{
  dmaStop(&driver->dmap);
  icuStopCapture(driver->config->icup);
  icuStop(driver->config->icup);
}

static volatile dmaerrormask_t lastDmaErr = 0;
static  void error_cb(DMADriver *_dmap, dmaerrormask_t err)
{
  (void) _dmap;
  lastDmaErr = err;
  
  chSysHalt("DMA error");
}

/*
#                 _            _    _            
#                (_)          (_)  | |           
#                 _    _ __    _   | |_          
#                | |  | '_ \  | |  | __|         
#                | |  | | | | | |  \ |_          
#                |_|  |_| |_| |_|   \__|         
*/

static  void initDma(DecodeDSHOTDriver *driver)
{
  driver->dma_conf = (DMAConfig) {
   .stream = driver->config->dma_stream,
   .channel = driver->config->dma_channel,
   .inc_peripheral_addr = false,
   .inc_memory_addr = true,
   .op_mode = DMA_ONESHOT,
   .end_cb = NULL,
   .error_cb = &error_cb,
   .direction = DMA_DIR_P2M,
   .dma_priority = 3U,
   .irq_priority = 6U,
   .psize = 4U,
   .msize = 4U,
   .pburst = 0,
   .mburst = 0,
   .fifo = 2U,
   .periph_inc_size_4 = false,
   .transfert_end_ctrl_by_periph = false
  };

  dmaObjectInit(&driver->dmap);
  dmaStart(&driver->dmap, &driver->dma_conf);
}


static  void initIcu(DecodeDSHOTDriver *driver)
{
  driver->timer_width = 2U;
#if STM32_ICU_USE_TIM2
  if (driver->config->icup == &ICUD2)
    driver->timer_width = 4U;
#endif
#if STM32_ICU_USE_TIM5
  if (driver->config->icup == &ICUD5)
    driver->timer_width = 4U;
#endif

  // calculate offset between CCR register and begining of timer bloc register
  // for DMAR use
  const size_t dcrDba =  (((uint32_t *) driver->config->icup->tim->CCR -
			   ((uint32_t *) driver->config->icup->tim)));
  
  driver->icu_conf = (ICUConfig) {
   .mode = ICU_INPUT_ACTIVE_HIGH,
   .frequency = driver->timer_width == 4U ? ICU_FREQUENCY_32B : ICU_FREQUENCY_16B,
   .width_cb = NULL,
   .period_cb = NULL,
   .overflow_cb = NULL,
   .channel = driver->config->icu_channel,
   .dier = TIM_DIER_UDE |
   ((driver->config->icu_channel == ICU_CHANNEL_1) ?
    TIM_DIER_CC1DE  : TIM_DIER_CC2DE),
#if CH_HAL_MAJOR >= 7
   .arr = 0xFFFFFFFF
#endif
  };
  
  icuStart(driver->config->icup, &driver->icu_conf);
#if CH_HAL_MAJOR < 7
  driver->config->icup->tim->ARR = 0xFFFFFFFF;
#endif
  driver->config->icup->tim->DCR = DCR_DBL | dcrDba;
  icuStartCapture(driver->config->icup);
}

/*
#                     _                              _                 
#                    | |                            | |                
#                  __| |    ___    ___    ___     __| |    ___         
#                 / _` |   / _ \  / __|  / _ \   / _` |   / _ \        
#                | (_| |  |  __/ | (__  | (_) | | (_| |  |  __/        
#                 \__,_|   \___|  \___|  \___/   \__,_|   \___|        
*/

static EscCmdMode guessMode(DecodeDSHOTDriver *driver)
{
  memset(driver->wp_buffer.raw, 0, sizeof(driver->wp_buffer));
  
  const msg_t status = dmaTransfertTimeout(&driver->dmap,
					   &driver->config->icup->tim->DMAR,
					   driver->wp_buffer.raw,
					   DMA_DATA_LEN, TIME_MS2I(100));
  const MinMax pmm = getMinMaxPeriod(driver);

  if (pmm.max == 0) {
    return ESC_NONE;
  }
  
  const uint32_t pMinUs = ((pmm.min+1U) * 1000000ULL) / driver->config->icup->config->frequency;
  const uint32_t pMaxUs = ((pmm.max+1U) * 1000000ULL) / driver->config->icup->config->frequency;

  if ((pMaxUs > 18000) && (status == MSG_TIMEOUT)) {
    return ESC_PWM50;
  }

  if ((pMinUs > 2000U) && (pMaxUs > 2000U)) {
    return ESC_PWM400;
  }

  // It's DSHOT, so first guess wich dshot
  if (pMinUs > 5U) {
    return ESC_DSHOT150;
  } else if (pMinUs > 2U) {
    return ESC_DSHOT300;
  } else {
    return ESC_DSHOT600;
  }

}

static uint8_t guessPWM(DecodeDSHOTDriver *driver)
{
  const MinMax wmm = getMinMaxWidth(driver);
  return ((((wmm.max+1U) * 1000000ULL) / driver->config->icup->config->frequency) - 1000UL) / 10U;
}

static DshotPacket guessDshot(DecodeDSHOTDriver *driver)
{
  DshotPacket dp = {.rawFrame = 0U};

  uint32_t ci;
  for(ci = 0;
      (ci <  (DMA_DATA_LEN / 2U)) && (getPeriod(driver, ci) < 1000);
      ci++);

  if (++ci >= (DMA_DATA_LEN / 2U)) {
    // decode logic error
    dp.rawFrame = 0xFFFF;
    return dp;
  }
  const uint32_t wThreshold = getPeriod(driver, ci) / 2U;
  uint32_t bitShift=0U;

  for(; ci <  (DMA_DATA_LEN / 2U); ci++) {
    dp.rawFrame |= getWidth(driver, ci) < wThreshold ? 0U : (1U << (15-bitShift));
    if (++bitShift == 16)
      break;
  }

  // compute checksum of DSHOT packet
  uint8_t crc=0;
  uint16_t csum = (dp.throttle << 1) | dp.telemetryRequest;
  for (int i = 0; i < 3; i++) {
    crc ^=  csum;   // xor data by nibbles
    csum >>= 4;
  }
  crc &= 0x0F;

  if (dp.crc != crc) {
    dp.rawFrame = 0xFFFF; // special value to indicate crc error
  }

  return dp;
}



static inline uint32_t getPeriod(DecodeDSHOTDriver *driver, size_t index)
{
  if ((driver->config->icu_channel) == ICU_CHANNEL_2) {
    return driver->wp_buffer.wp[index].p;
  } else {
    return driver->wp_buffer.wp[index].w;
  }
}

static inline uint32_t getWidth(DecodeDSHOTDriver *driver, size_t index)
{
  if ((driver->config->icu_channel) == ICU_CHANNEL_2) {
    return driver->wp_buffer.wp[index].w;
  } else {
    return driver->wp_buffer.wp[index].p;
  }
}


static MinMax getMinMaxPeriod(DecodeDSHOTDriver *driver)
{
  MinMax mm = {.min=UINT32_MAX, .max=0U};
  for (size_t i=0; i< (DMA_DATA_LEN / 2U); i++) {
    if (getPeriod(driver, i) < mm.min)
      mm.min = getPeriod(driver, i);
    if (getPeriod(driver, i) > mm.max)
      mm.max = getPeriod(driver, i);
  }
  
  return mm;
}

static MinMax getMinMaxWidth(DecodeDSHOTDriver *driver)
{
  MinMax mm = {.min=UINT32_MAX, .max=0U};
  for (size_t i=0; i< (DMA_DATA_LEN / 2U); i++) {
    if (getWidth(driver, i) < mm.min)
      mm.min = getWidth(driver, i);
    if (getWidth(driver, i) > mm.max)
      mm.max = getWidth(driver, i);
  }
  
  return mm;
}
