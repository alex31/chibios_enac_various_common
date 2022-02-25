#pragma once
#include <stdint.h>
#include <stddef.h>

uint16_t modbus_crc16(const uint8_t *buf, size_t len);

