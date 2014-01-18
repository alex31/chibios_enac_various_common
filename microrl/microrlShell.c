/*
    ChibiOS/RT - Copyright (C) 2006,2007,2008,2009,2010,
                 2011,2012 Giovanni Di Sirio.

    This file is part of ChibiOS/RT.

    ChibiOS/RT is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS/RT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file    shell.c
 * @brief   Enhanced CLI shell code.
 *
 * @addtogroup SHELL
 * @{
 */

#include <string.h>

#include "ch.h"
#include "hal.h"
#include "microrl/microrlShell.h"
#include "microrl/microrl.h"
#include "printf.h"
#include "stdutil.h"


#define printScreen(...) {chprintf (chpg, __VA_ARGS__); chprintf (chpg, "\r\n");}


typedef struct {
  void (*altFunc) (uint8_t c, uint32_t mode);
  uint32_t param;
} AltCbParam;
  

/**
 * @brief   Shell termination event source.
 */
EventSource shell_terminated;

static microrl_t rl;
static BaseSequentialStream *chpg;
const ShellCommand *scpg;
const char * compl_world[64];
uint32_t numOfCommand = 0;
static ShellCommand local_commands[];
static AltCbParam altCbParam = {.altFunc = NULL, .param = 0};

void microrlPrint (const char * str)
{
  int i = 0;

  while (str[i] != 0) {
    chSequentialStreamPut(chpg, str[i++]);
  }
}

void microrlExecute (int argc,  const char * const *argv)
{
  const ShellCommand *scp = scpg;
  const char *name = argv[0];

  while (scp->sc_name != NULL) {
    if (strcasecmp(scp->sc_name, name) == 0) {
      scp->sc_function(chpg, argc-1, &argv[1]);
      return;
    }
    scp++;
  }

  scp = local_commands;
   while (scp->sc_name != NULL) {
    if (strcasecmp(scp->sc_name, name) == 0) {
      scp->sc_function(chpg, argc-1, &argv[1]);
      return;
    }
    scp++;
  }
}

const char ** microrlComplet (int argc, const char * const * argv)
{
  uint32_t j = 0;

  compl_world [0] = NULL;

  // if there is token in cmdline
  if (argc == 1) {
    // get last entered token
    char * bit = (char*)argv [argc-1];
    // iterate through our available token and match it
    for (uint32_t i = 0; i < numOfCommand; i++) {
      // if token is matched (text is part of our token starting from 0 char)
      if (strstr(scpg[i].sc_name, bit) == scpg[i].sc_name) {
	// add it to completion set
	compl_world [j++] = scpg[i].sc_name;
      }
    }
  } else { // if there is no token in cmdline, just print all available token
    compl_world[0] = "info";
    for (j=0; j <= numOfCommand; j++) {
      compl_world[j+1] = scpg[j].sc_name;
    }
  }

  // note! last ptr in array always must be NULL!!!
  compl_world [j] = NULL;
  // return set of variants
  return compl_world;
}


void microrlSigint (void)
{
  chprintf (chpg, "^C catched!\n\r");
}


static void usage(BaseSequentialStream *chp, char *p) {

  chprintf(chp, "Usage: %s\r\n", p);
}



static void cmd_info(BaseSequentialStream *chp, int argc,  const char * const argv[]) {

  (void)argv;
  if (argc > 0) {
    usage(chp, "info");
    return;
  }

  chprintf(chp, "Kernel:       %s\r\n", CH_KERNEL_VERSION);
#ifdef CH_COMPILER_NAME
  chprintf(chp, "Compiler:     %s\r\n", CH_COMPILER_NAME);
#endif
  chprintf(chp, "Architecture: %s\r\n", CH_ARCHITECTURE_NAME);
#ifdef CH_CORE_VARIANT_NAME
  chprintf(chp, "Core Variant: %s\r\n", CH_CORE_VARIANT_NAME);
#endif
#ifdef CH_PORT_INFO
  chprintf(chp, "Port Info:    %s\r\n", CH_PORT_INFO);
#endif
#ifdef PLATFORM_NAME
  chprintf(chp, "Platform:     %s\r\n", PLATFORM_NAME);
#endif
#ifdef BOARD_NAME
  chprintf(chp, "Board:        %s\r\n", BOARD_NAME);
#endif
#ifdef __DATE__
#ifdef __TIME__
  chprintf(chp, "Build time:   %s%s%s\r\n", __DATE__, " - ", __TIME__);
#endif
#endif
  chprintf(chp, "systime= %lu\r\n", (unsigned long)chTimeNow());
}


