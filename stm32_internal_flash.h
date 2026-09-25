#ifndef STM32_INTERNAL_FLASH_H
#define STM32_INTERNAL_FLASH_H

#include "hal_flash.h"

#if !defined(STM32_FLASH_PAGE_COUNT_PER_BANK)
#error  "Missing STM32L4xx flash memory page count per bank"
#error  "Define it in mcuconf.h"
#endif

/* Flag for the whole STM32L4XX family. */
#if defined(STM32L431xx) || defined(STM32L432xx) || defined(STM32L433xx) || \
    defined(STM32L442xx) || defined(STM32L443xx) || defined(STM32L451xx) || \
    defined(STM32L452xx) || defined(STM32L462xx)

#define STM32_FLASH_NUMBER_OF_BANK   1
#define STM32_FLASH_PAGE_SIZE        2048U

#elif defined(STM32L471xx) || defined(STM32L475xx) || defined(STM32L476xx) || \
      defined(STM32L485xx) || defined(STM32L486xx) || defined(STM32L496xx) || \
      defined(STM32L4A6xx)

#define STM32_FLASH_NUMBER_OF_BANK   2
#define STM32_FLASH_PAGE_SIZE        2048U

/****************  Bit definition for FLASH_PDKEYR register  ******************/
#define  FLASH_PDKEY1                        ((uint32_t)0x04152637)        /*!
< PD Key1 */
#define  FLASH_PDKEY2                        ((uint32_t)0xFAFBFCFD)        /*!
< PD Key2 */

/******************  Bit definition for FLASH_KEYR register  ******************/
#define  FLASH_KEY1                          ((uint32_t)0x45670123)        /*!
< FPEC Key1 */
#define  FLASH_KEY2                          ((uint32_t)0xCDEF89AB)        /*!
< FPEC Key2 */

/****************  Bit definition for FLASH_OPTKEYR register  *****************/
#define  FLASH_OPTKEY1                       ((uint32_t)0x08192A3B)        /*!
< OPTR Key1 */
#define  FLASH_OPTKEY2                       ((uint32_t)0x4C5D6E7F)        /*!
< OPTR Key2 */

#else
#error "Undefined STM32L4xx processor type"
#endif

#if !defined(STM32_FLASH_WAIT_TIME_MS)
#define STM32_FLASH_WAIT_TIME_MS   10
#endif /* !defined(FLASH_LLD_WAIT_TIME_MS) */

#define STM32_ENABLE_FLASH_PGM(devp)   (devp->flash_reg->CR |= FLASH_CR_PG)
#define STM32_DISABLE_FLASH_PGM(devp)  (devp->flash_reg->CR &= (~(FLASH_CR_PG)))

#define STM32_FLASH_GET_SR_FLAG(devp, flag)   (devp->flash_reg->SR & flag)
#define STM32_FLASH_CLEAR_SR_FLAG(devp, flag) (devp->flash_reg->SR |= flag)

typedef uint32_t flash_bank_t;

#define _stm32_flash_methods                                                  \
  _base_flash_methods                                                         \
  void (*wait_busy)(void * instance);

struct STM32FlashVMT {
  _stm32_flash_methods
};

#define _stm32_flash_data                                                     \
  _base_flash_data                                                            \
  flash_bank_t bank;                                                          \
  const flash_descriptor_t * flash_descriptor;                                \
  FLASH_TypeDef * flash_reg;                                                  \
  size_t flash_size;                                                          \
  mutex_t flash_mutex;

typedef struct {
  const struct STM32FlashVMT * vmt;
  _stm32_flash_data
} STM32Flash;

#define flashWaitBusy(ip)                                                     \
  (ip)->vmt->wait_busy(ip)

void stm32FlashObjectInit(STM32Flash * devp, flash_bank_t bank);
void stm32FlashStart(STM32Flash * devp);
void stm32FlashStop(STM32Flash * devp);

#endif /* #ifndef STM32_INTERNAL_FLASH_H */
