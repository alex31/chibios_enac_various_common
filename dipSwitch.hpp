#pragma once
#include <ch.h>
#include <hal.h>
#include "stdutil.h"
#include <array>

/*
  abstraction of dip switch using random pins
  unfortunately cannot be constexpr because of reinterpret_cast<stm32_gpio_t*>' is not a constant expression
 */

template<size_t SZ>
class DipSwitch {
public:
  uint32_t read() const;
  DipSwitch(const std::array<ioline_t, SZ> _lines) : lines(_lines) {};
private:
  const std::array<ioline_t, SZ> lines;
};

template<size_t SZ>
uint32_t DipSwitch<SZ>::read() const
{
  uint32_t mask = 0;
  for(size_t i = 0; i < SZ; i++) {
      mask |= palReadLine(lines[i]) << i;
  }
  return mask;
}
