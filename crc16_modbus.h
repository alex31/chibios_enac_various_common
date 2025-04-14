#pragma once
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

  uint16_t modbus_crc16 (const void *buf, size_t len);

#ifdef __cplusplus
}
#endif

