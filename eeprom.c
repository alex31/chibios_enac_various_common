#include "ch.h"
#include "hal.h"
#include "eeprom.h"

typedef struct objInEeprom {
  struct objInEeprom *nextObjPtr;
  EepromStructType eepromStructType;
  uint32_t dataLen;
  uint32_t ramAdress;
  uint8_t data[0];
} objInEeprom;

static ErrorCond memcopyToFlash (const void *from, const void *flashAddr, 
				 size_t length, bool eraseBefore);
#if defined STM32F4XX
#include "eeprom_f4.c"
#elif defined STM32L4XX
#include "eeprom_l4.c"
#else
#error "eeprom subsystem now implemented only for F4XXX and L4XXX family" 
#endif
