#include <string.h>
#include "ch.h"
#include "hal.h"
#include "hal_flash.h"
#include "stm32_internal_flash.h"

static const flash_descriptor_t 
stm32_flash_bank_descriptor[STM32_FLASH_NUMBER_OF_BANK] = 
{
  {
    .attributes = FLASH_ATTR_ERASED_IS_ONE | FLASH_ATTR_MEMORY_MAPPED | 
      FLASH_ATTR_REWRITABLE,
    .page_size = STM32_FLASH_PAGE_SIZE,
    .sectors_count = STM32_FLASH_PAGE_COUNT_PER_BANK,
    .sectors = NULL, // uniform sector size
    .sectors_size = STM32_FLASH_PAGE_SIZE,
    .address = FLASH_BASE
  },
  {
    .attributes = FLASH_ATTR_ERASED_IS_ONE | FLASH_ATTR_MEMORY_MAPPED | 
      FLASH_ATTR_REWRITABLE,
    .page_size = STM32_FLASH_PAGE_SIZE,
    .sectors_count = STM32_FLASH_PAGE_COUNT_PER_BANK,
    .sectors = NULL, // uniform sector size
    .sectors_size = STM32_FLASH_PAGE_SIZE,
    .address = FLASH_BASE + 
    (STM32_FLASH_PAGE_COUNT_PER_BANK * STM32_FLASH_PAGE_SIZE)
  }
};

static void stm32_flash_lock(STM32Flash * devp)
{
  osalDbgCheck(devp != NULL);
  devp->flash_reg->CR |= FLASH_CR_LOCK;
}

static void stm32_flash_unlock(STM32Flash * devp)
{
  osalDbgCheck(devp != NULL);
  devp->flash_reg->KEYR |= FLASH_KEY1;
  devp->flash_reg->KEYR |= FLASH_KEY2;
}

static void stm32_flash_wait_busy(void * instance) 
{
  osalDbgCheck(instance != NULL);
  STM32Flash * devp = (STM32Flash *)instance;
  /* wait for busy bit clear */
  while( STM32_FLASH_GET_SR_FLAG(devp, FLASH_SR_BSY) != 0 ) {};
}

static flash_error_t stm32_flash_check_and_clear_errors(STM32Flash * devp) 
{
  osalDbgCheck(devp != NULL);

  do {
    if( STM32_FLASH_GET_SR_FLAG(devp, FLASH_SR_WRPERR) ) {
      /* Clear write protection error */
      STM32_FLASH_CLEAR_SR_FLAG(devp, FLASH_SR_WRPERR);
      return FLASH_ERROR_HW_FAILURE;
    }
    if( STM32_FLASH_GET_SR_FLAG(devp, FLASH_SR_RDERR) ) {
      /* Clear read error */
      STM32_FLASH_CLEAR_SR_FLAG(devp, FLASH_SR_RDERR);
      return FLASH_ERROR_READ;
    }
    if( (STM32_FLASH_GET_SR_FLAG(devp, FLASH_SR_PROGERR)) || \
        (STM32_FLASH_GET_SR_FLAG(devp, FLASH_SR_PGAERR))  || \
        (STM32_FLASH_GET_SR_FLAG(devp, FLASH_SR_SIZERR))  || \
        (STM32_FLASH_GET_SR_FLAG(devp, FLASH_SR_PGSERR)) ) {
      /* Clear all the errors related to program FLASH */
      STM32_FLASH_CLEAR_SR_FLAG(devp, FLASH_SR_PROGERR);
      STM32_FLASH_CLEAR_SR_FLAG(devp, FLASH_SR_PGAERR);
      STM32_FLASH_CLEAR_SR_FLAG(devp, FLASH_SR_SIZERR);
      STM32_FLASH_CLEAR_SR_FLAG(devp, FLASH_SR_PGSERR);
      return FLASH_ERROR_PROGRAM;
    }

  } while(STM32_FLASH_GET_SR_FLAG(devp, FLASH_SR_BSY) != 0);

  return FLASH_NO_ERROR;
}

