#pragma once

/*
  TODO : 
 */

typedef enum  {SCD30_OK, SCD30_CRC_ERROR, SCD30_I2C_ERROR} Scd30Status;
typedef struct Scd30Driver Scd30Driver;

void        scd30Start(Scd30Driver *sdpp, I2CDriver *i2cp);
Scd30Status scd30StartContinuous(const Scd30Driver *scdd, uint16_t optPressure);
Scd30Status scd30StopContinuous(const Scd30Driver *scdd);
Scd30Status scd30Reset(const Scd30Driver *scdd);
Scd30Status scd30SetInterval(const Scd30Driver *scdd, const uint16_t interval);
Scd30Status scd30GetInterval(const Scd30Driver *scdd, uint16_t *interval);
Scd30Status scd30FetchData(Scd30Driver *scdd);
Scd30Status scd30SetSelfCalibration(Scd30Driver *scdd, const bool activate);
Scd30Status scd30GetSelfCalibration(Scd30Driver *scdd, bool *activate);
Scd30Status scd30SetForcedCalibration(Scd30Driver *scdd, const uint16_t ppm);
Scd30Status scd30GetForcedCalibration(Scd30Driver *scdd, uint16_t *ppm);
Scd30Status scd30SetTemperatureOffset(Scd30Driver *scdd, const float offset);
Scd30Status scd30GetTemperatureOffset(Scd30Driver *scdd, float *offset);
Scd30Status scd30SetAltitudeCompensation(Scd30Driver *scdd, const uint16_t altitude);
Scd30Status scd30GetAltitudeCompensation(Scd30Driver *scdd, uint16_t *altitude);
Scd30Status scd30GetFwVersion(const Scd30Driver *scdd, uint16_t *version);

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
// Since we avoid dynamically allocated memory in embedded system, we cannot have a factory function returning
// a pointer on malloc area
struct Scd30Driver {
  I2CDriver    *i2cp;
  float		co2;
  float		humidity;
  float		temperature;
};

static inline	    float sdc30GetTemperature(const Scd30Driver *scdd) {return scdd->temperature;};
static inline	    float sdc30GetHumidity(const Scd30Driver *scdd) {return scdd->humidity;};
static inline	    float sdc30GetCo2(const Scd30Driver *scdd) {return scdd->co2;};
