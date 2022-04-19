#include "esc_dshot.h"
#include <stdnoreturn.h>
#include <math.h>
#include <string.h>
#include "stdutil.h"


/*
  TODO:

  ° TEST special code sending


 */


/*
#                     _            __   _            _    _      _
#                    | |          / _| (_)          (_)  | |    (_)
#                  __| |    ___  | |_   _    _ __    _   | |_    _     ___    _ __
#                 / _` |   / _ \ |  _| | |  | '_ \  | |  | __|  | |   / _ \  | '_ \
#                | (_| |  |  __/ | |   | |  | | | | | |  \ |_   | |  | (_) | | | | |
#                 \__,_|   \___| |_|   |_|  |_| |_| |_|   \__|  |_|   \___/  |_| |_|
*/
#define PWM_FREQ         (STM32_SYSCLK/2000) // the timer will beat @84Mhz on STM32F4
#define TICKS_PER_PERIOD 1000             // that let use any timer :
			                  // does not care if linked to PCLK1 or PCLK2
			                  // tick_per_period will be dynamically calculated

/** Base freq of DSHOT signal (in kHz)
 * Possible values are: 150, 300, 600
 */
#ifndef DSHOT_SPEED
#define DSHOT_SPEED 300
#endif

			                  // after pwm init
#if DSHOT_SPEED != 0 // statically defined
#   define DSHOT_FREQ (DSHOT_SPEED*1000)
#   define DSHOT_BIT0_DUTY (DSHOT_PWM_PERIOD * 373 / 1000)
#   define DSHOT_BIT1_DUTY (DSHOT_BIT0_DUTY*2)
#else			 // dynamically defined
#   define DSHOT_FREQ (driver->config->speed_khz*1000)
#   define DSHOT_BIT0_DUTY (driver->bit0Duty)
#   define DSHOT_BIT1_DUTY (driver->bit1Duty)
#endif
#define TICK_FREQ (PWM_FREQ * TICKS_PER_PERIOD)
#define DSHOT_PWM_PERIOD (TICK_FREQ/DSHOT_FREQ)

#define    DCR_DBL              ((DSHOT_CHANNELS-1) << 8) //  DSHOT_CHANNELS transfert(s)
// first register to get is CCR1
#define DCR_DBA(pwmd)                 (((uint32_t *) (&pwmd->tim->CCR) - ((uint32_t *) pwmd->tim)))


#define DSHOT_MAX_VALUE ((1<<11)-1) // 11 bits used to send command, so maximum value is 2047


/*
#                 _ __                   _              _      _   _    _ __
#                | '_ \                 | |            | |    | | | |  | '_ \
#                | |_) |  _ __    ___   | |_     ___   | |_   | |_| |  | |_) |   ___
#                | .__/  | '__|  / _ \  | __|   / _ \  | __|   \__, |  | .__/   / _ \
#                | |     | |    | (_) | \ |_   | (_) | \ |_     __/ |  | |     |  __/
#                |_|     |_|     \___/   \__|   \___/   \__|   |___/   |_|      \___|
*/
static DshotPacket makeDshotPacket(const uint16_t throttle, const bool tlmRequest);
static void setCrc4(DshotPacket *dp);
static inline void setDshotPacketThrottle(DshotPacket * const dp, const uint16_t throttle);
static inline void setDshotPacketTlm(DshotPacket * const dp, const bool tlmRequest);
static void buildDshotDmaBuffer(DSHOTDriver *driver);
static inline uint8_t updateCrc8(uint8_t crc, uint8_t crc_seed);
static uint8_t calculateCrc8(const uint8_t *Buf, const uint8_t BufLen);
static noreturn void dshotTlmRec (void *arg);
//static void dmaErrCb(DMADriver *dmap, dmaerrormask_t err);
/*
#                         _ __    _
#                        | '_ \  (_)
#                  __ _  | |_) |  _
#                 / _` | | .__/  | |
#                | (_| | | |     | |
#                 \__,_| |_|     |_|
*/

/**
 * @brief   Configures and activates the DSHOT peripheral.
 *
 * @param[in] driver    pointer to the @p DSHOTDriver object
 * @param[in] config    pointer to the @p DSHOTConfig object.
 * @api
 */
