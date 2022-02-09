#pragma once
#include "ch.h"
#include "hal.h"

typedef enum  {IAQCORE_OK=0, IAQCORE_I2C_ERROR=0xFF, IAQCORE_WARMING=0x10,
               IAQCORE_BUSY=0x1, IAQCORE_DEFECTIVE=0x80} IaqCoreStatus;

typedef struct IaqCoreDriver IaqCoreDriver;
typedef union IaqCoreDmaBuffer IaqCoreDmaBuffer;



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

union IaqCoreDmaBuffer {
  struct {
    uint16_t co2_ppm;
    uint8_t  status;
    uint32_t resistance;
    uint16_t tvoc_ppb;
  } __attribute__ ((__packed__));
  uint8_t data8[9];
} __attribute__ ((__packed__));

struct IaqCoreDriver {
  I2CDriver    *i2cp;
  IaqCoreDmaBuffer *dmab;
};


/*
#                 _ __            _       _    _                   ___    _____    _____  
#                | '_ \          | |     | |  (_)                 / _ \  |  __ \  |_   _| 
#                | |_) |  _   _  | |__   | |   _     ___         | |_| | | |__) |   | |   
#                | .__/  | | | | | '_ \  | |  | |   / __|        |  _  | |  ___/    | |   
#                | |     | |_| | | |_) | | |  | |  | (__         | | | | | |       _| |_  
#                |_|      \__,_| |_.__/  |_|  |_|   \___|        |_| |_| |_|      |_____| 
*/
void		 iaqCoreStart(IaqCoreDriver *hmp, I2CDriver *i2cp,
			    IaqCoreDmaBuffer *dmab);
IaqCoreStatus    iaqCoreFetchData(IaqCoreDriver *hmp);


static inline    uint16_t iaqCoreGetCo2(IaqCoreDriver *hmp) {
  return hmp->dmab->co2_ppm;
}
static inline    uint32_t iaqCoreGetResistance(IaqCoreDriver *hmp) {
  return hmp->dmab->resistance;
}
static inline    uint16_t iaqCoreGetTvoc(IaqCoreDriver *hmp) {
  return hmp->dmab->tvoc_ppb;
}

 
