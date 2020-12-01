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
 * @file    microrlShell.h
 * @brief   Simple CLI shell header.
 *
 * @addtogroup SHELL
 * @{
 */

#ifndef _MICRORL_SHELL_H_
#define _MICRORL_SHELL_H_

/**
 * @brief   Shell maximum input line length.
 */
#if !defined(SHELL_MAX_LINE_LENGTH) || defined(__DOXYGEN__)
#define SHELL_MAX_LINE_LENGTH       64
#endif

/**
 * @brief   Shell maximum arguments per command.
 */
#if !defined(SHELL_MAX_ARGUMENTS) || defined(__DOXYGEN__)
#define SHELL_MAX_ARGUMENTS         4
#endif

/**
 * @brief   Command handler function type.
 */
typedef void (*shellcmd_t)(BaseSequentialStream *chp, int argc, const char * const argv[]);

/**
 * @brief   Custom command entry type.
 */
typedef struct {
  const char            *sc_name;           /**< @brief Command name.       */
  shellcmd_t            sc_function;        /**< @brief Command function.   */
} ShellCommand;

/**
 * @brief   Shell descriptor type.
 */
typedef struct {
  BaseSequentialStream  *sc_channel;        /**< @brief I/O channel associated
                                                 to the shell.              */
  ShellCommand    *sc_commands;       /**< @brief Shell extra commands
                                                 table.                     */
} ShellConfig;

#include "portage.h"

#if !defined(__DOXYGEN__)
extern EventSource shell_terminated;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void shellInit(void);
  Thread *shellCreate(const ShellConfig *scp, size_t size, tprio_t prio);
  Thread *shellCreateStatic(const ShellConfig *scp, void *wsp,
                            size_t size, tprio_t prio);
  bool shellAddEntry(const ShellCommand sc);
  bool shellGetLine(BaseSequentialStream *chp, char *line, unsigned size);
  void modeAlternate (void (*) (uint8_t c, uint32_t mode), uint32_t mode);
  void modeShell (void);
#ifdef __cplusplus
}
#endif

#endif /* _MICRORL_SHELL_H_ */

/** @} */
