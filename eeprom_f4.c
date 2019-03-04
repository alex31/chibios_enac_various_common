#include "eeprom.h"
#include "stm32f4xx_flash.h"

//#define TRACE_EEPROM 1

#include <string.h>
#include "ch.h"
#include "hal.h"
#include "stdutil.h"

#ifdef TRACE_EEPROM
#include "printf.h"
#include "globalVar.h"
#endif


/* Private typedef -----------------------------------------------------------*/
typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;

/* Private define ------------------------------------------------------------*/

#define DATA_32                 ((uint32_t)0x12345678)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static uint32_t GetSector(const uint32_t Address);

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

int testFlash(void)
{
  uint32_t StartSector = 0, EndSector = 0;
  uint32_t Address = 0;
  volatile uint32_t data32 = 0 , MemoryProgramStatus = 0 ;

  /* Unlock the Flash to enable the flash control register access *************/ 
  FLASH_Unlock();
    
  /* Erase the user Flash area
    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

  /* Clear pending flags (if any) */  
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR); 

  /* Get the number of the start and end sectors */
  StartSector = GetSector(FLASH_USER_START_ADDR);
  EndSector = GetSector(FLASH_USER_END_ADDR);

  
  uint32_t sector = StartSector;
  do
  {
    /* Device voltage range supposed to be [2.7V to 3.6V], the operation will
       be done by word */ 
    if (FLASH_EraseSector(sector, VoltageRange_3) != FLASH_COMPLETE)
    { 
      /* Error occurred while sector erase. 
         User can add here some code to deal with this error  */
        FLASH_Lock(); 
	return (ERASE_ERR);
    } else {
      sector+=8;
    }
  } while (sector< EndSector);

  /* Program the user Flash area word by word
    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

  Address = FLASH_USER_START_ADDR;

  while (Address < FLASH_USER_END_ADDR)
  {
    if (FLASH_ProgramWord(Address, DATA_32) == FLASH_COMPLETE)
    {
      Address += 4;
    }
    else
    { 
      FLASH_Lock(); 
      return (PROGRAM_ERR);
    }
  }

  /* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) *********/
  FLASH_Lock(); 

  /* Check if the programmed data is OK 
      MemoryProgramStatus = 0: data programmed correctly
      MemoryProgramStatus != 0: number of words not programmed correctly ******/
  Address = FLASH_USER_START_ADDR;
  MemoryProgramStatus = 0x0;
  
  while (Address < FLASH_USER_END_ADDR)
  {
    data32 = *(__IO uint32_t*)Address;

    if (data32 != DATA_32)
    {
      MemoryProgramStatus++;  
    }

    Address += 4;
  }  
 
  return (MemoryProgramStatus);
}



ErrorCond eepromErase (void)
{
  const uint32_t flashStartAddr = FLASH_USER_START_ADDR;
  const uint32_t flashEndAddr   = FLASH_USER_END_ADDR;

  const uint32_t StartSector = GetSector(flashStartAddr);
  const uint32_t EndSector   = GetSector(flashEndAddr);
  uint32_t sector = StartSector;

  
  /* Unlock the Flash to enable the flash control register access *************/ 
  FLASH_Unlock();
    
  /* Erase the user Flash area
    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

  /* Clear pending flags (if any) */  
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR); 

  do
    {
      /* Device voltage range supposed to be [2.7V to 3.6V], the operation will
	 be done by word */ 
      if (FLASH_EraseSector(sector, VoltageRange_3) != FLASH_COMPLETE)
	{ 
	  /* Error occurred while sector erase. 
	     User can add here some code to deal with this error  */
	  FLASH_Lock(); 
	  return (ERASE_ERR);
	} else {
	sector+=8;
      }
    } while (sector< EndSector);

  FLASH_Lock(); 
  return (PROG_OK);
}

