#pragma once
#include "ch.h"
#include "eeprom_conf.h"
/*

  TODO : tester si on depasse la taille du secteur, si oui : 
sauvegarder chaque type en ram, effacer le secteur de flash,
et reflasher les dernieres valeurs pour chaques types

 */

/* public function prototypes -----------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif


typedef enum  {OK_RAM_ADDR_DIFFER =-1, PROG_OK=0,  ERASE_ERR, PROGRAM_ERR,
	       VERIFY_READ_ERROR, SECTOR_FULL_ERR, NOT_FOUND, SIZES_DIFFER, 
	       MALLOC_ERR} ErrorCond;



typedef struct objInEeprom {
  struct objInEeprom *nextObjPtr;
  EepromStructType eepromStructType;
  uint32_t dataLen;
  uint32_t ramAdress;
  uint8_t data[0];
} objInEeprom;

int testFlash(void);

ErrorCond eepromErase (void);
ErrorCond memcopyToFlash (const void *from, const void *flashAddr, 
			 size_t length, bool eraseBefore);
ErrorCond eepromStore (EepromStructType eepromStructType, const void *data, 
		       uint32_t len);
ErrorCond eepromLoad (EepromStructType eepromStructType, void *data, 
		       uint32_t len);
size_t eepromGetLenOfType (EepromStructType eepromStructType);
size_t eepromStatus (void); // return free eeprom space
ErrorCond eepromWipe (void);

#ifdef __cplusplus
}
#endif
