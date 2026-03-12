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
 * @brief   Internal transaction phases for the GPT-driven HD44780 engine.
 *
 * @details
 * The public API intentionally remains synchronous even when the driver is
 * configured with a GPT. The apparent contradiction is resolved by splitting
 * a register write into short hardware phases:
 * - pre-write busy polling
 * - high-nibble pulse
 * - low-nibble pulse in 4-bit mode
 * - optional long-command delay
 * - final busy polling for long commands
 *
 * The caller thread prepares the transaction, suspends once, and is resumed
 * only when the state machine reaches a terminal condition. Every transition
 * below is executed from the GPT callback in I-class context.
 *
 * This is more intricate than a classic polled HD44780 routine, but it buys
 * back CPU time on RTOS systems because millisecond-scale waits are no longer
 * spent spinning in a worker thread.
 */
typedef enum {
  /** @brief No transaction in progress. */
  HD44780_TX_IDLE = 0,
  /** @brief DB7 is sampled while E is high on the first busy-read nibble. */
  HD44780_TX_WAIT_BUSY_SAMPLE_HIGH,
#if HD44780_USE_4_BIT_MODE
  /** @brief Issues the dummy second pulse required by a 4-bit read cycle. */
  HD44780_TX_WAIT_BUSY_PULSE_LOW_RISE,
  /** @brief Completes the dummy second pulse of a 4-bit read cycle. */
  HD44780_TX_WAIT_BUSY_PULSE_LOW_FALL,
#endif
  /** @brief Restarts busy polling while the controller is still busy. */
  HD44780_TX_WAIT_BUSY_RETRY_RISE,
  /** @brief Drives the bus with the high nibble and raises E. */
  HD44780_TX_WRITE_SETUP_HIGH,
  /** @brief Completes the high-nibble E pulse. */
  HD44780_TX_WRITE_PULSE_HIGH_FALL,
#if HD44780_USE_4_BIT_MODE
  /** @brief Drives the low nibble and raises E. */
  HD44780_TX_WRITE_SETUP_LOW,
  /** @brief Completes the low-nibble E pulse. */
  HD44780_TX_WRITE_PULSE_LOW_FALL,
#endif
  /** @brief Waits for the nominal execution time of a long LCD instruction. */
  HD44780_TX_LONG_DELAY
} hd44780_tx_state_t;

static bool hd44780IsBusy(HD44780Driver *lcdp);

static bool hd44780IsLongInstruction(uint8_t reg, uint8_t value) {
  return (reg == HD44780_INSTRUCTION_R) &&
         ((value == HD44780_CLEAR_DISPLAY) || (value == HD44780_RETURN_HOME));
}

static bool hd44780UseGPT(const HD44780Driver *lcdp) {
  return (lcdp->config != NULL) && (lcdp->config->gpt.gptd != NULL);
}

static inline HD44780Driver *hd44780GetDriverByGPT(GPTDriver *gptp) {
  return (HD44780Driver *)((uint8_t *)gptp->config - offsetof(HD44780Driver, gptcfg));
}

static void hd44780SetDataInput(HD44780Driver *lcdp) {
  for (unsigned ii = 0; ii < LINE_DATA_LEN; ii++) {
    palSetLineMode(lcdp->config->pinmap->D[ii], PAL_MODE_INPUT);
  }
}

