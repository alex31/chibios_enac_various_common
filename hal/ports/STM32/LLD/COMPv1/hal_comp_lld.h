/*
    ChibiOS - Copyright (C) 2006..2016 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @file    COMPv1/hal_comp_lld.h
 * @brief   STM32 COMP subsystem low level driver header.
 *
 * @addtogroup COMP
 * @{
 */

#ifndef HAL_COMP_LLD_H
#define HAL_COMP_LLD_H

#include "stm32_community_registry.h"

#if HAL_USE_COMP || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/* SEE stm32f303xe.h @1889 for defined macros  */

// /**********************  Bit definition for COMPx_CSR register  ***************/
// #define COMPx_CSR_COMPxEN               ((uint32_t)0x00000001) /*!< COMPx enable */
// #define COMPx_CSR_COMPxSW1              ((uint32_t)0x00000002) /*!< COMPx SW1 switch control */
// #define COMPx_CSR_COMPxINSEL            ((uint32_t)0x00000070) /*!< COMPx inverting input select */
// #define COMPx_CSR_COMPxINSEL_0          ((uint32_t)0x00000010) /*!< COMPx inverting input select bit 0 */
// #define COMPx_CSR_COMPxINSEL_1          ((uint32_t)0x00000020) /*!< COMPx inverting input select bit 1 */
// #define COMPx_CSR_COMPxINSEL_2          ((uint32_t)0x00000040) /*!< COMPx inverting input select bit 2 */
// #define COMPx_CSR_COMPxOUTSEL           ((uint32_t)0x00003C00) /*!< COMPx output select */
// #define COMPx_CSR_COMPxOUTSEL_0         ((uint32_t)0x00000400) /*!< COMPx output select bit 0 */
// #define COMPx_CSR_COMPxOUTSEL_1         ((uint32_t)0x00000800) /*!< COMPx output select bit 1 */
// #define COMPx_CSR_COMPxOUTSEL_2         ((uint32_t)0x00001000) /*!< COMPx output select bit 2 */
// #define COMPx_CSR_COMPxOUTSEL_3         ((uint32_t)0x00002000) /*!< COMPx output select bit 3 */
// #define COMPx_CSR_COMPxPOL              ((uint32_t)0x00008000) /*!< COMPx output polarity */
// #define COMPx_CSR_COMPxBLANKING         ((uint32_t)0x000C0000) /*!< COMPx blanking */
// #define COMPx_CSR_COMPxBLANKING_0       ((uint32_t)0x00040000) /*!< COMPx blanking bit 0 */
// #define COMPx_CSR_COMPxBLANKING_1       ((uint32_t)0x00080000) /*!< COMPx blanking bit 1 */
// #define COMPx_CSR_COMPxBLANKING_2       ((uint32_t)0x00100000) /*!< COMPx blanking bit 2 */
// #define COMPx_CSR_COMPxOUT              ((uint32_t)0x40000000) /*!< COMPx output level */
// #define COMPx_CSR_COMPxLOCK             ((uint32_t)0x80000000) /*!< COMPx lock */

// /**********************  Bit definition for COMP_CSR register  ****************/
// #define COMP_CSR_COMPxEN               ((uint32_t)0x00000001) /*!< COMPx enable */
// #define COMP_CSR_COMPxINSEL            ((uint32_t)0x00400070) /*!< COMPx inverting input select */
// #define COMP_CSR_COMPxINSEL_0          ((uint32_t)0x00000010) /*!< COMPx inverting input select bit 0 */
// #define COMP_CSR_COMPxINSEL_1          ((uint32_t)0x00000020) /*!< COMPx inverting input select bit 1 */
// #define COMP_CSR_COMPxINSEL_2          ((uint32_t)0x00000040) /*!< COMPx inverting input select bit 2 */
// #define COMP_CSR_COMPxINSEL_3          ((uint32_t)0x00400000) /*!< COMPx inverting input select bit 3 */
// #define COMP_CSR_COMPxWNDWEN           ((uint32_t)0x00000200) /*!< COMPx window mode enable */
// #define COMP_CSR_COMPxOUTSEL           ((uint32_t)0x00003C00) /*!< COMPx output select */
// #define COMP_CSR_COMPxOUTSEL_0         ((uint32_t)0x00000400) /*!< COMPx output select bit 0 */
// #define COMP_CSR_COMPxOUTSEL_1         ((uint32_t)0x00000800) /*!< COMPx output select bit 1 */
// #define COMP_CSR_COMPxOUTSEL_2         ((uint32_t)0x00001000) /*!< COMPx output select bit 2 */
// #define COMP_CSR_COMPxOUTSEL_3         ((uint32_t)0x00002000) /*!< COMPx output select bit 3 */
// #define COMP_CSR_COMPxPOL              ((uint32_t)0x00008000) /*!< COMPx output polarity */
// #define COMP_CSR_COMPxBLANKING         ((uint32_t)0x000C0000) /*!< COMPx blanking */
// #define COMP_CSR_COMPxBLANKING_0       ((uint32_t)0x00040000) /*!< COMPx blanking bit 0 */
// #define COMP_CSR_COMPxBLANKING_1       ((uint32_t)0x00080000) /*!< COMPx blanking bit 1 */
// #define COMP_CSR_COMPxBLANKING_2       ((uint32_t)0x00100000) /*!< COMPx blanking bit 2 */
// #define COMP_CSR_COMPxOUT              ((uint32_t)0x40000000) /*!< COMPx output level */
// #define COMP_CSR_COMPxLOCK             ((uint32_t)0x80000000) /*!< COMPx lock */

