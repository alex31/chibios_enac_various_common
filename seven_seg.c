#include <ch.h>
#include <hal.h>
#include "seven_seg.h"

typedef struct  {
  IOBus cdedp;
  IOBus bafg;
} SeptSeg;



typedef union {
  struct {
    uint8_t dis_1:1;
    uint8_t dis_2:1;
    uint8_t dis_3:1;
    uint8_t dis_4:1;
  };
  uint8_t data4:4;
} SeptSegBus;

static  SeptSeg digitBuses = {
  .cdedp = _IOBUS_DATA (1, GPIOB, 4, 12),
  .bafg =  _IOBUS_DATA (2, GPIOE, 4, 2)
};

typedef union {
  struct {
    uint8_t dis_dp:1;
    uint8_t dis_g:1;
    uint8_t dis_f:1;
    uint8_t dis_e:1;
    uint8_t dis_d:1;
    uint8_t dis_c:1;
    uint8_t dis_b:1;
    uint8_t dis_a:1;
  };
  uint8_t data8;
} SeptSegDisplay;



/*
PB12    DIS_C                   
PB13    DIS_D                   
PB14    DIS_E                   
PB15    DIS_DP                  
PE02    DIS_B                   
PE03    DIS_A                   
PE04    DIS_F                   
PE05    DIS_G                   

PE1    DIS_B                   
PE2    DIS_A                   
PE3    DIS_F                   
PE4    DIS_G                   

PB1    DIS_C                   
PB2    DIS_D                   
PB3    DIS_E                   
PB4    DIS_DP                  
*/


static const uint8_t symbolLookUp[] = {
  [0]   = 0b10000000, // snake anim
  [1]   = 0b01000000,
  [2]   = 0b00100000,
  [3]   = 0b00010000,
  [4]   = 0b00001000,
  [5]   = 0b00000100,
  ['0'] = 0b11111100,
  ['1'] = 0b01100000,
  ['2'] = 0b11011010,
  ['3'] = 0b11110010,
  ['4'] = 0b01100110,
  ['5'] = 0b10110110,
  ['6'] = 0b10111110,
  ['7'] = 0b11100000,
  ['8'] = 0b11111110,
  ['9'] = 0b11110110,
  ['A'] = 0b11101110,
  ['B'] = 0b00111010,
  ['C'] = 0b10011100,
  ['D'] = 0b00111010,
  ['E'] = 0b10011110,
  ['F'] = 0b10001110,
  ['H'] = 0b01101110,
  ['I'] = 0b00001100, 
  ['J'] = 0b01101100,
  ['L'] = 0b00011100,
  ['O'] = 0b11111100,
  ['P'] = 0b11001110,
  ['R'] = 0b00001010,
  ['T'] = 0b00011110,
  ['U'] = 0b01111100,
  ['Y'] = 0b01110110
};

static uint32_t errorState = 0;
static unsigned char batteryLevel = '0';

static const uint8_t errorLookUp[] = {
  [SEVSEG_SDCORRUPT]   = 'D',
  [SEVSEG_NOSD]   = 'E',
  [SEVSEG_SDFULL]   = 'F',
  [SEVSEG_IMU]   = 'U',
  [SEVSEG_SPI]   = 'I',
  [SEVSEG_I2C]   = 'J',
  [SEVSEG_GPS]   = 'P',
  [SEVSEG_RC]   = 'R',
  [SEVSEG_TELEMETRY]   = 'T'
};

static void sevseg_display_segments (const uint8_t _disp);

static void sevseg_display_digit (const unsigned char c, const bool dp);


/*
     aaaaa
     f   b
     f   b
     ggggg
     e   c
     e   c
     ddddd h

 */


static void sevseg_display_segments (const uint8_t _disp)
{
  SeptSegBus busB ;
  SeptSegBus busE ;
  const SeptSegDisplay disp = {.data8 =_disp};
  
  busB.dis_1 = disp.dis_c;
  busB.dis_2 = disp.dis_d;
  busB.dis_3 = disp.dis_e;
  busB.dis_4 = disp.dis_dp;

  busE.dis_1 = disp.dis_b;
  busE.dis_2 = disp.dis_a;
  busE.dis_3 = disp.dis_f;
  busE.dis_4 = disp.dis_g;

  palWriteBus (&digitBuses.cdedp, busB.data4);
  palWriteBus (&digitBuses.bafg, busE.data4);
}

static void sevseg_display_digit (const unsigned char c, const bool dp)
{
  if (c < sizeof (symbolLookUp)) {
    sevseg_display_segments (symbolLookUp[c] | dp);
  }
}

static void sevseg_update_digit (void)
{
  const bool batteryDisplay = (errorState == 0);

  // is there more than one error registered ?
  const bool dp = __builtin_popcount (errorState) > 1;

  if (batteryDisplay) {
    sevseg_display_digit (batteryLevel, dp);
  } else {
    // get the most priority error
    const int errIdx = ((sizeof(errorState)*8)-1) - __builtin_clz (errorState);
    if (errIdx <  SEVSEG_END) {
      sevseg_display_digit(errorLookUp[errIdx], dp);
    }
  }
}

void sevseg_register_error (SevSegErrorState err)
{
  if (err < SEVSEG_END) {
    errorState |= 1 << err;
    sevseg_update_digit ();
  }
}

void sevseg_unregister_error (SevSegErrorState err)
{
  if (err < SEVSEG_END) {
    errorState &= ~ (1 << err);
    sevseg_update_digit ();
  }
}

void sevseg_set_battery_level (uint8_t level)
{
  if (level < 10) {
    batteryLevel = '0' + level;
  }
  sevseg_update_digit();
}
