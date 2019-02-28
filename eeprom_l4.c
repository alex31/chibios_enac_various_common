#include "eeprom.h"

#define USE_RTOS 0
#include "stm32l4xx_hal_flash.h"
#include "stm32l4xx_hal_flash_ex.h"

//#define TRACE_EEPROM 1

#include <string.h>
#include "ch.h"
#include "hal.h"
#include "stdutil.h"

#ifdef TRACE_EEPROM
#include "printf.h"
#include "globalVar.h"
#endif

static uint32_t GetPage(uint32_t Addr);
static uint32_t GetBank(uint32_t Addr);
static uint64_t read64(const uint32_t addr);

/* Private typedef -----------------------------------------------------------*/
typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;

/* Private define ------------------------------------------------------------*/

#define DATA_64                 ((uint64_t)0x1234567812345678)
#define DATA_32                 ((uint32_t)0x12345678)

#define WSIZE (sizeof(uint64_t))

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

int testFlash(void)
{
  FLASH_EraseInitTypeDef EraseInitStruct;
  uint32_t FirstPage = 0, NbOfPages = 0, BankNumber = 0;
  uint32_t Address = 0, PAGEError = 0;
  __IO uint32_t data32 = 0 , MemoryProgramStatus = 0;

    /* Unlock the Flash to enable the flash control register access *************/
  HAL_FLASH_Unlock();

  /* Erase the user Flash area
    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

  /* Clear OPTVERR bit set on virgin samples */
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR); 
  /* Get the 1st page to erase */
  FirstPage = GetPage(FLASH_USER_START_ADDR);
  /* Get the number of pages to erase from 1st page */
  NbOfPages = GetPage(FLASH_USER_END_ADDR) - FirstPage + 1;
  /* Get the bank */
  BankNumber = GetBank(FLASH_USER_START_ADDR);

  // DebugTrace("FirstPage=%d NbOfPages=%d", FirstPage, NbOfPages);
  
  /* Fill EraseInit structure*/
  EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.Banks       = BankNumber;
  EraseInitStruct.Page        = FirstPage;
  EraseInitStruct.NbPages     = NbOfPages;

  /* Note: If an erase operation in Flash memory also concerns data in the data or instruction cache,
     you have to make sure that these data are rewritten before they are accessed during code
     execution. If this cannot be done safely, it is recommended to flush the caches by setting the
     DCRST and ICRST bits in the FLASH_CR register. */
  if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)  {
    return -1;
  }

  /* Program the user Flash area word by word
    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

  Address = FLASH_USER_START_ADDR;

  while (Address < FLASH_USER_END_ADDR)  {
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, Address, DATA_64) == HAL_OK)  {
      Address = Address + WSIZE;
    } else {
      return -2;
    }
  }

  /* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) *********/
  HAL_FLASH_Lock();

  /* Check if the programmed data is OK
      MemoryProgramStatus = 0: data programmed correctly
      MemoryProgramStatus != 0: number of words not programmed correctly ******/
  Address = FLASH_USER_START_ADDR;
  MemoryProgramStatus = 0x0;

  while (Address < FLASH_USER_END_ADDR)
  {
    data32 = *(__IO uint32_t *)Address;

    if (data32 != DATA_32)
    {
      MemoryProgramStatus++;
    }
    Address = Address + 4;
  }
  
  return MemoryProgramStatus;
}


