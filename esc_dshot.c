#include "esc_dshot.h"
#include <stdnoreturn.h>
#include <math.h>
#include <string.h>
#include "stdutil.h"

/*
  TODO:

  ° TEST telemetry driving more than one ESC
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
			                  // after pwm init
#define DSHOT_SPEED (DSHOT_SPEED_KHZ*1000)
#define TICK_FREQ (PWM_FREQ * TICKS_PER_PERIOD)
#define DSHOT_PWM_PERIOD (TICK_FREQ/DSHOT_SPEED)
#define DSHOT_BIT0_DUTY (DSHOT_PWM_PERIOD * 373 / 1000)
#define DSHOT_BIT1_DUTY (DSHOT_BIT0_DUTY*2)		  
#define    DCR_DBL              ((DSHOT_CHANNELS-1) << 8) //  DSHOT_CHANNELS transfert(s)
// first register to get is CCR1
#define DCR_DBA(pwmd)                 (((uint32_t *) (&pwmd->tim->CCR) - ((uint32_t *) pwmd->tim)))
 





/*
#                 _ __                   _              _      _   _    _ __                 
#                | '_ \                 | |            | |    | | | |  | '_ \                
#                | |_) |  _ __    ___   | |_     ___   | |_   | |_| |  | |_) |   ___         
#                | .__/  | '__|  / _ \  | __|   / _ \  | __|   \__, |  | .__/   / _ \        
#                | |     | |    | (_) | \ |_   | (_) | \ |_     __/ |  | |     |  __/        
#                |_|     |_|     \___/   \__|   \___/   \__|   |___/   |_|      \___|        
*/
static DshotPacket makeDshotPacket(const uint16_t throttle, const bool tlmRequest);
static inline void setDshotPacketThrottle(DshotPacket * const dp, const uint16_t throttle);
static inline void setDshotPacketTlm(DshotPacket * const dp, const bool tlmRequest);
static void buildDshotDmaBuffer(DshotPackets * const dsp,  DshotDmaBuffer * const dma);
static inline uint8_t updateCrc8(uint8_t crc, uint8_t crc_seed);
static uint8_t calculateCrc8(const uint8_t *Buf, const uint8_t BufLen);
static noreturn void dshotTlmRec (void *arg);

/*
#                         _ __    _          
#                        | '_ \  (_)         
#                  __ _  | |_) |  _          
#                 / _` | | .__/  | |         
#                | (_| | | |     | |         
#                 \__,_| |_|     |_|         
*/
void dshotStart(DSHOTDriver *driver, const DSHOTConfig *config)
{
  static const SerialConfig  tlmcfg =  {
    .speed = DSHOT_TELEMETRY_BAUD,
    .cr1 = 0,                                      // pas de parité
    .cr2 = USART_CR2_STOP1_BITS,			 // 1 bit de stop
    .cr3 = 0                                       // pas de controle de flux hardware (CTS, RTS)
  };
  
  driver->config = config;
  
  driver->dma_conf = (DMAConfig) {
    .stream = config->dma_stream,
    .channel = config->dma_channel,
    .dma_priority = 2,
    .irq_priority = 6,
    .direction = DMA_DIR_M2P,
    .psize = sizeof(timer_reg_t), // if we change for a 32 bit timer just have to change
    .msize = sizeof(timer_reg_t), // type of width array
    .inc_peripheral_addr = false,
    .inc_memory_addr = true,
    .circular = false,
    .error_cb = NULL,
    .end_cb = NULL,
    .pburst = 0,
    // use mburst only if buffer size satisfy aligmnent requirement
    .mburst = DSHOT_DMA_BUFFER_SIZE % (sizeof(timer_reg_t) * 4) ? 0 : 4,
    .fifo = 0
  };

  driver->pwm_conf = (PWMConfig) {     
  .frequency = TICK_FREQ,       
  .period    = TICKS_PER_PERIOD,
  .callback  = NULL,            
  .channels  = {
    {.mode = PWM_OUTPUT_ACTIVE_HIGH,
     .callback = NULL},
    {.mode = DSHOT_CHANNELS > 1 ? PWM_OUTPUT_ACTIVE_HIGH : PWM_OUTPUT_DISABLED,
     .callback = NULL},
    {.mode = DSHOT_CHANNELS > 2 ? PWM_OUTPUT_ACTIVE_HIGH : PWM_OUTPUT_DISABLED,
     .callback = NULL},
    {.mode = DSHOT_CHANNELS > 3 ? PWM_OUTPUT_ACTIVE_HIGH : PWM_OUTPUT_DISABLED,
     .callback = NULL},
  },
  .cr2  =  STM32_TIM_CR2_CCDS, 
  .dier =  STM32_TIM_DIER_UDE
  };

  driver->crc_errors = 0;
  
  dmaObjectInit(&driver->dmap);
  chMBObjectInit (&driver->mb, driver->_mbBuf, ARRAY_LEN(driver->_mbBuf));

  dmaStart(&driver->dmap, &driver->dma_conf);

  if (driver->config->tlm_sd) {
    sdStart(driver->config->tlm_sd, &tlmcfg);
    chThdCreateStatic(driver->waDshotTlmRec, sizeof(driver->waDshotTlmRec), NORMALPRIO,
		      dshotTlmRec, driver);
  }
  
  pwmStart(driver->config->pwmp, &driver->pwm_conf);
  driver->config->pwmp->tim->DCR = DCR_DBL | DCR_DBA(driver->config->pwmp); // enable bloc register DMA transaction
  pwmChangePeriod(driver->config->pwmp, DSHOT_PWM_PERIOD);

  for (size_t j=0;j<DSHOT_CHANNELS;j++) {
    pwmEnableChannel(driver->config->pwmp, j, 0);
    driver->dshotMotors.dp[j] =  makeDshotPacket(0,0);
  }

}



