/*
    PLAY Embedded - Copyright (C) 2006..2015 Rocco Marco Guglielmi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/*
    Special thanks to Giovanni Di Sirio for teachings, his moral support and
    friendship. Note that some or every piece of this file could be part of
    the ChibiOS project that is intellectual property of Giovanni Di Sirio.
    Please refer to ChibiOS/RT license before use this file.
	
	For suggestion or Bug report - guglielmir@playembedded.org
 */

/**
 * @file    hd44780.h
 * @brief   HD44780 Complex Driver header.
 *
 * @addtogroup HD44780
 * @{
 */

#pragma once

#include "hal.h"

#if USERLIB_USE_HD44780 || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/


/**
 * @brief   Generic definition of right direction
 */
#define   HD44780_RIGHT                     0x04

/**
 * @brief   Generic definition of left direction
 */
#define   HD44780_LEFT                      0x00

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @brief   Enables back-light APIs.
 *
 * @note    Enabling this option HD44780 requires a PWM driver.
 * @note    The default is @p TRUE.
 */
#if !defined(HD44780_USE_DIMMABLE_BACKLIGHT) || defined(__DOXYGEN__)
#define HD44780_USE_DIMMABLE_BACKLIGHT               TRUE
#endif

/**
 * @brief   Delay applied after long LCD instructions before polling busy.
 * @note    Intended for CLEAR_DISPLAY and RETURN_HOME only.
 */
#if !defined(HD44780_LONG_INSTR_DELAY_US) || defined(__DOXYGEN__)
#define HD44780_LONG_INSTR_DELAY_US                  1600U
#endif



/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if HD44780_USE_4_BIT_MODE
#define LINE_DATA_LEN                   4
#define HD44780_DATA_LENGHT                 0x00
#else
#define LINE_DATA_LEN                   8
#define HD44780_DATA_LENGHT                 0x10
#endif

