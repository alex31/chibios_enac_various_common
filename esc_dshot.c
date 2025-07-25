#include "esc_dshot.h"
#include <stdnoreturn.h>
#include <math.h>
#include <string.h>
#include "stdutil.h"




/*
#                     _            __   _            _    _      _
#                    | |          / _| (_)          (_)  | |    (_)
#                  __| |    ___  | |_   _    _ __    _   | |_    _     ___    _ __
#                 / _` |   / _ \ |  _| | |  | '_ \  | |  | __|  | |   / _ \  | '_ \
#                | (_| |  |  __/ | |   | |  | | | | | |  \ |_   | |  | (_) | | | | |
#                 \__,_|   \___| |_|   |_|  |_| |_| |_|   \__|  |_|   \___/  |_| |_|
*/
#ifdef STM32H7XX
// each H7 timer have the same max clock speed
#define PWM_FREQ         (STM32_TIMCLK1 / 1000U) // the timer will beat @240Mhz on STM32H7
#elsifdef STM32G4XX
#define PWM_FREQ         (STM32_SYSCLK / 1000U) // the timer will beat @170Mhz on STM32G4
#else
// some F4 and F7 timers are limited to  / 2
// others are limited to STM32_SYSCLK
// so we take the max frequency that all timers can run
#define PWM_FREQ         (STM32_SYSCLK / 2000U) // the timer will beat @84Mhz on STM32F4
#endif
#define TICKS_PER_PERIOD 1000             // that let use any timer :
			                  // does not care if linked to PCLK1 or PCLK2
			                  // tick_per_period will be dynamically calculated

/** Base freq of DSHOT signal (in kHz)
 * Possible values are: 150, 300, 600
 */
#ifndef DSHOT_SPEED
#define DSHOT_SPEED 300U
#endif

#ifndef DSHOT_TELEMETRY_BAUD
#define DSHOT_TELEMETRY_BAUD 115200U
#endif

#ifndef DSHOT_BIDIR_EXTENTED_TELEMETRY
#define DSHOT_BIDIR_EXTENTED_TELEMETRY	FALSE
#endif

// ESCs are quite sensitive to the DSHOT duty cycle.
// 333 should work most of the time, but some ESC need 373
// so this parameter can be changed in esc_dshot_config.h
#ifndef DSHOT_BIT0_DUTY_RATIO
#define DSHOT_BIT0_DUTY_RATIO 333U
#endif

#if DSHOT_SPEED != 0 // statically defined
#   define DSHOT_FREQ (DSHOT_SPEED*1000)
#   define DSHOT_BIT0_DUTY (DSHOT_PWM_PERIOD * DSHOT_BIT0_DUTY_RATIO  / 1000U)
#   define DSHOT_BIT1_DUTY (DSHOT_BIT0_DUTY*2)
#else			 // dynamically defined
#   define DSHOT_FREQ (driver->config->speed_khz * 1000U)
#   define DSHOT_BIT0_DUTY (driver->bit0Duty)
#   define DSHOT_BIT1_DUTY (driver->bit1Duty)
#endif
#define TICK_FREQ (PWM_FREQ * TICKS_PER_PERIOD)
#define DSHOT_PWM_PERIOD (TICK_FREQ/DSHOT_FREQ)

#define    DCR_DBL              ((DSHOT_CHANNELS-1) << 8) //  DSHOT_CHANNELS transfert(s)
// first register to get is CCR1
#define DCR_DBA(pwmd)                 (((uint32_t *) (&pwmd->tim->CCR) - ((uint32_t *) pwmd->tim)))


#define DSHOT_MAX_VALUE ((1U<<11U)-1U) // 11 bits used to send command, so maximum value is 2047


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
//static void dshotSendFrameNoAnswer(DSHOTDriver *driver);
#if DSHOT_BIDIR
static void processBidirErpm(DSHOTDriver *driver);
static void     dshotRestart(DSHOTDriver *driver);
#if DSHOT_BIDIR_EXTENTED_TELEMETRY
static void updateTelemetryFromBidirEdt(const DshotErps *erps, DshotTelemetry *tlm);
#endif
#endif
  
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
 * @brief   Configures and activates the DSHOT driver.
 *
 * @param[in] driver    pointer to the @p DSHOTDriver object
 * @param[in] config    pointer to the @p DSHOTConfig object.
 * @api
 */
