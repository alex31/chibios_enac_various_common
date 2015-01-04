#pragma once

#ifndef KEYPAD_NUM_OF_ROWS
#define KEYPAD_NUM_OF_ROWS 8
#endif

#ifndef KEYPAD_NUM_OF_COLS
#define KEYPAD_NUM_OF_COLS 8
#endif

#define KEYPAD_NO_PRESS 254


#ifdef __cplusplus
extern "C" {
#endif

// store index of gpio instead of address to save memory (3*16 bytes saved) 
typedef enum  __attribute__ ((packed)) {
  Keypad_None=0, Keypad_GpioA, Keypad_GpioB, Keypad_GpioC, 
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


typedef void (*keypadCbType) (Keypad_key key, void *userData);

void launchScanKeypad (keypadCbType keyCb, const Keypad_Def *kd, void *userData);

#ifdef __cplusplus
}
#endif
