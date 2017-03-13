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
/*
   Concepts and parts of this file have been contributed by Fabio Utzig.
 */

/**
 * @file    chprintf.c
 * @brief   Mini printf-like functionality.
 *
 * @addtogroup chprintf
 * @{
 */


#include <stdarg.h>

#include "ch.h"
#include "printf.h"
#include "portage.h"
#include <stdnoreturn.h>
#include <ctype.h>


#define MAX_FILLER 11
#define FLOAT_PRECISION 100000

#ifdef CHPRINTF_USE_STDLIB
#ifndef CHPRINTF_BUFFER_SIZE
#define CHPRINTF_BUFFER_SIZE 160
#endif
#include <stdio.h>
#include <stdarg.h>
#else
#define CHPRINTF_USE_STDLIB 0
#endif


typedef struct {
  union  {
    BaseSequentialStream *chp;
    size_t size;
  };
  char *destBuf;
  const char *fmt;
  va_list ap;
} synchronous_print_arg_t;

static Thread *printThreadPtr = NULL;
MUTEX_DECL(printThreadMutex);

static WORKING_AREA(waSerialPrint, CHPRINTF_USE_STDLIB ? 1024 : 420);

#if (CH_KERNEL_MAJOR != 2)
static noreturn void serialPrint (void *arg)
#else
static msg_t serialPrint (void *arg)
#endif
{

  (void)arg;
  chRegSetThreadName("serialPrint");
  
  while (TRUE) { 
    Thread *sender = chMsgWait ();
    synchronous_print_arg_t *spat = (synchronous_print_arg_t *) chMsgGet (sender);
    // do the print
    if (spat->destBuf == NULL) {
      directchvprintf(spat->chp, spat->fmt, spat->ap);
    } else {
      chvsnprintf (spat->destBuf, spat->size, spat->fmt, spat->ap);
    }
    chMsgRelease (sender, RDY_OK);
  }
#if (CH_KERNEL_MAJOR == 2)
  return RDY_OK;
#endif
}

#if !CHPRINTF_USE_STDLIB
#if CHPRINTF_USE_FLOAT
static int intPow(int a, int b)
{
  uint32_t c=a;
  for (uint32_t n=b; n>1; n--) c*=a;
  return c;
}
#endif





static char *long_to_string_with_divisor(char *p,
                                         long num,
                                         unsigned radix,
                                         long divisor) {
  int i;
  char *q;
  long l, ll;

  l = num;
  if (divisor == 0) {
    ll = num;
  } else {
    ll = divisor;
  }

  q = p + MAX_FILLER;
  do {
    i = (int)(l % radix);
    i += '0';
    if (i > '9')
      i += 'A' - '0' - 10;
    *--q = i;
    l /= radix;
  } while ((ll /= radix) != 0);

  i = (int)(p + MAX_FILLER - q);
  do
    *p++ = *q++;
  while (--i);

  return p;
}

static char *ltoa(char *p, long num, unsigned radix) {

  return long_to_string_with_divisor(p, num, radix, 0);
}


#if CHPRINTF_USE_FLOAT
static char *ftoa(char *p, double num, uint32_t precision) {
  long l;
  //  unsigned long precision = FLOAT_PRECISION;

  l = num;
  p = long_to_string_with_divisor(p, l, 10, 0);
  *p++ = '.';
  l = (num - l) * precision;
  return long_to_string_with_divisor(p, l, 10, precision / 10);
}
#endif
#endif

/**
 * @brief   System formatted output function.
 * @details This function implements a minimal @p printf() like functionality
 *          with output on a @p BaseSequentialStream.
 *          The general parameters format is: %[-][width|*][.precision|*][l|L]p.
 *          The following parameter types (p) are supported:
 *          - <b>x</b> hexadecimal integer.
 *          - <b>X</b> hexadecimal long.
 *          - <b>o</b> octal integer.
 *          - <b>O</b> octal long.
 *          - <b>d</b> decimal signed integer.
 *          - <b>D</b> decimal signed long.
 *          - <b>u</b> decimal unsigned integer.
 *          - <b>U</b> decimal unsigned long.
 *          - <b>c</b> character.
 *          - <b>s</b> string.
 *          .
 *
 * @param[in] chp       pointer to a @p BaseSequentialStream implementing object
 * @param[in] fmt       formatting string
 */