static void hd44780SetDataOutput(HD44780Driver *lcdp) {
  for (unsigned ii = 0; ii < LINE_DATA_LEN; ii++) {
    palSetLineMode(lcdp->config->pinmap->D[ii],
                   PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
  }
}

static void hd44780WriteBusValue(HD44780Driver *lcdp, uint8_t value) {
  for (unsigned ii = 0; ii < LINE_DATA_LEN; ii++) {
    if (value & (1U << ii)) {
      palSetLine(lcdp->config->pinmap->D[ii]);
    } else {
      palClearLine(lcdp->config->pinmap->D[ii]);
    }
  }
}

static void hd44780ArmTimerI(HD44780Driver *lcdp, gptcnt_t ticks) {
  gptStartOneShotI(lcdp->config->gpt.gptd, ticks);
}

/**
 * @brief   Completes the current transaction and wakes the suspended caller.
 *
 * @details
 * This helper is the single exit point of the GPT-driven path. Centralizing
 * the cleanup here avoids leaving stale FSM state behind when a transaction
 * terminates through different branches.
 *
 * @param[in] lcdp      pointer to the driver object
 * @param[in] msg       wake-up code returned to the suspended caller
 *
 * @iclass
 */
static void hd44780ResumeWaiterI(HD44780Driver *lcdp, msg_t msg) {
  lcdp->tx_state = HD44780_TX_IDLE;
  lcdp->tx_poll_after_write = false;
  osalThreadResumeI(&lcdp->waiter, msg);
}

/**
 * @brief   Starts a busy-flag read cycle.
 *
 * @details
 * The meaning of a future "busy == false" sample depends on
 * @p tx_poll_after_write:
 * - @p false means the controller is ready to accept the next write
 * - @p true means a previously emitted long instruction has completed
 *
 * Keeping both use cases in the same read path avoids duplicating the
 * HD44780 4-bit read handshake, while still allowing the callback to make the
 * correct decision when DB7 finally drops.
 *
 * @param[in] lcdp      pointer to the driver object
 *
 * @iclass
 */
static void hd44780StartBusyPollingI(HD44780Driver *lcdp) {
  hd44780SetDataInput(lcdp);
  palSetLine(lcdp->config->pinmap->RW);
  palClearLine(lcdp->config->pinmap->RS);
  palSetLine(lcdp->config->pinmap->E);
  lcdp->tx_state = HD44780_TX_WAIT_BUSY_SAMPLE_HIGH;
  hd44780ArmTimerI(lcdp, lcdp->short_delay_ticks);
}

static void hd44780StartWriteHighI(HD44780Driver *lcdp) {
  hd44780SetDataOutput(lcdp);
  palClearLine(lcdp->config->pinmap->RW);
  palWriteLine(lcdp->config->pinmap->RS, lcdp->tx_reg);
#if HD44780_USE_4_BIT_MODE
  hd44780WriteBusValue(lcdp, lcdp->tx_value >> 4);
#else
  hd44780WriteBusValue(lcdp, lcdp->tx_value);
#endif
  palSetLine(lcdp->config->pinmap->E);
  lcdp->tx_state = HD44780_TX_WRITE_PULSE_HIGH_FALL;
  hd44780ArmTimerI(lcdp, lcdp->short_delay_ticks);
}

#if HD44780_USE_4_BIT_MODE
static void hd44780StartWriteLowI(HD44780Driver *lcdp) {
  hd44780WriteBusValue(lcdp, lcdp->tx_value);
  palSetLine(lcdp->config->pinmap->E);
  lcdp->tx_state = HD44780_TX_WRITE_PULSE_LOW_FALL;
  hd44780ArmTimerI(lcdp, lcdp->short_delay_ticks);
}
#endif

static void hd44780GptCb(GPTDriver *gptp) {
  HD44780Driver *lcdp = hd44780GetDriverByGPT(gptp);

  /*
   * Lions-book version:
   * the callback does one small piece of bus work, decides what the next
   * piece is, rearms the one-shot timer if needed, and leaves. There is no
   * polling loop here by design. The "loop" is encoded in the succession of
   * states and timer expirations.
   */
  osalSysLockFromISR();

  switch ((hd44780_tx_state_t)lcdp->tx_state) {
  case HD44780_TX_WAIT_BUSY_SAMPLE_HIGH:
    lcdp->tx_busy_sample =
      (palReadLine(lcdp->config->pinmap->D[LINE_DATA_LEN - 1]) == PAL_HIGH);
    palClearLine(lcdp->config->pinmap->E);
#if HD44780_USE_4_BIT_MODE
    lcdp->tx_state = HD44780_TX_WAIT_BUSY_PULSE_LOW_RISE;
    hd44780ArmTimerI(lcdp, lcdp->short_delay_ticks);
#else
    if (lcdp->tx_busy_sample) {
      lcdp->tx_state = HD44780_TX_WAIT_BUSY_RETRY_RISE;
      hd44780ArmTimerI(lcdp, lcdp->short_delay_ticks);
    } else {
      if (lcdp->tx_poll_after_write) {
        hd44780ResumeWaiterI(lcdp, MSG_OK);
      } else {
        lcdp->tx_state = HD44780_TX_WRITE_SETUP_HIGH;
        hd44780ArmTimerI(lcdp, lcdp->short_delay_ticks);
      }
    }
#endif
    break;

#if HD44780_USE_4_BIT_MODE
  case HD44780_TX_WAIT_BUSY_PULSE_LOW_RISE:
    palSetLine(lcdp->config->pinmap->E);
    lcdp->tx_state = HD44780_TX_WAIT_BUSY_PULSE_LOW_FALL;
    hd44780ArmTimerI(lcdp, lcdp->short_delay_ticks);
    break;

  case HD44780_TX_WAIT_BUSY_PULSE_LOW_FALL:
    palClearLine(lcdp->config->pinmap->E);
    if (lcdp->tx_busy_sample) {
      lcdp->tx_state = HD44780_TX_WAIT_BUSY_RETRY_RISE;
      hd44780ArmTimerI(lcdp, lcdp->short_delay_ticks);
    } else {
      if (lcdp->tx_poll_after_write) {
        hd44780ResumeWaiterI(lcdp, MSG_OK);
      } else {
        lcdp->tx_state = HD44780_TX_WRITE_SETUP_HIGH;
        hd44780ArmTimerI(lcdp, lcdp->short_delay_ticks);
      }
    }
    break;
#endif

  case HD44780_TX_WAIT_BUSY_RETRY_RISE:
    hd44780StartBusyPollingI(lcdp);
    break;

  case HD44780_TX_WRITE_SETUP_HIGH:
    hd44780StartWriteHighI(lcdp);
    break;

  case HD44780_TX_WRITE_PULSE_HIGH_FALL:
    palClearLine(lcdp->config->pinmap->E);
#if HD44780_USE_4_BIT_MODE
    lcdp->tx_state = HD44780_TX_WRITE_SETUP_LOW;
    hd44780ArmTimerI(lcdp, lcdp->short_delay_ticks);
#else
    if (hd44780IsLongInstruction(lcdp->tx_reg, lcdp->tx_value)) {
      lcdp->tx_state = HD44780_TX_LONG_DELAY;
      hd44780ArmTimerI(lcdp, lcdp->long_delay_ticks);
    } else {
      hd44780ResumeWaiterI(lcdp, MSG_OK);
    }
#endif
    break;

#if HD44780_USE_4_BIT_MODE
  case HD44780_TX_WRITE_SETUP_LOW:
    hd44780StartWriteLowI(lcdp);
    break;

  case HD44780_TX_WRITE_PULSE_LOW_FALL:
    palClearLine(lcdp->config->pinmap->E);
    if (hd44780IsLongInstruction(lcdp->tx_reg, lcdp->tx_value)) {
      lcdp->tx_state = HD44780_TX_LONG_DELAY;
      hd44780ArmTimerI(lcdp, lcdp->long_delay_ticks);
    } else {
      hd44780ResumeWaiterI(lcdp, MSG_OK);
    }
    break;
#endif

  case HD44780_TX_LONG_DELAY:
    lcdp->tx_poll_after_write = true;
    hd44780StartBusyPollingI(lcdp);
    break;

  case HD44780_TX_IDLE:
  default:
    hd44780ResumeWaiterI(lcdp, MSG_RESET);
    break;
  }

  osalSysUnlockFromISR();
}

static void hd44780WriteRegisterPolled(HD44780Driver *lcdp, uint8_t reg, uint8_t value){

  unsigned ii;

  while (hd44780IsBusy(lcdp))
    ;

  /* Configuring Data PINs as Output Push Pull. */
  hd44780SetDataOutput(lcdp);

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
  HD44780_ENABLE_PIN_DELAY();
  palClearLine(lcdp->config->pinmap->E);
  HD44780_ENABLE_PIN_DELAY();

  for(ii = 0; ii < LINE_DATA_LEN; ii++) {
    if(value & (1 << ii))
      palSetLine(lcdp->config->pinmap->D[ii]);
    else
      palClearLine(lcdp->config->pinmap->D[ii]);
  }
  palSetLine(lcdp->config->pinmap->E);
  HD44780_ENABLE_PIN_DELAY();
  palClearLine(lcdp->config->pinmap->E);
  HD44780_ENABLE_PIN_DELAY();
#else
  for(ii = 0; ii < LINE_DATA_LEN; ii++){
      if(value & (1 << ii))
        palSetLine(lcdp->config->pinmap->D[ii]);
      else
        palClearLine(lcdp->config->pinmap->D[ii]);
  }
  palSetLine(lcdp->config->pinmap->E);
  HD44780_ENABLE_PIN_DELAY();
  palClearLine(lcdp->config->pinmap->E);
  HD44780_ENABLE_PIN_DELAY();
#endif

  if (hd44780IsLongInstruction(reg, value)) {
    osalThreadSleepMicroseconds(HD44780_LONG_INSTR_DELAY_US);
    while (hd44780IsBusy(lcdp))
      ;
  }
}

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
  HD44780_ENABLE_PIN_DELAY();
  busy = (palReadLine(lcdp->config->pinmap->D[LINE_DATA_LEN - 1]) == PAL_HIGH);
  palClearLine(lcdp->config->pinmap->E);
  HD44780_ENABLE_PIN_DELAY();

#if HD44780_USE_4_BIT_MODE
  palSetLine(lcdp->config->pinmap->E);
  HD44780_ENABLE_PIN_DELAY();
  palClearLine(lcdp->config->pinmap->E);
  HD44780_ENABLE_PIN_DELAY();
#endif
  return busy;
}

