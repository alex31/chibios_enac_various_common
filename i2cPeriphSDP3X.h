#pragma once

#include "ch.h"
#include "hal.h"
#include "stdutil.h"

/*
  TODO : clean ident code after tests
 */

/**                                                               
 * @name    Scale factor on raw values to get
 * @brief   this value is given by sensor in scale field
 * @{
 */
#define  SPD31_SCALE_60  (60.0f) 
#define  SPD32_SCALE_240 (240.0f)
#define  SPD33_SCALE_20  (20.0f)
#define  SPD3X_TEMP_SCALE (200.0f) /**< @brief All sensors use same temperature scale */
#define  SPD3X_WAIT_AFTER_RESET_MS (20U) /**< @brief Time to wait between general reset and first transaction */
/** @} */

/**
 * @name    kind of operation permited
 * @brief   parameter used by start, restart, and fetch operation
 * @note    start, restart allways fetch all parameters : pressure, temp and scale
 *          about compensation, see 
 * https://www.sensirion.com/en/about-us/newsroom/sensirion-specialist-articles/efficient-gas-flow-measurements-in-bypass/
 * https://www.sensirion.com/fileadmin/user_upload/customers/sensirion/Dokumente/14_Specialist_Articles/Sensirion_Specialist_Article_Gas_Flow_Bypass_EN.pdf
 * @{
 */
typedef enum {
  SPD3X_none,				/**< @brief used by sdp3xStart if one shot operation are used later  */
  SPD3X_massflow,               	/**< @brief massflow compensated, continuous fetch only pressure     */
  SPD3X_massflow_temp,			/**< @brief massflow compensated, continuous fetch pressure and temp */
  SPD3X_massflow_temp_oneshot,		/**< @brief massflow compensated, one shot fetch all  */
  SPD3X_pressure,			/**< @brief temp compensated, continuous fetch only pressure     */
  SPD3X_pressure_temp,			/**< @brief temp compensated, continuous fetch pressure and temp */
  SPD3X_pressure_temp_scale_oneshot	/**< @brief temp compensated, one shot fetch all   */
}  Spd3xRequest;
/** @} */



/**
 * @name    I2C slave number for regular transation and general reset
 * @brief   SDP3x can be configured to have one of three I²C slave number
 * @{
 */
typedef enum __attribute__ ((__packed__)) {
  SPD3X_GENERAL_RESET_ADDRESS =0x0,
    SPD3X_ADDRESS1 =0x21, /**< @brief  ADDR Pin connected to GND		*/
    SPD3X_ADDRESS2,	  /**< @brief  ADDR Pin connected with 1.2 KOhm to GND 	*/	 
    SPD3X_ADDRESS3	  /**< @brief  ADDR Pin connected with 2.7 KOhm to GND	*/  
    } Spd3xAddress;
/** @} */

/**
 * @name    I2C command for general reset
 * @{
 */
typedef enum __attribute__ ((__packed__)) {
  SPD3X_GENERAL_RESET_COMMAND =0x06,   /**< @brief   reset all SDP3x devices on a given I²C bus */

    } Spd3xGeneralCommand;
/** @} */


/**
 * @name    I2C command for regular transation
 * @brief   can be used directly by sdp3xSend function for advanced use
 * @brief   read datasheet for details
 * @{
 */
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
/** @} */

/**
 * @name    structure containing identification data : part number and serial number
 * @brief   read datasheet for details
 * @{
 */
typedef struct {
  uint64_t sn; /**< @brief  SDP31:0x3010101, SDP32:0x03010201, SDP33:0x03010301 */
  uint32_t pn;
} Spd3xIdent;
/** @} */

/**
 * @name    handler of driver
 * @{
 */
typedef struct Spd3xDriver Spd3xDriver; /**< @brief  opaque type */
/** @} */

/**
 * @brief   reset all SDP3X sensors on the given bus
 * @details must be called at power on, or restart, but not on normal operation
 *          after such a reset, one must wait at least SPD3X_WAIT_AFTER_RESET_MS
 *          before attempting an I²C transaction
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 *
 * @return              The operation status.
 * @retval MSG_OK       if the function succeeded.
 * @retval MSG_RESET    if one or more I2C errors occurred, the errors can
 *                      be retrieved using @p i2cGetErrors().
 * @retval MSG_TIMEOUT  if a timeout occurred before operation end.
 *
 * @api
 */
msg_t  sdp3xGeneralReset(I2CDriver *i2cp);



/**
 * @brief   initialise object
 *
 * @param[in] sdpp      pointer to the @p uninitialized Spd3xDriver object
 * @param[in] i2cp      pointer to the @p I2CDriver object
 * @param[in] addr      I²C slave address (7 bits)
 *
 *
 * @api
 */
void  sdp3xStart(Spd3xDriver *sdpp, I2CDriver *i2cp,
		 const Spd3xAddress addr);



/**
 * @brief   send command and fetch data.
 * @details there several use case : ° launch again continuous sampling after a sdp3xStop,
 *				     ° start one shot measure,
 *				     ° start continuous sampling after use of sdp3xGetIdent :
 *				       start(..., SPD3X_none); getIdent(...), restart(..., one continuous mode)
 *
 * @param[in] sdpp      pointer to the @p initialized Spd3xDriver object
 * @param[in] request   request : type of request, continuous or one shot,
 *			massflow or temperature compensated
 *
 * @return              The operation status.
 * @retval MSG_OK       if the function succeeded.
 * @retval MSG_RESET    if one or more I2C errors occurred, the errors can
 *                      be retrieved using @p i2cGetErrors().
 * @retval MSG_TIMEOUT  if a timeout occurred before operation end.
 *
 * @api
 */
