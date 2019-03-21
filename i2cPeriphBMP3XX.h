#pragma once

#include "ch.h"
#include "hal.h"
#include "BMP3XX/bmp3.h"


typedef uint32_t  Bmp3xRequest;

typedef struct {
  uint8_t         slaveAddr;
  struct bmp3_dev bm3dev;
  uint32_t	  settings;
} Bmp3xxConfig;




typedef struct  {
  I2CDriver    *i2cp;
  const         Bmp3xxConfig *config;
  float         pressure;
  float         temp;
} Bmp3xxDriver;



msg_t  bmp3xxStart(Bmp3xxDriver *bmpp, const Bmp3xxConfig config);
msg_t  bmp3xxFetch(Bmp3xxDriver *bmpp, const Bmp3xRequest request);


/*
#                                                                                        
#                                                                                        
#                  __ _    ___    ___    ___   ___    ___     ___    _ __   ___          
#                 / _` |  / __|  / __|  / _ \ / __|  / __|   / _ \  | '__| / __|         
#                | (_| | | (__  | (__  |  __/ \__ \  \__ \  | (_) | | |    \__ \         
#                 \__,_|  \___|  \___|  \___| |___/  |___/   \___/  |_|    |___/         
*/


/**
 * @brief   get previously fetched absolute pressure
 * @details data has to be previoulsy fetched with fetch function
 *
 * @param[in] sdpp      pointer to the @p initialized Bmp3xxDriver object
 *
 * @return              absolute pressure in pascal
 *
 * @api
 */
static inline float  bmp3xxGetPressure(Bmp3xxDriver *bmpp) {
  return bmpp->pressure;
}

/**
 * @brief   get previously fetched temperature
 * @details data has to be previoulsy fetched with start, restart, or fetch function
 *
 * @param[in] bmpp      pointer to the @p initialized Bmp3xxDriver object
 *
 * @return              temperature in celcius degree.
 *
 * @api
 */
static inline float  bmp3xxGetTemp(Bmp3xxDriver *bmpp) {
  return bmpp->temp;  
}
