#pragma once

#include <stdint.h>
#include <stdlib.h>
#include "frskyFportAppId.h"

#define FPORT_CONTROL_LEN 0x19U
#define FPORT_UPLINK_LEN 0x08U

  /**
   * @brief  number of 11 bits payload value in a FPORT frame
   */
#define FPORT_NUM_CONTROL_CHANNELS 16U

/**
   * @brief   maximum number of values to upload. 
   * @details -multiplexed values like GPS take 2 slots.
   *          -can be adjusted at compile time by defining the symbol prior to 
   *           include this header
   *          -define it to 0 to only receive control frame but ignore telemetry frames
   *           to basically use fport like sbus : save flash, ram and cpu cycles
   */
#ifndef FPORT_MAX_DATAIDS
#define FPORT_MAX_DATAIDS 20U
#endif

#ifdef __cplusplus
extern "C" {
#endif
  
  /**
   * @brief  kind of errors that can be returned by the driver
   */
  typedef enum  {FPORT_OK=0, FPORT_MALFORMED_FRAME=1<<0,
    FPORT_CRC_ERROR=1<<1, FPORT_APPID_LEN_EXCEDEED=1<<2,
    FPORT_TIMEOUT=1<<3, FPORT_FAILSAFE=1<<4, FPORT_RADIO_LINK_LOST=1<<5,
    FPORT_READBACK_CTRL_FAIL=1<<6, FPORT_INTERNAL_ERROR=1<<7
  } FportErrorMask;
  
  typedef enum {FPORT_FSM_WAIT_HEADER, FPORT_FSM_WAIT_LEN,
    FPORT_FSM_WAIT_MESSAGE} FportFsmStep;

  /**
   * @brief  uplink fields can be time multiplexed, see the fields description below
   */
  typedef enum __attribute__ ((__packed__)) {
    MULTIPLEX_1DATA=0x1, MULTIPLEX_2DATAS, MULTIPLEX_3DATAS, MULTIPLEX_4DATAS
  } FportMultiplexData;

  /**
   * @brief    function pointer use for (sent) telemetry answer callback
   */
  typedef void(*FportSendTelemetryAfter300UsFnPtr)(const void *buffer, size_t len,
						   void *optArg);
  
   /**
   * @brief    function pointer use for (received) control message callback
   */
  typedef struct Fport_ControlFrame_s Fport_ControlFrame;
  typedef void (*FportControlFnPtr) (const Fport_ControlFrame *, void *optArg);
  
  typedef struct {
    FportFsmStep step;
    size_t	 writeIdx:5; // how many bytes to complete a multibyte step
    bool	 escaped:1; // last received byte was escape
  } FportFsmState;

  _Static_assert(sizeof(FportFsmState) == 4, "incorrect state size");
  

 
  /**
   * @brief  control frame sent by controller
   */
   struct Fport_ControlFrame_s {
    /**
     * @brief   array of signed number in the range -1024 + 1023
     */
    int16_t channel[FPORT_NUM_CONTROL_CHANNELS];
    /**
     * @brief   sbus special flags
     */
    uint8_t type;
    uint8_t flags;
    uint8_t rssi;
   };

  /**
   * @brief  fields value, 32 bit fixed point value
   * @notes  floating point value can be used by custom messages
   */
  typedef union {
    uint32_t u32;
    int32_t  s32;
    float    f32;
  } FportTelemetryValue;

  /**
   * @brief   Structure representing a pair of appId and appValue
   *          for telemetry messages.
   */
   typedef struct {
    FportTelemetryValue value; // not natural order to avoid padding
    uint16_t key;              // between 16b and 32b fields 
  } fportAppIdPair;

  
  typedef struct {
    FportSendTelemetryAfter300UsFnPtr tlmSendCb;
    FportControlFnPtr ctrlReceiveCb;
    void	      *optArg;
    int16_t  middlePoint;
  } FportFsmContextConfig;

  typedef struct {
    const FportFsmContextConfig *config;
    size_t aliveLen;
    uint8_t msgBuf[FPORT_CONTROL_LEN + 2U];
    uint8_t aliveBuf[FPORT_UPLINK_LEN + 2U];
    uint8_t lastLen;
    FportFsmState state;
    Fport_ControlFrame	lastFrame;
    struct {
      fportAppIdPair appIds[FPORT_MAX_DATAIDS];
      uint8_t appIdsSize;
      uint8_t nextIdToSent;
    };
  } FportFsmContext;

  

  /**
   * @brief  the four flags fields (1 bit) that are in the control frame flags field
   * @notes  only 4 bits a are used in the lest significant nibble, 
   *         most significant nibble is always 0
   */
  typedef enum __attribute__ ((__packed__)) {
    FPORT_FLAG_CH17 = 0x01, FPORT_FLAG_CH18 = 0x02,
    FPORT_FLAG_FRAME_LOST = 0x04, FPORT_FLAG_FAILSAFE = 0x08, 
  } FportFlags;
  
  


  void		 fportContextInit(FportFsmContext *context, const FportFsmContextConfig *cfg);
  FportErrorMask fportFeedFsm(FportFsmContext *context, const void *buffer, size_t len);
  size_t	 fportGetNextReadLen(const FportFsmContext *context);
  
#if FPORT_MAX_DATAIDS > 0
 /**
   * @brief   update an appId telemetry value for *NON* multiplexed appId
   *
   * @param[in]  fportp     pointer to a @p FportFsmContext structure
   * @param[in]  appId      see above description attached to fportAppId enum
   * @param[in]  v          new assocoated value for appId
   */
  void fportTelemetryUpData(FportFsmContext *context, const fportAppId appId, 
			    const FportTelemetryValue v);
  
 /**
   * @brief   update an appId telemetry value for multiplexed appId
   *
   * @param[in]  fportp     pointer to a @p FportFsmContext structure
   * @param[in]  appId      see above description attached to fportAppId enum
   * @param[in]  v          array of values for appId
   * @param[in]  nbMplx     size of the above array
   */
  void fportTelemetryUpDataMultiplexed(FportFsmContext *context, const fportAppId appId, 
				       const FportTelemetryValue *v,
				       const FportMultiplexData mbMplx);
#endif

  /**
   * @brief   
   *
   * @param[in]  fportp     pointer to a @p FportFsmContext structure
   * @param[in]  channelIdx in range [0 .. 17]
   * @return     control channel sent by the controller 
   *             in the range [-1024 .. +1023] for channels [0 .. 15]
   *             in the range [0 .. 1] for channels [16 .. 17]
   * @notes      beware that we map channels [1 .. 18] to [0 .. 17]
   */
  static inline int16_t fportGetChannel(const FportFsmContext *context, const size_t channelIdx) {
    if (channelIdx < FPORT_NUM_CONTROL_CHANNELS) {
      return context->lastFrame.channel[channelIdx] - context->config->middlePoint;
    } else {
      switch(channelIdx) {
	// channels 17 and 18 are boolean channel : return true or false
      case 16 : return (context->lastFrame.flags & FPORT_FLAG_CH17) ? true : false;
      case 17 : return (context->lastFrame.flags & FPORT_FLAG_CH18) ? true : false;
      default:
	return 0xffff;
      }
    }
  }
    
 /**
   * @brief   return failsafe status
   *
   * @param[in]  context     pointer to a @p FportFsmContext structure
   * @return     boolean :  true : failsafe mode active
   */
  static inline bool fportIsFailSafeFlag(const FportFsmContext *context) {
    return (context->lastFrame.flags & FPORT_FLAG_FAILSAFE) ? true : false;
  }

 /**
   * @brief   return radio link lost status
   *
   * @param[in]  context     pointer to a @p FportFsmContext structure
   * @return     boolean :  true : radio link is lost
   */
  static inline bool fportIsFrameLostFlag(const FportFsmContext *context) {
    return (context->lastFrame.flags & FPORT_FLAG_FRAME_LOST) ? true : false;
  }
    
 /**
   * @brief   return rssi
   *
   * @param[in]  context     pointer to a @p FportFsmContext structure
   * @return     return radio signal strength indicator
   */
  static inline int16_t fportGetRssi(const FportFsmContext *context) {
    return context->lastFrame.rssi;
  }
    

  

  /**
   * @brief   helper fonction to transcode latitude 
   * from double to internal fport format
   * @param[in]  latitude as a double
   * @param[out]  latitude as an uint32_t encoded value
   */
  uint32_t fportTranscodeLat(double lat);

  
  /**
   * @brief   helper fonction to transcode longitude 
   * from double to internal fport format
   * @param[in]  longitude as a double
   * @param[out]  longitude as an uint32_t encoded value
   */
  uint32_t fportTranscodeLong(double lon);


  
#ifdef __cplusplus
}
#endif