static flash_error_t stm32_flash_program_doubleword(STM32Flash * devp, 
flash_offset_t address, uint32_t * pSrc)
{
  osalDbgCheck(devp != NULL);

  volatile uint32_t * pDst = (volatile uint32_t *)address;

  /* Write first word (32-bit) */
  *pDst++ = *pSrc++;
  /* Write second word (32-bit) */
  *pDst = *pSrc;

  /* wait for busy bit to clear */
  stm32_flash_wait_busy(devp);

  /* check for errors */
  if (stm32_flash_check_and_clear_errors(devp)) {
    return FLASH_ERROR_PROGRAM;
  }
  else {
    return FLASH_NO_ERROR;
  }
}

static const flash_descriptor_t * stm32_flash_get_descriptor(void * instance)
{
  osalDbgCheck(instance != NULL);

  STM32Flash * devp = (STM32Flash *)instance;

  return devp->flash_descriptor;
}

static flash_error_t stm32_flash_read(void * instance, flash_offset_t offset, 
                                      size_t n, uint8_t * rp)
{
  osalDbgCheck((instance != NULL) && (rp != NULL) && (n > 0));

  STM32Flash * devp = (STM32Flash *)instance;

  if (devp->state == FLASH_ERASE)
    return FLASH_BUSY_ERASING;

  flash_offset_t max_bank_address = devp->flash_descriptor->address + 
    (STM32_FLASH_PAGE_COUNT_PER_BANK * STM32_FLASH_PAGE_SIZE);
  uint32_t address = devp->flash_descriptor->address + offset;

  if (address > max_bank_address)
    return FLASH_ERROR_READ; // invalid address

  if ((offset + n) > (devp->flash_size))
    return FLASH_ERROR_READ; /* invalid number of bytes to read (ideally should 
    return number of bytes it can actually read instead of error. But as per 
    hal_flash read API interface signature, we cannot return number of bytes 
    read) */

  osalMutexLock(&devp->flash_mutex);

  devp->state = FLASH_READ;
  
  memcpy(rp, (void *)address, n);

  devp->state = FLASH_READY;

  osalMutexUnlock(&devp->flash_mutex);

  return FLASH_NO_ERROR;
}

static flash_error_t stm32_flash_program(void * instance, flash_offset_t offset, 
                                         size_t n, const uint8_t * pp)
{
  osalDbgCheck((instance != NULL) && (pp != NULL) && (n > 0));

  uint8_t* ptr;
  uint32_t writeData[2];
  STM32Flash * devp = (STM32Flash *)instance;

  if (devp->state == FLASH_ERASE)
    return FLASH_BUSY_ERASING;

  flash_offset_t max_bank_address = devp->flash_descriptor->address + 
    (STM32_FLASH_PAGE_COUNT_PER_BANK * STM32_FLASH_PAGE_SIZE);
  uint32_t address = devp->flash_descriptor->address + offset;

  if (address > max_bank_address)
    return FLASH_ERROR_PROGRAM; // invalid address to program. out of bounds.

  if ((offset + n) > (devp->flash_size))
    return FLASH_ERROR_PROGRAM; // num of bytes to program is out of bounds

  if (!MEM_IS_ALIGNED(address, sizeof(uint64_t)))
    return FLASH_ERROR_PROGRAM;  // address should be double word aligned

  osalMutexLock(&devp->flash_mutex);

  devp->state = FLASH_PGM;

  stm32_flash_unlock(devp);

  stm32_flash_wait_busy(devp);

  if (stm32_flash_check_and_clear_errors(devp) != FLASH_NO_ERROR)
  {
    stm32_flash_lock(devp);
    devp->state = FLASH_READY;
    osalMutexUnlock(&devp->flash_mutex);
    return FLASH_ERROR_PROGRAM;
  }

  STM32_ENABLE_FLASH_PGM(devp);

  while(n > 7)
  {
    writeData[0] = (*(uint32_t*)pp);
    pp += 4;
    writeData[1] = (*(uint32_t*)pp);
    pp += 4;
    flash_error_t status = 
      stm32_flash_program_doubleword(devp, address, &writeData[0]);
    if(status != FLASH_NO_ERROR) {
      /* disable flash programming */
      STM32_DISABLE_FLASH_PGM(devp);
      stm32_flash_lock(devp);
      devp->state = FLASH_READY;
      osalMutexUnlock(&devp->flash_mutex);
      return FLASH_ERROR_PROGRAM;
    }

    address += 8;
    n       -= 8;
  }

  if (n > 0) {
    writeData[0] = 0xffffffff;
    writeData[1] = 0xffffffff;
    ptr = (uint8_t *)(&writeData[0]);
    while(n > 0) {
      *ptr++ = *pp++;
      n--;
    }

    flash_error_t status = 
      stm32_flash_program_doubleword(devp, address, &writeData[0]);
    if(status != FLASH_NO_ERROR) {
      /* disable flash programming */
      STM32_DISABLE_FLASH_PGM(devp);
      stm32_flash_lock(devp);
      devp->state = FLASH_READY;
      osalMutexUnlock(&devp->flash_mutex);
      return FLASH_ERROR_PROGRAM;
    }
  }

  STM32_DISABLE_FLASH_PGM(devp);

  stm32_flash_lock(devp);

  devp->state = FLASH_READY;

  osalMutexUnlock(&devp->flash_mutex);

  return FLASH_NO_ERROR;
}