ErrorCond memcopyToFlash(const void *from, const void *flashAddr, 
			 size_t length, bool eraseBefore)
{
  const uint32_t flashStartAddr = (flashAddr != NULL) ?  (const uint32_t) flashAddr 
						      :	 FLASH_USER_START_ADDR;
  const size_t alignOffset = (4-(length%4)) % 4;
  const uint32_t flashEndAddr = flashStartAddr + length + alignOffset;
  const uint32_t ramBeginAddr = (uint32_t) from;
  const uint32_t ramEndAddr = ramBeginAddr+length;
  volatile uint32_t data32 = 0 ;


  uint32_t StartSector = 0, EndSector = 0;

  if (flashEndAddr >=   FLASH_USER_END_ADDR)
    return (SECTOR_FULL_ERR);

  /* Unlock the Flash to enable the flash control register access *************/ 
  FLASH_Unlock();
#ifdef TRACE_EEPROM
  DebugTrace ("FLASH UNlock ;; len=%d alignOffset=%d", length, alignOffset);
#endif
  /* Erase the user Flash area
    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

  /* Clear pending flags (if any) */  
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR); 

  if (eraseBefore) {

    /* Get the number of the start and end sectors */
    StartSector = GetSector(flashStartAddr);
    EndSector = GetSector(flashEndAddr);
    
#ifdef TRACE_EEPROM
    DebugTrace ("startsector=%d, endsector=%d\n", StartSector, EndSector);
#endif
    
  
    uint32_t sector = StartSector;
    do
      {
	/* Device voltage range supposed to be [2.7V to 3.6V], the operation will
	   be done by word */ 
	if (FLASH_EraseSector(sector, VoltageRange_3) != FLASH_COMPLETE)
	  { 
	    /* Error occurred while sector erase. 
	       User can add here some code to deal with this error  */
	    FLASH_Lock(); 
	    return (ERASE_ERR);
	  } else {
	  sector+=8;
	}
      } while (sector< EndSector);
    
    /* Program the user Flash area word by word
       (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/
  }

  uint32_t flashPtr = flashStartAddr;
  uint32_t ramPtr = ramBeginAddr;

  while (flashPtr < flashEndAddr) {
    // if we are on a non aligned length, we should program last bytes
    if ((ramEndAddr - ramPtr) < 4) {
      uint32_t pos=0, lastword=0;
      char *rptr =  (char *) ramPtr;
      while (rptr < (char *) ramEndAddr) {
	lastword |= (((*rptr) << pos) & (0xff << pos));
	rptr++;
	pos+=8;
      }
#ifdef TRACE_EEPROM
      DebugTrace ("DBG> flash *LAST incomplete* value 0x%x @ 0x%x", lastword, flashPtr);
#endif
      if (FLASH_ProgramWord (flashPtr, lastword) != FLASH_COMPLETE) {
	  FLASH_Lock(); 
	  return (PROGRAM_ERR);
      } else {
	flashPtr += 4;
      }
    } else {
#ifdef TRACE_EEPROM
      DebugTrace ("DBG> flash value 0x%x @ 0x%x", *((uint32_t *) ramPtr), 
		flashPtr);
#endif
      if (FLASH_ProgramWord (flashPtr, *((uint32_t *) ramPtr) )== FLASH_COMPLETE)  {
	flashPtr += 4;
	ramPtr += 4;
      } else { 
	FLASH_Lock(); 
	return (PROGRAM_ERR);
      }
    }
  }
  /* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) *********/
  FLASH_Lock(); 
#ifdef TRACE_EEPROM
  DebugTrace ("FLASH LOCK");
#endif

  /* Check if the programmed data is OK 
      MemoryProgramStatus = 0: data programmed correctly
      MemoryProgramStatus != 0: number of words not programmed correctly ******/
 
  flashPtr = flashStartAddr;
  ramPtr = ramBeginAddr;

  while (flashPtr < flashEndAddr) {
#ifdef TRACE_EEPROM
    DebugTrace ("verify address 0x%x", flashPtr);
    chThdSleepMilliseconds(100);
#endif
    data32 = *(__IO uint32_t*) flashPtr;
    // if we are on a non aligned length, we should test last bytes
    if ((ramEndAddr - ramPtr) < 4) {
      uint32_t pos=0, lastword=0;
      char *rptr =  (char *) ramPtr;
      while (rptr < (char *) ramEndAddr) {
	lastword |= (((*rptr) << pos) & (0xff << pos));
	pos+=8;
	rptr++;
      }
      flashPtr += 4;
      if (data32 !=lastword) {
	return VERIFY_READ_ERROR;
      }
    } else {
      if (data32 != *((uint32_t *) ramPtr) ) {
	return VERIFY_READ_ERROR;
      }
      flashPtr += 4;
      ramPtr += 4;
    }
  }

  return PROG_OK;
}




