#include <stdint.h>
#include <stddef.h>

#define ERPS_BIT1_DUTY 100U

__attribute__((noinline))
uint32_t impl_div(const uint16_t *capture, size_t dmaLen)
{
  static const size_t frameLen = 20U;
  uint32_t erpsVal = 0;
  uint_fast8_t bit = 0x0;
  uint_fast8_t bitIndex = 0;
  uint_fast16_t prec = capture[0];

  for (size_t i = 1U; i < dmaLen; i++) {
    const uint_fast16_t len = capture[i] - prec;
    prec = capture[i];

    const uint_fast8_t nbConsecutives = (len + (ERPS_BIT1_DUTY / 2U)) / ERPS_BIT1_DUTY;
    if (bit) {
      switch(nbConsecutives) {
      case 1U:  erpsVal |= (0b001u << (frameLen - bitIndex)); break;
      case 2U:  erpsVal |= (0b011u << (frameLen - bitIndex - 1U)); break;
      default:  erpsVal |= (0b111u << (frameLen - bitIndex - 2U)); break;
      }
    }
    bit = ~bit;
    bitIndex += nbConsecutives;
  }
  for (size_t j = bitIndex; j <= frameLen; j++)
    erpsVal |= (1U << (frameLen - j));

  return erpsVal;
}

__attribute__((noinline))
uint32_t impl_thresh(const uint16_t *capture, size_t dmaLen)
{
  static const size_t frameLen = 20U;
  uint32_t erpsVal = 0;
  uint_fast8_t bit = 0x0;
  uint_fast8_t bitIndex = 0;
  uint_fast16_t prec = capture[0];
  const uint_fast16_t th1 = (3U * ERPS_BIT1_DUTY) / 2U;
  const uint_fast16_t th2 = (5U * ERPS_BIT1_DUTY) / 2U;

  for (size_t i = 1U; i < dmaLen; i++) {
    const uint_fast16_t len = capture[i] - prec;
    prec = capture[i];

    const uint_fast8_t nbConsecutives = (len < th1) ? 1U : ((len < th2) ? 2U : 3U);
    if (bit) {
      switch(nbConsecutives) {
      case 1U:  erpsVal |= (0b001u << (frameLen - bitIndex)); break;
      case 2U:  erpsVal |= (0b011u << (frameLen - bitIndex - 1U)); break;
      default:  erpsVal |= (0b111u << (frameLen - bitIndex - 2U)); break;
      }
    }
    bit = ~bit;
    bitIndex += nbConsecutives;
  }
  for (size_t j = bitIndex; j <= frameLen; j++)
    erpsVal |= (1U << (frameLen - j));

  return erpsVal;
}

__attribute__((noinline))
uint32_t impl_branchless(const uint16_t *capture, size_t dmaLen)
{
  static const size_t frameLen = 20U;
  uint32_t erpsVal = 0;
  uint_fast8_t bit = 0x0;
  uint_fast8_t bitIndex = 0;
  uint_fast16_t prec = capture[0];
  const uint_fast16_t th1 = (3U * ERPS_BIT1_DUTY) / 2U;
  const uint_fast16_t th2 = (5U * ERPS_BIT1_DUTY) / 2U;

  for (size_t i = 1U; i < dmaLen; i++) {
    const uint_fast16_t len = capture[i] - prec;
    prec = capture[i];

    const uint_fast8_t nbConsecutives = 1U + (uint_fast8_t)(len >= th1) + (uint_fast8_t)(len >= th2);
    if (bit) {
      switch(nbConsecutives) {
      case 1U:  erpsVal |= (0b001u << (frameLen - bitIndex)); break;
      case 2U:  erpsVal |= (0b011u << (frameLen - bitIndex - 1U)); break;
      default:  erpsVal |= (0b111u << (frameLen - bitIndex - 2U)); break;
      }
    }
    bit = ~bit;
    bitIndex += nbConsecutives;
  }
  for (size_t j = bitIndex; j <= frameLen; j++)
    erpsVal |= (1U << (frameLen - j));

  return erpsVal;
}
