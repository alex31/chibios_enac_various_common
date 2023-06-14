#pragma once

#include <ch.h>
#include <hal.h>
#include "frskyFportAppId.h"
#include "frskyFport_fsm.h"
/*
  TODO


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
    UARTDriver *uart;
    
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

    int16_t	 middlePoint;

    /**
     * @brief   fport *could* use an inverted signal compared to standard usart (idle is low) 
     *          inform the driver is the elecrical signal has already been inverted
     *		or if the driver should take care of the inversion
     * @note    Only USARTv2 (found on newer MCU : G4, L4, F7, H7 ...) can invert signal. 
     *          Old USARTv1 peripheral (F1, F2, F4 ...) cannot invert the signal and a 
     *          receiver that provide uninverted signal (R-XSR, R9M, etc) must be used
     */
    bool	 driverShouldInvert;
  } FPORTConfig;
  


 /**
   * @brief   Structure representing a FPORTDriver driver.
   */
  struct FPORTDriver_s {
    /**
     * @brief   Current configuration data.
     */
    const FPORTConfig	*config;
    FportFsmContext	context;
    /**
     * @brief   pointer to a thread which does the reception
     * @note    callback function are called from this thread
     */
    thread_t		*wth;
    
    FportErrorMask	errorMsk;
  };

  /**
   * @brief   Initializes a FPORT driver
   *
   * @param[out]  fportp    pointer to a @p FPORTDriver structure
   * @init
   */
  void fportUartObjectInit(FPORTDriver *fportp);

  /**
   * @brief   start a FPORT driver
   *
   * @param[out]  fportp     pointer to a @p FPORTDriver structure
   * @param[in]   configp    pointer to a @p FPORTConfig structure
   * @brief configure the USART to receive fport frames
   */
  void fportUartStart(FPORTDriver *fportp, const FPORTConfig *configp);

  /**
   * @brief   stop a FPORT driver
   *
   * @param[in]  fportp     pointer to a @p FPORTDriver structure
   * @brief stop and release the timer. After stop, any operation on fportp
   *        will result in undefined behavior and probably hardware fault
   */
  void fportUartStop(FPORTDriver *fportp);

  
  /**
   * @brief   launch the worker thread that wait for fport frame
   *          and send back telemetry frame if any
   * @param[in]  fportp     pointer to a @p FPORTDriver structure
   */
  void fportUartStartReceive(FPORTDriver *fportp);



  /**
   * @brief   stop and join the worker thread
   *
   * @param[in]  fportp     pointer to a @p FPORTDriver structure
   */
  void fportUartStopReceive(FPORTDriver *fportp);

  /**
   * @brief   get and clear error status
   *
   * @param[in]  fportp     pointer to a @p FPORTDriver structure
   */
  static inline FportErrorMask fportUartGetAnClearErrors(FPORTDriver *fportp)
  {
    const FportErrorMask errMsk = fportp->errorMsk;
    fportp->errorMsk = FPORT_OK;
    return errMsk;
  }

   static inline int16_t fportUartGetChannel(const FPORTDriver *fportp, const size_t channelIdx) {
     return fportGetChannel(&fportp->context, channelIdx);
   }

#if FPORT_MAX_DATAIDS > 0
 /**
   * @brief   update an appId telemetry value for *NON* multiplexed appId
   *
   * @param[in]  fportp     pointer to a @p FPORTDriver structure
   * @param[in]  appId      see above description attached to fportAppId enum
   * @param[in]  v          new assocoated value for appId
   */
  void fportUartTelemetryUpData(FPORTDriver *fportp, const fportAppId appId, 
			    const FportTelemetryValue v);
  
 /**
   * @brief   update an appId telemetry value for multiplexed appId
   *
   * @param[in]  fportp     pointer to a @p FPORTDriver structure
   * @param[in]  appId      see above description attached to fportAppId enum
   * @param[in]  v          array of values for appId
   * @param[in]  nb         size of the above array
   */
  void fportUartTelemetryUpDataMultiplexed(FPORTDriver *fportp, const fportAppId appId, 
				       const FportTelemetryValue *v,
				       const FportMultiplexData nbMplx);
#endif

  
#ifdef __cplusplus
}
#endif