/**
 * @brief   Write a data into a register for the lcd
 *
 * @details
 * In GPT mode this routine is still synchronous from the caller point of
 * view, but the actual bus transaction is delegated to the callback-driven
 * FSM. The call sequence is:
 * - reject reentry if another transaction is active
 * - latch the target register and byte
 * - start pre-write busy polling
 * - suspend the caller thread once
 * - resume when the FSM reaches completion
 *
 * The caller therefore pays one suspend/resume pair per transaction instead
 * of spending the whole LCD execution time in active polling.
 *
 * @param[in] lcdp          HD44780 driver
 * @param[in] reg           Register id
 * @param[in] value         Writing value
 *
 * @notapi
 */
static void hd44780WriteRegister(HD44780Driver *lcdp, uint8_t reg, uint8_t value){
  if (!hd44780UseGPT(lcdp)) {
    hd44780WriteRegisterPolled(lcdp, reg, value);
    return;
  }

  osalDbgAssert(lcdp->tx_state == HD44780_TX_IDLE, "hd44780 transaction reentry");

  lcdp->tx_reg = reg;
  lcdp->tx_value = value;
  lcdp->tx_busy_sample = false;
  lcdp->tx_poll_after_write = false;

  osalSysLock();
  hd44780StartBusyPollingI(lcdp);
  (void) osalThreadSuspendS(&lcdp->waiter);
  osalSysUnlock();
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
  HD44780_ENABLE_PIN_DELAY();
  palClearLine(lcdp->config->pinmap->E);
  osalThreadSleepMilliseconds(5);

  palSetLine(lcdp->config->pinmap->E);
  HD44780_ENABLE_PIN_DELAY();
  palClearLine(lcdp->config->pinmap->E);
  HD44780_ENABLE_PIN_DELAY();

  palSetLine(lcdp->config->pinmap->E);
  HD44780_ENABLE_PIN_DELAY();
  palClearLine(lcdp->config->pinmap->E);

#if HD44780_USE_4_BIT_MODE
  palSetLine(lcdp->config->pinmap->D[LINE_DATA_LEN - 3]);
  palClearLine(lcdp->config->pinmap->D[LINE_DATA_LEN - 4]);
  palSetLine(lcdp->config->pinmap->E);
  HD44780_ENABLE_PIN_DELAY();
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
  lcdp->waiter = NULL;
  lcdp->short_delay_ticks = 0;
  lcdp->long_delay_ticks = 0;
  lcdp->tx_state = HD44780_TX_IDLE;
  lcdp->tx_reg = 0;
  lcdp->tx_value = 0;
  lcdp->tx_busy_sample = false;
  lcdp->tx_poll_after_write = false;
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
  if (hd44780UseGPT(lcdp)) {
    osalDbgAssert(lcdp->config->gpt.frequency >= 1000000U,
                  "hd44780 gpt frequency must be >= 1MHz");
    osalDbgAssert(lcdp->config->gpt.step_delay_us > 0U,
                  "hd44780 gpt step delay must be > 0");
    lcdp->gptcfg = (GPTConfig) {
      .frequency = lcdp->config->gpt.frequency,
      .callback = hd44780GptCb,
      .cr2 = 0U,
      .dier = 0U
    };
    lcdp->short_delay_ticks = (gptcnt_t)US2RTC(lcdp->config->gpt.frequency,
                                               lcdp->config->gpt.step_delay_us);
    lcdp->long_delay_ticks = (gptcnt_t)US2RTC(lcdp->config->gpt.frequency,
                                              HD44780_LONG_INSTR_DELAY_US);
    gptStart(lcdp->config->gpt.gptd, &lcdp->gptcfg);
  }
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
  if (hd44780UseGPT(lcdp)) {
    gptStopTimer(lcdp->config->gpt.gptd);
    gptStop(lcdp->config->gpt.gptd);
    lcdp->waiter = NULL;
    lcdp->tx_state = HD44780_TX_IDLE;
  }
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
void hd44780RawWrite(HD44780Driver *lcdp, uint8_t pos, const char* buf){
  const char *s = buf;
  osalDbgCheck((lcdp != NULL) && (buf != NULL));
  osalDbgAssert((lcdp->state == HD44780_ACTIVE),
                "Write(), invalid state");

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
