/* stubs.h
 *
 *  Created on: Aug 6, 2011
 *      Author: mabl
 */

#ifndef STUBS_H_
#define STUBS_H_

#ifdef __cplusplus
extern "C" {
#endif

/* The ABI requires a 64-bit type.  */
__extension__ typedef int __guard __attribute__((mode (__DI__)));

int __cxa_guard_acquire(__guard *);
void __cxa_guard_release (__guard *);
void __cxa_guard_abort (__guard *);

void *__dso_handle = NULL;

#ifdef __cplusplus
}
#endif

#endif /* STUBS_H_ */

