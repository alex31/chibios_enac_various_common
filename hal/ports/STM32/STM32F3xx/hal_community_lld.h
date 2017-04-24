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
 * @file    community STM32F3xx/stm32_registry.h
 * @brief   communitySTM32F3xx capabilities registry.
 *
 * @addtogroup HAL
 * @{
 */

#ifndef HAL_COMMUNITY_LLD_H
#define HAL_COMMUNITY_LLD_H

/**
 * @brief   Sub-family identifier.
 */

/*===========================================================================*/
/* Common features.                                                          */
/*===========================================================================*/

/*===========================================================================*/
/* Platform capabilities.                                                    */
/*===========================================================================*/


/**
 * @name    STM32F3xx capabilities
 * @{
 */
/*===========================================================================*/
/* STM32F303xC.                                                              */
/*===========================================================================*/
#if defined(STM32F303xC) || defined(__DOXYGEN__)
#define STM32_HAS_COMP1	 1
#define STM32_HAS_COMP2	 1
#define STM32_HAS_COMP3	 1
#define STM32_HAS_COMP4	 1
#define STM32_HAS_COMP5	 1
#define STM32_HAS_COMP6	 1
#define STM32_HAS_COMP7	 1
#define STM32_HAS_OPAMP1 1
#define STM32_HAS_OPAMP2 1
#define STM32_HAS_OPAMP3 1
#define STM32_HAS_OPAMP4 1
#endif /* defined(STM32F303xC) */

/*===========================================================================*/
/* STM32F303xE.                                                              */
/*===========================================================================*/
#if defined(STM32F303xE)
#define STM32_HAS_COMP1	 1
#define STM32_HAS_COMP2	 1
#define STM32_HAS_COMP3	 1
#define STM32_HAS_COMP4	 1
#define STM32_HAS_COMP5	 1
#define STM32_HAS_COMP6	 1
#define STM32_HAS_COMP7	 1
#define STM32_HAS_OPAMP1 1
#define STM32_HAS_OPAMP2 1
#define STM32_HAS_OPAMP3 1
#define STM32_HAS_OPAMP4 1
#endif /* defined(STM32F303xE) */

/*===========================================================================*/
/* STM32F303x8.                                                              */
/*===========================================================================*/
#if defined(STM32F303x8)
#define STM32_HAS_COMP1	 0
#define STM32_HAS_COMP2	 1
#define STM32_HAS_COMP3	 0
#define STM32_HAS_COMP4	 1
#define STM32_HAS_COMP5	 0
#define STM32_HAS_COMP6	 1
#define STM32_HAS_COMP7	 0
#define STM32_HAS_OPAMP1 0
#define STM32_HAS_OPAMP2 1
#define STM32_HAS_OPAMP3 0
#define STM32_HAS_OPAMP4 0
#endif /* defined(STM32F303x8) */

/*===========================================================================*/
/* STM32F301x8.                                                              */
/*===========================================================================*/
#if defined(STM32F301x8)
#error "not yet implemented"
#endif /* defined(STM32F301x8) */

/*===========================================================================*/
/* STM32F302x8.                                                              */
/*===========================================================================*/
#if defined(STM32F302x8)
#error "not yet implemented"
#endif /* defined(STM32F302x8) */

/*===========================================================================*/
/* STM32F302xC.                                                              */
/*===========================================================================*/
#if defined(STM32F302xC)
#error "not yet implemented"
#endif /* defined(STM32F302xC) */

/*===========================================================================*/
/* STM32F302xE.                                                              */
/*===========================================================================*/
#if defined(STM32F302xE)
#error "not yet implemented"
#endif /* defined(STM32F302xE) */

/*===========================================================================*/
/* STM32F318x8.                                                              */
/*===========================================================================*/
#if defined(STM32F318x8)
#error "not yet implemented"
#endif /* defined(STM32F318x8) */

/*===========================================================================*/
/* STM32F328x8.                                                              */
/*===========================================================================*/
#if defined(STM32F328x8)
#define STM32_HAS_COMP1	 0
#define STM32_HAS_COMP2	 1
#define STM32_HAS_COMP3	 0
#define STM32_HAS_COMP4	 1
#define STM32_HAS_COMP5	 0
#define STM32_HAS_COMP6	 1
#define STM32_HAS_COMP7	 0
#define STM32_HAS_OPAMP1 0
#define STM32_HAS_OPAMP2 1
#define STM32_HAS_OPAMP3 0
#define STM32_HAS_OPAMP4 0
#endif /* defined(STM32F328x8) */

/*===========================================================================*/
/* STM32F358xC.                                                              */
/*===========================================================================*/
#if defined(STM32F358xC)
#define STM32_HAS_COMP1	 1
#define STM32_HAS_COMP2	 1
#define STM32_HAS_COMP3	 1
#define STM32_HAS_COMP4	 1
#define STM32_HAS_COMP5	 1
#define STM32_HAS_COMP6	 1
#define STM32_HAS_COMP7	 1
#define STM32_HAS_OPAMP1 1
#define STM32_HAS_OPAMP2 1
#define STM32_HAS_OPAMP3 1
#define STM32_HAS_OPAMP4 1
#endif /* defined(STM32F358xC) */

/*===========================================================================*/
/* STM32F334x8.                                                              */
/*===========================================================================*/
#if defined(STM32F334x8)
#error "not yet implemented"
#endif /* defined(STM32F334x8) */

/*===========================================================================*/
/* STM32F398xx.                                                              */
/*===========================================================================*/
#if defined(STM32F398xx)
#define STM32_HAS_COMP1	 1
#define STM32_HAS_COMP2	 1
#define STM32_HAS_COMP3	 1
#define STM32_HAS_COMP4	 1
#define STM32_HAS_COMP5	 1
#define STM32_HAS_COMP6	 1
#define STM32_HAS_COMP7	 1
#define STM32_HAS_OPAMP1 1
#define STM32_HAS_OPAMP2 1
#define STM32_HAS_OPAMP3 1
#define STM32_HAS_OPAMP4 1
#endif /* defined(STM32F398xx) */

/*===========================================================================*/
/* COMPARATOR CONSTANTS							     */
/*===========================================================================*/


/* SEE stm32f303xe.h @1889 for defined macros  */
#if defined(STM32F398xx)
#define STM32_HAS_COMP1	1
#define STM32_HAS_COMP2	1
#define STM32_HAS_COMP3	1
#define STM32_HAS_COMP4	1
#define STM32_HAS_COMP5	1
#define STM32_HAS_COMP6	1
#define STM32_HAS_COMP7	1
#define STM32_HAS_OPAMP1 1
#define STM32_HAS_OPAMP2 1
#define STM32_HAS_OPAMP3 1
#define STM32_HAS_OPAMP4 1
#endif /* defined(STM32F398xx) */

// this is common for all 7 comparators
#define COMP_CSR_COMPxIN_1VREF_DIV_4	  (0b0000 << 4)
#define COMP_CSR_COMPxIN_1VREF_DIV_2	  (0b0001 << 4)
#define COMP_CSR_COMPxIN_3VREF_DIV_4	  (0b0010 << 4)
#define COMP_CSR_COMPxIN_VREF		  (0b0011 << 4)
#define COMP_CSR_COMPxIN_PA04_OR_DAC1_CH1  (0b0100 << 4)
#define COMP_CSR_COMPxIN_PA05_OR_DAC1_CH2  (0b0101 << 4)

// this is singular for each comparator
#if STM32_HAS_COMP1
#define COMP_CSR_COMP1IN_PA00		  (0b0110 << 4)
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
#define COMP_CSR_COMP1_BLANKING_NOBLANK				(0b000 << 18)
#define COMP_CSR_COMP1_BLANKING_TIM1_OC5			(0b001 << 18)
#define COMP_CSR_COMP1_BLANKING_TIM2_OC3			(0b010 << 18)
#define COMP_CSR_COMP1_BLANKING_TIM3_OC3			(0b011 << 18)
#endif

#if STM32_HAS_COMP2
#define COMP_CSR_COMP2IN_PA02		  (0b0110 << 4)
#define COMP_CSR_COMP2IN_DAC2_CH1	  (0b1000 << 4)
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
#define COMP_CSR_COMP2_BLANKING_NOBLANK				(0b000 << 18)
#define COMP_CSR_COMP2_BLANKING_TIM1_OC5			(0b001 << 18)
#define COMP_CSR_COMP2_BLANKING_TIM2_OC3			(0b010 << 18)
#define COMP_CSR_COMP2_BLANKING_TIM3_OC3			(0b011 << 18)
#endif

#if STM32_HAS_COMP3
#define COMP_CSR_COMP3IN_PD15		  (0b0110 << 4)
#define COMP_CSR_COMP3IN_PB12		  (0b0111 << 4)
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
#define COMP_CSR_COMP3_BLANKING_NOBLANK				(0b000 << 18)
#define COMP_CSR_COMP3_BLANKING_TIM1_OC5			(0b001 << 18)
#define COMP_CSR_COMP3_BLANKING_TIM2_OC4			(0b011 << 18)
#endif


#if STM32_HAS_COMP4
#define COMP_CSR_COMP4IN_PE08		  (0b0110 << 4)
#define COMP_CSR_COMP4IN_PB02		  (0b0111 << 4)
#define COMP_CSR_COMP4IN_DAC2_CH1	  (0b1000 << 4)
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
#define COMP_CSR_COMP4_BLANKING_NOBLANK				(0b000 << 18)
#define COMP_CSR_COMP4_BLANKING_TIM3_OC4			(0b001 << 18)
#define COMP_CSR_COMP4_BLANKING_TIM8_OC5			(0b010 << 18)
#define COMP_CSR_COMP4_BLANKING_TIM15_OC1			(0b011 << 18)
#endif


#if STM32_HAS_COMP5
#define COMP_CSR_COMP5IN_PD13		  (0b0110 << 4)
#define COMP_CSR_COMP5IN_PB10		  (0b0111 << 4)
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
#define COMP_CSR_COMP5_BLANKING_NOBLANK				(0b000 << 18)
#define COMP_CSR_COMP5_BLANKING_TIM8_OC5			(0b010 << 18)
#define COMP_CSR_COMP5_BLANKING_TIM3_OC3			(0b011 << 18)
#endif


#if STM32_HAS_COMP6
#define COMP_CSR_COMP6IN_PD10		  (0b0110 << 4)
#define COMP_CSR_COMP6IN_PB15		  (0b0111 << 4)
#define COMP_CSR_COMP6IN_DAC2_CH1	  (0b1000 << 4)
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
#define COMP_CSR_COMP6_BLANKING_NOBLANK				(0b000 << 18)
#define COMP_CSR_COMP6_BLANKING_TIM8_OC5			(0b010 << 18)
#define COMP_CSR_COMP6_BLANKING_TIM2_OC4			(0b011 << 18)
#define COMP_CSR_COMP6_BLANKING_TIM15_OC2			(0b100 << 18)
#endif


#if STM32_HAS_COMP7
#define COMP_CSR_COMP7IN_PC00		  (0b0110 << 4)
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
#define COMP_CSR_COMP7_BLANKING_NOBLANK				(0b000 << 18)
#define COMP_CSR_COMP7_BLANKING_TIM1_OC5			(0b001 << 18)
#define COMP_CSR_COMP7_BLANKING_TIM8_OC5			(0b010 << 18)
#define COMP_CSR_COMP7_BLANKING_TIM15_OC2			(0b100 << 18)
#endif


// this is common for all 4 operationnal amplifier
#define OPAMP_CSR_OPAMPx_CALSEL_3P3	  (0b00 << 12)
#define OPAMP_CSR_OPAMPx_CALSEL_10	  (0b01 << 12)
#define OPAMP_CSR_OPAMPx_CALSEL_50	  (0b10 << 12)
#define OPAMP_CSR_OPAMPx_CALSEL_90	  (0b11 << 12)

_Static_assert(OPAMP_CSR_OPAMPx_CALSEL_10 == OPAMP1_CSR_CALSEL_0,
	       "opamp macro definition mismatch");

#define OPAMP_CSR_OPAMPx_PGAGAIN_2		(0b00 << 14)
#define OPAMP_CSR_OPAMPx_PGAGAIN_4	  	(0b01 << 14)
#define OPAMP_CSR_OPAMPx_PGAGAIN_8	  	(0b10 << 14)
#define OPAMP_CSR_OPAMPx_PGAGAIN_16	  	(0b11 << 14)
#define OPAMP_CSR_OPAMPx_PGAGAIN_LESS_IFB_VM0   (0b10 << 16)
#define OPAMP_CSR_OPAMPx_PGAGAIN_LESS_IFB_VM1   (0b11 << 16)
#define OPAMP_CSR_OPAMPx_TRIMOFFSETN_POS	(24U) 
#define OPAMP_CSR_OPAMPx_TRIMOFFSETP_POS	(19U) 



#if STM32_HAS_OPAMP1
#define OPAMP_CSR_OPAMP1_VPSEL_PA07				(0b00 << 2)
#define OPAMP_CSR_OPAMP1_VPSEL_PA05				(0b01 << 2)
#define OPAMP_CSR_OPAMP1_VPSEL_PA03				(0b10 << 2)
#define OPAMP_CSR_OPAMP1_VPSEL_PA01				(0b11 << 2)

#define OPAMP_CSR_OPAMP1_VMSEL_PC05				(0b00 << 5)
#define OPAMP_CSR_OPAMP1_VMSEL_PA03				(0b01 << 5)
#define OPAMP_CSR_OPAMP1_VMSEL_RFOPGA				(0b10 << 5)
#define OPAMP_CSR_OPAMP1_VMSEL_FOLWR				(0b11 << 5)

#define OPAMP_CSR_OPAMP1_VMSSEL_PC05				(0b0 << 8)
#define OPAMP_CSR_OPAMP1_VMSSEL_PA03				(0b1 << 8)

#define OPAMP_CSR_OPAMP1_VPSSEL_PA07				(0b00 << 9)
#define OPAMP_CSR_OPAMP1_VPSSEL_PA05				(0b01 << 9)
#define OPAMP_CSR_OPAMP1_VPSSEL_PA03				(0b10 << 9)
#define OPAMP_CSR_OPAMP1_VPSSEL_PA01				(0b11 << 9)
#endif


#if STM32_HAS_OPAMP2
#define OPAMP_CSR_OPAMP2_VPSEL_PD14				(0b00 << 2)
#define OPAMP_CSR_OPAMP2_VPSEL_PB14				(0b01 << 2)
#define OPAMP_CSR_OPAMP2_VPSEL_PB00				(0b10 << 2)
#define OPAMP_CSR_OPAMP2_VPSEL_PA07				(0b11 << 2)

#define OPAMP_CSR_OPAMP2_VMSEL_PC05				(0b00 << 5)
#define OPAMP_CSR_OPAMP2_VMSEL_PA05				(0b01 << 5)
#define OPAMP_CSR_OPAMP2_VMSEL_RFOPGA				(0b10 << 5)
#define OPAMP_CSR_OPAMP2_VMSEL_FOLWR				(0b11 << 5)

#define OPAMP_CSR_OPAMP2_VMSSEL_PC05				(0b0 << 8)
#define OPAMP_CSR_OPAMP2_VMSSEL_PA05				(0b1 << 8)

#define OPAMP_CSR_OPAMP2_VPSSEL_PD14				(0b00 << 9)
#define OPAMP_CSR_OPAMP2_VPSSEL_PB14				(0b01 << 9)
#define OPAMP_CSR_OPAMP2_VPSSEL_PB00				(0b10 << 9)
#define OPAMP_CSR_OPAMP2_VPSSEL_PA07				(0b11 << 9)
#endif

#if STM32_HAS_OPAMP3
#define OPAMP_CSR_OPAMP3_VPSEL_PB13				(0b00 << 2)
#define OPAMP_CSR_OPAMP3_VPSEL_PA05				(0b01 << 2)
#define OPAMP_CSR_OPAMP3_VPSEL_PA01				(0b10 << 2)
#define OPAMP_CSR_OPAMP3_VPSEL_PB00				(0b11 << 2)

#define OPAMP_CSR_OPAMP3_VMSEL_PB10				(0b00 << 5)
#define OPAMP_CSR_OPAMP3_VMSEL_PB02				(0b01 << 5)
#define OPAMP_CSR_OPAMP3_VMSEL_RFOPGA				(0b10 << 5)
#define OPAMP_CSR_OPAMP3_VMSEL_FOLWR				(0b11 << 5)

#define OPAMP_CSR_OPAMP3_VMSSEL_PB10				(0b0 << 8)
#define OPAMP_CSR_OPAMP3_VMSSEL_PB02				(0b1 << 8)

#define OPAMP_CSR_OPAMP3_VPSSEL_PB13				(0b00 << 9)
#define OPAMP_CSR_OPAMP3_VPSSEL_PA05				(0b01 << 9)
#define OPAMP_CSR_OPAMP3_VPSSEL_PA01				(0b10 << 9)
#define OPAMP_CSR_OPAMP3_VPSSEL_PB00				(0b11 << 9)
#endif

#if STM32_HAS_OPAMP4
#define OPAMP_CSR_OPAMP4_VPSEL_PD11				(0b00 << 2)
#define OPAMP_CSR_OPAMP4_VPSEL_PB11				(0b01 << 2)
#define OPAMP_CSR_OPAMP4_VPSEL_PA04				(0b10 << 2)
#define OPAMP_CSR_OPAMP4_VPSEL_PB13				(0b11 << 2)

#define OPAMP_CSR_OPAMP4_VMSEL_PB10				(0b00 << 5)
#define OPAMP_CSR_OPAMP4_VMSEL_PD08				(0b01 << 5)
#define OPAMP_CSR_OPAMP4_VMSEL_RFOPGA				(0b10 << 5)
#define OPAMP_CSR_OPAMP4_VMSEL_FOLWR				(0b11 << 5)

#define OPAMP_CSR_OPAMP4_VMSSEL_PB10				(0b0 << 8)
#define OPAMP_CSR_OPAMP4_VMSSEL_PD08				(0b1 << 8)

#define OPAMP_CSR_OPAMP4_VPSSEL_PD11				(0b00 << 9)
#define OPAMP_CSR_OPAMP4_VPSSEL_PB11				(0b01 << 9)
#define OPAMP_CSR_OPAMP4_VPSSEL_PA04				(0b10 << 9)
#define OPAMP_CSR_OPAMP4_VPSSEL_PB13				(0b11 << 9)
#endif




/** @} */

#endif /* HAL_COMMUNITY_LLD_H */

/** @} */