ErrorCond eepromStore (const EepromStructType eepromStructType, const void *data,
		       const uint32_t len)
{
  const  objInEeprom *flashStartAddr = (objInEeprom *) FLASH_USER_START_ADDR;
  objInEeprom *currentObjInFlash = (objInEeprom *) flashStartAddr;
  objInEeprom objToFlash;
  ErrorCond  retval = PROG_OK;

#ifdef TRACE_EEPROM
    DebugTrace ("before loop currentObjInFlash = 0x%x", currentObjInFlash);
    chThdSleepMilliseconds(100);
#endif

    if (((int) eepromStatus() - (int) len) < 8)  {
#ifdef TRACE_EEPROM
      DebugTrace ("eeprom full, has to wipe (remove obsolete data) before writing");
      chThdSleepMilliseconds(100);
#endif
      eepromWipe(); 
    }

    while ((uint32_t) currentObjInFlash->nextObjPtr != 0xffffffff) {
      currentObjInFlash = currentObjInFlash->nextObjPtr;
#ifdef TRACE_EEPROM
      DebugTrace ("IN loop currentObjInFlash = 0x%x", currentObjInFlash);
      chThdSleepMilliseconds(100);
#endif
    }
    
#ifdef TRACE_EEPROM
    DebugTrace ("currentObjInFlash = 0x%x", currentObjInFlash);
    chThdSleepMilliseconds(100);
#endif
    
  // now currentObjInFlash is the first free address in flash
  objToFlash.eepromStructType = eepromStructType;
  const uint32_t ptrCurr = (uint32_t) currentObjInFlash;
  const uint32_t alignOffset = (4-(len%4)) % 4;
  const uint32_t ptrNext = ptrCurr+len+alignOffset+sizeof(objToFlash);
  const void * ptrDataInFlash = (void *) (ptrCurr + sizeof (objToFlash));
  objToFlash.nextObjPtr = (objInEeprom *) ptrNext;
  objToFlash.dataLen = len;
  objToFlash.ramAdress = (uint32_t) data;

  #ifdef TRACE_EEPROM
    DebugTrace (" objToFlash.nextObjPtr = 0x%x",  objToFlash.nextObjPtr);
    chThdSleepMilliseconds(100);
#endif


  retval = memcopyToFlash (&objToFlash, currentObjInFlash, sizeof (objToFlash), FALSE);
  if (retval != PROG_OK) {
    return retval;
  }

  return memcopyToFlash (data, ptrDataInFlash, len, FALSE);
}


ErrorCond eepromLoad (const EepromStructType eepromStructType, void *data, 
		      const uint32_t len)
{
  const  objInEeprom *flashStartAddr = (objInEeprom *) FLASH_USER_START_ADDR;
  objInEeprom *currentObjInFlash = (objInEeprom *) flashStartAddr;
  objInEeprom *lastTypeObject = currentObjInFlash->eepromStructType == eepromStructType ? 
								currentObjInFlash : NULL;
  ErrorCond retVal = PROG_OK;
  
#ifdef TRACE_EEPROM
    DebugTrace ("before loop currentObjInFlash = 0x%x", currentObjInFlash);
    chThdSleepMilliseconds(100);
#endif


    while ((uint32_t) currentObjInFlash->nextObjPtr !=  0xffffffff) {
    currentObjInFlash = currentObjInFlash->nextObjPtr;
#ifdef TRACE_EEPROM
    DebugTrace ("IN loop currentObjInFlash = 0x%x", currentObjInFlash);
    chThdSleepMilliseconds(100);
#endif


    if (currentObjInFlash->eepromStructType == eepromStructType) {
      lastTypeObject=currentObjInFlash;
    }
  }

  if (lastTypeObject == NULL)  {
    return NOT_FOUND;
  }

  if (len != lastTypeObject->dataLen) {
    return SIZES_DIFFER;
  }

  if ((uint32_t) data != lastTypeObject->ramAdress) {
    retVal = OK_RAM_ADDR_DIFFER;
  }



  memcpy (data, lastTypeObject->data, lastTypeObject->dataLen);
#ifdef TRACE_EEPROM
  DebugTrace ("memcopy 0x%x, 0x%x, %d", 
	    data, lastTypeObject->data, lastTypeObject->dataLen);
  chThdSleepMilliseconds(100);
#endif

  return retVal;
}

typedef struct {
  uint32_t len;
  void *dataPtr;
} EepromStructTypeInfo;