ErrorCond eepromErase (void)
{
  FLASH_EraseInitTypeDef EraseInitStruct;
  uint32_t FirstPage = 0, NbOfPages = 0, BankNumber = 0, PAGEError = 0;
  ErrorCond  retval = PROG_OK;
    /* Unlock the Flash to enable the flash control register access *************/
  HAL_FLASH_Unlock();

  /* Erase the user Flash area
    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

  /* Clear OPTVERR bit set on virgin samples */
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR); 
  /* Get the 1st page to erase */
  FirstPage = GetPage(FLASH_USER_START_ADDR);
  /* Get the number of pages to erase from 1st page */
  NbOfPages = GetPage(FLASH_USER_END_ADDR) - FirstPage + 1;
  /* Get the bank */
  BankNumber = GetBank(FLASH_USER_START_ADDR);

  // DebugTrace("FirstPage=%d NbOfPages=%d", FirstPage, NbOfPages);
  
  /* Fill EraseInit structure*/
  EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.Banks       = BankNumber;
  EraseInitStruct.Page        = FirstPage;
  EraseInitStruct.NbPages     = NbOfPages;

  /* Note: If an erase operation in Flash memory also concerns data in the data or instruction cache,
     you have to make sure that these data are rewritten before they are accessed during code
     execution. If this cannot be done safely, it is recommended to flush the caches by setting the
     DCRST and ICRST bits in the FLASH_CR register. */
  if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)  {
    retval = ERASE_ERR;
  }
   HAL_FLASH_Lock();
   
   return retval;
}


ErrorCond memcopyToFlash(const void *from, const void *flashAddr, 
			 size_t length, bool eraseBefore)
{
  const uint32_t flashStartAddr = (flashAddr != NULL) ?  (const uint32_t) flashAddr 
						      :	 FLASH_USER_START_ADDR;
  const size_t alignOffset = (WSIZE-(length%WSIZE)) % WSIZE;
  const uint32_t flashEndAddr = flashStartAddr + length + alignOffset;
  const uint32_t ramBeginAddr = (uint32_t) from;
  const uint32_t ramEndAddr = ramBeginAddr+length;
  volatile uint64_t data64 = 0 ;

  chDbgAssert(((uint32_t)(flashAddr) % WSIZE) == 0, "flashAddr have to be 64 bits aligned");

#ifdef TRACE_EEPROM
  DebugTrace ("DBG> ramAddr=%p, flashAddr=%p length=%u eraseBefore=%u",
	      from, flashAddr, length, eraseBefore);
#endif


  
  if (flashEndAddr >= FLASH_USER_END_ADDR)
    return (SECTOR_FULL_ERR);

  /* Unlock the Flash to enable the flash control register access *************/ 
  HAL_FLASH_Unlock();
#ifdef TRACE_EEPROM
  DebugTrace ("FLASH UNlock ;; len=%d alignOffset=%d", length, alignOffset);
#endif

  /* Clear pending flags (if any) */  
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR); 

  if (eraseBefore) {
    eepromErase();
  }

  uint32_t flashPtr = flashStartAddr;
  uint32_t ramPtr = ramBeginAddr;

  while (flashPtr < flashEndAddr) {
    // if we are on a non aligned length, we should program last bytes
    if ((ramEndAddr - ramPtr) < WSIZE) {
      uint32_t pos=0;
      uint64_t lastword=0;
      char *rptr =  (char *) ramPtr;
      while (rptr < (char *) ramEndAddr) {
	lastword |= ((((uint64_t)(*rptr)) << pos) & (((uint64_t)(0xff)) << pos));
	rptr++;
	pos+=8 /*bits*/;
      }
#ifdef TRACE_EEPROM
      DebugTrace ("DBG> flash LAST INCOMPLETE value %lx:%lx @ 0x%lx -> 0x%lx",
		  (uint32_t) (lastword & 0xffffffff), (uint32_t)(lastword>>32),
		  ramPtr, flashPtr);
#endif
      if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, flashPtr, lastword) != HAL_OK)  {
	HAL_FLASH_Lock();
	return (PROGRAM_ERR);
      } else {
	flashPtr += WSIZE;
      }
    } else {
#ifdef TRACE_EEPROM
      DebugTrace ("DBG> flash value %lx:%lx @ 0x%lx -> 0x%lx", *((uint32_t *) ramPtr), *((uint32_t *) (ramPtr+4)),
		  ramPtr, flashPtr);
#endif
      HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, flashPtr,
						   read64(ramPtr));
      if (status == HAL_OK)  {
	flashPtr += WSIZE;
	ramPtr += WSIZE;
      } else { 
	HAL_FLASH_Lock(); 
	return (PROGRAM_ERR);
      }
    }
  }

  /* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) *********/
  HAL_FLASH_Lock(); 
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
    DebugTrace ("verify address 0x%lx", flashPtr);
    chThdSleepMilliseconds(100);
