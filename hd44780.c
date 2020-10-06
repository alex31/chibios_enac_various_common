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
 * @file    hd44780.c
 * @brief   HD44780 complex driver code.
 *
 * @addtogroup HD44780
 * @{
 */

#include "hd44780.h"
#include <stdio.h>
#include <stdarg.h>

#ifdef PAL_STM32_OSPEED_HIGH
#define PAL_STM32_OSPEED_HIGHEST PAL_STM32_OSPEED_HIGH
#endif

#if USERLIB_USE_HD44780 || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/


/* HD44780 REGISTERS */
#define HD44780_INSTRUCTION_R               PAL_LOW
#define HD44780_DATA_R                      PAL_HIGH

/* HD44780_INSTRUCTIONS */
#define HD44780_CLEAR_DISPLAY               0x01

#define HD44780_RETURN_HOME                 0x02

#define HD44780_EMS                         0x04
#define HD44780_EMS_S                       0x01
#define HD44780_EMS_ID                      0x02

#define HD44780_DC                          0x08
#define HD44780_DC_B                        0x01
#define HD44780_DC_C                        0x02
#define HD44780_DC_D                        0x04

#define HD44780_CDS                         0x10
#define HD44780_CDS_RL                      0x04
#define HD44780_CDS_SC                      0x08

#define HD44780_FS                          0x20
#define HD44780_FS_F                        0x04
#define HD44780_FS_N                        0x08
#define HD44780_FS_DL                       0x10

#define HD44780_NO_CURSOR                   0x0C
#define HD44780_BLINK_CURSOR                0x0F



#define HD44780_SET_CGRAM_ADDRESS           0x40
#define HD44780_SET_CGRAM_ADDRESS_MASK      0X3F

#define HD44780_SET_DDRAM_ADDRESS           0x80
#define HD44780_SET_DDRAM_ADDRESS_MASK      0X7F

#define HD44780_BUSY_FLAG                   0X80

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   HD44780D1 driver identifier.
 */
HD44780Driver HD44780D1;

/*===========================================================================*/
/* Driver local types.                                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Get the busy flag
 *
 * @param[in] hd44780p          HD44780 driver
 *
 * @return                  The HD44780 status.
 * @retval TRUE             if the HD44780 is busy on internal operation.
 * @retval FALSE            if the HD44780 is in idle.
 *
 * @notapi
 */
static bool hd44780IsBusy(HD44780Driver *lcdp) {
  bool busy;
  unsigned ii;

  /* Configuring Data PINs as Input. */
  for(ii = 0; ii < LINE_DATA_LEN; ii++)
    palSetLineMode(lcdp->config->pinmap->D[ii], PAL_MODE_INPUT);

  palSetLine(lcdp->config->pinmap->RW);
  palClearLine(lcdp->config->pinmap->RS);

  palSetLine(lcdp->config->pinmap->E);
  osalThreadSleepMicroseconds(100);
  busy = (palReadLine(lcdp->config->pinmap->D[LINE_DATA_LEN - 1]) == PAL_HIGH);
  palClearLine(lcdp->config->pinmap->E);
  osalThreadSleepMicroseconds(100);

#if HD44780_USE_4_BIT_MODE
  palSetLine(lcdp->config->pinmap->E);
  osalThreadSleepMicroseconds(100);
  palClearLine(lcdp->config->pinmap->E);
  osalThreadSleepMicroseconds(100);
#endif
  return busy;
}

/**
 * @brief   Write a data into a register for the lcd
 *
 * @param[in] lcdp          HD44780 driver
 * @param[in] reg           Register id
 * @param[in] value         Writing value
 *
 * @notapi
 */
static void hd44780WriteRegister(HD44780Driver *lcdp, uint8_t reg, uint8_t value){

  unsigned ii;

  while (hd44780IsBusy(lcdp))
    ;

  /* Configuring Data PINs as Output Push Pull. */
  for(ii = 0; ii < LINE_DATA_LEN; ii++)
    palSetLineMode(lcdp->config->pinmap->D[ii], PAL_MODE_OUTPUT_PUSHPULL |
                   PAL_STM32_OSPEED_HIGHEST);

  palClearLine(lcdp->config->pinmap->RW);
  palWriteLine(lcdp->config->pinmap->RS, reg);

#if HD44780_USE_4_BIT_MODE
  for(ii = 0; ii < LINE_DATA_LEN; ii++) {
    if(value & (1 << (ii + 4)))
      palSetLine(lcdp->config->pinmap->D[ii]);
    else
      palClearLine(lcdp->config->pinmap->D[ii]);
  }
  palSetLine(lcdp->config->pinmap->E);
  osalThreadSleepMicroseconds(100);
  palClearLine(lcdp->config->pinmap->E);
  osalThreadSleepMicroseconds(100);

  for(ii = 0; ii < LINE_DATA_LEN; ii++) {
    if(value & (1 << ii))
      palSetLine(lcdp->config->pinmap->D[ii]);
    else
      palClearLine(lcdp->config->pinmap->D[ii]);
  }
  palSetLine(lcdp->config->pinmap->E);
  osalThreadSleepMicroseconds(100);
  palClearLine(lcdp->config->pinmap->E);
  osalThreadSleepMicroseconds(100);
#else
  for(ii = 0; ii < LINE_DATA_LEN; ii++){
      if(value & (1 << ii))
        palSetLine(lcdp->config->pinmap->D[ii]);
      else
        palClearLine(lcdp->config->pinmap->D[ii]);
  }
  palSetLine(lcdp->config->pinmap->E);
  osalThreadSleepMicroseconds(100);
  palClearLine(lcdp->config->pinmap->E);
  osalThreadSleepMicroseconds(100);
#endif
}

/**
 * @brief   Perform a initialization by instruction as explained in HD44780
 *          datasheet.
 * @note    This reset is required after a mis-configuration or if there aren't
 *          condition to enable internal reset circuit.
 *
 * @param[in] lcdp          HD44780 driver
 *
 * @notapi
 */
static void hd44780InitByIstructions(HD44780Driver *lcdp) {
  unsigned ii;

  osalThreadSleepMilliseconds(50);
  for(ii = 0; ii < LINE_DATA_LEN; ii++) {
    palSetLineMode(lcdp->config->pinmap->D[ii], PAL_MODE_OUTPUT_PUSHPULL |
                   PAL_STM32_OSPEED_HIGHEST);
    palClearLine(lcdp->config->pinmap->D[ii]);
  }

  palClearLine(lcdp->config->pinmap->E);
  palClearLine(lcdp->config->pinmap->RW);
  palClearLine(lcdp->config->pinmap->RS);
  palSetLine(lcdp->config->pinmap->D[LINE_DATA_LEN - 3]);
  palSetLine(lcdp->config->pinmap->D[LINE_DATA_LEN - 4]);

  palSetLine(lcdp->config->pinmap->E);
  osalThreadSleepMicroseconds(100);
  palClearLine(lcdp->config->pinmap->E);
  osalThreadSleepMilliseconds(5);

  palSetLine(lcdp->config->pinmap->E);
  osalThreadSleepMicroseconds(100);
  palClearLine(lcdp->config->pinmap->E);
  osalThreadSleepMicroseconds(100);

  palSetLine(lcdp->config->pinmap->E);
  osalThreadSleepMicroseconds(100);
  palClearLine(lcdp->config->pinmap->E);

#if HD44780_USE_4_BIT_MODE
  palSetLine(lcdp->config->pinmap->D[LINE_DATA_LEN - 3]);
  palClearLine(lcdp->config->pinmap->D[LINE_DATA_LEN - 4]);
  palSetLine(lcdp->config->pinmap->E);
  osalThreadSleepMicroseconds(100);
  palClearLine(lcdp->config->pinmap->E);
#endif

  /* Configuring data interface */
  hd44780WriteRegister(lcdp, HD44780_INSTRUCTION_R, HD44780_FS | HD44780_DATA_LENGHT |
                       lcdp->config->font | lcdp->config->lines);

  /* Turning off display and clearing */
  hd44780WriteRegister(lcdp, HD44780_INSTRUCTION_R, HD44780_DC);
  hd44780WriteRegister(lcdp, HD44780_INSTRUCTION_R, HD44780_CLEAR_DISPLAY);

  /* Setting display control turning on display */
  hd44780WriteRegister(lcdp, HD44780_INSTRUCTION_R, HD44780_DC | HD44780_DC_D |
                       lcdp->config->cursor | lcdp->config->blinking);

  /* Setting Entry Mode */
  hd44780WriteRegister(lcdp, HD44780_INSTRUCTION_R, HD44780_EMS | HD44780_EMS_ID);
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/


/**
 * @brief   Initializes an instance.
 *
 * @param[out] lcdp         pointer to the @p HD44780Driver object
 *
 * @init
 */
void hd44780ObjectInit(HD44780Driver *lcdp){

  lcdp->state  = HD44780_STOP;
  lcdp->config = NULL;
  lcdp->backlight = 0;
  lcdp->contrast = 0;
}

/**
 * @brief   Configures and activates the HD44780 Complex Driver  peripheral.
 *
 * @param[in] lcd   pointer to the @p HD44780Driver object
 * @param[in] config    pointer to the @p HD44780Config object
 *
 * @api
 */
void hd44780Start(HD44780Driver *lcdp, const HD44780Config *config) {

  osalDbgCheck((lcdp != NULL) && (config != NULL));

  osalDbgAssert((lcdp->state == HD44780_STOP) || (lcdp->state == HD44780_ACTIVE),
              "hd44780Start(), invalid state");

  lcdp->config = config;
  lcdp->backlight = lcdp->config->backlight;
#if HD44780_USE_DIMMABLE_BACKLIGHT
  lcdp->contrast = lcdp->config->contrast;
#endif
  /* Initializing HD44780 by instructions. */
  hd44780InitByIstructions(lcdp);

#if HD44780_USE_DIMMABLE_BACKLIGHT
  pwmStart(lcdp->config->pwmp, lcdp->config->pwmcfgp);
  pwmEnableChannel(lcdp->config->pwmp, lcdp->config->backlight_ch,
                   PWM_PERCENTAGE_TO_WIDTH(lcdp->config->pwmp,
                                           lcdp->config->backlight * 100));
  pwmEnableChannel(lcdp->config->pwmp, lcdp->config->contrast_ch,
                   PWM_PERCENTAGE_TO_WIDTH(lcdp->config->pwmp,
                                           (100 - lcdp->config->contrast) * 100));
  
#else
  palWriteLine(lcdp->config->pinmap->A,
              lcdp->config->backlight ? PAL_HIGH : PAL_LOW);
#endif

  lcdp->state = HD44780_ACTIVE;
}

/**
 * @brief   Deactivates the HD44780 Complex Driver  peripheral.
 *
 * @param[in] lcdp      pointer to the @p HD44780Driver object
 *
 * @api
 */
void hd44780Stop(HD44780Driver *lcdp) {

  osalDbgCheck(lcdp != NULL);

  osalDbgAssert((lcdp->state == HD44780_STOP) || (lcdp->state == HD44780_ACTIVE),
              "hd44780Stop(), invalid state");
#if HD44780_USE_DIMMABLE_BACKLIGHT
  pwmStop(lcdp->config->pwmp);
#else
  palClearLine(lcdp->config->pinmap->A);
#endif
  lcdp->backlight = 0;
  lcdp->contrast = 0;
  hd44780WriteRegister(lcdp, HD44780_INSTRUCTION_R, HD44780_DC);
  hd44780WriteRegister(lcdp, HD44780_INSTRUCTION_R, HD44780_CLEAR_DISPLAY);
  lcdp->state = HD44780_STOP;
}

/**
 * @brief   Turn on back-light.
 *
 * @param[in] lcdp      pointer to the @p HD44780Driver object
 *
 * @api
 */
void hd44780BacklightOn(HD44780Driver *lcdp) {

  osalDbgCheck(lcdp != NULL);
  osalDbgAssert((lcdp->state == HD44780_ACTIVE), "hd44780BacklightOn(), invalid state");
#if HD44780_USE_DIMMABLE_BACKLIGHT
  pwmEnableChannel(lcdp->config->pwmp, lcdp->config->backlight_ch,
                   PWM_PERCENTAGE_TO_WIDTH(lcdp->config->pwmp, 10000));

#else
  palSetLine(lcdp->config->pinmap->A);
#endif
  lcdp->backlight = 100;
}

/**
 * @brief   Turn off back-light.
 *
 * @param[in] lcdp      pointer to the @p HD44780Driver object
 *
 * @api
 */
void hd44780BacklightOff(HD44780Driver *lcdp) {

  osalDbgCheck(lcdp != NULL);
  osalDbgAssert((lcdp->state == HD44780_ACTIVE), "hd44780BacklightOff(), invalid state");
#if HD44780_USE_DIMMABLE_BACKLIGHT
  pwmDisableChannel(lcdp->config->pwmp, lcdp->config->backlight_ch);

#else
  palClearLine(lcdp->config->pinmap->A);
#endif
  lcdp->backlight = 0;
}

/**
 * @brief   Clears display and return cursor in the first position.
 *
 * @param[in] lcdp      pointer to the @p HD44780Driver object
 *
 * @api
 */
void hd44780ClearDisplay(HD44780Driver *lcdp){

  osalDbgCheck(lcdp != NULL);
  osalDbgAssert((lcdp->state == HD44780_ACTIVE), "hd44780ClearDisplay(), invalid state");
  hd44780WriteRegister(lcdp, HD44780_INSTRUCTION_R, HD44780_CLEAR_DISPLAY);
}

/**
 * @brief   Return cursor in the first position.
 *
 * @param[in] lcdp      pointer to the @p HD44780Driver object
 *
 * @api
 */
void hd44780ReturnHome(HD44780Driver *lcdp){

  osalDbgCheck(lcdp != NULL);
  osalDbgAssert((lcdp->state == HD44780_ACTIVE), "hd44780ReturnHome(), invalid state");
  hd44780WriteRegister(lcdp, HD44780_INSTRUCTION_R, HD44780_RETURN_HOME);
}


/**
 * @brief   Set DDRAM address position leaving data unchanged.
 *
 * @param[in] lcdp      pointer to the @p HD44780Driver object
 * @param[in] add       DDRAM address (from 0 to HD44780_DDRAM_MAX_ADDRESS)
 *
 * @api
 */
void hd44780SetAddress(HD44780Driver *lcdp, uint8_t add){

  osalDbgCheck(lcdp != NULL);
  osalDbgAssert((lcdp->state == HD44780_ACTIVE),
                "hd44780SetAddress(), invalid state");
  if(add > HD44780_SET_DDRAM_ADDRESS_MASK)
    return;
  hd44780WriteRegister(lcdp, HD44780_INSTRUCTION_R, HD44780_SET_DDRAM_ADDRESS | add);
}

/**
 * @brief   Set DDRAM address position leaving data unchanged.
 *
 * @param[in] lcdp      pointer to the @p HD44780Driver object
 * @param[in] add       DDRAM address (from 0 to HD44780_DDRAM_MAX_ADDRESS)
 *
 * @api
 */
void hd44780SetCGAddress(HD44780Driver *lcdp, uint8_t add){

  osalDbgCheck(lcdp != NULL);
  osalDbgAssert((lcdp->state == HD44780_ACTIVE),
                "hd44780SeCGtAddress(), invalid state");
  if(add > HD44780_SET_CGRAM_ADDRESS_MASK)
    return;
  hd44780WriteRegister(lcdp, HD44780_INSTRUCTION_R, HD44780_SET_CGRAM_ADDRESS | add);
}

/**
 * @brief   Writes string starting from a certain position.
 *
 * @detail  If string lenght exceeds, then is cutted
 *
 * @param[in] lcdp      pointer to the @p HD44780Driver object
 * @param[in] string    string to write
 * @param[in] pos       position for cursor (from 0 to HD44780_DDRAM_MAX_ADDRESS)
 *
 * @api
 */
void hd44780Write(HD44780Driver *lcdp, uint8_t pos, const char* fmt, ...){
  va_list ap;
  char string[80];
  const char *s = string;
  osalDbgCheck((lcdp != NULL) && (string != NULL));
  osalDbgAssert((lcdp->state == HD44780_ACTIVE),
                "Write(), invalid state");

  va_start(ap, fmt);
  vsnprintf(string, sizeof(string), fmt, ap);
  va_end(ap);
  
  int32_t  iteration = HD44780_SET_DDRAM_ADDRESS_MASK - pos + 1;
  if(iteration > 0){
    hd44780SetAddress(lcdp, pos);
    while((*s != '\0') && (iteration > 0)){
      hd44780WriteRegister(lcdp, HD44780_DATA_R, *s);
      s++;
      iteration--;
    }
  }
}
/**
 * @brief  register custom graphic character at position.
 *
 *
 * @param[in] lcdp      pointer to the @p HD44780Driver object
 * @param[in] pos       position for custom character : from 0 to 7
 * @param[in] bitmap    string to write
 *
 * @api
 */
void hd44780CustomGraphic(HD44780Driver *lcdp, uint8_t pos, const uint8_t bitmap[8]) {
  osalDbgCheck((lcdp != NULL) && (bitmap != NULL));
  osalDbgAssert((lcdp->state == HD44780_ACTIVE),
                "CustomGraphic(), invalid state");
  
  hd44780SetCGAddress(lcdp, (pos*8));
  for (int i=0; i<8; i++) {
    hd44780WriteRegister(lcdp, HD44780_DATA_R, bitmap[i]);
  }
}


/**
 * @brief   Makes a shift according to an arbitrary direction
 *
 * @param[in] lcdp      pointer to the @p HD44780Driver object
 * @param[in] dir       direction (HD44780_RIGHT or HD44780_LEFT)
 *
 * @api
 */
void hd44780DoDisplayShift(HD44780Driver *lcdp, uint8_t dir){

  osalDbgCheck(lcdp != NULL);
  osalDbgAssert((lcdp->state == HD44780_ACTIVE),
                "hd44780DoDisplayShift(), invalid state");
  hd44780WriteRegister(lcdp, HD44780_INSTRUCTION_R, HD44780_CDS | HD44780_CDS_SC | dir);
}

void hd44780ShowCursor(HD44780Driver *lcdp, bool show){

  osalDbgCheck(lcdp != NULL);
  osalDbgAssert((lcdp->state == HD44780_ACTIVE),
                "hd44780DisplayCursor, invalid state");
  hd44780WriteRegister(lcdp, HD44780_INSTRUCTION_R,
		       show ? HD44780_BLINK_CURSOR : HD44780_NO_CURSOR);
}

#if HD44780_USE_DIMMABLE_BACKLIGHT
/**
 * @brief   Set back-light percentage.
 *
 * @param[in] lcdp      pointer to the @p HD44780Driver object
 * @param[i>n] perc      back-light percentage (from 0 to 100)
 *
 * @api
 */
void hd44780SetBacklight(HD44780Driver *lcdp, uint32_t perc){

  osalDbgCheck(lcdp != NULL);
  if (perc > 100U)
    perc = 100;
  osalDbgAssert((lcdp->state == HD44780_ACTIVE), "hd44780SetBacklight(), invalid state");
  pwmEnableChannel(lcdp->config->pwmp, lcdp->config->backlight_ch,
                   PWM_PERCENTAGE_TO_WIDTH(lcdp->config->pwmp, perc * 100));
  lcdp->backlight = perc;
}

void hd44780SetContrast(HD44780Driver *lcdp, uint32_t perc){

  osalDbgCheck(lcdp != NULL);
  if (perc > 100U)
    perc = 100;
  osalDbgAssert((lcdp->state == HD44780_ACTIVE), "hd44780SetContrast(), invalid state");
  pwmEnableChannel(lcdp->config->pwmp, lcdp->config->contrast_ch,
                   PWM_PERCENTAGE_TO_WIDTH(lcdp->config->pwmp, (100 - perc) * 100));
  lcdp->contrast = perc;
}

/**
 * @brief   Shift back-light from current value to 0.
 *
 * @param[in] lcdp      pointer to the @p HD44780Driver object
 *
 * @api
 */
void hd44780BacklightFadeOut(HD44780Driver *lcdp){
  uint32_t curr = lcdp->backlight;
  osalDbgCheck(lcdp != NULL);
  osalDbgAssert((lcdp->state == HD44780_ACTIVE),
                "hd44780BacklightFadeOut(), invalid state");
  while(curr != 0){
    curr--;
    hd44780SetBacklight(lcdp, curr);
    osalThreadSleepMilliseconds(10);
  }
}

/**
 * @brief   Shift back-light from current value to 100.
 *
 * @param[in] lcdp      pointer to the @p HD44780Driver object
 *
 * @api
 */
void hd44780BacklightFadeIn(HD44780Driver *lcdp){
  uint32_t curr = lcdp->backlight;
  osalDbgCheck(lcdp != NULL);
  osalDbgAssert((lcdp->state == HD44780_ACTIVE),
                "hd44780BacklightFadeIn(), invalid state");
  while(curr != 100){
    curr++;
    hd44780SetBacklight(lcdp, curr);
    osalThreadSleepMilliseconds(10);
  }
}
#endif

/** @} */
#endif /* USERLIB_USE_HD44780 */