void dshotSetThrottle(DSHOTDriver *driver, const  uint8_t index,
		      const  uint16_t throttle)
{
  if (index < DSHOT_CHANNELS) {
    setDshotPacketThrottle(&driver->dshotMotors.dp[index], throttle);
  } else if (index == DSHOT_ALL_MOTORS) {
    for (uint8_t _index=0; _index < DSHOT_CHANNELS; _index++)
      setDshotPacketThrottle(&driver->dshotMotors.dp[_index], throttle);
  }
}

void dshotSendSpecialCommand(DSHOTDriver *driver, const  uint8_t index,
			     const dshot_special_commands_t specmd)
{
  if (index < DSHOT_CHANNELS) {
    setDshotPacketThrottle(&driver->dshotMotors.dp[index], specmd);
    setDshotPacketTlm(&driver->dshotMotors.dp[index], true);
  } else if (index == DSHOT_ALL_MOTORS) {
    for (uint8_t _index=0; _index < DSHOT_CHANNELS; _index++) {
      setDshotPacketThrottle(&driver->dshotMotors.dp[_index], specmd);
      setDshotPacketTlm(&driver->dshotMotors.dp[_index], true);
    }
  }
  
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
    repeat = 10;
    break;
  default:
    repeat = 1;
  }

  while (repeat--) {
    dshotSendFrame(driver);
    chThdSleepMilliseconds(1);
  }
}

void dshotSendThrottles(DSHOTDriver *driver, const  uint16_t throttles[DSHOT_CHANNELS])
{
  for (uint8_t index=0; index < DSHOT_CHANNELS; index++)
    setDshotPacketThrottle(&driver->dshotMotors.dp[index], throttles[index]);
  
  dshotSendFrame(driver);
}




void dshotSendFrame(DSHOTDriver *driver)
{
   if (driver->dmap.state == DMA_READY) {
     if ((driver->config->tlm_sd != NULL) &&
	 (driver->dshotMotors.onGoingQry == false)) {
       driver->dshotMotors.onGoingQry = true;
       const uint32_t index = (driver->dshotMotors.currentTlmQry + 1) % DSHOT_CHANNELS;
       driver->dshotMotors.currentTlmQry = index;
       setDshotPacketTlm(&driver->dshotMotors.dp[index], true);
       chMBPostTimeout(&driver->mb, driver->dshotMotors.currentTlmQry, TIME_IMMEDIATE);
     }
   
     buildDshotDmaBuffer(&driver->dshotMotors, &driver->dsdb);
     dmaStartTransfert(&driver->dmap,
		       &driver->config->pwmp->tim->DMAR,
		       &driver->dsdb, DSHOT_DMA_BUFFER_SIZE * DSHOT_CHANNELS);
   
   }
}