#endif
    data64 = *(__IO uint64_t*) flashPtr;
    // if we are on a non aligned length, we should test last bytes
    if ((ramEndAddr - ramPtr) < WSIZE) {
      uint32_t pos=0;
      uint64_t lastword=0;
      char *rptr =  (char *) ramPtr;
      while (rptr < (char *) ramEndAddr) {
	lastword |= ((((uint64_t)(*rptr)) << pos) & (((uint64_t)(0xff)) << pos));
	pos+=8 /*bits*/;
	rptr++;
      }
      flashPtr += WSIZE;
      if (data64 != lastword) {
	return VERIFY_READ_ERROR;
      }
    } else {
      if (data64 != read64(ramPtr)) {
	return VERIFY_READ_ERROR;
      }
      flashPtr += WSIZE;
      ramPtr += WSIZE;
    }
  }

  return PROG_OK;
}




ErrorCond eepromStore (EepromStructType eepromStructType, const void *data, uint32_t len)
{
  const  objInEeprom *flashStartAddr = (objInEeprom *) FLASH_USER_START_ADDR;
  objInEeprom *objToFlashEepromAddr = (objInEeprom *) flashStartAddr;

  const size_t objToFlashSize = sizeof(objInEeprom)+len;
  objInEeprom *objToFlash = (objInEeprom *) malloc_m(objToFlashSize);

#ifdef TRACE_EEPROM
  DebugTrace ("before loop objToFlashEepromAddr = %p", objToFlashEepromAddr);
  chThdSleepMilliseconds(100);
#endif

    if (((int) eepromStatus() - (int) len) < (int) WSIZE)  {
#ifdef TRACE_EEPROM
      DebugTrace ("eeprom full, has to wipe (remove obsolete data) before writing");
      chThdSleepMilliseconds(100);
#endif
      eepromWipe(); 
    }

    while ((uint32_t) objToFlashEepromAddr->nextObjPtr != 0xffffffff) {
      objToFlashEepromAddr = objToFlashEepromAddr->nextObjPtr;
#ifdef TRACE_EEPROM
      DebugTrace ("IN loop objToFlashEepromAddr = %p", objToFlashEepromAddr);
      chThdSleepMilliseconds(100);
#endif
    }
    
#ifdef TRACE_EEPROM
    DebugTrace ("objToFlashEepromAddr = %p", objToFlashEepromAddr);
    chThdSleepMilliseconds(100);
#endif
    
  // now objToFlashEepromAddr is the first free address in flash
  objToFlash->eepromStructType = eepromStructType;
  const uint32_t ptrCurr = (uint32_t) objToFlashEepromAddr;
  const uint32_t alignOffset = (WSIZE-(objToFlashSize%WSIZE)) % WSIZE;
  const uint32_t ptrNext = ptrCurr+alignOffset+objToFlashSize;
  objToFlash->nextObjPtr = (objInEeprom *) ptrNext;
  objToFlash->dataLen = len;
  objToFlash->ramAdress = (uint32_t) data;
  memcpy(&objToFlash->data, data, len);

#ifdef TRACE_EEPROM
    DebugTrace (" objToFlash->nextObjPtr = 0x%p",  objToFlash->nextObjPtr);
    chThdSleepMilliseconds(100);
#endif


  const ErrorCond retval =  memcopyToFlash (objToFlash, objToFlashEepromAddr,
					    objToFlashSize, FALSE);

 #ifdef TRACE_EEPROM
  {
    objInEeprom *storeObject = (objInEeprom *) objToFlashEepromAddr;
    DebugTrace("STO : nextObjPtr = %p, type = %u, dataLen=%lu, ramAdr=0x%lx",
	       storeObject->nextObjPtr, storeObject->eepromStructType,
	       storeObject->dataLen, storeObject->ramAdress);
    
  }
#endif
 
  free_m(objToFlash);
  return retval;
}