// this is common for all 7 comparators
#define COMP_CSR_COMPxIN_1VREF_DIV_4	  (0b0000 << 4)
#define COMP_CSR_COMPxIN_1VREF_DIV_2	  (0b0001 << 4)
#define COMP_CSR_COMPxIN_3VREF_DIV_4	  (0b0010 << 4)
#define COMP_CSR_COMPxIN_VREF		  (0b0011 << 4)
#define COMP_CSR_COMPxIN_PA04_OR_DAC1_CH1  (0b0100 << 4)
#define COMP_CSR_COMPxIN_PA05_OR_DAC1_CH2  (0b0101 << 4)

// this is singular for each comparator
#define COMP_CSR_COMP1IN_PA00		  (0b0110 << 4)

#define COMP_CSR_COMP2IN_PA02		  (0b0110 << 4)
#define COMP_CSR_COMP2IN_DAC2_CH1	  (0b1000 << 4)

#define COMP_CSR_COMP3IN_PD15		  (0b0110 << 4)
#define COMP_CSR_COMP3IN_PB12		  (0b0111 << 4)

#define COMP_CSR_COMP4IN_PE08		  (0b0110 << 4)
#define COMP_CSR_COMP4IN_PB02		  (0b0111 << 4)
#define COMP_CSR_COMP4IN_DAC2_CH1	  (0b1000 << 4)

#define COMP_CSR_COMP5IN_PD13		  (0b0110 << 4)
#define COMP_CSR_COMP5IN_PB10		  (0b0111 << 4)

#define COMP_CSR_COMP6IN_PD10		  (0b0110 << 4)
#define COMP_CSR_COMP6IN_PB15		  (0b0111 << 4)
#define COMP_CSR_COMP6IN_DAC2_CH1	  (0b1000 << 4)

#define COMP_CSR_COMP7IN_PC00		  (0b0110 << 4)

#define COMP_CSR_COMP1OUT_NOSEL					(0b0000 << 10)
#define COMP_CSR_COMP1OUT_TIM1_BK1				(0b0001 << 10)
#define COMP_CSR_COMP1OUT_TIM1_BK2				(0b0010 << 10)
#define COMP_CSR_COMP1OUT_TIM8_BK1				(0b0011 << 10)
#define COMP_CSR_COMP1OUT_TIM8_BK2				(0b0100 << 10)
#define COMP_CSR_COMP1OUT_TIM1_BK2_TIM8_BK2			(0b0101 << 10)
#define COMP_CSR_COMP1OUT_TIM1_OCREFCLR				(0b011O << 10)
#define COMP_CSR_COMP1OUT_TIM1_IC1				(0b0111 << 10)
#define COMP_CSR_COMP1OUT_TIM2_IC4				(0b1000 << 10)
#define COMP_CSR_COMP1OUT_TIM2_OCREFCLR				(0b1001 << 10)
#define COMP_CSR_COMP1OUT_TIM3_IC1				(0b1010 << 10)
#define COMP_CSR_COMP1OUT_TIM3_OCREFCLR				(0b1011 << 10)
#define COMP_CSR_COMP1OUT_TIM20_BK1				(0b1100 << 10)
#define COMP_CSR_COMP1OUT_TIM20_BK2				(0b1101 << 10)
#define COMP_CSR_COMP1OUT_TIM1_BK2_TIM8_BK2_TIM20_BK2		(0b1110 << 10)