void dshotStart(DSHOTDriver *driver, const DSHOTConfig *config)
{
  chDbgAssert(config->dma_buf != NULL, ".dma_buf must reference valid DshotDmaBuffer object");
  memset((void *) config->dma_buf, 0, sizeof(*(config->dma_buf)));
  const size_t timerWidthInBytes = getTimerWidth(config->pwmp);
  /* DebugTrace("timerWidthInBytes = %u; mburst = %u", */
  /* 	     timerWidthInBytes, */
  /* 	     DSHOT_DMA_BUFFER_SIZE % (timerWidthInBytes * 4) ? 0U : 4U); */

  static const SerialConfig  tlmcfg =  {
    .speed = DSHOT_TELEMETRY_BAUD,
    .cr1 = 0,                                      // pas de parité
    .cr2 = USART_CR2_STOP1_BITS,       // 1 bit de stop
    .cr3 = 0                                       // pas de controle de flux hardware (CTS, RTS)
  };

  // when dshot is bidir, the polarity is inverted
  const uint32_t pwmPolarity = config->bidir ? PWM_OUTPUT_ACTIVE_LOW : PWM_OUTPUT_ACTIVE_HIGH;

  driver->config = config;
  driver->dma_conf = (DMAConfig) {
    .stream = config->dma_stream,
    .channel = config->dma_channel,
    .dma_priority = 3,
    .irq_priority = 3,
    .direction = DMA_DIR_M2P,

    .psize = timerWidthInBytes,
    .msize = timerWidthInBytes,
#if __DCACHE_PRESENT
    .dcache_memory_in_use = config->dcache_memory_in_use,
#endif
    .inc_peripheral_addr = false,
    .inc_memory_addr = true,
    .op_mode = DMA_ONESHOT,
    .error_cb = NULL,
    .end_cb = NULL,
    .pburst = 0,
    .mburst = 0,
    .fifo = 4
  };

  driver->pwm_conf = (PWMConfig) {
  .frequency = TICK_FREQ,
  .period    = TICKS_PER_PERIOD,
  .callback  = NULL,
  .channels  = {
    {.mode = pwmPolarity,
     .callback = NULL},
    {.mode = DSHOT_CHANNELS > 1 ? pwmPolarity : PWM_OUTPUT_DISABLED,
     .callback = NULL},
    {.mode = DSHOT_CHANNELS > 2 ? pwmPolarity : PWM_OUTPUT_DISABLED,
     .callback = NULL},
    {.mode = DSHOT_CHANNELS > 3 ? pwmPolarity : PWM_OUTPUT_DISABLED,
     .callback = NULL},
  },
  .cr2  =  STM32_TIM_CR2_CCDS,
  .dier =  STM32_TIM_DIER_UDE
  };

  driver->crc_errors = 0;
  driver->tlm_frame_nb = 0;
#if DSHOT_SPEED == 0
  driver->bit0Duty = (DSHOT_PWM_PERIOD * 373U / 1000U);
  driver->bit1Duty = (driver->bit0Duty*2U)            ;
#endif



  dmaObjectInit(&driver->dmap);
  chMBObjectInit(&driver->mb, driver->_mbBuf, ARRAY_LEN(driver->_mbBuf));

  const bool dmaOk = dmaStart(&driver->dmap, &driver->dma_conf);
  chDbgAssert(dmaOk == true, "dshot dma start error");

  if (driver->config->tlm_sd) {
    sdStart(driver->config->tlm_sd, &tlmcfg);
    chThdCreateStatic(driver->waDshotTlmRec, sizeof(driver->waDshotTlmRec), NORMALPRIO,
		      dshotTlmRec, driver);
  }

  pwmStart(driver->config->pwmp, &driver->pwm_conf);
  driver->config->pwmp->tim->DCR = DCR_DBL | DCR_DBA(driver->config->pwmp); // enable bloc register DMA transaction
  pwmChangePeriod(driver->config->pwmp, DSHOT_PWM_PERIOD);

  for (size_t j=0; j<DSHOT_CHANNELS; j++) {
    pwmEnableChannel(driver->config->pwmp, j, 0);
    driver->dshotMotors.dp[j] =  makeDshotPacket(0,0);
    chMtxObjectInit(&driver->dshotMotors.tlmMtx[j]);
  }
  driver->dshotMotors.onGoingQry = false;
  driver->dshotMotors.currentTlmQry = 0U;
}

/**
 * @brief   stop the DSHOT peripheral and free the 
 *          related resources : pwm driver and dma driver.
 *
 * @param[in] driver    pointer to the @p DSHOTDriver object
 * @api
 */
void     dshotStop(DSHOTDriver *driver)
{
  pwmStop(driver->config->pwmp);
  dmaStopTransfert(&driver->dmap);
  dmaStop(&driver->dmap);
}


