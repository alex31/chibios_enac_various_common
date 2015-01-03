#pragma once



#if STM32_I2C_USE_I2C1    
extern I2cChannel _I2C_CHANNEL_1 ;
#define I2C_CHANNEL_1 _I2C_CHANNEL_1 
#endif

#if STM32_I2C_USE_I2C2                  
extern I2cChannel _I2C_CHANNEL_2 ;
#define I2C_CHANNEL_2 _I2C_CHANNEL_2 
#endif

#if STM32_I2C_USE_I2C3                  
extern I2cChannel _I2C_CHANNEL_3 ;
#define I2C_CHANNEL_3 _I2C_CHANNEL_3 
#endif
