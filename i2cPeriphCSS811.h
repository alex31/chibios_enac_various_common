#pragma once
#include "ch.h"
#include "hal.h"

/*
  TODO : 
 */

/*
  Note: 
  The driver is made for fast measure, it don't care about
  sleep mode, so 
  ° nWake pin is not handled by driver and must be grounded.
  ° nReset must be hold high
  ° nInt must be pulled up
  ° ADDR level is the LSB of I²C address
 */

typedef enum  {CSS811_OK, CSS811_I2C_ERROR,
  CSS811_HEATER_SUPPLY, CSS811_HEATER_FAULT,
  CSS811_MAX_RESISTANCE,  CSS811_MEASMODE_INVALID,
  CSS811_READ_REG_INVALID,  CSS811_WRITE_REG_INVALID,
  CSS811_RESET_FAILED, CSS811_ERASE_FAILED,
  CSS811_FWLOAD_FAILED
} Css811Status;

typedef enum  {CSS811_FIRMWARE_RESET, CSS811_FIRMWARE_ERASE,
  CSS811_FIRMWARE_LOAD, CSS811_FIRMWARE_VERIFY
} Css811CurrentOperation;

typedef struct Css811Driver Css811Driver;

typedef enum __attribute__ ((__packed__)) {
  CSS811_MEASMODE_DISABLE = 0U << 4,
  CSS811_MEASMODE_EVERY_SECOND = 0b001U << 4,
  CSS811_MEASMODE_EVERY_10_SECONDS = 0b010U << 4,
  CSS811_MEASMODE_EVERY_60_SECONDS = 0b011U << 4,
  CSS811_MEASMODE_EVERY_250_MILLISECOND = 0b100U << 4,
  CSS811_MEASMODE_INT_DATARDY = 0b1 << 3
} Css811MeasMode;

typedef struct  {
  uint8_t hw_id;
  uint8_t version_major:4;
  uint8_t version_minor:4;
  uint8_t boot_major:4;
  uint8_t boot_minor:4;
  uint8_t boot_trivial;
  uint8_t app_major:4;
  uint8_t app_minor:4;
  uint8_t app_trivial;
} Css811Version;

typedef struct {
  uint8_t tx[9]; // 9 for firmware flash : regaddr + chunck of 8 bytes
  uint8_t rx[8];
} Css811DmaBuffer;

typedef void (*FirmwareUploadCB) (Css811Driver *cssp,
				  Css811CurrentOperation currOpt,
				  const size_t nbBytesWritten);

void         css811Start(Css811Driver *cssp, I2CDriver *i2cp,
			 Css811DmaBuffer *dmab, ioline_t nInt,
			 uint8_t addressPinLevel);

Css811Status css811StartContinuous(const Css811Driver *cssp,
				   Css811MeasMode mode);
Css811Status css811StopContinuous(const Css811Driver *cssp);
Css811Status css811FetchData(Css811Driver *cssp);
Css811Status css811GetBaseline(Css811Driver *cssp, uint16_t *baseLine);
Css811Status css811GetVersions(Css811Driver *cssp, Css811Version *versions);
Css811Status css811SetBaseline(Css811Driver *cssp, uint16_t baseLine);
Css811Status css811SetEnvironment(Css811Driver *cssp,
				  float humidity, float temperature);
Css811Status css811FlashFirmware(Css811Driver *cssp, const uint8_t *firmware,
				 size_t firmSize, FirmwareUploadCB onGoingCb);

Css811Status css811Reset(const Css811Driver *cssp);
 
/*
#                 _ __           _                    _                   
#                | '_ \         (_)                  | |                  
#                | |_) |  _ __   _   __   __   __ _  | |_     ___         
#                | .__/  | '__| | |  \ \ / /  / _` | | __|   / _ \        
#                | |     | |    | |   \ V /  | (_| | \ |_   |  __/        
#                |_|     |_|    |_|    \_/    \__,_|  \__|   \___|        
#                         _ __             __ _                        
#                        | '_ \           / _` |                       
#                  ___   | |_) |   __ _  | (_| |  _   _    ___         
#                 / _ \  | .__/   / _` |  \__, | | | | |  / _ \        
#                | (_) | | |     | (_| |     | | | |_| | |  __/        
#                 \___/  |_|      \__,_|     |_|  \__,_|  \___|        
#                 _      _   _    _ __                 
#                | |    | | | |  | '_ \                
#                | |_   | |_| |  | |_) |   ___         
#                | __|   \__, |  | .__/   / _ \        
#                \ |_     __/ |  | |     |  __/        
#                 \__|   |___/   |_|      \___|        
*/

// Cannot be hidden in C file since size must be known by source file using API.
// Since we avoid dynamically allocated memory in embedded system,
// we cannot have a factory function returning
// a pointer on malloc area
struct Css811Driver {
  I2CDriver    *i2cp;
  Css811DmaBuffer *dmab;
  ioline_t	nInt;
  float resistance;
  uint16_t	co2;
  uint16_t	tvoc;
  struct {
    uint8_t  current:6;
    uint16_t adc:10;
  } raw;
  uint8_t	i2cAdd;
};

static inline	    uint16_t css811GetCo2(const Css811Driver *cssp) {return cssp->co2;}
static inline	    uint16_t css811GetTvoc(const Css811Driver *cssp) {return cssp->tvoc;}
static inline	    float css811GetResistance(const Css811Driver *cssp) {return cssp->resistance;}