#if !CHPRINTF_USE_STDLIB
static void _chvsnprintf(char *buffer, BaseSequentialStream *chp, size_t size, const char *fmt, va_list ap) {
  char *p, *s, c, filler;
  int i, precision, width;
  bool is_long, left_align, plus_on_float;
  long l;
#if CHPRINTF_USE_FLOAT
  int fprec=1000000;
  double d;
  char tmpbuf[2*MAX_FILLER + 1];
#else
  char tmpbuf[MAX_FILLER + 1];
#endif

  // return TRUE if space exhausted
  bool _putChar (const char _c)  {
    if (buffer != NULL) {
      if (size) {
	*buffer = _c;
	buffer++;
	return (--size == 0);
      } else {
	return TRUE;
      }
    } else if (chp != NULL) {
#if (CH_KERNEL_MAJOR <= 3)
      chSequentialStreamPut(chp, _c);
#else
      streamPut (chp, _c);
#endif
      return FALSE;
    }
    return FALSE;
  }



  
  while (TRUE) {
    c = *fmt++;
    if (c == 0) {
      // only add end of string marker when filling buffer, not when outputing on I/O channel
      if (buffer != NULL)
	_putChar (0);
      return;
    }
    if (c != '%') {
      if (_putChar (c)) return;
      continue;
    }
    p = tmpbuf;
    s = tmpbuf;
    left_align = plus_on_float =  FALSE;
    if (*fmt == '-') {
      fmt++;
      left_align = TRUE;
    } else if (*fmt == '+') {
      fmt++;
      plus_on_float = TRUE;
    }
    filler = ' ';
    if (*fmt == '.') {
      fmt++;
      filler = '0';
#if CHPRINTF_USE_FLOAT
      fprec = intPow (10, (*fmt)-'0');
#endif
    }
    width = 0;
    while (TRUE) {
      c = *fmt++;
      if (c >= '0' && c <= '9')
        c -= '0';
      else if (c == '*')
        c = va_arg(ap, int);
      else
        break;
      width = width * 10 + c;
    }
    precision = 0;
    if (c == '.') {
      while (TRUE) {
        c = *fmt++;
        if (c >= '0' && c <= '9') {
          c -= '0';
#if CHPRINTF_USE_FLOAT
	  fprec = intPow (10, c);
#endif
        } else if (c == '*')
          c = va_arg(ap, int);
        else
          break;
        precision *= 10;
        precision += c;
      }
    }
    /* Long modifier.*/
    if (c == 'l' || c == 'L') {
      is_long = TRUE;
      if (*fmt)
        c = *fmt++;
    }
    else
      is_long = (c >= 'A') && (c <= 'Z');

    /* Command decoding.*/
    switch (c) { 
    case 'n': 
      *(va_arg(ap, int*)) = p-buffer;
      break;
    case 'c':
      filler = ' ';
      *p++ = va_arg(ap, int);
      break;
    case 's':
      filler = ' ';
      if ((s = va_arg(ap, char *)) == 0)
        s = "(null)";
      if (precision == 0)
        precision = 32767;
      for (p = s; *p && (--precision >= 0); p++)
        ;
      break;
    case 'D':
    case 'd':
    case 'I':
    case 'i':
      if (is_long)
        l = va_arg(ap, long);
      else
        l = va_arg(ap, int);
      if (l < 0) {
        *p++ = '-';
        l = -l;
      }
      p = ltoa(p, l, 10);
      break;
#if CHPRINTF_USE_FLOAT
    case 'e':
    case 'f':
    case 'g':
    case 'E':
    case 'F':
    case 'G':
      d = (double) va_arg(ap, double);
      if (d < 0) {
        *p++ = '-';
        d = -d;
      } else if (plus_on_float) {
	*p++ = '+';
      }
      p = ftoa(p, d, fprec);
      break;
#endif
    case 'p':
    case 'P':
      _putChar ('0');
      _putChar (islower (c) ? 'x' : 'X');
    case 'X':
    case 'x':
      c = 16;
      goto unsigned_common;
    case 'U':
    case 'u':
      c = 10;
      goto unsigned_common;
    case 'O':
    case 'o':
      c = 8;
unsigned_common:
      if (is_long)
        l = va_arg(ap, long);
      else
        l = va_arg(ap, int);
      p = ltoa(p, l, c);
      break;
    default:
      *p++ = c;
      break;
    }
    i = (int)(p - s);
    if ((width -= i) < 0)
      width = 0;
    if (left_align == FALSE)
      width = -width;
    if (width < 0) {
      if (*s == '-' && filler == '0') {
	if (_putChar ( (uint8_t)*s++)) return;
        i--;
      }
      do
        if (_putChar (  (uint8_t)filler)) return;
      while (++width != 0);
    }
    while (--i >= 0)
      if (_putChar ( (uint8_t)*s++)) return;

    while (width) {
      if (_putChar (  (uint8_t)filler)) return;
      width--;
    }
  }
  _putChar (0) ;
}
#endif