uint32_t dshotGetCrcErrorsCount(DSHOTDriver *driver)
{
  return driver->crc_errors;
}

const DshotTelemetry * dshotGetTelemetry(const DSHOTDriver *driver, const uint32_t index)
{
  return &driver->dshotMotors.dt[index];
}







/*
#                 _ __           _                    _                   
#                | '_ \         (_)                  | |                  
#                | |_) |  _ __   _   __   __   __ _  | |_     ___         
#                | .__/  | '__| | |  \ \ / /  / _` | | __|   / _ \        
#                | |     | |    | |   \ V /  | (_| | \ |_   |  __/        
#                |_|     |_|    |_|    \_/    \__,_|  \__|   \___|        
*/
static DshotPacket makeDshotPacket(const uint16_t _throttle, const bool tlmRequest)
{
  DshotPacket dp = {.throttle = _throttle,
		    .telemetryRequest =  (tlmRequest ? 1 : 0),
		    .crc = 0};
  
  // compute checksum
  uint16_t csum = (_throttle << 1) | dp.telemetryRequest;
  for (int i = 0; i < 3; i++) {
    dp.crc ^=  csum;   // xor data by nibbles
    csum >>= 4;
  }
  
  return dp;
}

static inline void setDshotPacketThrottle(DshotPacket * const dp, const uint16_t throttle)
{
  dp->throttle = throttle;
  dp->telemetryRequest = 0;
}

static inline void setDshotPacketTlm(DshotPacket * const dp, const bool tlmRequest)
{
  dp->telemetryRequest =  tlmRequest ? 1 : 0;
}

static void buildDshotDmaBuffer(DshotPackets * const dsp,  DshotDmaBuffer * const dma)
{
  for (size_t chanIdx=0; chanIdx < DSHOT_CHANNELS; chanIdx++) {
  // compute checksum
    DshotPacket * const dp = &dsp->dp[chanIdx];
    dp->crc = 0;
    uint16_t csum = (dp->throttle << 1) | dp->telemetryRequest;
    for (int i = 0; i < 3; i++) {
      dp->crc ^=  csum;   // xor data by nibbles
      csum >>= 4;
    }
    // generate pwm frame
    for (size_t bitIdx=0; bitIdx < DSHOT_BIT_WIDTHS; bitIdx++) {
      dma->widths[bitIdx][chanIdx] = dsp->dp[chanIdx].rawFrame &
	(1 << ((DSHOT_BIT_WIDTHS -1) - bitIdx)) ?
	DSHOT_BIT1_DUTY : DSHOT_BIT0_DUTY;
    }
    // silence for sync in case of continous sending
    for (size_t bitIdx=DSHOT_BIT_WIDTHS; bitIdx < DSHOT_DMA_BUFFER_SIZE; bitIdx++) {
      dma->widths[bitIdx][chanIdx] = 0;
    }
  }
}

static inline uint8_t updateCrc8(uint8_t crc, uint8_t crc_seed)
{
    uint8_t crc_u = crc;
    crc_u ^= crc_seed;

    for (int i=0; i<8; i++) {
        crc_u = ( crc_u & 0x80 ) ? 0x7 ^ ( crc_u << 1 ) : ( crc_u << 1 );
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
  
  uint32_t escIdx=0;
  
  chRegSetThreadName("dshotTlmRec");
  while (true) {
    chMBFetchTimeout(&driver->mb, (msg_t *) &escIdx, TIME_INFINITE);
    const uint32_t idx = escIdx;
    const bool success =
      (sdReadTimeout(driver->config->tlm_sd, driver->dshotMotors.dt[idx].rawData, sizeof(DshotTelemetry),
		     TIME_MS2I(DSHOT_TELEMETRY_TIMEOUT_MS)) == sizeof(DshotTelemetry));
    if (!success ||
	(calculateCrc8(driver->dshotMotors.dt[idx].rawData, 
		       sizeof(driver->dshotMotors.dt[idx].rawData)) != driver->dshotMotors.dt[idx].crc8)) {
      // empty buffer to resync
      while (sdGetTimeout(driver->config->tlm_sd, TIME_IMMEDIATE) >= 0) {};
      bzero(driver->dshotMotors.dt[idx].rawData, sizeof(DshotTelemetry));
      // count errors
      driver->crc_errors++;
    }
    driver->dshotMotors.onGoingQry = false;
  }
}


