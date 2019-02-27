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
 * @file    community STM32L4x2/stm32_registry.h
 * @brief   communitySTM32L4x2 capabilities registry.
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
 * @name    STM32L4x2 capabilities
 * @{
 */
/*===========================================================================*/
/* STM32L432                                                            */
/*===========================================================================*/
#if defined(STM32L432xx) || defined(__DOXYGEN__)
#define STM32_HAS_COMP1	 1
#define STM32_HAS_COMP2	 1
#define STM32_HAS_COMP3	 0
#define STM32_HAS_COMP4	 0
#define STM32_HAS_COMP5	 0
#define STM32_HAS_COMP6	 0
#define STM32_HAS_COMP7	 0
#define STM32_HAS_OPAMP1 1
#define STM32_HAS_OPAMP2 0
#define STM32_HAS_OPAMP3 0
#define STM32_HAS_OPAMP4 0
#else
#error MCU not yet implemented
#endif /* defined(STM32F303xC) */


/*===========================================================================*/
/* COMPARATOR CONSTANTS							     */
/*===========================================================================*/


// this is common for all 7 comparators
#define COMP_CSR_COMPxINM_1VREF_DIV_4	  (0b000 << 4)
#define COMP_CSR_COMPxINM_1VREF_DIV_2	  (0b001 << 4)
#define COMP_CSR_COMPxINM_3VREF_DIV_4	  (0b010 << 4)
#define COMP_CSR_COMPxINM_VREF		  (0b011 << 4)
#define COMP_CSR_COMPxINM_DAC1_CH1	  (0b100 << 4)
#define COMP_CSR_COMPxINM_DAC1_CH2	  (0b101 << 4)
#define COMP_CSR_COMPxHYSTER_NO	          (0b00 << 16)
#define COMP_CSR_COMPxHYSTER_LOW          (0b01 << 16)
#define COMP_CSR_COMPxHYSTER_MEDIUM       (0b10 << 16)
#define COMP_CSR_COMPxHYSTER_HIGH         (0b11 << 16)


// this is singular for each comparator
#if STM32_HAS_COMP1
#define COMP_CSR_COMP1INM_PB01		  (0b110 << 4)
#define COMP_CSR_COMP1INM_PC04		  ((0b111 << 4) | (0b00 << 25))
#define COMP_CSR_COMP1INM_PA00		  ((0b111 << 4) | (0b01 << 25))
#define COMP_CSR_COMP1INM_PA04		  ((0b111 << 4) | (0b10 << 25))
#define COMP_CSR_COMP1INM_PA05		  ((0b111 << 4) | (0b11 << 25))

#define COMP_CSR_COMP1INP_PC00		  (0b00 << 7) 
#define COMP_CSR_COMP1INP_PB02		  (0b01 << 7) 
#define COMP_CSR_COMP1INP_PA01		  (0b10 << 7) 
#define COMP_CSR_COMP1_BLANKING_NOBLANK	  (0b000 << 18)
#define COMP_CSR_COMP1_BLANKING_TIM1_OC5  (0b100 << 18)
#endif

#if STM32_HAS_COMP2
#define COMP_CSR_COMP2INM_PB03		  (0b110 << 4)
#define COMP_CSR_COMP2INM_PB07		  ((0b111 << 4) | (0b00 << 25))
#define COMP_CSR_COMP2INM_PA02		  ((0b111 << 4) | (0b01 << 25))
#define COMP_CSR_COMP2INM_PA04		  ((0b111 << 4) | (0b10 << 25))
#define COMP_CSR_COMP2INM_PA05		  ((0b111 << 4) | (0b11 << 25))

#define COMP_CSR_COMP2INP_PB04		  (0b00 << 7) 
#define COMP_CSR_COMP2INP_PB06		  (0b01 << 7) 
#define COMP_CSR_COMP2INP_PA03		  (0b10 << 7) 
#define COMP_CSR_COMP2_BLANKING_NOBLANK	  (0b000 << 18)
#define COMP_CSR_COMP2_BLANKING_TIM15_OC1 (0b100 << 18)
#endif





#if STM32_HAS_OPAMP1
#define OPAMP_CSR_OPAMP1_OPAMODE_PGADISABLE			(0b00 << 2)
#define OPAMP_CSR_OPAMP1_OPAMODE_PGAENABLE			(0b10 << 2)
#define OPAMP_CSR_OPAMP1_OPAMODE_FOLLOWER			(0b11 << 2)

#define OPAMP_CSR_OPAMP1_PGAGAIN_2				(0b00 << 4)
#define OPAMP_CSR_OPAMP1_PGAGAIN_4	  			(0b01 << 4)
#define OPAMP_CSR_OPAMP1_PGAGAIN_8	  			(0b10 << 4)
#define OPAMP_CSR_OPAMP1_PGAGAIN_16	  			(0b11 << 4)

#define OPAMP_CSR_OPAMP1_VMSEL_GPIO				(0b00 << 8)
#define OPAMP_CSR_OPAMP1_VMSEL_PGA				(0b10 << 5)

#define OPAMP_CSR_OPAMP1_VMSSEL_PC05				(0b0 << 8)
#define OPAMP_CSR_OPAMP1_VMSSEL_PA03				(0b1 << 8)

#define OPAMP_CSR_OPAMP1_VPSSEL_PA07				(0b00 << 9)
#define OPAMP_CSR_OPAMP1_VPSSEL_PA05				(0b01 << 9)
#define OPAMP_CSR_OPAMP1_VPSSEL_PA03				(0b10 << 9)
#define OPAMP_CSR_OPAMP1_VPSSEL_PA01				(0b11 << 9)


#define OPAMP_CSR_OPAMP1_CALSEL_3P3	  (0b00 << 12)
#define OPAMP_CSR_OPAMP1_CALSEL_10	  (0b01 << 12)
#define OPAMP_CSR_OPAMP1_CALSEL_50	  (0b10 << 12)
#define OPAMP_CSR_OPAMP1_CALSEL_90	  (0b11 << 12)


#endif



/** @} */

#endif /* HAL_COMMUNITY_LLD_H */

/** @} */
