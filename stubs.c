/*
 * stubs.c
 *
 *  Created on: Jul 10, 2011
 *      Author: mabl
 */

#include <stdio.h>

#include "ch.h"
#include "hal.h"

#include "stubs.h"

void _exit(int status){
	(void) status;
	chSysHalt();

	while(TRUE){}
}

pid_t _getpid(void){
	return 1;
}

void _kill(pid_t id){
	(void) id;
}

void __cxa_pure_virtual(void) {
  chDbgPanic("__cxa_pure_virtual() #1");
}

int __cxa_guard_acquire(__guard *g) {
  chSysLock();
  const int result = !*(char *)(g);
  chSysUnlock();

  return result;
}

void __cxa_guard_release(__guard *g) {
  chSysLock();
  *(char *)g = 1;
  chSysUnlock();
}

void __cxa_guard_abort(__guard *g) {
  chSysLock();
  *(char *)g = 1;
  chSysUnlock();
}

int _open_r(void *reent, const char *file, int flags, int mode) {
	(void)reent; (void)file; (void)flags; (void)mode;
	return -1;
}

int __register_exitproc(int type, void (*fn) (void), void *arg, void *d) {
  (void) type;
  (void) fn;
  (void) arg;
  (void) d;


  return 0;
}


