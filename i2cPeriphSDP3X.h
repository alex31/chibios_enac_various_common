#pragma once

#include "ch.h"
#include "hal.h"
#include "stdutil.h"

/**
 * @name    Scale factor on raw values to get
 * @{
 */
#define  SPD31_SCALE_60  (60.0f)
#define  SPD32_SCALE_240 (240.0f)
#define  SPD33_SCALE_20  (20.0f)
#define  SPD3X_TEMP_SCALE (200.0f)
#define  SPD3X_WAIT_AFTER_RESET_MS (20U)

typedef enum {
  SPD3X_none,
  SPD3X_massflow,
  SPD3X_massflow_temp,
  SPD3X_massflow_temp_oneshot,
  SPD3X_pressure,
  SPD3X_pressure_temp,
  SPD3X_pressure_temp_scale,
  SPD3X_pressure_temp_scale_oneshot
}  Spd3xRequest;

typedef enum __attribute__ ((__packed__)) {
  SPD3X_GENERAL_RESET_ADDRESS =0x0,
    SPD3X_ADDRESS1 =0x21, // ADDR Pin connected to GND
    SPD3X_ADDRESS2,	  // ADDR Pin connected with 1.2 KOhm to GND  
    SPD3X_ADDRESS3	  // ADDR Pin connected with 2.7 KOhm to GND  
    } Spd3xAddress;

typedef enum __attribute__ ((__packed__)) {
  SPD3X_GENERAL_RESET_COMMAND =0x06,
    } Spd3xGeneralCommand;

typedef enum __attribute__ ((__packed__)) {
  SPD3X_NOT_INITIALIZED = 0x0,
    SPD3X_CONTINUOUS_MASSFLOW_AVERAGE = SWAP_ENDIAN16(0x3603),
    SPD3X_CONTINUOUS_MASSFLOW_IMMEDIATE = SWAP_ENDIAN16(0x3608),
    SPD3X_CONTINUOUS_DIFFPRESS_AVERAGE = SWAP_ENDIAN16(0x3615),
    SPD3X_CONTINUOUS_DIFFPRESS_IMMEDIATE = SWAP_ENDIAN16(0x361E),
    SPD3X_STOP_CONTINUOUS = SWAP_ENDIAN16(0x3FF9),
    SPD3X_ONESHOT_MASSFLOW = SWAP_ENDIAN16(0x3726),
    SPD3X_ONESHOT_DIFFPRESS = SWAP_ENDIAN16(0x372D),
    SPD3X_SLEEP = SWAP_ENDIAN16(0x3677),
    SPD3X_READ_PRODUCT_ID1 = SWAP_ENDIAN16(0x367C),
    SPD3X_READ_PRODUCT_ID2 = SWAP_ENDIAN16(0xE102),
    }  Spd3xCommand;

typedef struct {
  uint64_t sn;
  uint32_t pn;
} Spd3xIdent;


typedef struct Spd3xDriver {
  I2CDriver	 *i2cp;
  Spd3xAddress slaveAddr;
  float		scale;
  float		pressure;
  float		temp;
} Spd3xDriver;

msg_t  sdp3xGeneralReset(I2CDriver *i2cp);

msg_t  sdp3xStart(Spd3xDriver *sdpp, I2CDriver *i2cp,
		  const Spd3xAddress addr, const Spd3xRequest request);

msg_t  sdp3xRestart(Spd3xDriver *sdpp, const Spd3xRequest request);

msg_t  sdp3xStop(Spd3xDriver *sdpp);

msg_t  sdp3xCache(Spd3xDriver *sdpp, const Spd3xRequest request);

msg_t sdp3xSend(const Spd3xDriver *sdpp, const Spd3xCommand cmd);

msg_t  sdp3xGetIdent(Spd3xDriver *sdpp, Spd3xIdent *id);

float  sdp3xGetPressure(Spd3xDriver *sdpp);

float  sdp3xGetTemp(Spd3xDriver *sdpp);

float  sdp3xGetScale(Spd3xDriver *sdpp);