void dshotStart(DSHOTDriver *driver, const DSHOTConfig *config)
{
  chDbgAssert(config->dma_command != NULL, ".dma_buf must reference valid DshotDmaBuffer object");

#if DSHOT_BIDIR
  dshotRpmCaptureStart(&driver->rpm_capture, &config->dma_capt_cfg, config->pwmp->tim);
#endif
  
  memset((void *) config->dma_command, 0, sizeof(*(config->dma_command)));
  const size_t timerWidthInBytes = getTimerWidth(config->pwmp);

  static const SerialConfig  tlmcfg =  {
    .speed = DSHOT_TELEMETRY_BAUD,
    .cr1 = 0,                          // pas de parité
    .cr2 = USART_CR2_STOP1_BITS,       // 1 bit de stop
    .cr3 = 0                           // pas de controle de flux hardware (CTS, RTS)
  };

  // when dshot is bidir, the polarity is inverted
  static const uint32_t pwmPolarity = DSHOT_BIDIR ? PWM_OUTPUT_ACTIVE_LOW : PWM_OUTPUT_ACTIVE_HIGH;

  driver->config = config;
  driver->dma_conf = (DMAConfig) {
    .stream = config->dma_stream,
#if STM32_DMA_SUPPORTS_DMAMUX
    .dmamux = config->dmamux,
#else
    .channel = config->dma_channel,
#endif
    .dma_priority = 3,
    .irq_priority = CORTEX_MAX_KERNEL_PRIORITY + 1,
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
#if STM32_DMA_ADVANCED
    .pburst = 0,
    .mburst = 0,
    .fifo = 4
#endif
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
  driver->bit0Duty = (DSHOT_PWM_PERIOD * DSHOT_BIT0_DUTY_RATIO / 1000U);
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
 * @brief   stop the DSHOT driver and free the 
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

#if DSHOT_BIDIR
/**
 * @brief   restart the driver in outgoing mode
 *          
 *
 * @param[in] driver    pointer to the @p DSHOTDriver object
 * @api
 */
static void dshotRestart(DSHOTDriver *driver)
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
#endif

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
    case DSHOT_CMD_BIDIR_EDT_MODE_ON:
    case DSHOT_CMD_BIDIR_EDT_MODE_OFF:
      repeat = 6;
      break;
    default:
      repeat = 1;
  }

  while (repeat--) {
    systime_t now = chVTGetSystemTimeX();
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
    dshotSendFrame(driver);
    if (repeat)
      chThdSleepUntilWindowed(now, now + TIME_US2I(500));
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
#if DSHOT_BIDIR
    const tprio_t currentPrio = chThdSetPriority(HIGHPRIO);
#endif
    if ((driver->config->tlm_sd != NULL) &&
        (driver->dshotMotors.onGoingQry == false)) {
      driver->dshotMotors.onGoingQry = true;
      const msg_t index = (driver->dshotMotors.currentTlmQry + 1U) % DSHOT_CHANNELS;
      driver->dshotMotors.currentTlmQry = (uint8_t) index;
      setDshotPacketTlm(&driver->dshotMotors.dp[index], true);
      chMBPostTimeout(&driver->mb, index, TIME_IMMEDIATE);
    }
#if  DSHOT_BIDIR
    dshotRpmResetCaptureBuffer(&driver->rpm_capture);
#endif
    buildDshotDmaBuffer(driver);
    dmaTransfert(&driver->dmap,
		 &driver->config->pwmp->tim->DMAR,
		 driver->config->dma_command, DSHOT_DMA_BUFFER_SIZE * DSHOT_CHANNELS);
    
#if DSHOT_BIDIR
    dshotStop(driver);
    dshotRpmCatchErps(&driver->rpm_capture);
    processBidirErpm(driver);
    dshotRestart(driver);
    chThdSetPriority(currentPrio);
#endif
  }
}

/* static void dshotSendFrameNoAnswer(DSHOTDriver *driver) */
/* { */
/*   if (driver->dmap.state == DMA_READY) { */
/* #if DSHOT_BIDIR */
/*     const tprio_t currentPrio = chThdSetPriority(HIGHPRIO); */
/* #endif */
/*     if ((driver->config->tlm_sd != NULL) && */
/*         (driver->dshotMotors.onGoingQry == false)) { */
/*       driver->dshotMotors.onGoingQry = true; */
/*       const msg_t index = (driver->dshotMotors.currentTlmQry + 1U) % DSHOT_CHANNELS; */
/*       driver->dshotMotors.currentTlmQry = (uint8_t) index; */
/*       setDshotPacketTlm(&driver->dshotMotors.dp[index], true); */
/*       chMBPostTimeout(&driver->mb, index, TIME_IMMEDIATE); */
/*     } */

/*     buildDshotDmaBuffer(driver); */
/*     dmaTransfert(&driver->dmap, */
/* 		 &driver->config->pwmp->tim->DMAR, */
/* 		 driver->config->dma_command, DSHOT_DMA_BUFFER_SIZE * DSHOT_CHANNELS); */
    
/* #if DSHOT_BIDIR */
/*     chThdSetPriority(currentPrio); */
/* #endif */
/*   } */
/* } */

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

    [2] 0010 mmmm mmmm - Temperature frame in degree Celsius, just like Blheli_32 and KISS [0, 1, ..., 255]
    [4] 0100 mmmm mmmm - Voltage frame with a step size of 0,25V [0, 0.25 ..., 63,75]
    [6] 0110 mmmm mmmm - Current frame with a step size of 1A [0, 1, ..., 255]
    [8] 1000 mmmm mmmm - Debug frame 1 not associated with any specific value, can be used to debug ESC firmware
    [10] 1010 mmmm mmmm - Debug frame 2 not associated with any specific value, can be used to debug ESC firmware
    [12] 1100 mmmm mmmm - Stress level frame [0, 1, ..., 255] (since v2.0.0)
    [14] 1110 mmmm mmmm - Status frame: Bit[7] = alert event, Bit[6] = warning event, Bit[5] = error event, Bit[3-1] - Max. stress level [0-15] (since v2.0.0)

 */