void dshotRestart(DSHOTDriver *driver)
{
  const bool dmaOk = dmaStart(&driver->dmap, &driver->dma_conf);
  chDbgAssert(dmaOk == true, "dshot dma start error");

  pwmStart(driver->config->pwmp, &driver->pwm_conf);
  driver->config->pwmp->tim->DCR = DCR_DBL | DCR_DBA(driver->config->pwmp); // enable bloc register DMA transaction
  pwmChangePeriod(driver->config->pwmp, DSHOT_PWM_PERIOD);

  for (size_t j=0; j<DSHOT_CHANNELS; j++) {
    pwmEnableChannel(driver->config->pwmp, j, 0);
    driver->dshotMotors.dp[j] =  makeDshotPacket(0,0);
  }
}


/**
 * @brief   prepare throttle order for specified ESC
 *
 * @param[in] driver    pointer to the @p DSHOTDriver object
 * @param[in] index     channel : [0..3] or [0..1] depending on driver used
 * @param[in] throttle  [48 .. 2047]
 * @note      dshotSendFrame has to be called after using this function
 * @note      see also dshotSendThrottles
 * @api
 */
void dshotSetThrottle(DSHOTDriver *driver, const uint8_t index,
                      const  uint16_t throttle)
{
  if (throttle > 0 && throttle <= DSHOT_CMD_MAX) {
    chDbgAssert(false, "dshotSetThrottle throttle error");
    return; // special commands (except MOTOR_STOP) can't be applied from this function
  } else {
    // send normal throttle
    if (index == DSHOT_ALL_MOTORS) {
      for (uint8_t _index = 0; _index < DSHOT_CHANNELS; _index++) {
        setDshotPacketThrottle(&driver->dshotMotors.dp[_index], MIN(throttle, DSHOT_MAX_VALUE));
      }
    } else if ((index - DSHOT_CHANNEL_FIRST_INDEX) < DSHOT_CHANNELS) {
      setDshotPacketThrottle(&driver->dshotMotors.dp[index - DSHOT_CHANNEL_FIRST_INDEX],
			     MIN(throttle, DSHOT_MAX_VALUE));
    } else {
      chDbgAssert(false, "dshotSetThrottle index error");
    }
  }
}

/**
 * @brief   send special order to one of the ESC (BHELIX, KISS, ...)
 *
 * @param[in] driver    pointer to the @p DSHOTDriver object
 * @param[in] index     channel : [0..3] or [0..1] depending on driver used
 * @param[in] specmd   special commands, see enum
 * @api
 */
void dshotSendSpecialCommand(DSHOTDriver *driver, const  uint8_t index,
			     const dshot_special_commands_t specmd)
{
  if (specmd > DSHOT_CMD_MAX) {
    return; // Don't apply special commands from this function
  }
  if (index < DSHOT_CHANNELS) {
    setDshotPacketThrottle(&driver->dshotMotors.dp[index], specmd);
    setDshotPacketTlm(&driver->dshotMotors.dp[index], driver->config->tlm_sd != NULL);
  } else if (index == DSHOT_ALL_MOTORS) {
    for (uint8_t _index = 0; _index < DSHOT_CHANNELS; _index++) {
      setDshotPacketThrottle(&driver->dshotMotors.dp[_index], specmd);
      setDshotPacketTlm(&driver->dshotMotors.dp[_index], driver->config->tlm_sd != NULL);
    }
  } else {
    chDbgAssert(false, "dshotSetThrottle index error");
  }

  // some dangerous special commands need to be repeated 6 times
  // to avoid catastrophic failure
  uint8_t repeat;
  switch (specmd) {
    case DSHOT_CMD_SPIN_DIRECTION_1:
    case DSHOT_CMD_SPIN_DIRECTION_2:
    case DSHOT_CMD_3D_MODE_OFF:
    case DSHOT_CMD_3D_MODE_ON:
    case DSHOT_CMD_SAVE_SETTINGS:
    case DSHOT_CMD_SETTINGS_REQUEST:
    case DSHOT_CMD_AUDIO_STREAM_MODE_ON_OFF:
    case DSHOT_CMD_SILENT_MODE_ON_OFF:
      repeat = 6;
      break;
    default:
      repeat = 1;
  }

  while (repeat--) {
    dshotSendFrame(driver);
    chThdSleepMicroseconds(500);
  }
}

/**
 * @brief   send throttle packed order to all of the ESCs
 *
 * @param[in] driver    pointer to the @p DSHOTDriver object
 * @param[in] throttle[DSHOT_CHANNELS]  [48 .. 2047]
 * @note dshotSendFrame is called by this function
 * @note telemetry bit is set in turn for each ESC of the ESCs
 * @api
 */
