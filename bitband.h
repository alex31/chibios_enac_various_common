
/**
 * @file bitband.h
 *
 * @brief Bit-banding utility functions
 */
#ifndef _BITBAND_H_
#define _BITBAND_H_

#include <ch.h>

#ifdef __cplusplus
  extern "C" {
#endif
 

static inline volatile uint32_t* __bb_addr(volatile void* const,
                                         const uint32_t,
                                         const uint32_t,
                                         const uint32_t);

/**
 * @brief Obtain a pointer to the bit-band address corresponding to a
 * bit in a volatile SRAM address.
 * @param address Address in the bit-banded SRAM region
 * @param bit     Bit in address to bit-band
 */
static inline volatile uint32_t* bb_sramp(volatile void * const address, const uint32_t bit) {
    return __bb_addr(address, bit, SRAM_BB_BASE, SRAM_BASE);
}

/**
 * @brief Get a bit from an address in the SRAM bit-band region.
 * @param address Address in the SRAM bit-band region to read from
 * @param bit Bit in address to read
 * @return bit's value in address.
 */
static inline uint32_t bb_sram_get_bit(volatile void * const address, const uint32_t bit) {
    return *bb_sramp(address, bit);
}

/**
 * @brief Set a bit in an address in the SRAM bit-band region.
 * @param address Address in the SRAM bit-band region to write to
 * @param bit Bit in address to write to
 * @param val Value to write for bit, either 0 or 1.
 */
static inline void bb_sram_set_bit(volatile void * const address,
                                   const uint32_t bit,
                                   const uint32_t val) {
    *bb_sramp(address, bit) = val;
}

/**
 * @brief Obtain a pointer to the bit-band address corresponding to a
 * bit in a peripheral address.
 * @param address Address in the bit-banded peripheral region
 * @param bit     Bit in address to bit-band
 */
static inline volatile uint32_t* bb_perip(volatile void * const address, const uint32_t bit) {
    return __bb_addr(address, bit, PERIPH_BB_BASE, PERIPH_BASE);
}

/**
 * @brief Get a bit from an address in the peripheral bit-band region.
 * @param address Address in the peripheral bit-band region to read from
 * @param bit Bit in address to read
 * @return bit's value in address.
 */
static inline uint32_t bb_peri_get_bit(volatile void * const address, const uint32_t bit) {
    return *bb_perip(address, bit);
}

/**
 * @brief Set a bit in an address in the peripheral bit-band region.
 * @param address Address in the peripheral bit-band region to write to
 * @param bit Bit in address to write to
 * @param val Value to write for bit, either 0 or 1.
 */
static inline void bb_peri_set_bit(volatile void * const address,
                                   const uint32_t bit,
                                   const uint32_t val) {
    *bb_perip(address, bit) = val;
}

static inline volatile uint32_t* __bb_addr(volatile void * const address,
                                         const uint32_t bit,
                                         const uint32_t bb_base,
                                         const uint32_t bb_ref) {
    return (volatile uint32_t*)(bb_base + ((uint32_t)address - bb_ref) * 32 +
                              bit * 4);
}

#ifdef __cplusplus
  }
#endif
 

#endif  /* _BITBAND_H_ */
