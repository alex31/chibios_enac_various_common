/**
 * @file    microrlShell.h
 * @brief   Simple CLI shell header.
 *
 * @addtogroup SHELL
 * @{
 */

#pragma once


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

#if !defined(__DOXYGEN__)
extern  event_source_t shell_terminated;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void shellInit(void);
   thread_t *shellCreate(const ShellConfig *scp, size_t size, tprio_t prio);
   thread_t *shellCreateStatic(const ShellConfig *scp, void *wsp,
                            size_t size, tprio_t prio);
  bool shellAddEntry(const ShellCommand sc);
  bool shellGetLine(BaseSequentialStream *chp, char *line, unsigned size);
  void modeAlternate (void (*) (uint8_t c, uint32_t mode), uint32_t mode);
  void modeShell (void);
#ifdef __cplusplus
}
#endif


/** @} */
