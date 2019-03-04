#pragma once
#include "ch.h"
#include "eeprom_conf.h"

/* public function prototypes -----------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif


typedef enum  {OK_RAM_ADDR_DIFFER =-1, PROG_OK=0,  ERASE_ERR, PROGRAM_ERR,
	       VERIFY_READ_ERROR, SECTOR_FULL_ERR, NOT_FOUND, SIZES_DIFFER, 
	       MALLOC_ERR} ErrorCond;



/**
 * @brief   copy ram area (record) in eeprom, taking care of wear leveling
 *
 * @param[in] eepromStructType identifiant of the record
 * @param[in] data             address of the ram area to copy (usually a struct variable address)
 * @param[in] len              length of the ram area to copy (usually the struct size)
 * @return              	The operation result.
 * @retval PROG_OK     		 everything is fine
 * @retval ERASE_ERR		not able to ERASE flash
 * @retval PROGRAM_ERR  	not able ro write in flash
 * @retval VERIFY_READ_ERROR verify read after write has failed
 * @retval MALLOC_ERR		copy in ram was not possible due to malloc error
 * @api
 */
ErrorCond eepromStore (const EepromStructType eepromStructType, const void *data, 
		       const uint32_t len);

/**
 * @brief   copy eeprom area (record) in ram
 *
 * @param[in] eepromStructType identifiant of the record
 * @param[in] data             address of the ram area to copy (usually a struct variable address)
 * @param[in] len              length of the ram area to copy (usually the struct size)
 * @return			The operation result.
 * @retval PROG_OK		everything is fine
 * @retval OK_RAM_ADDR_DIFFER	Ok, but warn that the ram address used to read differs 
 *                              from the ram address used to previously store)
 * @retval NOT_FOUND		index of record not found in eeprom
 * @retval SIZES_DIFFER		the size used to store is different from the size used to load
 *	                        for the given eepromStructType identifiant
 * @api
 */
ErrorCond eepromLoad (const EepromStructType eepromStructType, void *data, 
		      const uint32_t len);

/**
 * @brief   get the record length assiciated with an epromStructType identifiant
 *
 * @param[in] eepromStructType identifiant of the record
 * @return    size of the record used from last store for the given eepromStructType identifiant
 * @api
 */
size_t eepromGetLenOfType (const EepromStructType eepromStructType);

/**
 * @brief   get free size of partition eeprom dedidated to data storage
 *
 * @return    free size in bytes
 * @api
 */
size_t eepromStatus (void); // return free eeprom space

/**
 * @brief   completely erase partition eeprom dedidated to data storage
 * @return              	The operation result.
 * @retval PROG_OK     		 everything is fine
 * @retval ERASE_ERR		not able to ERASE flash
 *
 * @api
 */
ErrorCond eepromErase (void);

/**
 * @brief   save currents records in ram, erase eeprom dedidated to data storage, restore
 *          saved record in eeprom
 * @return              	The operation result.
 * @retval PROG_OK     		 everything is fine
 * @retval ERASE_ERR		not able to ERASE flash
 * @retval PROGRAM_ERR  	not able to write in flash
 * @retval VERIFY_READ_ERROR    verify read after write has failed
 * @retval MALLOC_ERR		copy in ram was not possible due to malloc error
 * @api
 */
ErrorCond eepromWipe (void);

/* private or library tuning function prototypes -----------------------------*/

/**
 * @brief   low level test of eeprom read/write
 *          use only to tune the eeprom library
 */
int testFlash(void);




#ifdef __cplusplus
}
#endif
