#pragma once

/*
  configuration should be provided in a keypad_conf.h wich will
  define constant as in this example :

#define KEYPAD_GPIO_ROW_SIZE 5

#define KEYPAD_GPIO_COL_SIZE 4

#define KEYPAD_GPIO_ROW LINE_KP_ROW0, LINE_KP_ROW1, LINE_KP_ROW2, LINE_KP_ROW3, LINE_KP_ROW4

#define KEYPAD_GPIO_COL LINE_KP_COL0, LINE_KP_COL1, LINE_KP_COL2, LINE_KP_COL3


#define KEYPAD_SYMBOLS  KP_F1,    KP_F2, KP_HASH,     KP_MULTIPLY, \
			KP_1,     KP_2,  KP_3,        KP_UP, \
			KP_4,     KP_5,  KP_6,        KP_DOWN, \
			KP_7,     KP_8,  KP_9,        KP_ESC, \
                        KP_LEFT,  KP_0,  KP_RIGHT,    KP_ENTER

 */



/*
  TODO : 
  * gestion multi press (notion de modifier)
  * bind à une touche, ou suite de touches
 */

#include "keypad_conf.h"

#ifndef KEYPAD_GPIO_ROW_SIZE
#error KEYPAD_GPIO_ROW_SIZE should be defined in keypad_conf.h
#endif

#ifndef KEYPAD_GPIO_COL_SIZE
#error KEYPAD_GPIO_COL_SIZE should be defined in keypad_conf.h
#endif

#ifndef KEYPAD_SYMBOLS
#error KEYPAD_SYMBOLS should be defined in keypad_conf.h
#endif



#define KEYPAD_NO_PRESS 254


#ifdef __cplusplus
extern "C" {
#endif


  typedef enum {KEYPAD_SYMBOLS} Keypad_Symbol;



typedef struct __attribute__ ((packed)) {
  ioline_t kpRow[KEYPAD_GPIO_ROW_SIZE]; // configure in output for selecting row
  ioline_t kpCol[KEYPAD_GPIO_COL_SIZE]; // configure in input for reading col
} Keypad_Def;

typedef struct {
  uint8_t row;
  uint8_t col;
} Keypad_key;


typedef void (*keypadCbType) (Keypad_Symbol ks, void *userData);

void launchScanKeypad (keypadCbType keyCb, void *userData);

#ifdef __cplusplus
}
#endif
