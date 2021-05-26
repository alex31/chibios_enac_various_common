#pragma once

#include "ch.h"
#include "hal.h"
#include "lps33hw_reg.h"


typedef enum {LPS33HW_FETCH_PRESS=1U<<0, LPS33HW_FETCH_TEMP=1U<<1} LPS33HWFetch;
typedef enum {LPS33HW_I2C_SLAVE_SA0_LOW=LPS33HW_I2C_ADD_L>>1,
	      LPS33HW_I2C_SLAVE_SA0_HIGH=LPS33HW_I2C_ADD_H>>1} LPS33HWI2C_Slave;

typedef struct {
  I2CDriver		*i2cp;
  LPS33HWI2C_Slave	slaveAdr;
  lps33hw_odr_t		odr;
  lps33hw_lpfp_t	lpf;
  bool			blockDataUpdateEnable;
  bool			dataReadyItrEnable;
} LPS33HWConfig;


typedef struct  {
  const LPS33HWConfig     *config;
  stmdev_ctx_t		   dev_ctx;

  float		    pressure_hPa;
  float		    temperature_degC;
} LPS33HWDriver;



msg_t  lps33Start(LPS33HWDriver *lpsp, const LPS33HWConfig *config);
msg_t  lps33Fetch(LPS33HWDriver *lpsp, LPS33HWFetch fetch);


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
 * @param[in] sdpp      pointer to the @p initialized LPS33HWDriver object
 *
 * @return              absolute pressure in hectopascal
 *
 * @api
 */
static inline float  lps33GetPressure(LPS33HWDriver *lpsp) {
  return lpsp->pressure_hPa;
}

/**
 * @brief   get previously fetched temperature
 * @details data has to be previoulsy fetched with start, restart, or fetch function
 *
 * @param[in] lpsp      pointer to the @p initialized LPS33HWDriver object
 *
 * @return              temperature in celcius degree.
 *
 * @api
 */
static inline float  lps33GetTemp(LPS33HWDriver *lpsp) {
  return lpsp->temperature_degC;  
}