void dshotSendThrottles(DSHOTDriver *driver, const  uint16_t throttles[DSHOT_CHANNELS])
{
  for (uint8_t index = 0; index < DSHOT_CHANNELS; index++) {
    setDshotPacketThrottle(&driver->dshotMotors.dp[index], throttles[index]);
  }

  dshotSendFrame(driver);
}




/**
 * @brief   send throttle  order
 *
 * @param[in] driver    pointer to the @p DSHOTDriver object
 * @note dshotSetXXX api should be called prior to this function
 * @api
 */
void dshotSendFrame(DSHOTDriver *driver)
{
  if (driver->dmap.state == DMA_READY) {
    if ((driver->config->tlm_sd != NULL) &&
        (driver->dshotMotors.onGoingQry == false)) {
      driver->dshotMotors.onGoingQry = true;
      const msg_t index = (driver->dshotMotors.currentTlmQry + 1U) % DSHOT_CHANNELS;
      driver->dshotMotors.currentTlmQry = (uint8_t) index;
      setDshotPacketTlm(&driver->dshotMotors.dp[index], true);
      chMBPostTimeout(&driver->mb, index, TIME_IMMEDIATE);
    }

    buildDshotDmaBuffer(driver);
    if (driver->config->bidir == true) {
      dmaTransfert(&driver->dmap,
		   &driver->config->pwmp->tim->DMAR,
		   driver->config->dma_buf, DSHOT_DMA_BUFFER_SIZE * DSHOT_CHANNELS);
    } else {
      dmaStartTransfert(&driver->dmap,
			&driver->config->pwmp->tim->DMAR,
			driver->config->dma_buf, DSHOT_DMA_BUFFER_SIZE * DSHOT_CHANNELS);
    }
  }
}

/**
 * @brief   return number of telemetry crc error since dshotStart
 *
 * @param[in] driver    pointer to the @p DSHOTDriver object
 * @return    number of CRC errors
 * @api
 */
uint32_t dshotGetCrcErrorCount(const DSHOTDriver *driver)
{
  return driver->crc_errors;
}

/**
 * @brief   return number of telemetry succesfull frame since  dshotStart
 *
 * @param[in] driver    pointer to the @p DSHOTDriver object
 * @return    number of frames
 * @api
 */
uint32_t dshotGetTelemetryFrameCount(const DSHOTDriver *driver)
{
  return driver->tlm_frame_nb;
}

  
/**
 * @brief   return last received telemetry data
 *
 * @param[in] driver    pointer to the @p DSHOTDriver object
 * @param[in] index     channel : [0..3] or [0..1] depending on driver used
 * @return    telemetry structure by copy
 * @api
 */
DshotTelemetry dshotGetTelemetry(DSHOTDriver *driver, const uint32_t index)
{
  chDbgAssert(index <= DSHOT_CHANNELS, "dshot index error");
  chMtxLock(&driver->dshotMotors.tlmMtx[index]);
  const DshotTelemetry tlm = driver->dshotMotors.dt[index];
  chMtxUnlock(&driver->dshotMotors.tlmMtx[index]);
  return tlm;
}







/*
#                 _ __           _                    _
#                | '_ \         (_)                  | |
#                | |_) |  _ __   _   __   __   __ _  | |_     ___
#                | .__/  | '__| | |  \ \ / /  / _` | | __|   / _ \
#                | |     | |    | |   \ V /  | (_| | \ |_   |  __/
#                |_|     |_|    |_|    \_/    \__,_|  \__|   \___|
*/

static void setCrc4(DshotPacket *dp)
{
  // compute checksum
  dp->crc = 0;
  uint16_t csum = (dp->throttle << 1) | dp->telemetryRequest;
  for (int i = 0; i < 3; i++) {
    dp->crc ^=  csum;   // xor data by nibbles
    csum >>= 4;
  }
}

static DshotPacket makeDshotPacket(const uint16_t _throttle, const bool tlmRequest)
{
  DshotPacket dp = {.throttle = _throttle,
                    .telemetryRequest = (tlmRequest ? 1 : 0),
                    .crc = 0
                   };

  setCrc4(&dp);
  return dp;
}

static inline void setDshotPacketThrottle(DshotPacket *const dp, const uint16_t throttle)
{
  dp->throttle = throttle;
  dp->telemetryRequest = 0;
}

static inline void setDshotPacketTlm(DshotPacket *const dp, const bool tlmRequest)
{
  dp->telemetryRequest =  tlmRequest ? 1 : 0;
}