#define COMP_CSR_COMP2OUT_NOSEL					(0b0000 << 10)
#define COMP_CSR_COMP2OUT_TIM1_BK1				(0b0001 << 10)
#define COMP_CSR_COMP2OUT_TIM1_BK2				(0b0010 << 10)
#define COMP_CSR_COMP2OUT_TIM8_BK1				(0b0011 << 10)
#define COMP_CSR_COMP2OUT_TIM8_BK2				(0b0100 << 10)
#define COMP_CSR_COMP2OUT_TIM1_BK2_TIM8_BK2			(0b0101 << 10)
#define COMP_CSR_COMP2OUT_TIM1_OCREFCLR				(0b011O << 10)
#define COMP_CSR_COMP2OUT_TIM1_IC1				(0b0111 << 10)
#define COMP_CSR_COMP2OUT_TIM2_IC4				(0b1000 << 10)
#define COMP_CSR_COMP2OUT_TIM2_OCREFCLR				(0b1001 << 10)
#define COMP_CSR_COMP2OUT_TIM3_IC1				(0b1010 << 10)
#define COMP_CSR_COMP2OUT_TIM3_OCREFCLR				(0b1011 << 10)
#define COMP_CSR_COMP2OUT_TIM20_BK1				(0b1100 << 10)
#define COMP_CSR_COMP2OUT_TIM20_BK2				(0b1101 << 10)
#define COMP_CSR_COMP2OUT_TIM1_BK2_TIM8_BK2_TIM20_BK2		(0b1110 << 10)
#define COMP_CSR_COMP2OUT_TIM20_OCREFCLR			(0b1111 << 10)

#define COMP_CSR_COMP3OUT_NOSEL					(0b0000 << 10)
#define COMP_CSR_COMP3OUT_TIM1_BK1				(0b0001 << 10)
#define COMP_CSR_COMP3OUT_TIM1_BK2				(0b0010 << 10)
#define COMP_CSR_COMP3OUT_TIM8_BK1				(0b0011 << 10)
#define COMP_CSR_COMP3OUT_TIM8_BK2				(0b0100 << 10)
#define COMP_CSR_COMP3OUT_TIM1_BK2_TIM8_BK2			(0b0101 << 10)
#define COMP_CSR_COMP3OUT_TIM1_OCREFCLR				(0b011O << 10)
#define COMP_CSR_COMP3OUT_TIM4_IC1				(0b0111 << 10)
#define COMP_CSR_COMP3OUT_TIM3_IC2				(0b1000 << 10)
#define COMP_CSR_COMP3OUT_TIM2_OCREFCLR				(0b1001 << 10)
#define COMP_CSR_COMP3OUT_TIM15_IC1				(0b1010 << 10)
#define COMP_CSR_COMP3OUT_TIM15_BK1				(0b1011 << 10)
#define COMP_CSR_COMP3OUT_TIM20_BK1				(0b1100 << 10)
#define COMP_CSR_COMP3OUT_TIM20_BK2				(0b1101 << 10)
#define COMP_CSR_COMP3OUT_TIM1_BK2_TIM8_BK2_TIM20_BK2		(0b1110 << 10)

#define COMP_CSR_COMP4OUT_NOSEL					(0b0000 << 10)
#define COMP_CSR_COMP4OUT_TIM1_BK1				(0b0001 << 10)
#define COMP_CSR_COMP4OUT_TIM1_BK2				(0b0010 << 10)
#define COMP_CSR_COMP4OUT_TIM8_BK1				(0b0011 << 10)
#define COMP_CSR_COMP4OUT_TIM8_BK2				(0b0100 << 10)
#define COMP_CSR_COMP4OUT_TIM1_BK2_TIM8_BK2			(0b0101 << 10)
#define COMP_CSR_COMP4OUT_TIM3_IC3				(0b011O << 10)
#define COMP_CSR_COMP4OUT_TIM8_OCREFCLR				(0b0111 << 10)
#define COMP_CSR_COMP4OUT_TIM15_IC2				(0b1000 << 10)
#define COMP_CSR_COMP4OUT_TIM4_IC2				(0b1001 << 10)
#define COMP_CSR_COMP4OUT_TIM15_OCREFCLR			(0b1010 << 10)
#define COMP_CSR_COMP4OUT_TIM3_OCREFCLR				(0b1011 << 10)
#define COMP_CSR_COMP4OUT_TIM20_BK1				(0b1100 << 10)
#define COMP_CSR_COMP4OUT_TIM20_BK2				(0b1101 << 10)
#define COMP_CSR_COMP4OUT_TIM1_BK2_TIM8_BK2_TIM20_BK2		(0b1110 << 10)