#if DSHOT_BIDIR && DSHOT_BIDIR_EXTENTED_TELEMETRY
static void updateTelemetryFromBidirEdt(const DshotErps *erps, DshotTelemetry *tlm)
{
  switch(DshotErpsEdtType(erps)) {
  case EDT_TEMP:
    tlm->frame.temp = DshotErpsEdtTempCentigrade(erps); break;
    
  case EDT_VOLT:
    tlm->frame.voltage = DshotErpsEdtCentiVolts(erps); break;
    
  case EDT_CURRENT:
    tlm->frame.current = DshotErpsEdtCurrentAmp(erps) * 100U; break;

  case EDT_STRESS:
   tlm->stress = DshotErpsEdtStress(erps); break;

  case EDT_STATUS:
    tlm->status = DshotErpsEdtStatus(erps);break;
    
  default: {};
  }
  tlm->ts = chVTGetSystemTimeX();
}
#endif

#if DSHOT_BIDIR
/**
 * @brief   return last received telemetry ERPM data
 *
 * @param[in] driver    pointer to the @p DSHOTDriver object
 * @param[in] index     channel : [0..3]
 * @return    eperiod or special values DSHOT_BIDIR_ERR_CRC, DSHOT_BIDIR_TLM_EDT
 * @note     ° special values DSHOT_BIDIR_ERR_CRC, DSHOT_BIDIR_TLM_EDT
 *	     must be checked after every call to dshotGetEperiod     
 *           ° this fonction is less cpu intensive than dshotGetRpm
 */
uint32_t dshotGetEperiod(DSHOTDriver *driver, const uint32_t index)
{
  chDbgAssert(index < DSHOT_CHANNELS, "index check failed");
   DshotErpsSetFromFrame(&driver->erps,  driver->rpms_frame[index]);
   if (DshotErpsCheckCrc4(&driver->erps)) {
#if DSHOT_BIDIR_EXTENTED_TELEMETRY
     if (DshotErpsIsEdt(&driver->erps)) {
       if (driver->config->tlm_sd == NULL) {
	 DshotTelemetry *tlm = &driver->dshotMotors.dt[index];
	 updateTelemetryFromBidirEdt(&driver->erps, tlm);
       }
       return DSHOT_BIDIR_TLM_EDT;
     }
#endif
     return DshotErpsGetEperiod(&driver->erps);
   } else {
     return DSHOT_BIDIR_ERR_CRC;
   }
}
/**
 * @brief   return last received telemetry ERPM data
 *
 * @param[in] driver    pointer to the @p DSHOTDriver object
 * @param[in] index     channel : [0..3]
 * @return    erpm or special values DSHOT_BIDIR_ERR_CRC, DSHOT_BIDIR_TLM_EDT
 * @note     special values DSHOT_BIDIR_ERR_CRC, DSHOT_BIDIR_TLM_EDT
 *	     must be checked after every call to dshotGetRpm     
 */

uint32_t dshotGetRpm(DSHOTDriver *driver, const uint32_t index)
{
  chDbgAssert(index < DSHOT_CHANNELS, "index check failed");
   DshotErpsSetFromFrame(&driver->erps,  driver->rpms_frame[index]);
   if (DshotErpsCheckCrc4(&driver->erps)) {
#if DSHOT_BIDIR_EXTENTED_TELEMETRY
     if (DshotErpsIsEdt(&driver->erps)) {
       if (driver->config->tlm_sd == NULL) {
	 DshotTelemetry *tlm = &driver->dshotMotors.dt[index];
	 updateTelemetryFromBidirEdt(&driver->erps, tlm);
       }
       return DSHOT_BIDIR_TLM_EDT;
     }
#endif
     return DshotErpsGetRpm(&driver->erps);
   } else {
     return DSHOT_BIDIR_ERR_CRC;
   }
}
#endif



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
#if DSHOT_BIDIR
  dp->crc = ~(dp->crc); // crc is inverted when dshot bidir protocol is choosed
#endif
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
  DshotDmaBuffer *const dma =  driver->config->dma_command;
  const size_t timerWidth = getTimerWidth(driver->config->pwmp);
  
  for (size_t chanIdx = 0; chanIdx < DSHOT_CHANNELS; chanIdx++) {
    // compute checksum
    DshotPacket * const dp  = &dsp->dp[chanIdx];
    setCrc4(dp);
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


#if DSHOT_BIDIR
static void processBidirErpm(DSHOTDriver *driver)
{
  for (size_t idx = 0; idx < DSHOT_CHANNELS; idx++) {
    driver->rpms_frame[idx] = dshotRpmGetFrame(&driver->rpm_capture, idx);
  }
}
#endif


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