static flash_error_t stm32_flash_start_erase_all(void * instance)
{
  osalDbgCheck(instance != NULL);

  STM32Flash * devp = (STM32Flash *)instance;

  if (devp->state == FLASH_ERASE)
    return FLASH_BUSY_ERASING;

  if (!((devp->bank > 0)  && (devp->bank <= STM32_FLASH_NUMBER_OF_BANK)))
    return FLASH_ERROR_ERASE; // Invalid flash device's bank number.

  osalMutexLock(&devp->flash_mutex);

  devp->state = FLASH_ERASE;

  /* unlock the flash */
  stm32_flash_unlock(devp);

  /* make sure the flash is not busy */
  stm32_flash_wait_busy(devp);

  /* check and clear all error flags due to a previous operation */
  if (stm32_flash_check_and_clear_errors(devp) != FLASH_NO_ERROR)
  {
    stm32_flash_lock(devp);
    devp->state = FLASH_READY;
    osalMutexUnlock(&devp->flash_mutex);
    return FLASH_ERROR_ERASE;
  }

  if(devp->bank == 1) 
  {
    /* select first bank to be erased */
    devp->flash_reg->CR |= (FLASH_CR_MER1);
  }
  else if (devp->bank == 2)
  {
    /* select second bank to be erased */
    devp->flash_reg->CR |= (FLASH_CR_MER2);
  }

  /* start the erase */
  devp->flash_reg->CR |= FLASH_CR_STRT;

  /* flash_lld_lock must be done in query_erase */

  osalMutexUnlock(&devp->flash_mutex);

  return FLASH_NO_ERROR;
}

static flash_error_t stm32_flash_start_erase_sector(void * instance,
                                                    flash_sector_t sector)
{
  osalDbgCheck(instance != NULL);
  STM32Flash * devp = (STM32Flash *)instance;

  if (sector > devp->flash_descriptor->sectors_count)
    return FLASH_ERROR_ERASE;

  if (!((devp->bank > 0)  && (devp->bank <= STM32_FLASH_NUMBER_OF_BANK)))
    return FLASH_ERROR_ERASE;

  if (devp->state == FLASH_ERASE)
    return FLASH_BUSY_ERASING;

  osalMutexLock(&devp->flash_mutex);

  devp->state = FLASH_ERASE;

  /* unlock the flash */
  stm32_flash_unlock(devp);

  /* make sure the flash is not busy */
  stm32_flash_wait_busy(devp);

  /* enable page erase */
  devp->flash_reg->CR |= FLASH_CR_PER;

  if(devp->bank == 1) 
  {
    /* the sector is in first bank */
    devp->flash_reg->CR &= (~(FLASH_CR_BKER));
  }
  else if (devp->bank == 2)
  {
    /* the sector is in second bank */
    devp->flash_reg->CR |= (FLASH_CR_BKER);
  }

  /* mask off the page selection bits */
  devp->flash_reg->CR &= (~(FLASH_CR_PNB));

  /* set the page selection bits */
  devp->flash_reg->CR |= ((sector << FLASH_CR_PNB_Pos) & (FLASH_CR_PNB_Msk));

  /* start the erase */
  devp->flash_reg->CR |= FLASH_CR_STRT;

  /* flash_lld_lock must be done in query_erase */

  osalMutexUnlock(&devp->flash_mutex);

  return FLASH_NO_ERROR;
}

