#pragma once

/*
  TODO : 
 */

typedef enum  {CSS811_OK, CSS811_CRC_ERROR, CSS811_I2C_ERROR} Css811Status;
typedef struct Css811Driver Css811Driver;

void         css811Start(Css811Driver *sdpp, I2CDriver *i2cp);
Css811Status css811StartContinuous(const Css811Driver *scdd, uint16_t optPressure);
Css811Status css811StopContinuous(const Css811Driver *scdd);
Css811Status css811Reset(const Css811Driver *scdd);
 
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
  float		co2;
};

static inline	    float sdc30GetCo2(const Css811Driver *scdd) {return scdd->co2;};
