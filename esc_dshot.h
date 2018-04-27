#pragma once

#include <ch.h>
#include <hal.h>
#include "hal_stm32_dma.h"
#include "esc_dshot_config.h"

/**
 * @brief   Driver state machine possible states.
 */
typedef enum {
  DSHOT_UNINIT = 0,                       /**< Not initialized.          */
  DSHOT_STOP,                             /**< Stopped.                  */
  DSHOT_READY,                            /**< Ready.                    */
  DSHOT_ONGOING_TELEMETRY_QUERY,          /**< Transfering.              */
  DSHOT_ERROR                             /**< Transfert error.          */
} dshotstate_t;

typedef struct {
  union {
    struct {
      uint8_t  temp;
      uint16_t voltage;
      uint16_t current;
      uint16_t consumption;
      uint16_t rpm;
    } __attribute__ ((__packed__, scalar_storage_order ("big-endian")));
    uint8_t rawData[9];
  };
  uint8_t  crc8;
}  __attribute__ ((__packed__)) DshotTelemetry ;


/**
 * @brief   Type of a structure representing an DSHOT driver.
 */
typedef struct DSHOTDriver DSHOTDriver;


/**
 * @brief   DSHOT  configuration structure.
 */
typedef struct  {
  /**
   * @brief : dma stream associated with pwm timer
   */
  uint32_t	dma_stream;

  /**
   * @brief : dma channel associated with pwm timer
   */
  uint8_t	dma_channel;

  /**
   * @brief PWM driver that feed up to 4 dshot lines 
   */
  PWMDriver	*pwmp;
  
  /**
   * @brief if non null : dshot telemetry serial driver
   */
  SerialDriver	*tlm_sd;
} DSHOTConfig;





  
void     dshotStart(DSHOTDriver *driver, const DSHOTConfig *config);
void     dshotSetThrottle(DSHOTDriver *driver, const  uint8_t index, const  uint16_t throttle);
void     dshotSendFrame(DSHOTDriver *driver);
void     dshotSendThrottles(DSHOTDriver *driver, const  uint16_t throttles[DSHOT_CHANNELS]);

uint32_t dshotGetCrcErrorsCount(DSHOTDriver *driver);
const DshotTelemetry * dshotGetTelemetry(const DSHOTDriver *driver, const uint32_t index);


/*
#                 _ __           _                    _                   
#                | '_ \         (_)                  | |                  
#                | |_) |  _ __   _   __   __   __ _  | |_     ___         
#                | .__/  | '__| | |  \ \ / /  / _` | | __|   / _ \        
#                | |     | |    | |   \ V /  | (_| | \ |_   |  __/        
#                |_|     |_|    |_|    \_/    \__,_|  \__|   \___|        
*/
typedef uint16_t timer_reg_t;

typedef union {
  struct {
    uint16_t crc:4;
    uint16_t telemetryRequest:1;
    uint16_t throttle:11;
  };
  uint16_t rawFrame;
}  DshotPacket;


typedef struct {
  DshotPacket       dp[DSHOT_CHANNELS];
  DshotTelemetry    dt[DSHOT_CHANNELS];
  volatile uint8_t  currentTlmQry;
  volatile bool	    onGoingQry;
} DshotPackets;

typedef struct {
  // alignment to satisfy dma requirement
  timer_reg_t widths[DSHOT_DMA_BUFFER_SIZE][DSHOT_CHANNELS] __attribute__((aligned(16)));
} DshotDmaBuffer;

/**
 * @brief   DSHOT  driver structure.
 */
struct  DSHOTDriver {
  /**
   * @brief DMA config associated with pwm timer
   */
  const DSHOTConfig	*config;

  /**
   * @brief DMA config associated with pwm timer
   */
  DMAConfig	dma_conf;

  /**
   * @brief PWM config associated with pwm timer
   */
  PWMConfig	pwm_conf;

  /**
   * @brief DMA driver associated with pwm timer
   */
  DMADriver	dmap;

  /**
   * @brief mailbox buffer for dshot telemetry thread
   */
  msg_t  _mbBuf[1];

  /**
   * @brief mailbox for dshot telemetry thread
   */
  mailbox_t mb;

  /**
   * @brief number of crc errors
   */
  uint32_t crc_errors;

  
  /**
   * @brief stack working area for dshot telemetry thread
   */
  THD_WORKING_AREA(waDshotTlmRec, 512);

  DshotPackets dshotMotors;
  DshotDmaBuffer dsdb;
};