static flash_error_t stm32_flash_query_erase(void * instance, 
                                             uint32_t * wait_time)
{
  osalDbgCheck(instance != NULL);
  STM32Flash * devp = (STM32Flash *)instance;
  
  if (!((devp->bank > 0)  && (devp->bank <= STM32_FLASH_NUMBER_OF_BANK)))
    return FLASH_ERROR_ERASE;

  osalMutexLock(&devp->flash_mutex);

  uint32_t SR = devp->flash_reg->SR;

  /* get the default wait time */
  if(wait_time) {
    *wait_time = STM32_FLASH_WAIT_TIME_MS;
  }

  /* return if it's still busy erasing */
  if( (SR & FLASH_SR_BSY) == FLASH_SR_BSY ) {
    osalMutexUnlock(&devp->flash_mutex);
    return FLASH_BUSY_ERASING;
  }

  /* disable page and bank erase */
  devp->flash_reg->CR &= (~(FLASH_CR_PER));
  if(devp->bank == 1) 
  {
    devp->flash_reg->CR &= (~(FLASH_CR_MER1));
  }
  else if(devp->bank == 2) 
  {
    devp->flash_reg->CR &= (~(FLASH_CR_MER2));
  }

  /* lock the flash */
  stm32_flash_lock(devp);

  /* check for error flags */
  if(STM32_FLASH_GET_SR_FLAG(devp, (FLASH_SR_PROGERR | FLASH_SR_WRPERR)) != 0) 
  {
    STM32_FLASH_CLEAR_SR_FLAG(devp, (FLASH_SR_PROGERR | FLASH_SR_WRPERR));
    devp->state = FLASH_READY;
    osalMutexUnlock(&devp->flash_mutex);
    return FLASH_ERROR_ERASE;
  }

  devp->state = FLASH_READY;

  osalMutexUnlock(&devp->flash_mutex);

  return FLASH_NO_ERROR;
}

static flash_error_t stm32_flash_verify_erase(void * instance, 
                                              flash_sector_t sector)
{
  osalDbgCheck(instance != NULL);

  STM32Flash * devp = (STM32Flash *)instance;

  if (sector > devp->flash_descriptor->sectors_count)
    return FLASH_ERROR_ERASE;

  if (devp->state == FLASH_ERASE)
    return FLASH_BUSY_ERASING;

  uint32_t sector_address = devp->flash_descriptor->address + 
    flashGetSectorOffset(getBaseFlash(devp), sector);

  uint32_t sector_end_address = sector_address + 
    devp->flash_descriptor->sectors_size;

  osalMutexLock(&devp->flash_mutex);

  devp->state = FLASH_READ;

  for (;sector_address < sector_end_address; sector_address += 4)
  {
    uint32_t word_data = *(volatile uint32_t *)sector_address;
    if (word_data != 0xFFFFFFFF)
    {
      osalMutexUnlock(&devp->flash_mutex);
      return FLASH_ERROR_VERIFY;
    }
  }

  devp->state = FLASH_READY;

  osalMutexUnlock(&devp->flash_mutex);

  return FLASH_NO_ERROR;
}

static const struct STM32FlashVMT vmt = { (size_t)0,
  stm32_flash_get_descriptor, stm32_flash_read, stm32_flash_program, 
  stm32_flash_start_erase_all, stm32_flash_start_erase_sector, 
  stm32_flash_query_erase, stm32_flash_verify_erase, stm32_flash_wait_busy,
};

void stm32FlashObjectInit(STM32Flash * devp, flash_bank_t bank)
{
  osalDbgCheck(devp != NULL);
  osalDbgAssert((bank > 0)  && (bank <= STM32_FLASH_NUMBER_OF_BANK), 
    "Invalid flash device's bank number");

  devp->vmt = &vmt;
  devp->bank = bank;
  devp->flash_descriptor = &stm32_flash_bank_descriptor[bank-1];
  devp->flash_reg = FLASH;
  devp->flash_size = devp->flash_descriptor->sectors_count *
                     devp->flash_descriptor->sectors_size;
  devp->state = FLASH_READY;
  osalMutexObjectInit(&devp->flash_mutex);
}

void stm32FlashStart(STM32Flash * devp)
{
  (void)devp;
}

void stm32FlashStop(STM32Flash * devp)
{
  (void)devp;
}
