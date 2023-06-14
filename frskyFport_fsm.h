#pragma once

#include <stdint.h>
#include <stdlib.h>
#include "frskyFportAppId.h"

  /**
   * @brief  len of FPORT control message (downlink)
   */
#define FPORT_CONTROL_LEN 0x19U
  /**
   * @brief  len of FPORT telemetry message (downlink and uplink)
   */
#define FPORT_UPLINK_LEN  0x08U

  /**
   * @brief  number of 11 bits payload value in a FPORT frame
   * @note   taranis remote must be left wi0th 16 channels standard length
             smaller or longer frame is not yet implemented
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
  
  /**
   * @brief  private internal steps of the FSM 
   */
  typedef enum {FPORT_FSM_WAIT_HEADER, FPORT_FSM_WAIT_LEN,
    FPORT_FSM_WAIT_MESSAGE} FportFsmStep;

  /**
   * @brief  private internal state of the FSM 
   */
  typedef struct {
    FportFsmStep step;
    size_t	 writeIdx:5; // how many bytes to complete a multibyte step
    bool	 escaped:1;  // last received byte was escape
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
   * @notes  depending on message Ids, fields can be u32 or s32
   *         floating point f32 value can be used by custom messages
   */
  typedef union {
    uint32_t u32;
    int32_t  s32;
    float    f32;
  } FportTelemetryValue;

  /**
   * @brief   Structure representing a pair of appId and appValue
   *          for uplink telemetry messages.
   */
   typedef struct {
    FportTelemetryValue value; 
    uint16_t key;              
  } fportAppIdPair;
  
  /**
   * @brief   Structure representing configuration of the FSM Fport driver
   */
  typedef struct {
    /**
     * @brief   callback to a function that must send telemetry uplink message over uart
     * @note    this function, wich is architecture dependant, will be called in the context
                where fportFeedFsm has been called : if fportFeedFsm is called in thread context,
                tlmSendCb will be called in thread context also, otherwise, if fportFeedFsm
                is called on ISR context, tlmSendCb will also be called in ISR context.
     */
    FportSendTelemetryAfter300UsFnPtr tlmSendCb;

    /**
     * @brief   callback to a function that will receive control message
     * @note    this function will be called in the context
                where fportFeedFsm has been called : if fportFeedFsm is called in thread context,
                ctrlReceiveCb will be called in thread context also, otherwise, if fportFeedFsm
                is called on ISR context, ctrlReceiveCb will also be called in ISR context.
     */
    FportControlFnPtr ctrlReceiveCb;

    
    /**
     * @brief   optional user data
     * @note    can be used to transmit state to callbacks
     */
    void	      *optArg;

    /**
     * @brief   middle point for the downlink channels
     * @notes  channels are 11 bits wide, but depending on remote controller, configuration, 
               zero can be offsetted. it's usually 992 on taranis controller. 
     */
    int16_t  middlePoint;
  } FportFsmContextConfig;

  /**
   * @brief   Structure representing context for the Fport FSM
   */
  typedef struct {
    /**
     * @brief  pointer to configuration
     * @notes  beware to supply a pointer that whill remain valid (should not be on the stack)
     */
    const FportFsmContextConfig *config;

    /**
     * @brief  pre-calculated length of alive frame
     */
    size_t aliveLen;

    /**
     * @brief   buffer for the received frame
     */
    uint8_t msgBuf[FPORT_CONTROL_LEN + 2U];

    /**
     * @brief   pre-calculated alive frame
     * @notes   alive frame is calculated once at startup to save cpu cycles
     */
    uint8_t aliveBuf[FPORT_UPLINK_LEN + 2U];

    /**
     * @brief   remain length of the last received message
     * @notes   state machine will uplink telemetry message if lastLen indicates
     *          a telemetry request
     */
    uint8_t lastLen;

    /**
     * @brief   internal state of the FSM
     */
    FportFsmState state;

    /**
     * @brief   last valid received control frame
     */
    Fport_ControlFrame	lastFrame;

    /**
     * @brief   array of telemetry uplink datas
     * @notes   maintains lengths, and index of next data to be transmitted
     */
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
  
  


 /**
   * @brief   initialize Fport FSM
   *
   * @param[out] context   pointer to a @p FportFsmContext structure
   * @param[in]  cfg	   pointer to a @p FportFsmContextConfig structure
   */
  void		 fportContextInit(FportFsmContext *context, const FportFsmContextConfig *cfg);

 /**
  * @brief   feed the FSM with data coming from the FPORT radio module
  *
  * @param[in, out] context   pointer to a @p FportFsmContext structure
  * @param[in]      buffer    pointer to an array of bytes
  * @param[in]      len       above buffer length
  * @return         error bitfield
  * @notes	    for performance and cpu cycle saving, it's recommanded to supply
                    the longest possible message. The application must ask the FSM
                    what is the longest possible non blocking read by calling
		    fportGetNextReadLen function.
		    Since the protocol uses escaped value for beacon protection, message length
                    cannot be known in advance, most message can be read in one shot, but some must be read in
                    two shots
 */
  FportErrorMask fportFeedFsm(FportFsmContext *context, const void *buffer, size_t len);

  /**
   * @brief   cpu cycle optimisation helper fonction
   *
   * @param[in] context   pointer to a @p FportFsmContext structure
   * @return    maximum length of the next uart read which is garanteed non blocking 
   */
  size_t	 fportGetNextReadLen(const FportFsmContext *context);
  
#if FPORT_MAX_DATAIDS > 0
 /**
   * @brief   update an appId telemetry value for *NON* multiplexed appId
   *
   * @param[in, out]  fportp     pointer to a @p FportFsmContext structure
   * @param[in]       appId      see above description attached to fportAppId enum
   * @param[in]       v          new assocoated value for appId
   */
  void fportTelemetryUpData(FportFsmContext *context, const fportAppId appId, 
			    const FportTelemetryValue v);
  
 /**
   * @brief   update an appId telemetry value for *MULTIPLEXED* appId
   *
   * @param[in, out]  fportp     pointer to a @p FportFsmContext structure
   * @param[in]       appId      see above description attached to fportAppId enum
   * @param[in]       v          array of values for appId
   * @param[in]       nbMplx     size of the above array
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
   *             11 bits values for channels [0 .. 15]
   *             1 bit values for channels [16 .. 17]
   *             special value 0xffff if supplied channelIdx if out of range
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
   * @return     boolean :   if true -> failsafe mode active
   */
  static inline bool fportIsFailSafeFlag(const FportFsmContext *context) {
    return (context->lastFrame.flags & FPORT_FLAG_FAILSAFE) ? true : false;
  }

 /**
   * @brief   return radio link lost status
   *
   * @param[in]  context     pointer to a @p FportFsmContext structure
   * @return     boolean :   if true -> radio link is lost
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
   * @return     latitude as an uint32_t encoded value
   */
  uint32_t fportTranscodeLat(double lat);

  
  /**
   * @brief   helper fonction to transcode longitude 
   * from double to internal fport format
   * @param[in]  longitude as a double
   * @return     longitude as an uint32_t encoded value
   */
  uint32_t fportTranscodeLong(double lon);


  
#ifdef __cplusplus
}
#endif
