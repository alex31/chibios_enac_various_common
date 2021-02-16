#pragma once

#include "ch.h"
#include "hal.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief   return value structure.
 */
typedef struct {
/**
 * @brief   current counter value
 */
  uint32_t cnt;
/**
 * @brief   true if counter has been updated since last read
 */
  bool     hasChanged;
} QEncoderCnt;

/**
 * @brief   QEncoder Driver configuration structure.
 */
typedef struct {
/**
 * @brief   hardware timer pointer (example : &STM32_TIM1)
 */
  stm32_tim_t * const timer;
} QEncoderConfig;


/**
 * @brief   Structure representing a QEncoder driver.
 */
typedef struct {
  /**
   * @brief   Current configuration data.
   */
  const QEncoderConfig *config;
  /**
   * @brief   last read value to indicate if there is change 
   */
  uint32_t lastCnt;
  /**
   * @brief   mutex to protect data read/write in concurrent context
   */
  mutex_t mut;
} QEncoderDriver;



/**
 * @brief   Initializes a quadrature encoder driver
 *
 * @param[out]  qencoderp     pointer to a @p QEncoderDriver structure
 * @init
 */
void qencoderObjectInit(QEncoderDriver *qencoderp);

/**
 * @brief   start a quadrature encoder driver
 *
 * @param[out]  qencoderp     pointer to a @p QEncoderDriver structure
 * @param[in]   configp    pointer to a @p QEncoderConfig structure
 * @brief configure and start the timer to decode a quadrature rotary
 *        encoder. After start, timer counter refect rotation on the rotary axle 
 */
void qencoderStart(QEncoderDriver *qencoderp, const QEncoderConfig *configp);


/**
 * @brief   stop a quadrature encoder driver
 *
 * @param[in]  qencoderp     pointer to a @p QEncoderDriver structure
 * @brief stop and release the timer. After stop, any operation on qencoderp
 *        will result in undefined behavior and probably hardware fault
 */
void qencoderStop(QEncoderDriver *qencoderp);



/**
 * @brief   get absolute value and indication if value has changed
            since last call, of the timer tied with the rotary encoder driver
 *
 * @param[in]  qencoderp     pointer to a @p QEncoderDriver structure
 * @ret QEncoderCnt structure
 */
QEncoderCnt qencoderGetCNT(QEncoderDriver * const qencoderp);




/**
 * @brief   set absolute value of the timer tied with the rotary encoder driver
 *
 * @param[in]  qencoderp     pointer to a @p QEncoderDriver structure
 * @param[in]  cnt	     new value for the counter
 */
void qencoderSetCNT(QEncoderDriver *qencoderp, const uint32_t cnt);


#ifdef __cplusplus
}
#endif