ErrorCond eepromLoad (EepromStructType eepromStructType, void *data, 
		      uint32_t len)
{
  const  objInEeprom *flashStartAddr = (objInEeprom *) FLASH_USER_START_ADDR;
  objInEeprom *objToFlashEepromAddr = (objInEeprom *) flashStartAddr;
  objInEeprom *lastTypeObject = objToFlashEepromAddr->eepromStructType == eepromStructType ? 
								objToFlashEepromAddr : NULL;
  ErrorCond retVal = PROG_OK;
  
#ifdef TRACE_EEPROM
    DebugTrace ("before loop objToFlashEepromAddr = %p", objToFlashEepromAddr);
    chThdSleepMilliseconds(100);
#endif


    while ((uint32_t) objToFlashEepromAddr->nextObjPtr !=  0xffffffff) {
    objToFlashEepromAddr = objToFlashEepromAddr->nextObjPtr;
#ifdef TRACE_EEPROM
    DebugTrace ("IN loop objToFlashEepromAddr = %p", objToFlashEepromAddr);
    chThdSleepMilliseconds(100);
#endif


    if (objToFlashEepromAddr->eepromStructType == eepromStructType) {
      lastTypeObject=objToFlashEepromAddr;
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
  DebugTrace ("memcopy %p, %p, %ld", 
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


size_t eepromGetLenOfType (EepromStructType eepromStructType)
{
  const  objInEeprom *flashStartAddr = (objInEeprom *) FLASH_USER_START_ADDR;
  objInEeprom *objToFlashEepromAddr = (objInEeprom *) flashStartAddr;
  objInEeprom *lastTypeObject = objToFlashEepromAddr->eepromStructType == eepromStructType ? 
    objToFlashEepromAddr : NULL;


  while ((uint32_t) objToFlashEepromAddr->nextObjPtr !=  0xffffffff) {
    objToFlashEepromAddr = objToFlashEepromAddr->nextObjPtr;
#ifdef TRACE_EEPROM
    DebugTrace ("IN loop objToFlashEepromAddr = %p", objToFlashEepromAddr);
    chThdSleepMilliseconds(100);
#endif


    if (objToFlashEepromAddr->eepromStructType == eepromStructType) {
      lastTypeObject=objToFlashEepromAddr;
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
  objInEeprom *objToFlashEepromAddr = (objInEeprom *) flashStartAddr;



  while ((uint32_t) objToFlashEepromAddr->nextObjPtr !=  0xffffffff) {
#ifdef TRACE_EEPROM
    DebugTrace ("eeprom status objToFlashEepromAddr = %p", objToFlashEepromAddr);
    chThdSleepMilliseconds(100);
#endif
    objToFlashEepromAddr = objToFlashEepromAddr->nextObjPtr;
  }

  return ((size_t) FLASH_USER_END_ADDR - (size_t) &objToFlashEepromAddr->nextObjPtr);
}


/**
  * @brief  Gets the page of a given address
  * @param  Addr: Address of the FLASH Memory
  * @retval The page of a given address
  */
static uint32_t GetPage(uint32_t Addr)
{
  uint32_t page = 0;
  
  if (Addr < (FLASH_BASE + FLASH_BANK_SIZE))
  {
    /* Bank 1 */
    page = (Addr - FLASH_BASE) / FLASH_PAGE_SIZE;
  }
  else
  {
    /* Bank 2 */
    page = (Addr - (FLASH_BASE + FLASH_BANK_SIZE)) / FLASH_PAGE_SIZE;
  }
  
  return page;
}

/**
  * @brief  Gets the bank of a given address
  * @param  Addr: Address of the FLASH Memory
  * @retval The bank of a given address
  */
static uint32_t GetBank(uint32_t Addr)
{
#ifdef FLASH_BANK_2
  return (Addr < 0x08080000) ? FLASH_BANK_1 : FLASH_BANK_2;
#else
  (void) Addr;
  return FLASH_BANK_1;
#endif
}


static uint64_t read64(const uint32_t addr)
{
  return *((uint64_t *) addr);
}