#define COMP_CSR_COMP5OUT_NOSEL					(0b0000 << 10)
#define COMP_CSR_COMP5OUT_TIM1_BK1				(0b0001 << 10)
#define COMP_CSR_COMP5OUT_TIM1_BK2				(0b0010 << 10)
#define COMP_CSR_COMP5OUT_TIM8_BK1				(0b0011 << 10)
#define COMP_CSR_COMP5OUT_TIM8_BK2				(0b0100 << 10)
#define COMP_CSR_COMP5OUT_TIM1_BK2_TIM8_BK2			(0b0101 << 10)
#define COMP_CSR_COMP5OUT_TIM2_IC1				(0b011O << 10)
#define COMP_CSR_COMP5OUT_TIM8_OCREFCLR				(0b0111 << 10)
#define COMP_CSR_COMP5OUT_TIM17_IC1				(0b1000 << 10)
#define COMP_CSR_COMP5OUT_TIM4_IC3				(0b1001 << 10)
#define COMP_CSR_COMP5OUT_TIM16_BK1				(0b1010 << 10)
#define COMP_CSR_COMP5OUT_TIM3_OCREFCLR				(0b1011 << 10)
#define COMP_CSR_COMP5OUT_TIM20_BK1				(0b1100 << 10)
#define COMP_CSR_COMP5OUT_TIM20_BK2				(0b1101 << 10)
#define COMP_CSR_COMP5OUT_TIM1_BK2_TIM8_BK2_TIM20_BK2		(0b1110 << 10)

#define COMP_CSR_COMP6OUT_NOSEL					(0b0000 << 10)
#define COMP_CSR_COMP6OUT_TIM1_BK1				(0b0001 << 10)
#define COMP_CSR_COMP6OUT_TIM1_BK2				(0b0010 << 10)
#define COMP_CSR_COMP6OUT_TIM8_BK1				(0b0011 << 10)
#define COMP_CSR_COMP6OUT_TIM8_BK2				(0b0100 << 10)
#define COMP_CSR_COMP6OUT_TIM1_BK2_TIM8_BK2			(0b0101 << 10)
#define COMP_CSR_COMP6OUT_TIM2_IC2				(0b011O << 10)
#define COMP_CSR_COMP6OUT_TIM8_OCREFCLR				(0b0111 << 10)
#define COMP_CSR_COMP6OUT_TIM2_OCREFCLR				(0b1000 << 10)
#define COMP_CSR_COMP6OUT_TIM16_OCREFCLR			(0b1001 << 10)
#define COMP_CSR_COMP6OUT_TIM16_IC1				(0b1010 << 10)
#define COMP_CSR_COMP6OUT_TIM4_IC4				(0b1011 << 10)
#define COMP_CSR_COMP6OUT_TIM20_BK1				(0b1100 << 10)
#define COMP_CSR_COMP6OUT_TIM20_BK2				(0b1101 << 10)
#define COMP_CSR_COMP6OUT_TIM1_BK2_TIM8_BK2_TIM20_BK2		(0b1110 << 10)

#define COMP_CSR_COMP7OUT_NOSEL					(0b0000 << 10)
#define COMP_CSR_COMP7OUT_TIM1_BK1				(0b0001 << 10)
#define COMP_CSR_COMP7OUT_TIM1_BK2				(0b0010 << 10)
#define COMP_CSR_COMP7OUT_TIM8_BK1				(0b0011 << 10)
#define COMP_CSR_COMP7OUT_TIM8_BK2				(0b0100 << 10)
#define COMP_CSR_COMP7OUT_TIM1_BK2_TIM8_BK2			(0b0101 << 10)
#define COMP_CSR_COMP7OUT_TIM1_OCREFCLR				(0b011O << 10)
#define COMP_CSR_COMP7OUT_TIM8_OCREFCLR				(0b0111 << 10)
#define COMP_CSR_COMP7OUT_TIM2_IC3				(0b1000 << 10)
#define COMP_CSR_COMP7OUT_TIM1_IC2				(0b1001 << 10)
#define COMP_CSR_COMP7OUT_TIM17_OCREFCLR			(0b1010 << 10)
#define COMP_CSR_COMP7OUT_TIM17_BK1				(0b1011 << 10)
#define COMP_CSR_COMP7OUT_TIM20_BK1				(0b1100 << 10)
#define COMP_CSR_COMP7OUT_TIM20_BK2				(0b1101 << 10)
#define COMP_CSR_COMP7OUT_TIM1_BK2_TIM8_BK2_TIM20_BK2		(0b1110 << 10)


