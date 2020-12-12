#pragma once

#include <ch.h>
#include <hal.h>
#include "hal_stm32_dma.h"
#include "esc_dshot_config.h"

#ifndef DSHOT_CHANNEL_FIRST_INDEX
#error DSHOT_CHANNEL_FIRST_INDEX must be defined
#define DSHOT_CHANNEL_FIRST_INDEX 0U
#endif

/**
 * @brief   special value for index : send order to all channels
 * @note    could be used as index in dshotSetThrottle and
 *          dshotSendSpecialCommand functions
 */
#define DSHOT_ALL_MOTORS 255


/*
  DshotSettingRequest (KISS24). Spin direction,
 3d and save Settings require 10 requests.. and the
 TLM Byte must always be high if 1-47 are used to send settings

  3D Mode:
  0 = stop
  48   (low) - 1047 (high) -> negative direction
  1048 (low) - 2047 (high) -> positive direction
 */

/**
 * @brief   DSHOT special commands (0-47) for KISS and BLHELI ESC
 * @note    commands 48-2047 are used to send motor power
 */
typedef enum {
    DSHOT_CMD_MOTOR_STOP = 0,
    DSHOT_CMD_BEACON1,
    DSHOT_CMD_BEACON2,
    DSHOT_CMD_BEACON3,
    DSHOT_CMD_BEACON4,
    DSHOT_CMD_BEACON5,
    DSHOT_CMD_ESC_INFO, // V2 includes settings
    DSHOT_CMD_SPIN_DIRECTION_1,
    DSHOT_CMD_SPIN_DIRECTION_2,
    DSHOT_CMD_3D_MODE_OFF,
    DSHOT_CMD_3D_MODE_ON,
    DSHOT_CMD_SETTINGS_REQUEST, // Currently not implemented
    DSHOT_CMD_SAVE_SETTINGS,
    DSHOT_CMD_SPIN_DIRECTION_NORMAL = 20,
    DSHOT_CMD_SPIN_DIRECTION_REVERSED = 21,
    DSHOT_CMD_LED0_ON, // BLHeli32 only
    DSHOT_CMD_LED1_ON, // BLHeli32 only
    DSHOT_CMD_LED2_ON, // BLHeli32 only
    DSHOT_CMD_LED3_ON, // BLHeli32 only
    DSHOT_CMD_LED0_OFF, // BLHeli32 only
    DSHOT_CMD_LED1_OFF, // BLHeli32 only
    DSHOT_CMD_LED2_OFF, // BLHeli32 only
    DSHOT_CMD_LED3_OFF, // BLHeli32 only
    DSHOT_CMD_AUDIO_STREAM_MODE_ON_OFF = 30, // KISS audio Stream mode on/Off
    DSHOT_CMD_SILENT_MODE_ON_OFF = 31, // KISS silent Mode on/Off
    DSHOT_CMD_MAX = 47
} dshot_special_commands_t;

/**
 * @brief   telemetry packed as sent by some KISS ESC
 * @note    if other ESC use different binary representation in the future
 *          we'll have to add a little bit abstraction here
 */
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

typedef union {
#if DSHOT_AT_LEAST_ONE_32B_TIMER
  uint32_t widths32[DSHOT_DMA_BUFFER_SIZE][DSHOT_CHANNELS];
#endif
  uint16_t widths16[DSHOT_DMA_BUFFER_SIZE][DSHOT_CHANNELS];
} DshotDmaBuffer;   // alignment to satisfy dma requirement

/**
 * @brief   Type of a structure representing an DSHOT driver.
 */
typedef struct DSHOTDriver DSHOTDriver;


/**
 * @brief   DSHOT  Driver configuration structure.
 */
typedef struct  {
  /**
   * @brief : dma stream associated with pwm timer used to generate dshot output
   */
  uint32_t	dma_stream;

  /**
   * @brief : dma channel associated with pwm timer used to generate dshot output
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

  /**
   * @brief dshot dma buffer, sgould be defined in a non Dcached region
   */
  DshotDmaBuffer *dma_buf;

  /**
   * @brief   DMA memory is in a cached section and beed to be flushed
   */
  bool		 dcache_memory_in_use;
} DSHOTConfig;






void     dshotStart(DSHOTDriver *driver, const DSHOTConfig *config);
void     dshotSetThrottle(DSHOTDriver *driver, const uint8_t index, const uint16_t throttle);
void     dshotSendFrame(DSHOTDriver *driver);
void     dshotSendThrottles(DSHOTDriver *driver, const uint16_t throttles[DSHOT_CHANNELS]);
void     dshotSendSpecialCommand(DSHOTDriver *driver, const uint8_t index, const dshot_special_commands_t specmd);

uint32_t dshotGetCrcErrorsCount(DSHOTDriver *driver);
const DshotTelemetry *dshotGetTelemetry(const DSHOTDriver *driver, const uint32_t index);


/*
#                 _ __           _                    _
#                | '_ \         (_)                  | |
#                | |_) |  _ __   _   __   __   __ _  | |_     ___
#                | .__/  | '__| | |  \ \ / /  / _` | | __|   / _ \
#                | |     | |    | |   \ V /  | (_| | \ |_   |  __/
#                |_|     |_|    |_|    \_/    \__,_|  \__|   \___|
*/

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
  uint8_t  currentTlmQry;
  volatile bool	    onGoingQry;
} DshotPackets;


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
};
