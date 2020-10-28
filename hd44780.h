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

#if !defined(HD44780_ENABLE_PULSE_US) || defined(__DOXYGEN__)
#define HD44780_ENABLE_PULSE_US               (100U)
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