static void buildDshotDmaBuffer(DSHOTDriver *driver)
{
  DshotPackets *const dsp = &driver->dshotMotors;
  DshotDmaBuffer *const dma =  driver->config->dma_buf;
  const size_t timerWidth = getTimerWidth(driver->config->pwmp);
  
  for (size_t chanIdx = 0; chanIdx < DSHOT_CHANNELS; chanIdx++) {
    // compute checksum
    DshotPacket * const dp  = &dsp->dp[chanIdx];
    setCrc4(dp);
    if (driver->config->bidir == true)
      dp->crc = ~dp->crc; // crc is inverted when dshot bidir protocol is choosed
    // generate pwm frame
    for (size_t bitIdx = 0; bitIdx < DSHOT_BIT_WIDTHS; bitIdx++) {
      const uint16_t value = dp->rawFrame &
                             (1 << ((DSHOT_BIT_WIDTHS - 1) - bitIdx)) ?
                             DSHOT_BIT1_DUTY : DSHOT_BIT0_DUTY;
      if (timerWidth == 2) {
        dma->widths16[bitIdx+DSHOT_PRE_FRAME_SILENT_SYNC_BITS][chanIdx] = value;
      } else {
#if DSHOT_AT_LEAST_ONE_32B_TIMER
        dma->widths32[bitIdx+DSHOT_PRE_FRAME_SILENT_SYNC_BITS][chanIdx] = value;
#else
        chSysHalt("use of 32 bit timer implies to define DSHOT_AT_LEAST_ONE_32B_TIMER to TRUE");
#endif
      }
    }
    // the bits for silence sync (pre and post) in case of continous sending are zeroed once at init
  }

}


static inline uint8_t updateCrc8(uint8_t crc, uint8_t crc_seed)
{
  uint8_t crc_u = crc;
  crc_u ^= crc_seed;

  for (int i = 0; i < 8; i++) {
    crc_u = (crc_u & 0x80) ? 0x7 ^ (crc_u << 1) : (crc_u << 1);
  }

  return (crc_u);
}

static uint8_t calculateCrc8(const uint8_t *Buf, const uint8_t BufLen)
{
  uint8_t crc = 0;
  for (int i = 0; i < BufLen; i++) {
    crc = updateCrc8(Buf[i], crc);
  }

  return crc;
}




/*
#                 _      _                                 _
#                | |    | |                               | |
#                | |_   | |__    _ __    ___    __ _    __| |   ___
#                | __|  | '_ \  | '__|  / _ \  / _` |  / _` |  / __|
#                \ |_   | | | | | |    |  __/ | (_| | | (_| |  \__ \
#                 \__|  |_| |_| |_|     \___|  \__,_|  \__,_|  |___/
*/


static noreturn void dshotTlmRec (void *arg)
{
  DSHOTDriver *driver = (DSHOTDriver *) arg;
  DshotTelemetry tlm;
  
  msg_t escIdx = 0;

  chRegSetThreadName("dshotTlmRec");
  while (true) {
    chMBFetchTimeout(&driver->mb,  &escIdx, TIME_INFINITE);
    const uint32_t idx = escIdx;
    const bool success =
      (sdReadTimeout(driver->config->tlm_sd, tlm.frame.rawData, sizeof(DshotTelemetryFrame),
                     TIME_MS2I(1000)) == sizeof(DshotTelemetryFrame));
    if (!success ||
        (calculateCrc8(tlm.frame.rawData, sizeof(tlm.frame.rawData)) != tlm.frame.crc8)) {
      // empty buffer to resync
      while (sdGetTimeout(driver->config->tlm_sd, TIME_IMMEDIATE) >= 0) {};
      memset(tlm.frame.rawData, 0U, sizeof(DshotTelemetry));
      // count errors
      if (success)
	driver->crc_errors++;
    } else {
      // big-endian to little-endian conversion
      tlm.frame.voltage = __builtin_bswap16(tlm.frame.voltage);
      tlm.frame.current = __builtin_bswap16(tlm.frame.current);
      tlm.frame.consumption = __builtin_bswap16(tlm.frame.consumption);
      tlm.frame.rpm = __builtin_bswap16(tlm.frame.rpm);
      tlm.ts = chVTGetSystemTimeX();
      driver->tlm_frame_nb++;
    }
    
    chMtxLock(&driver->dshotMotors.tlmMtx[idx]);
    driver->dshotMotors.dt[idx] = tlm;
    chMtxUnlock(&driver->dshotMotors.tlmMtx[idx]);
    driver->dshotMotors.onGoingQry = false;
  }
}



/* static void dmaErrCb(DMADriver *dmap, dmaerrormask_t err) */
/* { */
/*   (void) dmap; */
/*   (void) err; */
/*   chDbgAssert(false, "dma error"); */
/* } */