ErrorCond eepromWipe (void)
{
  EepromStructTypeInfo stInfo[EEPROM_STRUCT_LAST_ITEM];
  ErrorCond retVal =  PROG_OK;
  
  memset (stInfo, 0, sizeof (stInfo));

  for (EepromStructType typeIdx=0; typeIdx<EEPROM_STRUCT_LAST_ITEM; typeIdx++) {
    const uint32_t len = stInfo[typeIdx].len = eepromGetLenOfType (typeIdx);
    if (len != 0) {
      stInfo[typeIdx].dataPtr = malloc_m (len);
      if (stInfo[typeIdx].dataPtr == NULL) {
	retVal=MALLOC_ERR;
	goto freeMemAndExit;
      }
      retVal = eepromLoad(typeIdx, stInfo[typeIdx].dataPtr, len);

      // not the same ram address,  but is is normal
      if (retVal == OK_RAM_ADDR_DIFFER)
	retVal = PROG_OK;

      if (retVal != PROG_OK) {
	goto freeMemAndExit;
      }
    } 
  }

  eepromErase();
  for (EepromStructType typeIdx=0; typeIdx<EEPROM_STRUCT_LAST_ITEM; typeIdx++) {
    const uint32_t len = stInfo[typeIdx].len ;
    if (len != 0) {
      retVal = eepromStore(typeIdx, stInfo[typeIdx].dataPtr, len);
      if (retVal != PROG_OK) {
	goto freeMemAndExit;
      }
    } 
  }


 freeMemAndExit:
  for (EepromStructType typeIdx=0; typeIdx<EEPROM_STRUCT_LAST_ITEM; typeIdx++) {
    void *dataPtr = stInfo[typeIdx].dataPtr ;
    if (dataPtr != NULL) {
      free_m (dataPtr);
    } 
  }
  
  return retVal;
}


size_t eepromGetLenOfType (const EepromStructType eepromStructType)
{
  const  objInEeprom *flashStartAddr = (objInEeprom *) FLASH_USER_START_ADDR;
  objInEeprom *currentObjInFlash = (objInEeprom *) flashStartAddr;
  objInEeprom *lastTypeObject = currentObjInFlash->eepromStructType == eepromStructType ? 
    currentObjInFlash : NULL;


  while ((uint32_t) currentObjInFlash->nextObjPtr !=  0xffffffff) {
    currentObjInFlash = currentObjInFlash->nextObjPtr;
#ifdef TRACE_EEPROM
    DebugTrace ("IN loop currentObjInFlash = 0x%x", currentObjInFlash);
    chThdSleepMilliseconds(100);
#endif


    if (currentObjInFlash->eepromStructType == eepromStructType) {
      lastTypeObject=currentObjInFlash;
    }
  }

  if (lastTypeObject == NULL)  {
    return 0;
  }

  return lastTypeObject->dataLen;
}



size_t eepromStatus (void)
{  
  const  objInEeprom *flashStartAddr = (objInEeprom *) FLASH_USER_START_ADDR;
  objInEeprom *currentObjInFlash = (objInEeprom *) flashStartAddr;



  while ((uint32_t) currentObjInFlash->nextObjPtr !=  0xffffffff) {
    currentObjInFlash = currentObjInFlash->nextObjPtr;
  }

  return ((size_t) FLASH_USER_END_ADDR - (size_t) &currentObjInFlash->nextObjPtr);
}


/**
  * @brief  Gets the sector of a given address
  * @param  None
  * @retval The sector of a given address
  */
static uint32_t GetSector(const uint32_t Address)
{
  uint32_t sector = 0;
  
  if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
  {
    sector = FLASH_Sector_0;  
  }
  else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
  {
    sector = FLASH_Sector_1;  
  }
  else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
  {
    sector = FLASH_Sector_2;  
  }
  else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
  {
    sector = FLASH_Sector_3;  
  }
  else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
  {
    sector = FLASH_Sector_4;  
  }
  else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
  {
    sector = FLASH_Sector_5;  
  }
  else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
  {
    sector = FLASH_Sector_6;  
  }
  else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
  {
    sector = FLASH_Sector_7;  
  }
  else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
  {
    sector = FLASH_Sector_8;  
  }
  else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
  {
    sector = FLASH_Sector_9;  
  }
  else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
  {
    sector = FLASH_Sector_10;  
  }
  else/*(Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_11))*/
  {
    sector = FLASH_Sector_11;  
  }

  return sector;
}

