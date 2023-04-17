#pragma once

#include <ch.h>
#include <hal.h>
#include "frskyFportAppId.h"

/*
  TODO

  * voir si la radio peut emettre une alarme sonore sur seuil de la batterie de l'UAV
  * fichier de config pour rendre le code portable en utilisant des pointeurs sur fonction ?
  * tester en logique inverse sur MCU qui supporte la logique inverse

  LOW PRIORITY ENHANCEMENTS
  * API pour abonnement à des messages de telemetrie down with payload (pas encore
  utilisé par betafligh ou ardupilot, à creuser)
  * utilisation du DMA pour être moins gourmand en temps de cycle
    sur F7/H7/G4 ou autres STM récents avec UART qui fait du symbol detect pour arreter 
    la transaction  (voir si SIO dait faire ?) 
    + est-ce qu'il serait possible de faire du hardware assisted byte stuffing ?
*/

#ifdef __cplusplus
extern "C" {
#endif

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


 /**
   * @brief   enable jam control on half dupplex line when emitting
   * @details if enabled, readback what is emitted to detect if the 
   *          rxtx line is busy or jammed when sending telemetry
   *          use more ram and cpu, useful in debug mode, 
   *          not in release mode
   */
#ifndef FPORT_TX_READBACK_DOCHECK
#define FPORT_TX_READBACK_DOCHECK false
#endif
  
  /**
   * @brief  control frame sent by controller
   */
  typedef struct {
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
  } FPORT_ControlFrame;
  
  /**
   * @brief  kind of errors that can be returned by the driver
   */
  typedef enum  {FPORT_OK=0, FPORT_MALFORMED_FRAME=1<<0,
    FPORT_CRC_ERROR=1<<1, FPORT_APPID_LEN_EXCEDEED=1<<2,
    FPORT_TIMEOUT=1<<3, FPORT_FAILSAFE=1<<4, FPORT_RADIO_LINK_LOST=1<<5,
    FPORT_READBACK_CTRL_FAIL=1<<6
  } FPORTErrorMask;
  
 /**
   * @brief  uplink fields can be time multiplexed, see the fields description below
   */
  typedef enum __attribute__ ((__packed__)) {
    MULTIPLEX_1DATA=0x1, MULTIPLEX_2DATAS, MULTIPLEX_3DATAS, MULTIPLEX_4DATAS
  } FPORT_MultiplexData;

  /**
   * @brief  the four flags fields (1 bit) that are in the control frame flags field
   * @notes  only 4 bits a are used in the lest significant nibble, 
   *         most significant nibble is always 0
   */
  typedef enum __attribute__ ((__packed__)) {
    FPORT_FLAG_CH17 = 0x01, FPORT_FLAG_CH18 = 0x02,
    FPORT_FLAG_FRAME_LOST = 0x04, FPORT_FLAG_FAILSAFE = 0x08, 
  } FPORT_Flags;
  
  
  /**
   * @brief  fields value, 32 bit fixed point value
   * @notes  floating point value can be used by custom messages
   */
  typedef union {
    uint32_t u32;
    int32_t  s32;
    float    f32;
  } fportTelemetryValue;


  /**
   * @brief    forward declaration of FPORTDriver type used in callback functions
   */
   typedef struct FPORTDriver_s FPORTDriver;

  /**
   * @brief    function pointer use for control message callback
   */
  typedef void (*FPORTControlMsgCb) (const FPORTDriver*);

  /**
   * @brief    function pointer use for error condition callback
   */
  typedef void (*FPORTErrorCb) (FPORTDriver*);

  
  /**
   * @brief    Structure representing a Fport configuration.
   */
  typedef struct {
    /**
     * @brief   SerialDriver for the U(S)ART used by FPORT Driver
     * @note	U(S)ART should be in half dupplex mode, only one
     *          PIN (TX) is used
     */
    SerialDriver *sd;
    
    /**
     * @brief   Callback called synchronously when control message is correctly decoded
     * @note	if null, no callback will be called
     *          called from thread context
     */
    FPORTControlMsgCb ctrlMsgCb;

    /**
     * @brief   Callback called synchronously when error is encounted
     * @note	if null, no callback will be called
     *          error should be cleared calling fportGetAnClearErrors function
     *          else callback will be invoked continuously
     *          called from thread context
     */
    FPORTErrorCb      errCb;     

    /**
     * @brief   fport *could* use an inverted signal compared to standard usart (idle is low) 
     *          inform the driver is the elecrical signal has already been inverted
     *		or if the driver should take care of the inversion
     * @note    Only USARTv2 (found on newer MCU : G4, L4, F7, H7 ...) can invert signal. 
     *          Old USARTv1 peripheral (F1, F2, F4 ...) cannot invert the signal and a 
     *          receiver that provide uninverted signal (R-XSR, R9M, etc) must be used
     */
    int16_t	 middlePoint;
    bool	 driverShouldInvert;
  } FPORTConfig;
  

 /**
   * @brief   Structure representing a pair of appId and appValue
   *          for telemetry messages.
   */
   typedef struct {
    fportTelemetryValue value; // not natural order to avoid padding
    uint16_t key;              // between 16b and 32b fields 
  } fportAppIdPair;

 /**
   * @brief   Structure representing a FPORTDriver driver.
   */
   struct FPORTDriver_s {
    /**
     * @brief   Current configuration data.
     */
    const FPORTConfig *config;
    /**
     * @brief   pointer to a thread which does the reception
     * @note    callback function are called from this thread
     */
    thread_t *wth;
    FPORT_ControlFrame lastFrame;
    FPORTErrorMask	errorMsk;
    struct {
      fportAppIdPair appIds[FPORT_MAX_DATAIDS];
      uint8_t appIdsSize;
      uint8_t nextIdToSent;
    };
  };

  /**
   * @brief   Initializes a FPORT driver
   *
   * @param[out]  fportp    pointer to a @p FPORTDriver structure
   * @init
   */
  void fportObjectInit(FPORTDriver *fportp);

  /**
   * @brief   start a FPORT driver
   *
   * @param[out]  fportp     pointer to a @p FPORTDriver structure
   * @param[in]   configp    pointer to a @p FPORTConfig structure
   * @brief configure the USART to receive fport frames
   */
  void fportStart(FPORTDriver *fportp, const FPORTConfig *configp);

  /**
   * @brief   stop a FPORT driver
   *
   * @param[in]  fportp     pointer to a @p FPORTDriver structure
   * @brief stop and release the timer. After stop, any operation on fportp
   *        will result in undefined behavior and probably hardware fault
   */
  void fportStop(FPORTDriver *fportp);

  
  /**
   * @brief   launch the worker thread that wait for fport frame
   *          and send back telemetry frame if any
   * @param[in]  fportp     pointer to a @p FPORTDriver structure
   */
  void fportStartReceive(FPORTDriver *fportp);



  /**
   * @brief   stop and join the worker thread
   *
   * @param[in]  fportp     pointer to a @p FPORTDriver structure
   */
  void fportStopReceive(FPORTDriver *fportp);

  /**
   * @brief   get and clear error status
   *
   * @param[in]  fportp     pointer to a @p FPORTDriver structure
   */
  static inline FPORTErrorMask fportGetAnClearErrors(FPORTDriver *fportp)
  {
    const FPORTErrorMask errMsk = fportp->errorMsk;
    fportp->errorMsk = FPORT_OK;
    return errMsk;
  }

#if FPORT_MAX_DATAIDS > 0
 /**
   * @brief   update an appId telemetry value for *NON* multiplexed appId
   *
   * @param[in]  fportp     pointer to a @p FPORTDriver structure
   * @param[in]  appId      see above description attached to fportAppId enum
   * @param[in]  v          new assocoated value for appId
   */
  void fportTelemetryUpData(FPORTDriver *fportp, const fportAppId appId, 
			    const fportTelemetryValue v);
  
 /**
   * @brief   update an appId telemetry value for multiplexed appId
   *
   * @param[in]  fportp     pointer to a @p FPORTDriver structure
   * @param[in]  appId      see above description attached to fportAppId enum
   * @param[in]  v          array of values for appId
   * @param[in]  nb         size of the above array
   */
  void fportTelemetryUpDataMultiplexed(FPORTDriver *fportp, const fportAppId appId, 
				       const fportTelemetryValue *v,
				       const FPORT_MultiplexData nb);
#endif
  /**
   * @brief   
   *
   * @param[in]  fportp     pointer to a @p FPORTDriver structure
   * @param[in]  channelIdx in range [0 .. 17]
   * @return     control channel sent by the controller 
   *             in the range [-1024 .. +1023] for channels [0 .. 15]
   *             in the range [0 .. 1] for channels [16 .. 17]
   * @notes      beware that we map channels [1 .. 18] to [0 .. 17]
   */
  static inline int16_t fportGetChannel(const FPORTDriver *fportp, const size_t channelIdx) {
    if (channelIdx < FPORT_NUM_CONTROL_CHANNELS) {
      return fportp->lastFrame.channel[channelIdx] - fportp->config->middlePoint;
    } else {
      switch(channelIdx) {
	// channels 17 and 18 are boolean channel : return true or false
      case 16 : return (fportp->lastFrame.flags & FPORT_FLAG_CH17) ? true : false;
      case 17 : return (fportp->lastFrame.flags & FPORT_FLAG_CH18) ? true : false;
      default:
	return 0xffff;
      }
    }
  }
    
 /**
   * @brief   return failsafe status
   *
   * @param[in]  fportp     pointer to a @p FPORTDriver structure
   * @return     boolean :  true : failsafe mode active
   */
  static inline bool fportIsFailSafeFlag(FPORTDriver *fportp) {
    return (fportp->lastFrame.flags & FPORT_FLAG_FAILSAFE) ? true : false;
  }

 /**
   * @brief   return radio link lost status
   *
   * @param[in]  fportp     pointer to a @p FPORTDriver structure
   * @return     boolean :  true : radio link is lost
   */
  static inline bool fportIsFrameLostFlag(FPORTDriver *fportp) {
    return (fportp->lastFrame.flags & FPORT_FLAG_FRAME_LOST) ? true : false;
  }
    
 /**
   * @brief   return rssi
   *
   * @param[in]  fportp     pointer to a @p FPORTDriver structure
   * @return     return radio signal strength indicator
   */
  static inline int16_t fportGetRssi(const FPORTDriver *fportp) {
    return fportp->lastFrame.rssi;
  }
    

  

  /**
   * @brief   helper fonction to transcode latitude 
   * from double to internal fport format
   * @param[in]  latitude as a double
   * @param[out]  latitude as an uint32_t encoded value
   */
  uint32_t transcodeLat(double lat);

  
  /**
   * @brief   helper fonction to transcode longitude 
   * from double to internal fport format
   * @param[in]  longitude as a double
   * @param[out]  longitude as an uint32_t encoded value
   */
  uint32_t transcodeLong(double lon);


  
#ifdef __cplusplus
}
#endif