#define COMP_CSR_COMP1_BLANKING_NOBLANK				(0b000 << 18)
#define COMP_CSR_COMP1_BLANKING_TIM1_OC5			(0b001 << 18)
#define COMP_CSR_COMP1_BLANKING_TIM2_OC3			(0b010 << 18)
#define COMP_CSR_COMP1_BLANKING_TIM3_OC3			(0b011 << 18)

#define COMP_CSR_COMP2_BLANKING_NOBLANK				(0b000 << 18)
#define COMP_CSR_COMP2_BLANKING_TIM1_OC5			(0b001 << 18)
#define COMP_CSR_COMP2_BLANKING_TIM2_OC3			(0b010 << 18)
#define COMP_CSR_COMP2_BLANKING_TIM3_OC3			(0b011 << 18)

#define COMP_CSR_COMP3_BLANKING_NOBLANK				(0b000 << 18)
#define COMP_CSR_COMP3_BLANKING_TIM1_OC5			(0b001 << 18)
#define COMP_CSR_COMP3_BLANKING_TIM2_OC4			(0b011 << 18)

#define COMP_CSR_COMP4_BLANKING_NOBLANK				(0b000 << 18)
#define COMP_CSR_COMP4_BLANKING_TIM3_OC4			(0b001 << 18)
#define COMP_CSR_COMP4_BLANKING_TIM8_OC5			(0b010 << 18)
#define COMP_CSR_COMP4_BLANKING_TIM15_OC1			(0b011 << 18)

#define COMP_CSR_COMP5_BLANKING_NOBLANK				(0b000 << 18)
#define COMP_CSR_COMP5_BLANKING_TIM8_OC5			(0b010 << 18)
#define COMP_CSR_COMP5_BLANKING_TIM3_OC3			(0b011 << 18)

#define COMP_CSR_COMP6_BLANKING_NOBLANK				(0b000 << 18)
#define COMP_CSR_COMP6_BLANKING_TIM8_OC5			(0b010 << 18)
#define COMP_CSR_COMP6_BLANKING_TIM2_OC4			(0b011 << 18)
#define COMP_CSR_COMP6_BLANKING_TIM15_OC2			(0b100 << 18)

#define COMP_CSR_COMP7_BLANKING_NOBLANK				(0b000 << 18)
#define COMP_CSR_COMP7_BLANKING_TIM1_OC5			(0b001 << 18)
#define COMP_CSR_COMP7_BLANKING_TIM8_OC5			(0b010 << 18)
#define COMP_CSR_COMP7_BLANKING_TIM15_OC2			(0b100 << 18)






/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */

/**
 * @brief   COMPD1 driver enable switch.
 * @details If set to @p TRUE the support for COMPD1 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(STM32_COMP_USE_COMP1) || defined(__DOXYGEN__)
#define STM32_COMP_USE_COMP1                  FALSE
#endif

/**
 * @brief   COMPD2 driver enable switch.
 * @details If set to @p TRUE the support for COMPD2 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(STM32_COMP_USE_COMP2) || defined(__DOXYGEN__)
#define STM32_COMP_USE_COMP2                  FALSE
#endif

/**
 * @brief   COMPD3 driver enable switch.
 * @details If set to @p TRUE the support for COMPD3 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(STM32_COMP_USE_COMP3) || defined(__DOXYGEN__)
#define STM32_COMP_USE_COMP3                  FALSE
#endif

/**
 * @brief   COMPD4 driver enable switch.
 * @details If set to @p TRUE the support for COMPD4 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(STM32_COMP_USE_COMP4) || defined(__DOXYGEN__)
#define STM32_COMP_USE_COMP4                  FALSE
#endif

/**
 * @brief   COMPD5 driver enable switch.
 * @details If set to @p TRUE the support for COMPD5 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(STM32_COMP_USE_COMP5) || defined(__DOXYGEN__)
#define STM32_COMP_USE_COMP5                  FALSE
#endif

/**
 * @brief   COMPD8 driver enable switch.
 * @details If set to @p TRUE the support for COMPD8 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(STM32_COMP_USE_COMP6) || defined(__DOXYGEN__)
#define STM32_COMP_USE_COMP6                  FALSE
#endif

/**
 * @brief   COMPD9 driver enable switch.
 * @details If set to @p TRUE the support for COMPD9 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(STM32_COMP_USE_COMP6) || defined(__DOXYGEN__)
#define STM32_COMP_USE_COMP6                  FALSE
#endif

/** @} */