/**
 * @brief   Array of the default commands.
 */
static ShellCommand local_commands[] = {
  {"info", cmd_info},
  {NULL, NULL}
};



/**
 * @brief   Shell thread function.
 *
 * @param[in] p         pointer to a @p BaseSequentialStream object
 * @return              Termination reason.
 * @retval RDY_OK       terminated by command.
 * @retval RDY_RESET    terminated by reset condition on the I/O channel.
 */
static msg_t shell_thread(void *p) {
  msg_t msg = RDY_OK;
  chpg = ((ShellConfig *)p)->sc_channel;
  scpg = ((ShellConfig *)p)->sc_commands;
  bool_t readOk=TRUE;

  
  const ShellCommand *scp = scpg;
  while (scp->sc_name != NULL) {
    scp++;
    numOfCommand++;
  }

  chRegSetThreadName("Enhanced_shell");
  printScreen ("ChibiOS/RT Enhanced Shell");
  while (!chThdShouldTerminate() && readOk) {
    uint8_t c;
    if (chSequentialStreamRead(chpg, &c, 1) == 0) {
       readOk=FALSE;
    } else {
      if (altCbParam.altFunc == NULL) {
	microrl_insert_char (&rl, c);
      } else {
	(*altCbParam.altFunc) (c, altCbParam.param);
      }
    }
  }
  /* Atomically broadcasting the event source and terminating the thread,
     there is not a chSysUnlock() because the thread terminates upon return.*/
  printScreen ("exit");
  chSysLock();
  chEvtBroadcastI(&shell_terminated);
  chThdExitS(msg);
  return 0; /* Never executed.*/
}

/**
 * @brief   Shell manager initialization.
 */
void shellInit(void) {

  chEvtInit(&shell_terminated);
  microrl_init (&rl, microrlPrint);
  microrl_set_execute_callback (&rl, &microrlExecute);
  microrl_set_complete_callback (&rl, &microrlComplet);
  microrl_set_sigint_callback (&rl, &microrlSigint);
}

/**
 * @brief   Spawns a new shell.
 * @pre     @p CH_USE_MALLOC_HEAP and @p CH_USE_DYNAMIC must be enabled.
 *
 * @param[in] scp       pointer to a @p ShellConfig object
 * @param[in] size      size of the shell working area to be allocated
 * @param[in] prio      priority level for the new shell
 * @return              A pointer to the shell thread.
 * @retval NULL         thread creation failed because memory allocation.
 */
#if CH_USE_HEAP && CH_USE_DYNAMIC
Thread *shellCreate(const ShellConfig *scp, size_t size, tprio_t prio) {

  return chThdCreateFromHeap(NULL, size, prio, shell_thread, (void *)scp);
}
#endif

/**
 * @brief   Create statically allocated shell thread.
 *
 * @param[in] scp       pointer to a @p ShellConfig object
 * @param[in] wsp       pointer to a working area dedicated to the shell thread stack
 * @param[in] size      size of the shell working area
 * @param[in] prio      priority level for the new shell
 * @return              A pointer to the shell thread.
 */
Thread *shellCreateStatic(const ShellConfig *scp, void *wsp,
                          size_t size, tprio_t prio) {
  return chThdCreateStatic(wsp, size, prio, shell_thread, (void *)scp);
}


void modeAlternate(void (*funcp) (uint8_t c, uint32_t mode), uint32_t mode)
{
  altCbParam.altFunc = funcp;
  altCbParam.param = mode;
}

void modeShell(void)
{
  altCbParam.altFunc = NULL;
  printScreen ("retour au shell");
}