void directchvprintf(BaseSequentialStream *chp, const char *fmt, va_list ap) {
#if CHPRINTF_USE_STDLIB
  uint8_t buffer[CHPRINTF_BUFFER_SIZE];
  const uint32_t len = vsnprintf ((char *) buffer, CHPRINTF_BUFFER_SIZE, fmt, ap);
  streamWrite (chp, buffer, len);
#else
  _chvsnprintf(NULL, chp, 0, fmt, ap);
#endif
}

void chvsnprintf(char *buffer, size_t size, const char *fmt, va_list ap) {
#if CHPRINTF_USE_STDLIB
  vsnprintf (buffer, size, fmt, ap);
#else
  _chvsnprintf(buffer, NULL, size, fmt, ap);
#endif
}

void chsnprintf(char *buffer, size_t size, const char *fmt, ...) 
{
  va_list ap;
  
  va_start(ap, fmt);
#if CHPRINTF_USE_STDLIB
  vsnprintf (buffer, size, fmt, ap);
#else
  _chvsnprintf(buffer, NULL, size, fmt, ap);
#endif
  va_end(ap);
}

void directchprintf(BaseSequentialStream *chp, const char *fmt, ...) 
{
  va_list ap;

  va_start(ap, fmt);
#if CHPRINTF_USE_STDLIB
  uint8_t buffer[CHPRINTF_BUFFER_SIZE];
  const uint32_t len = vsnprintf ((char *) buffer, CHPRINTF_BUFFER_SIZE, fmt, ap);
  streamWrite (chp, buffer, len);
#else
  _chvsnprintf(NULL, chp, 0, fmt, ap);
#endif

  va_end(ap);
}

void chprintf(BaseSequentialStream *lchp, const char *fmt, ...) 
{
  chMtxLock (&printThreadMutex); {

    va_list ap;
    
    if (printThreadPtr == NULL)
      printThreadPtr = chThdCreateStatic(waSerialPrint, sizeof(waSerialPrint), NORMALPRIO+1, serialPrint, NULL);
    
    va_start(ap, fmt);
    synchronous_print_arg_t spat = {.chp = lchp,
				    .destBuf = NULL,
				    .fmt = fmt,
				    .ap = ap};
    
    chMsgSend (printThreadPtr, (msg_t) &spat);
    va_end(ap);
  }
  chMtxUnlock (&printThreadMutex);
}

void chvprintf(BaseSequentialStream *lchp, const char *fmt, va_list ap)
{
  if (printThreadPtr == NULL)
    printThreadPtr = chThdCreateStatic(waSerialPrint, sizeof(waSerialPrint), NORMALPRIO+1, serialPrint, NULL);
  
  synchronous_print_arg_t spat = {.chp = lchp,
				  .destBuf = NULL,
				  .fmt = fmt,
				  .ap = ap};
  
  chMsgSend (printThreadPtr, (msg_t) &spat);
}



void smchsnprintf(char *buffer, size_t size, const char *fmt, ...)
{
  va_list ap;
  
  if (printThreadPtr == NULL)
    printThreadPtr = chThdCreateStatic(waSerialPrint, sizeof(waSerialPrint), NORMALPRIO+1, serialPrint, NULL);

  va_start(ap, fmt);
  synchronous_print_arg_t spat = {.size = size,
				  .destBuf = buffer,
				  .fmt = fmt,
				  .ap = ap};
  
  chMsgSend (printThreadPtr, (msg_t) &spat);
  
  va_end(ap);
}
  

void smchvsnprintf(char *buffer, size_t size, const char *fmt, va_list ap)
{
  if (printThreadPtr == NULL)
    printThreadPtr = chThdCreateStatic(waSerialPrint, sizeof(waSerialPrint), NORMALPRIO+1, serialPrint, NULL);
  
  synchronous_print_arg_t spat = {.size = size,
				  .destBuf = buffer,
				  .fmt = fmt,
				  .ap = ap};
  
  chMsgSend (printThreadPtr, (msg_t) &spat);
}


/** @} */
