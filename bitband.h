
/**
 * @file bitband.h
 *
 * @brief Bit-banding utility functions
 */
#pragma once

#include <ch.h>

#ifdef __cplusplus
  extern "C" {
#endif


#if (!defined PERIPH_BB_BASE) || (!defined SRAM_BB_BASE)
# error "architecture  is not bitbanding capable"
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
#if CH_DBG_ENABLE_ASSERTS == TRUE
  chDbgAssert(address <= (void *) 0x400FFFFF, "not mapped to alias region");
#endif
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

/**
 * @brief Set a bit in GPIO using same api than chibios but via bitband
 * @param gpio (with chibios type) 
 * @param bit Bit in gpio to write to
 */
static inline void bb_palSetPad   (ioportid_t gpio,
                                   const uint32_t bit) {
   bb_peri_set_bit(&(gpio->ODR), bit, 1); 
}

/**
 * @brief Write a bit in GPIO using same api than chibios but via bitband
 * @param gpio (with chibios type) 
 * @param bit Bit in gpio to write to
 */
static inline void bb_palWritePad   (ioportid_t gpio,
				     const uint32_t bit,
				     const uint32_t val) {
   bb_peri_set_bit(&(gpio->ODR), bit, val); 
}

/**
 * @brief Clear a bit in GPIO using same api than chibios but via bitband
 * @param gpio (with chibios type) 
 * @param bit Bit in gpio to write to
 */
static inline void bb_palClearPad   (ioportid_t gpio,
                                   const uint32_t bit) {
  bb_peri_set_bit(&(gpio->ODR), bit, 0); 
}

/**
 * @brief Toggle a bit in GPIO using same api than chibios but via bitband
 * @param gpio (with chibios type) 
 * @param bit Bit in gpio to write to
 */
static inline void bb_palTogglePad   (ioportid_t gpio,
                                   const uint32_t bit) {
  bb_peri_set_bit(&(gpio->ODR), bit, bb_peri_get_bit(&(gpio->IDR), bit) ^ 0x1); 
}

/**
 * @brief Read a bit in GPIO using same api than chibios but via bitband
 * @param gpio (with chibios type) 
 * @param bit Bit in gpio to write to
 */
static inline uint32_t bb_palReadPad   (ioportid_t gpio,
					const uint32_t bit) {
  return bb_peri_get_bit(&(gpio->IDR), bit); 
}




#if (CH_KERNEL_MAJOR > 2)

    
/**
 * @brief Set a bit in GPIO using same api than chibios but via bitband
 * @param gpio (with chibios type) 
 * @param bit Bit in gpio to write to
 */
static inline void bb_palSetLine   (ioline_t line) {
  bb_peri_set_bit(&(PAL_PORT(line)->ODR), PAL_PAD(line), 1);
  //bb_peri_set_bit(&(PAL_PORT(line)->BSRR), PAL_PAD(line), 1);
}

/* 
 * @brief Write a bit in GPIO using same api than chibios but via bitband
 * @param gpio (with chibios type)
 * @param bit Bit in gpio to write to
 */
static inline void bb_palWriteLine   (ioline_t line,
				     const uint32_t val) {
  bb_peri_set_bit(&(PAL_PORT(line)->ODR), PAL_PAD(line), val);
  //bb_peri_set_bit(&(PAL_PORT(line)->BSRR), PAL_PAD(line)+(val ? 16 : 0), 1);
}

/**
 * @brief Clear a bit in GPIO using same api than chibios but via bitband
 * @param gpio (with chibios type)
 * @param bit Bit in gpio to write to
 */
static inline void bb_palClearLine   (ioline_t line) {
  bb_peri_set_bit(&(PAL_PORT(line)->ODR), PAL_PAD(line), 0);
  //  bb_peri_set_bit(&(PAL_PORT(line)->BSRR), PAL_PAD(line)+16, 1);
}

/**
 * @brief Toggle a bit in GPIO using same api than chibios but via bitband
 * @param gpio (with chibios type)
 * @param bit Bit in gpio to write to
 */
static inline void bb_palToggleLine   (ioline_t line) {
  bb_peri_set_bit(&(PAL_PORT(line)->ODR), PAL_PAD(line),
		  bb_peri_get_bit(&(PAL_PORT(line)->IDR), PAL_PAD(line)) ^ 0x1);
}

/**
 * @brief Read a bit in GPIO using same api than chibios but via bitband
 * @param gpio (with chibios type)
 * @param bit Bit in gpio to write to
 */
static inline uint32_t bb_palReadLine   (ioline_t line) {
  return bb_peri_get_bit(&(PAL_PORT(line)->IDR), PAL_PAD(line));
}


#endif // (CH_KERNEL_MAJOR > 2)    

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
 
