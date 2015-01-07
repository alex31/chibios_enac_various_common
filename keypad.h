#pragma once

/*
  TODO : 
  * col en output et row en input
  * config dans un rowColKeypadConf.h
  * gestion multi press (notion de modifier)
  * bind à une touche, ou suite de touches
 */

#include "keypad_conf.h"

#ifndef KEYPAD_NUM_OF_ROWS
#error KEYPAD_NUM_OF_ROWS should be defined in keypad_conf.h
#endif

#ifndef KEYPAD_NUM_OF_COLS
#error KEYPAD_NUM_OF_COLS should be defined in keypad_conf.h
#endif

#ifndef KEYPAD_SYMBOLS
#error KEYPAD_SYMBOLS should be defined in keypad_conf.h
#endif



#define KEYPAD_NO_PRESS 254


#ifdef __cplusplus
extern "C" {
#endif


typedef enum KEYPAD_SYMBOLS Keypad_Symbol;


// store index of gpio instead of address to save memory (3*16 bytes saved) 
typedef enum  __attribute__ ((packed)) {
  Keypad_GpioA, Keypad_GpioB, Keypad_GpioC, 
  Keypad_GpioD, Keypad_GpioE,  Keypad_GpioF, Keypad_GpioG, 
  Keypad_GpioH, Keypad_GpioI}  Keypad_Gpio;

typedef struct __attribute__ ((packed)) {
  Keypad_Gpio kpGgpio;
  uint8_t     kpPin;
}  Keypad_Pin;

typedef struct __attribute__ ((packed)) {
  Keypad_Pin kpRow[KEYPAD_NUM_OF_ROWS]; // configure in output for selecting row
  Keypad_Pin kpCol[KEYPAD_NUM_OF_COLS]; // configure in input for reading col
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
