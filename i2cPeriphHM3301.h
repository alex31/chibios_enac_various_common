#pragma once
#include "ch.h"
#include "hal.h"

typedef enum  {HM3301_OK, HM3301_I2C_ERROR, HM3301_CHKSUM_ERROR
} Hm3301Status;

typedef struct Hm3301Driver Hm3301Driver;
typedef union Hm3301DmaBuffer Hm3301DmaBuffer;



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

union Hm3301DmaBuffer {
  struct {
    uint16_t reserved;
    uint16_t sensor_num;
    uint16_t pm1_std;
    uint16_t pm2p5_std;
    uint16_t pm10_std;
    uint16_t pm1_atmo;
    uint16_t pm2p5_atmo;
    uint16_t pm10_atmo;
    uint16_t n_part_0p3;
    uint16_t n_part_0p5;
    uint16_t n_part_1;
    uint16_t n_part_2p5;
    uint16_t n_part_5;
    uint16_t n_part_10;
    uint8_t checksum;
  } __attribute__ ((__packed__));
  uint8_t data8[29];
  uint16_t data16[14];
} __attribute__ ((__packed__));

struct Hm3301Driver {
  I2CDriver    *i2cp;
  Hm3301DmaBuffer *dmab;
};


/*
#                 _ __            _       _    _                   ___    _____    _____  
#                | '_ \          | |     | |  (_)                 / _ \  |  __ \  |_   _| 
#                | |_) |  _   _  | |__   | |   _     ___         | |_| | | |__) |   | |   
#                | .__/  | | | | | '_ \  | |  | |   / __|        |  _  | |  ___/    | |   
#                | |     | |_| | | |_) | | |  | |  | (__         | | | | | |       _| |_  
#                |_|      \__,_| |_.__/  |_|  |_|   \___|        |_| |_| |_|      |_____| 
*/
void		hm3301Start(Hm3301Driver *hmp, I2CDriver *i2cp,
			    Hm3301DmaBuffer *dmab);
Hm3301Status    hm3301StartContinuous(Hm3301Driver *hmp);
Hm3301Status    hm3301FetchData(Hm3301Driver *hmp);

static inline	uint16_t hm3301GetPpm1(const Hm3301Driver *hmp) {return hmp->dmab->pm1_std;}
static inline	uint16_t hm3301GetPpm2p5(const Hm3301Driver *hmp) {return hmp->dmab->pm2p5_std;}
static inline	uint16_t hm3301GetPpm10(const Hm3301Driver *hmp) {return hmp->dmab->pm10_std;}
static inline	uint16_t hm3301GetN0p3(const Hm3301Driver *hmp) {return hmp->dmab->n_part_0p3;}
static inline	uint16_t hm3301GetN0p5(const Hm3301Driver *hmp) {return hmp->dmab->n_part_0p5;}
static inline	uint16_t hm3301GetN1(const Hm3301Driver *hmp) {return hmp->dmab->n_part_1;}
static inline	uint16_t hm3301GetN2p5(const Hm3301Driver *hmp) {return hmp->dmab->n_part_2p5;}
static inline	uint16_t hm3301GetN5(const Hm3301Driver *hmp) {return hmp->dmab->n_part_5;}
static inline	uint16_t hm3301GetN10(const Hm3301Driver *hmp) {return hmp->dmab->n_part_10;}


 
