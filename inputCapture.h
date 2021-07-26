#pragma once

#include "ch.h"
#include "hal.h"

#ifdef __cplusplus
extern "C" {
#endif

  enum  TimICMode {TIMIC_INPUT_CAPTURE, TIMIC_PWM_IN} ;
  enum  TimICChannel {TIMIC_CH1=1, TIMIC_CH2=2, TIMIC_CH3=4, TIMIC_CH4=8} ;
  enum  TimICActive {CH1_RISING_EDGE=1<<0, CH1_FALLING_EDGE=1<<1,  CH1_BOTH_EDGES=1<<2,
		 CH2_RISING_EDGE=1<<3, CH2_FALLING_EDGE=1<<4,  CH2_BOTH_EDGES=1<<5,
		 CH3_RISING_EDGE=1<<6, CH3_FALLING_EDGE=1<<7,  CH3_BOTH_EDGES=1<<8,
		 CH4_RISING_EDGE=1<<9, CH4_FALLING_EDGE=1<<10, CH4_BOTH_EDGES=1<<11
  } ;


  /**
   * @brief   TimIC Driver configuration structure.
   */
  typedef struct {
    /**
     * @brief   hardware timer pointer (example : &STM32_TIM1)
     */
    stm32_tim_t * const timer;
    enum TimICMode	mode;
    uint32_t channel;
    uint32_t active;
    uint32_t dier;
    uint32_t prescaler:17;
  } TimICConfig;


  /**
   * @brief   Structure representing a TimIC driver.
   */
  typedef struct {
    /**
     * @brief   Current configuration data.
     */
    const TimICConfig *config;
    /**
     * @brief   mutex to protect data read/write in concurrent context
     */
    mutex_t mut;
  } TimICDriver;


  /**
   * @brief   Initializes a quadrature encoder driver
   *
   * @param[out]  inputCapturep     pointer to a @p TimICDriver structure
   * @init
   */
  void timIcObjectInit(TimICDriver *pwmInp);

  /**
   * @brief   start an input capture driver
   *
   * @param[in]  pwmInp     pointer to a @p TimICDriver structure
   * @param[in]   configp    pointer to a @p TimICConfig structure
   * @brief configure the timer to get input capture data from timer
   */
  void timIcStart(TimICDriver *pwmInp, const TimICConfig *configp);

  /**
   * @brief   start to capture
   *
   * @param[in]  pwmInp     pointer to a @p TimICDriver structure
   * @brief start the input capture data from timer
   */
  void timIcStartCapture(TimICDriver *pwmInp);

 /**
   * @brief   stop to capture
   *
   * @param[in]  pwmInp     pointer to a @p TimICDriver structure
   * @brief stop the input capture data from timer
   */
  void timIcStopCapture(TimICDriver *pwmInp);

  /**
   * @brief   stop a quadrature encoder driver
   *
   * @param[in]  pwmInp     pointer to a @p TimICDriver structure
   * @brief stop and release the timer. After stop, any operation on pwmInp
   *        will result in undefined behavior and probably hardware fault
   */
  void timIcStop(TimICDriver *pwmInp);




#ifdef __cplusplus
}
#endif
