/*
    ChibiOS - Copyright (C) 2006..2016 Giovanni Di Sirio

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

#include <stdio.h>
#include <string.h>

#include "ch.h"
#include "hal.h"

#include "chprintf.h"

#include "usb_msd.h"
#include "ff.h"



/*===========================================================================*/
/* FatFs related.                                                            */
/*===========================================================================*/

/**
 * @brief FS object.
 */
static FATFS SDC_FS;

/* FS mounted and ready.*/
static bool fs_ready = FALSE;

#define RED_ON palSetPad(GPIOF, GPIOF_PIN8);
void debug_print_str(char * string);


/*
 * LED blinker thread, times are in milliseconds.
 */
static THD_WORKING_AREA(waThread1, 128);
static THD_FUNCTION(Thread1, arg) {

  (void)arg;
  chRegSetThreadName("blinker");
  while (true)
  {
    palTogglePad(GPIOF, GPIOF_PIN9);
    chThdSleepMilliseconds(fs_ready ? 125 : 500);
  }
}


THD_WORKING_AREA(dbg_wa, 512);
THD_FUNCTION(dbg, arg)
{
  (void) arg;
  chRegSetThreadName("dbg_printer");

  while (true)
  {
    eventmask_t events = chEvtWaitAny
        (
                EVT_USB_RESET        |
                EVT_BOT_RESET        |
                EVT_SEM_TAKEN        |
                EVT_SEM_RELEASED     |
                EVT_USB_CONFIGURED   |
                EVT_SCSI_REQ_READ_FMT_CAP |
                EVT_SCSI_REQ_TEST_UNIT_READY |
                EVT_SCSI_REQ_SENSE6 |
                EVT_SCSI_REQ_SENSE10 |
                EVT_WAIT_FOR_COMMAND_BLOCK |
                EVT_SCSI_REQ_SEND_DIAGNOSTIC |
                EVT_SCSI_REQ_READ_CAP10 |
                EVT_SCSI_PROC_INQ
                );


    if (events & EVT_USB_RESET)
    {
      debug_print_str("Standard USB Reset issued");
    }

    if (events & EVT_BOT_RESET)
    {
      debug_print_str("BOT Reset issued");
    }

    if (events & EVT_SEM_TAKEN)
    {
      debug_print_str("Semaphore was taken");
    }
    if (events & EVT_SEM_RELEASED)
    {
      debug_print_str("Semaphore was released");
    }
    if (events & EVT_USB_CONFIGURED)
    {
      debug_print_str("USB was confugured");
    }
    if (events & EVT_SCSI_REQ_TEST_UNIT_READY)
    {
      debug_print_str("SCSI_REQ_TEST_UNIT_READY");
    }
    if (events & EVT_SCSI_REQ_READ_FMT_CAP)
    {
      debug_print_str("SCSI_REQ_READ_FMT_CAP");
    }
    if (events & EVT_SCSI_REQ_SENSE6)
    {
      debug_print_str("SCSI_REQ_SENSE6");
    }
    if (events & EVT_WAIT_FOR_COMMAND_BLOCK)
    {
      debug_print_str("WAIT_FOR_COMMAND_BLOCK");
    }
    if (events & EVT_SCSI_REQ_SENSE10)
    {
      debug_print_str("SCSI_REQ_SENSE10");
    }
    if (events & EVT_SCSI_REQ_SEND_DIAGNOSTIC)
    {
      debug_print_str("SCSI_REQ_SEND_DIAGNOSTIC");
    }
    if (events & EVT_SCSI_REQ_READ_CAP10)
    {
      debug_print_str("SCSI_REQ_READ_CAP10");
    }
    if (events & EVT_SCSI_PROC_INQ)
    {
      debug_print_str("SCSI_PROC_INQ");
    }
  }
}


uint8_t mmc_scrachpad[512];
SDCConfig cfg =
{
 mmc_scrachpad,
 SDC_MODE_4BIT
};


/*
 * FS thread
 */
static THD_WORKING_AREA(waThreadFS, 512);
static THD_FUNCTION(ThreadFS, arg)
{

  (void)arg;
  chRegSetThreadName("FS");
  chThdSleepMilliseconds(550);


  /*
   * SD card insertion monitor.
   */
  sdcStart(&SDCD1, &cfg);
  if (sdcConnect(&SDCD1) == HAL_FAILED)
  {
    RED_ON;
    debug_print_str("Can not initialize SD card");
  };

  FRESULT err = f_mount(&SDC_FS, "/", 1);
  if (err != FR_OK)
  {
    RED_ON;
    fs_ready = FALSE;
    debug_print_str("Can not mount SD card filesystem");
  }
  else
  {
    fs_ready = true;
    debug_print_str("Here we go!");
  }

  init_msd_driver(arg);

  while (true)
  {
    chThdSleepMilliseconds(500);
  }
}




/*
 * Serial driver configuration structure.
 */
static SerialConfig sercfg1 =
{
 19200,
 0,
 0,//USART_CR2_LINEN,
 0
};

BaseSequentialStream * chp;


void debug_print_str(char * string)
{
  chprintf(chp, "%s\n\r", string);
}

static thread_t * prn;

/*
 *  * Application entry point.
 */
int main(void)
{

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  /*
   * Activates the serial driver 6 and SDC driver 1 using default
   * configuration.
   */
  sdStart(&SD1, &sercfg1);

  chp = (BaseSequentialStream *)&SD1;

  /*
   * Creates debug printer thread.
   */
  prn = chThdCreateStatic(dbg_wa, sizeof(dbg_wa), NORMALPRIO - 1, dbg, NULL);

  /*
   * Creates the blinker thread.
   */
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

  /*
   * Creates the FS thread.
   */
  chThdCreateStatic(waThreadFS, sizeof(waThreadFS), NORMALPRIO + 1, ThreadFS, prn);



  /*
   * Normal main() thread activity, handling SD card events and shell
   * start/exit.
   */

  while (true)
  {
    chThdSleepMilliseconds(500);
  }
}
