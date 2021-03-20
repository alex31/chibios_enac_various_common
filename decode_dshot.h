#pragma once

#include <ch.h>
#include <hal.h>
#include "hal_stm32_dma.h"

#define DSHOT_MIN_THROTTLE 48U
#define DSHOT_MAX_THROTTLE 2047U

#ifdef __cplusplus
extern "C" {
#endif




/**
 * @brief   Type of a structure representing an DecodeDSHOT driver.
 */
typedef struct DecodeDSHOTDriver DecodeDSHOTDriver;


/**
 * @brief   DecodeDSHOT  Driver configuration structure.
 */
typedef struct  {
  /**
   * @brief : dma stream associated with icu timer used to generate decode_dshot output
   */
  uint32_t	dma_stream;

  /**
   * @brief : dma channel associated with icu timer used to generate decode_dshot output
   */
  uint8_t	dma_channel;

  /**
   * @brief ICU driver that can listen on channel 1 ou channel 2
   */
  ICUDriver	*icup;
  icuchannel_t  icu_channel;
} DecodeDSHOTConfig;

  typedef  enum  {ESC_NONE, ESC_PWM50, ESC_PWM400,
		   ESC_DSHOT150, ESC_DSHOT300, ESC_DSHOT600} EscCmdMode;
  
  typedef  struct  {
    uint32_t w;
    uint32_t p;
  } DecodePWMFrame ;

  typedef union {
  struct {
    uint16_t crc:4;
    uint16_t telemetryRequest:1;
    uint16_t throttle:11;
  };
  uint16_t rawFrame;
}  DshotPacket;

/*   typedef struct  { */
/*     uint16_t    dshotFrequency; */
/*     DshotPacket packet; */
/* } DecodeDSHOTFrame; */

  typedef struct  {
    EscCmdMode mode;
    union{
      DecodePWMFrame pwmf;
      DshotPacket dshotp;
    };
} DecodeESCFrame;



void             decodeDshotStart(DecodeDSHOTDriver *driver, const DecodeDSHOTConfig *config);
DecodeESCFrame   decodeDshotCapture(DecodeDSHOTDriver *driver);
void             decodeDshotStop(DecodeDSHOTDriver *driver);


/*
#                 _ __           _                    _
#                | '_ \         (_)                  | |
#                | |_) |  _ __   _   __   __   __ _  | |_     ___
#                | .__/  | '__| | |  \ \ / /  / _` | | __|   / _ \
#                | |     | |    | |   \ V /  | (_| | \ |_   |  __/
#                |_|     |_|    |_|    \_/    \__,_|  \__|   \___|
*/
#define DMA_DATA_LEN   64U


  typedef union  {
    DecodePWMFrame wp[DMA_DATA_LEN / 2];
    uint32_t    raw[DMA_DATA_LEN];
  } WidthPeriodBuffer;



/**
 * @brief   DecodeDSHOT  driver structure.
 */
struct  DecodeDSHOTDriver {
  /**
   * @brief DMA config associated with icu timer
   */
  const DecodeDSHOTConfig	*config;

  /**
   * @brief DMA config associated with icu timer
   */
  DMAConfig	dma_conf;

  /**
   * @brief ICU config associated with icu timer
   */
  ICUConfig	icu_conf;

  /**
   * @brief DMA driver associated with icu timer
   */
  DMADriver	dmap;

  /**
   * @brief DMA buffer to store widths and periods
   */
  WidthPeriodBuffer wp_buffer;

  /**
   * @brief timer width in bytes : 2 or 4
   */
  uint8_t		timer_width;
};

#ifdef __cplusplus
 }
#endif
