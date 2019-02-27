#pragma once
#include "ch.h"
#include "hal.h"
#if defined STM32F4XX
#include "eeprom_f4.h"
#elif defined STM32L4XX
#include "eeprom_l4.h"
#else
#error "eeprom subsystem now implemented only for F4XXX and L4XXX family" 
#endif
