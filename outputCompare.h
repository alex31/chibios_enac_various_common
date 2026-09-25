#pragma once

#include "ch.h"
#include "hal.h"

#ifdef __cplusplus
extern "C" {
#endif

  /**
   * @brief   Timer Output Compare modes
   */
  enum TimOCMode {
    TIMOC_FROZEN = 0,         /* 000: Frozen */
    TIMOC_ACTIVE = 1,         /* 001: Set channel to active level on match */
    TIMOC_INACTIVE = 2,       /* 010: Set channel to inactive level on match */
    TIMOC_TOGGLE = 3,         /* 011: Toggle on match (CCRx is forced to 0 at init) */
    TIMOC_FORCE_INACTIVE = 4, /* 100: Force inactive level */
    TIMOC_FORCE_ACTIVE = 5,   /* 101: Force active level */
    TIMOC_PWM1 = 6,           /* 110: PWM mode 1 */
    TIMOC_PWM2 = 7            /* 111: PWM mode 2 */
  };

  /**
   * @brief   Channels configuration
   */
  enum TimOCActive {
    CH1_OUT_ACTIVE = 1<<0,
    CH2_OUT_ACTIVE = 1<<1,
    CH3_OUT_ACTIVE = 1<<2,
    CH4_OUT_ACTIVE = 1<<3
  };

  enum TimOCState {TIMOC_STATE_STOP, TIMOC_STATE_READY, TIMOC_STATE_ACTIVE};

  typedef struct TimOCDriver TimOCDriver;
  
  /**
   * @brief   Compare match callback.
   */
  typedef void (*TimOCCallbackCompare_t)(const TimOCDriver *timocp, uint32_t channel);
  
  /**
   * @brief   Timer overflow callback.
   */
  typedef void (*TimOCCallbackOverflow_t)(const TimOCDriver *timocp);

  /**
   * @brief   TimOC Driver configuration structure.
   */
  typedef struct {
    /**
     * @brief   hardware timer pointer (example : &STM32_TIM1)
     */
    stm32_tim_t *timer;
    TimOCCallbackCompare_t compare_cb;
    TimOCCallbackOverflow_t overflow_cb;
    enum TimOCMode  mode;
    uint32_t active; /* channels active, e.g., CH1_OUT_ACTIVE | CH2_OUT_ACTIVE */
    uint32_t dier;
    uint32_t dcr;
    uint32_t prescaler:17;
    uint32_t arr;
  } TimOCConfig;

  /**
   * @brief   Structure representing a TimOC driver.
   */
  struct TimOCDriver {
    /**
     * @brief   Current configuration data.
     */
    const TimOCConfig *config;
    /**
     * @brief   mutex to protect data read/write in concurrent context
     */
    uint32_t channel;
    uint32_t dier;
    mutex_t mut;
    enum TimOCState state;
  };

  /**
   * @brief   Initializes an output compare driver
   *
   * @param[out] timocp     pointer to a @p TimOCDriver structure
   * @init
   */
  void timOcObjectInit(TimOCDriver *timocp);

  /**
   * @brief   start an output compare driver
   *
   * @param[in]  timocp     pointer to a @p TimOCDriver structure
   * @param[in]  configp    pointer to a @p TimOCConfig structure
   * @brief configure the timer to output compare data
   */
  void timOcStart(TimOCDriver *timocp, const TimOCConfig *configp);

  /**
   * @brief   start the compare
   *
   * @param[in]  timocp     pointer to a @p TimOCDriver structure
   * @brief start the output compare
   */
  void timOcStartCompare(TimOCDriver *timocp);

  /**
   * @brief   start the compare with DMA requests enabled
   *
   * @param[in]  timocp     pointer to a @p TimOCDriver structure
   * @brief start the output compare and enable CCx DMA requests
   */
  void timOcStartCompareDma(TimOCDriver *timocp);

  /**
   * @brief   stop the compare
   *
   * @param[in]  timocp     pointer to a @p TimOCDriver structure
   * @brief stop the output compare
   */
  void timOcStopCompare(TimOCDriver *timocp);

  /**
   * @brief   stop the driver
   *
   * @param[in]  timocp     pointer to a @p TimOCDriver structure
   * @brief stop and release the timer. After stop, any operation on timocp
   *        will result in undefined behavior and probably hardware fault
   */
  void timOcStop(TimOCDriver *timocp);

  void timOcRccEnable(const TimOCDriver * const timocp);
  void timOcRccDisable(const TimOCDriver * const timocp);

  /**
   * @brief   Get the address of the ARR register for DMA transfers
   *
   * @param[in]  timocp     pointer to a @p TimOCDriver structure
   * @return                Pointer to the ARR register
   * @brief Utlity to link DMA easily to timer ARR in Toggle mode
   */
  volatile uint32_t* timOcGetArrAddress(const TimOCDriver *timocp);

#ifdef __cplusplus
}
#endif