#if HD44780_USE_DIMMABLE_BACKLIGHT && !HAL_USE_PWM
#error "HD44780_USE_DIMMABLE_BACKLIGHT requires HAL_USE_PWM"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @name    hd44780 data structures and types
 * @{ */

/**
 * @brief   hd44780 PIN-map.
 * @note    these PINs are required by write/read operation.
 */
typedef struct {
  /**
   * @brief  Register selector PIN
   */
  ioline_t RS;
  /**
   * @brief  Read/Write PIN
   */
  ioline_t RW;
  /**
   * @brief  Enable PIN
   */
  ioline_t E;
  /**
   * @brief  Back-light anode pin
   */
  ioline_t A;
  /**
   * @brief  Data PINs
   */
  ioline_t D[LINE_DATA_LEN];
} hd44780_pins_t;

/**
 * @brief  Optional GPT-driven transaction engine configuration.
 *
 * @details
 * When @p gptd is provided, the public HD44780 API remains synchronous for
 * callers but the low level bus transaction is no longer executed as a long
 * polling loop in thread context. Instead, the caller prepares a transaction,
 * suspends once, then a one-shot GPT callback advances a finite state machine
 * until the LCD transfer is complete and the caller thread is resumed.
 *
 * This keeps the high level API simple while avoiding millisecond-scale busy
 * waits for slow instructions such as @p CLEAR_DISPLAY and
 * @p RETURN_HOME.
 *
 * @note   The GPT must be dedicated to the LCD driver instance.
 */
typedef struct {
  /** @brief Dedicated GPT instance used to clock the internal FSM. */
  GPTDriver *gptd;
  /** @brief GPT input clock used to derive one-shot delays. */
  gptfreq_t frequency;
  /** @brief Delay, in microseconds, used between short FSM phases. */
  uint32_t step_delay_us;
} hd44780_gptcfg_t;

/**
 * @brief  HD44780 cursor control
 */
typedef enum {
  HD44780_CURSOR_OFF = 0x00,
  HD44780_CURSOR_ON = 0x02,
} hd44780_cursor_t;

/**
 * @brief  HD44780 blinking control
 */
typedef enum {
  HD44780_BLINKING_OFF = 0x00,
  HD44780_BLINKING_ON = 0x01,
} hd44780_blinking_t;

/**
 * @brief  HD44780 display settings
 */
typedef enum {
  HD44780_SET_FONT_5X8 = 0x00,
  HD44780_SET_FONT_5X10 = 0x04
} hd44780_set_font_t;

/**
 * @brief  HD44780 display settings
 */
typedef enum {
  HD44780_SET_1LINE = 0x00,
  HD44780_SET_2LINES = 0x08
} hd44780_set_lines_t;
/** @}  */

/**
 * @brief   Driver state machine possible states.
 */
typedef enum {
  HD44780_UNINIT = 0,
  HD44780_STOP = 1,
  HD44780_ACTIVE = 2,
} hd44780_state_t;

/**
 * @brief   HD44780 configuration structure.
 */
typedef struct {
  /**
   * @brief  HD44780 cursor control
   */
  hd44780_cursor_t cursor;
  /**
   * @brief  HD44780 blinking control
   */
  hd44780_blinking_t blinking;
  /**
   * @brief  HD44780 font settings
   */
  hd44780_set_font_t font;
  /**
   * @brief  HD44780 lines settings
   */
  hd44780_set_lines_t lines;
  /**
   * @brief  HD44780 PIN-map
   */
  hd44780_pins_t const *pinmap;
  /**
   * @brief  Optional timer used to drive the register-write FSM.
   * @note   If @p gptd is NULL then the legacy polled implementation is used.
   */
  hd44780_gptcfg_t gpt;
#if HD44780_USE_DIMMABLE_BACKLIGHT
  /**
   * @brief  PWM driver for back-light managing
   */
  PWMDriver *pwmp;
  /**
   * @brief  PWM driver configuration
   */
  PWMConfig const *pwmcfgp;
  /**
   * @brief  PWM channel id
   */
  uint32_t contrast_ch;
  uint32_t backlight_ch;
  uint16_t contrast;
#endif
  /**
   * @brief  Initial Back-light percentage (from 0 to 100)
   * @note   If !HD44780_USE_DIMMABLE_BACKLIGHT this is just true or false
   */
  uint16_t backlight;
} HD44780Config;

/**
 * @brief   Structure representing an HD44780 driver.
 *
 * @details
 * In legacy mode the driver behaves like a conventional polled HD44780
 * implementation and the fields below the public configuration pointer are
 * mostly unused. In GPT mode the same object also carries the transient state
 * of one in-flight LCD transaction.
 *
 * The intended reading model is:
 * - public calls stay synchronous
 * - only one transaction can be active at a time
 * - the caller thread sleeps once on @p waiter
 * - the GPT callback advances the bus FSM until completion
 * - the caller is resumed exactly once at the end
 */
typedef struct {
  /**
   * @brief Driver state.
   */
  hd44780_state_t         state;
  /**
   * @brief  Current Back-light percentage (from 0 to 100)
   *
   * @detail When HD44780_USE_DIMMABLE_BACKLIGHT is false, this is considered like boolean
   */
  uint16_t           backlight;
  uint16_t           contrast;
  /**
   * @brief Current configuration data.
   */
  const HD44780Config    *config;
  /**
   * @brief Suspended caller waiting for the current transaction completion.
   */
  thread_reference_t     waiter;
  /**
   * @brief One-shot delay used for short bus phases such as the E pulse.
   */
  gptcnt_t               short_delay_ticks;
  /**
   * @brief One-shot delay used before polling the busy flag after long commands.
   */
  gptcnt_t               long_delay_ticks;
  /**
   * @brief Current phase of the internal GPT-driven transaction FSM.
   */
  uint8_t                tx_state;
  /**
   * @brief Register selector latched for the current transaction.
   */
  uint8_t                tx_reg;
  /**
   * @brief Byte currently being transferred to the LCD controller.
   */
  uint8_t                tx_value;
  /**
   * @brief Last busy-flag sample read from DB7.
   */
  bool                   tx_busy_sample;
  /**
   * @brief Selects the meaning of a successful busy poll.
   *
   * @details
   * When @p false, a clear busy flag means "the controller can accept the next
   * write". When @p true, it means "the long instruction already written has
   * completed and the suspended caller may be resumed".
   */
  bool                   tx_poll_after_write;
  /**
   * @brief Number of distinct GPIO ports used by the LCD data bus.
   */
  uint8_t                data_port_count;
  /**
   * @brief Distinct GPIO ports participating in LCD data transfers.
   */
  ioportid_t             data_ports[LINE_DATA_LEN];
  /**
   * @brief Per-data-line GPIO bit mask.
   */
  ioportmask_t           data_bits[LINE_DATA_LEN];
  /**
   * @brief Per-data-line index into @p data_ports.
   */
  uint8_t                data_port_index[LINE_DATA_LEN];
  /**
   * @brief Runtime GPT configuration owned by the driver object.
   */
  GPTConfig              gptcfg;
} HD44780Driver;

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/



#ifdef __cplusplus
extern "C" {
#endif
  void hd44780ObjectInit(HD44780Driver *lcdp);
  void hd44780Start(HD44780Driver *lcdp, const HD44780Config *config);
  void hd44780Stop(HD44780Driver *lcdp);
  void hd44780BacklightOn(HD44780Driver *lcdp);
  void hd44780BacklightOff(HD44780Driver *lcdp);
  void hd44780ClearDisplay(HD44780Driver *lcdp);
  void hd44780ReturnHome(HD44780Driver *lcdp);
  void hd44780SetAddress(HD44780Driver *lcdp, uint8_t add);
  void hd44780Write(HD44780Driver *lcdp, uint8_t pos, const char* fmt, ...)
    __attribute__ ((format (printf, 3, 4)));
  void hd44780RawWrite(HD44780Driver *lcdp, uint8_t pos, const char* buf);
  void hd44780CustomGraphic(HD44780Driver *lcdp, uint8_t pos, const uint8_t bitmap[8]);
  void hd44780DoDisplayShift(HD44780Driver *lcdp, uint8_t dir);
  void hd44780ShowCursor(HD44780Driver *lcdp, bool display);
#if HD44780_USE_DIMMABLE_BACKLIGHT
  void hd44780SetBacklight(HD44780Driver *lcdp, uint32_t perc);
  void hd44780SetContrast(HD44780Driver *lcdp, uint32_t perc);
  void hd44780BacklightFadeOut(HD44780Driver *lcdp);
  void hd44780BacklightFadeIn(HD44780Driver *lcdp);
#endif /* HD44780_USE_DIMMABLE_BACKLIGHT */
#ifdef __cplusplus
}
#endif
#endif /* USERLIB_USE_HD44780 */


/** @} */