/*===========================================================================*/
/* Configuration checks.                                                     */
/*===========================================================================*/

#if STM32_COMP_USE_COMP1 && !STM32_HAS_COMP1
#error "COMP1 not present in the selected device"
#endif

#if STM32_COMP_USE_COMP2 && !STM32_HAS_COMP2
#error "COMP2 not present in the selected device"
#endif

#if STM32_COMP_USE_COMP3 && !STM32_HAS_COMP3
#error "COMP3 not present in the selected device"
#endif

#if STM32_COMP_USE_COMP4 && !STM32_HAS_COMP4
#error "COMP4 not present in the selected device"
#endif

#if STM32_COMP_USE_COMP5 && !STM32_HAS_COMP5
#error "COMP5 not present in the selected device"
#endif

#if STM32_COMP_USE_COMP6 && !STM32_HAS_COMP6
#error "COMP6 not present in the selected device"
#endif

#if STM32_COMP_USE_COMP6 && !STM32_HAS_COMP6
#error "COMP7 not present in the selected device"
#endif

#if !STM32_COMP_USE_COMP1 && !STM32_COMP_USE_COMP2 &&                           \
    !STM32_COMP_USE_COMP3 && !STM32_COMP_USE_COMP4 &&                           \
    !STM32_COMP_USE_COMP5 && !STM32_COMP_USE_COMP6 &&                           \
    !STM32_COMP_USE_COMP7
#error "COMP driver activated but no COMP peripheral assigned"
#endif



/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type of a COMP driver configuration structure.
 */
typedef struct {
  /**
   * @brief   Output polarity

   * @note    invert the comparator output if set to true
   *          
   */
  bool			output_inverted;
   /**
    * @brief COMP DIER register initialization data.
    * @note  The value of this field should normally be equal to zero.
    * @note  Only the DMA-related bits can be specified in this field.
    */
  uint32_t		csr;
} COMPConfig;

/**
 * @brief   Structure representing a COMP driver.
 */
struct COMPDriver {
  /**
   * @brief Driver state.
   */
  compstate_t                state;
  /**
   * @brief Current driver configuration data.
   */
  const COMPConfig           *config;
  /**
   * @brief   Current COMP period in ticks.
   */
  COMP_TypeDef                *comp;
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/


/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if STM32_COMP_USE_COMP1 && !defined(__DOXYGEN__)
extern COMPDriver COMPD1;
#endif

#if STM32_COMP_USE_COMP2 && !defined(__DOXYGEN__)
extern COMPDriver COMPD2;
#endif

#if STM32_COMP_USE_COMP3 && !defined(__DOXYGEN__)
extern COMPDriver COMPD3;
#endif

#if STM32_COMP_USE_COMP4 && !defined(__DOXYGEN__)
extern COMPDriver COMPD4;
#endif

#if STM32_COMP_USE_COMP5 && !defined(__DOXYGEN__)
extern COMPDriver COMPD5;
#endif

#if STM32_COMP_USE_COMP6 && !defined(__DOXYGEN__)
extern COMPDriver COMPD6;
#endif

#if STM32_COMP_USE_COMP7 && !defined(__DOXYGEN__)
extern COMPDriver COMPD7;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void comp_lld_init(void);
  void comp_lld_start(COMPDriver *compp);
  void comp_lld_stop(COMPDriver *compp);
  void comp_lld_enable(COMPDriver *compp);
  void comp_lld_disable(COMPDriver *compp);
#ifdef __cplusplus
}
#endif

/**
 * @brief     return output state of the comparator
 * @pre       The COMP unit must have been activated using @p compStart().
 * @param[in] compp      pointer to a @p COMPDriver object
 * @return    output state of comparator
 * @notapi
 */

static inline uint32_t comp_lld_getOutput(const COMPDriver *compp) {
  return (compp->comp->CSR | COMP_CSR_COMPxOUT) ? COMP_INV_BELOW_NONINV : COMP_NONINV_BELOW_INV;
}



#endif /* HAL_USE_COMP */

#endif /* HAL_COMP_LLD_H */

/** @} */
