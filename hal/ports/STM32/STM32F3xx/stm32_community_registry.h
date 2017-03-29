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

#ifndef STM32_COMMUNITY_REGISTRY_H
#define STM32_COMMUNITY_REGISTRY_H

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
#define STM32_HAS_COMP1	1
#define STM32_HAS_COMP2	1
#define STM32_HAS_COMP3	1
#define STM32_HAS_COMP4	1
#define STM32_HAS_COMP5	1
#define STM32_HAS_COMP6	1
#define STM32_HAS_COMP7	1
#endif /* defined(STM32F303xC) */

/*===========================================================================*/
/* STM32F303xE.                                                              */
/*===========================================================================*/
#if defined(STM32F303xE)
#define STM32_HAS_COMP1	1
#define STM32_HAS_COMP2	1
#define STM32_HAS_COMP3	1
#define STM32_HAS_COMP4	1
#define STM32_HAS_COMP5	1
#define STM32_HAS_COMP6	1
#define STM32_HAS_COMP7	1
#endif /* defined(STM32F303xE) */

/*===========================================================================*/
/* STM32F303x8.                                                              */
/*===========================================================================*/
#if defined(STM32F303x8)
#define STM32_HAS_COMP1	0
#define STM32_HAS_COMP2	1
#define STM32_HAS_COMP3	0
#define STM32_HAS_COMP4	1
#define STM32_HAS_COMP5	0
#define STM32_HAS_COMP6	1
#define STM32_HAS_COMP7	0
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
#error "not yet implemented"
#endif /* defined(STM32F328x8) */

/*===========================================================================*/
/* STM32F358xC.                                                              */
/*===========================================================================*/
#if defined(STM32F358xC)
#define STM32_HAS_COMP1	1
#define STM32_HAS_COMP2	1
#define STM32_HAS_COMP3	1
#define STM32_HAS_COMP4	1
#define STM32_HAS_COMP5	1
#define STM32_HAS_COMP6	1
#define STM32_HAS_COMP7	1
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
#define STM32_HAS_COMP1	1
#define STM32_HAS_COMP2	1
#define STM32_HAS_COMP3	1
#define STM32_HAS_COMP4	1
#define STM32_HAS_COMP5	1
#define STM32_HAS_COMP6	1
#define STM32_HAS_COMP7	1
#endif /* defined(STM32F398xx) */

/** @} */

#endif /* STM32_COMMUNITY_REGISTRY_H */

/** @} */