msg_t  sdp3xRequest(Spd3xDriver *sdpp, const Spd3xRequest request);



/**
 * @brief   stop a previously launched continuous operation
 *
 * @param[in] sdpp      pointer to the @p initialized Spd3xDriver object
 *
 * @return              The operation status.
 * @retval MSG_OK       if the function succeeded.
 * @retval MSG_RESET    if one or more I2C errors occurred, the errors can
 *                      be retrieved using @p i2cGetErrors().
 * @retval MSG_TIMEOUT  if a timeout occurred before operation end.
 *
 * @api
 */
msg_t  sdp3xStop(Spd3xDriver *sdpp);


/**
 * @brief   put device in sleep mode
 * @details continuous operations, if any, must be stopped before entering sleep mode
 *          triggered operation, if any, must be terminated before entering sleep mode
 *
 * @param[in] sdpp      pointer to the @p initialized Spd3xDriver object
 *
 * @return              The operation status.
 * @retval MSG_OK       if the function succeeded.
 * @retval MSG_RESET    if one or more I2C errors occurred, the errors can
 *                      be retrieved using @p i2cGetErrors().
 * @retval MSG_TIMEOUT  if a timeout occurred before operation end.
 *
 * @api
 */
msg_t  sdp3xSleep(Spd3xDriver *sdpp);


/**
 * @brief	wake up device
 * @details	device must be in sleep mode before waking it
 *
 * @param[in] sdpp      pointer to the @p initialized Spd3xDriver object
 *
 * @return              The operation status.
 * @retval MSG_OK       if the function succeeded.
 * @retval MSG_RESET    if one or more I2C errors occurred, the errors can
 *                      be retrieved using @p i2cGetErrors().
 * @retval MSG_TIMEOUT  if a timeout occurred before operation end.
 *
 * @api
 */
msg_t  sdp3xWakeup(Spd3xDriver *sdpp);



/**
 * @brief   fetch data after a continuous operation has been previously launched with
 *	    start or restart
 *
 * @param[in] sdpp      pointer to the @p initialized Spd3xDriver object
 * @param[in] request   request : length of request, only pressure or pressure and temperature
 *			          fetching only pressure is faster so sampling frequency can be higher
 *
 * @return              The operation status.
 * @retval MSG_OK       if the function succeeded.
 * @retval MSG_RESET    if one or more I2C errors occurred, the errors can
 *                      be retrieved using @p i2cGetErrors().
 * @retval MSG_TIMEOUT  if a timeout occurred before operation end.
 *
 * @api
 */
msg_t  sdp3xFetch(Spd3xDriver *sdpp, const Spd3xRequest request);







/**
 * @brief   ask for part number and serial number
 * @details part number is unique for each sensor, serial number can permit to determine type of sensor
 *	    between sdp31, sdp32, or sdp33 and chip revision. Since scale factor also gives this information	
 *	    but without chip revision.
 *
 * @param[in] sdpp      pointer to the @p initialized Spd3xDriver object
 * @param[in] id        pointer to the @p uninitialized Spd3xIdent object
 *
 *
 * @return              The operation status.
 * @retval MSG_OK       if the function succeeded.
 * @retval MSG_RESET    if one or more I2C errors occurred, the errors can
 *                      be retrieved using @p i2cGetErrors().
 * @retval MSG_TIMEOUT  if a timeout occurred before operation end.
 *
 * @api
 */
msg_t  sdp3xGetIdent(Spd3xDriver *sdpp, Spd3xIdent *id);




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
struct Spd3xDriver {
  I2CDriver    *i2cp;
  Spd3xAddress  slaveAddr;
  float		scale;
  float		pressure;
  float		temp;
} ;

/*
#                                                                                        
#                                                                                        
#                  __ _    ___    ___    ___   ___    ___     ___    _ __   ___          
#                 / _` |  / __|  / __|  / _ \ / __|  / __|   / _ \  | '__| / __|         
#                | (_| | | (__  | (__  |  __/ \__ \  \__ \  | (_) | | |    \__ \         
#                 \__,_|  \___|  \___|  \___| |___/  |___/   \___/  |_|    |___/         
*/


/**
 * @brief   get previously fetched differential pressure
 * @details data has to be previoulsy fetched with start, restart, or fetch function
 *
 * @param[in] sdpp      pointer to the @p initialized Spd3xDriver object
 *
 * @return              differential pressure in pascal
 *
 * @api
 */
static inline float  sdp3xGetPressure(Spd3xDriver *sdpp) {
  return sdpp->pressure;
}

/**
 * @brief   get previously fetched temperature
 * @details data has to be previoulsy fetched with start, restart, or fetch function
 *
 * @param[in] sdpp      pointer to the @p initialized Spd3xDriver object
 *
 * @return              temperature in celcius degree.
 *
 * @api
 */
static inline float  sdp3xGetTemp(Spd3xDriver *sdpp) {
  return sdpp->temp;  
}

/**
 * @brief   get previously fetched differential pressure
 * @details data has to be previoulsy fetched with start or restart
 *
 * @param[in] sdpp      pointer to the @p initialized Spd3xDriver object
 *
 * @return              scale factor, which indicate sensor type 
 *			sdp31:60, sdp32:240, sdp33:20
 *			scale factor is already applied by getPressure function.
 *
 * @api
 */
static inline float  sdp3xGetScale(Spd3xDriver *sdpp) {
  return sdpp->scale;
}


