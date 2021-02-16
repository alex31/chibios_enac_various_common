#pragma once

#include <ch.h>
#include <hal.h>

#ifdef __cplusplus
extern "C" {
#endif

  /**
   * @brief  number of 11 bits payload value in a SBUS frame
   */
#define SBUS_NUM_CHANNEL 16U

  /**
   * @brief  kind of errors that can be returned by the driver
   */
  typedef enum  {SBUS_LOST_FRAME, SBUS_FAILSAFE, SBUS_MALFORMED_FRAME,
		 SBUS_TIMOUT} SBUSError;


  /**
   * @brief  structure representing a complete SBUS frame
   */
  typedef struct {
    /**
     * @brief   array of  11 bits signed number from
     */
    int16_t channel[SBUS_NUM_CHANNEL];
    /**
     * @brief   sbus special flags
     */
    uint8_t flags;
  } SBUSFrame;
  
  /**
   * @brief  type of error function callback
   */
  typedef void (SBUSErrorCb) (SBUSError err) ;

  /**
   * @brief  type of regular frame function callback
   */
  typedef void (SBUSFrameCb) (SBUSFrame *frame) ;

  /**
   * @brief   Futaba Sbus Driver configuration structure.
   */
  typedef struct {
    /**
     * @brief   SerialDriver for the U(S)ART used by SBUS Driver
     * @note	Only U(S)ART RX is need : it's unidirectional communication
     */
    SerialDriver *sd;
    /**
     * @brief   pointer to a callback function called each time an error is detected
     */
    SBUSErrorCb  *errorCb;
    /**
     * @brief   pointer to a callback function called for each new frame
     * @note	called @ 70Hz or 140Hz depending on receiver
     */
    SBUSFrameCb  *frameCb;
    /**
     * @brief   size of the stack for the thread that will decode message
     * @note	callback will be called in this thread context, size the stack
     *		accordingly to the stack usage of the callback you provide
     */
    size_t	 threadWASize;
    /**
     * @brief   sbus use an inverted signal compared to standard usart (idle is low) 
     *          inform the driver is the elecrical signal has already been inverted
     *		or if the driver should take care of the inversion
     * @note    Only USARTv2 (found on newer MCU) can invert signal. Old USARTv1 peripheral
     *		(F1, F2, F4) cannot invert the signal and an external inversion device should 
     *		be provided.
     */
    bool	 externallyInverted;
  } SBUSConfig;
  

  /**
   * @brief   Structure representing a SBUSDriver driver.
   */

  typedef struct {
    /**
     * @brief   Current configuration data.
     */
    const SBUSConfig *config;
    /**
     * @brief   pointer to a thread which does the reception
     * @note    callback function are called from this thread
     */
    thread_t *wth;
  } SBUSDriver;

  /**
   * @brief   Initializes a SBUS driver
   *
   * @param[out]  sbusp    pointer to a @p SBUSDriver structure
   * @init
   */
  void sbusObjectInit(SBUSDriver *sbusp);

  /**
   * @brief   start a SBUS driver
   *
   * @param[out]  sbusp     pointer to a @p SBUSDriver structure
   * @param[in]   configp    pointer to a @p SBUSConfig structure
   * @brief configure the USART to receive sbus frames
   */
  void sbusStart(SBUSDriver *sbusp, const SBUSConfig *configp);

  /**
   * @brief   stop a SBUS driver
   *
   * @param[in]  sbusp     pointer to a @p SBUSDriver structure
   * @brief stop and release the timer. After stop, any operation on sbusp
   *        will result in undefined behavior and probably hardware fault
   */
  void sbusStop(SBUSDriver *sbusp);

  
  /**
   * @brief   launch the worker thread that wait for sbus frame
   *
   * @param[in]  sbusp     pointer to a @p SBUSDriver structure
   */
  void sbusStartReceive(SBUSDriver *sbusp);



  /**
   * @brief   stop and join the worker thread
   *
   * @param[in]  sbusp     pointer to a @p SBUSDriver structure
   */
  void sbusStopReceive(SBUSDriver *sbusp);


  
  /**
   * @brief   encode and send a sbus frame
   *
   * @param[in]  sbusp     pointer to a @p SBUSDriver structure
   * @note       the TX pin of the usart must be configured in board.cfg
   */
  void sbusSend(SBUSDriver *sbusp, const SBUSFrame *frame);

#ifdef __cplusplus
}
#endif
