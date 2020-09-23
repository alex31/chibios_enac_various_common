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

/*
 * This file has been automatically generated using ChibiStudio board
 * generator plugin. Do not edit manually.
 */

#ifndef BOARD_H
#define BOARD_H

/*
 * Board identifier.
 */
#define BOARD_OLIMEX_STM32_E407
#define BOARD_NAME                  "WAVESHARE-OPEN407Z"
/*
 * The board has an ULPI USB PHY.
 */
#define BOARD_OTG2_USES_ULPI

/*
 * Board oscillators-related settings.
 */
#if !defined(STM32_LSECLK)
#define STM32_LSECLK                32768
#endif

#if !defined(STM32_HSECLK)
#define STM32_HSECLK                8000000
#endif




/*
 * Board voltages.
 * Required for performance limits calculation.
 */
#define STM32_VDD                   330

/*
 * MCU type as defined in the ST header.
 */
#ifndef STM32F407xx
  #define STM32F407xx
#endif
/*
 * IO pins assignments.
 */
#define GPIOA_UART4_TX              0
#define GPIOA_UART4_RX              1
#define GPIOA2_BATT_SENSE           2
#define GPIOA_USB_OTG_HS_D0         3
#define GPIOA_4                     4
#define GPIOA_USB_OTG_HS_CK         5
#define GPIOA_BT_RESET              6
#define GPIOA_BT_AUTORUN            7
#define GPIOA_8                     8
#define GPIOA_OTG_FS_VBUS           9
#define GPIOA_PIN10                 10
#define GPIOA_OTG_FS_DM             11
#define GPIOA_OTG_FS_DP             12
#define GPIOA_SWDIO                 13
#define GPIOA_SWDCLK                14
#define GPIOA_BT_MODE               15

#define GPIOB_USB_OTG_HS_D1         0
#define GPIOB_USB_OTG_HS_D2         1
#define GPIOB2_USB_DETECT           2
#define GPIOB_SWO                   3 // SEMIHOSTING>
#define GPIOB_SD_INDICATE           4
#define GPIOB_USB_OTG_HS_D7         5
#define GPIOB_UART1_TX              6
#define GPIOB_UART1_RX              7
#define GPIOB_I2C_ENABLE            8
#define GPIOB_I2C_INDICATE          9
#define GPIOB_USB_OTG_HS_D3         10
#define GPIOB_USB_OTG_HS_D4         11
#define GPIOB_USB_OTG_HS_D5         12
#define GPIOB_USB_OTG_HS_D6         13
#define GPIOB_14_EXT                14
#define GPIOB_15_EXT                15

#define GPIOC_USB_OTG_HS_STP        0
#define GPIOC_USB_INDICATE          1
#define GPIOC_USB_OTG_HS_DIR        2
#define GPIOC_USB_OTG_HS_NXT        3
#define GPIOC_USB_ENABLE            4
#define GPIOC_USB_RESET             5
#define GPIOC_1W_TX                 6
#define GPIOC_1W_RX                 7
#define GPIOC_SD_D0                 8  // MUST BE PULLED UP IF NO EXT PULLUP
#define GPIOC_SD_D1                 9  // MUST BE PULLED UP IF NO EXT PULLUP
#define GPIOC_SD_D2                 10 // MUST BE PULLED UP IF NO EXT PULLUP
#define GPIOC_SD_D3                 11 // MUST BE PULLED UP IF NO EXT PULLUP
#define GPIOC_SD_CLK                12
#define GPIOC_SD_ENABLE             13
#define GPIOC_OSC32_IN              14
#define GPIOC_OSC32_OUT             15

#define GPIOD_CAN1_RX               0
#define GPIOD_CAN1_TX               1
#define GPIOD_SD_CMD                2 // MUST BE PULLED UP IF NO EXT PULLUP
#define GPIOD_CAN_ENABLE            3
#define GPIOD_USART2_INDICATE       4
#define GPIOD_USART2_TX             5
#define GPIOD_USART2_RX             6
#define GPIOD_USART2_ENABLE         7
#define GPIOD_PIN8                  8 // SPI for ext 24bit ADC (not used yet)
#define GPIOD_PIN9                  9 // SPI for ext 24bit ADC (not used yet)
#define GPIOD_PIN10                 10// SPI for ext 24bit ADC (not used yet)
#define GPIOD_PIN11                 11// SPI for ext 24bit ADC (not used yet)
#define GPIOD_PIN12                 12// SPI for ext 24bit ADC (not used yet)
#define GPIOD_PIN13                 13// SPI for ext 24bit ADC (not used yet)
#define GPIOD_PIN14                 14// SPI for ext 24bit ADC (not used yet)
#define GPIOD_BTN                   15

#define GPIOE_1W_ENABLE             0
#define GPIOE_1W_INDICATE           1
#define GPIOE2_RED_LED              2
#define GPIOE3_YEL_LED              3
#define GPIOE4_GREEN_LED            4
#define GPIOE_PIN5                  5
#define GPIOE_PIN6                  6
#define GPIOE_PIN7                  7
#define GPIOE_PIN8                  8
#define GPIOE_PIN9                  9
#define GPIOE_PIN10                 10
#define GPIOE_PIN11                 11
#define GPIOE_PIN12                 12
#define GPIOE_PIN13                 13
#define GPIOE_PIN14                 14
#define GPIOE_PIN15                 15

#define GPIOF_PIN0                  0
#define GPIOF_PIN1                  1
#define GPIOF_PIN2                  2
#define GPIOF_PIN3                  3
#define GPIOF_PIN4                  4
#define GPIOF_PIN5                  5
#define GPIOF_PIN6                  6
#define GPIOF_PIN7                  7
#define GPIOF_PIN8                  8
#define GPIOF_PIN9                  9
#define GPIOF_PIN10                 10
#define GPIOF_PIN11                 11
#define GPIOF_PIN12                 12
#define GPIOF_PIN13                 13
#define GPIOF_PIN14                 14
#define GPIOF_PIN15                 15

#define GPIOG_PIN0                  0
#define GPIOG_PIN1                  1
#define GPIOG_PIN2                  2
#define GPIOG_PIN3                  3
#define GPIOG_PIN4                  4
#define GPIOG_PIN5                  5
#define GPIOG_PIN6                  6
#define GPIOG_PIN7                  7
#define GPIOG_PIN8                  8
#define GPIOG_PIN9                  9
#define GPIOG_PIN10                 10
#define GPIOG_PIN11                 11
#define GPIOG_PIN12                 12
#define GPIOG_PIN13                 13
#define GPIOG_PIN14                 14
#define GPIOG_PIN15                 15

/* external quartz 8MHz */
#define GPIOH_OSC_IN                0
#define GPIOH_OSC_OUT               1
#define GPIOH_PIN2                  2
#define GPIOH_PIN3                  3
#define GPIOH_PIN4                  4
#define GPIOH_PIN5                  5
#define GPIOH_PIN6                  6
#define GPIOH_PIN7                  7
#define GPIOH_PIN8                  8
#define GPIOH_PIN9                  9
#define GPIOH_PIN10                 10
#define GPIOH_PIN11                 11
#define GPIOH_PIN12                 12
#define GPIOH_PIN13                 13
#define GPIOH_PIN14                 14
#define GPIOH_PIN15                 15

#define GPIOI_PIN0                  0
#define GPIOI_PIN1                  1
#define GPIOI_PIN2                  2
#define GPIOI_PIN3                  3
#define GPIOI_PIN4                  4
#define GPIOI_PIN5                  5
#define GPIOI_PIN6                  6
#define GPIOI_PIN7                  7
#define GPIOI_PIN8                  8
#define GPIOI_PIN9                  9
#define GPIOI_PIN10                 10
#define GPIOI_PIN11                 11
#define GPIOI_PIN12                 12
#define GPIOI_PIN13                 13
#define GPIOI_PIN14                 14
#define GPIOI_PIN15                 15

/*
 * I/O ports initial setup, this configuration is established soon after reset
 * in the initialization code.
 * Please refer to the STM32 Reference Manual for details.
 */
#define PIN_MODE_INPUT(n)           (0U << ((n) * 2))
#define PIN_MODE_OUTPUT(n)          (1U << ((n) * 2))
#define PIN_MODE_ALTERNATE(n)       (2U << ((n) * 2))
#define PIN_MODE_ANALOG(n)          (3U << ((n) * 2))
#define PIN_ODR_LOW(n)              (0U << (n))
#define PIN_ODR_HIGH(n)             (1U << (n))
#define PIN_OTYPE_PUSHPULL(n)       (0U << (n))
#define PIN_OTYPE_OPENDRAIN(n)      (1U << (n))
#define PIN_OSPEED_2M(n)            (0U << ((n) * 2))
#define PIN_OSPEED_25M(n)           (1U << ((n) * 2))
#define PIN_OSPEED_50M(n)           (2U << ((n) * 2))
#define PIN_OSPEED_100M(n)          (3U << ((n) * 2))
#define PIN_PUPDR_FLOATING(n)       (0U << ((n) * 2))
#define PIN_PUPDR_PULLUP(n)         (1U << ((n) * 2))
#define PIN_PUPDR_PULLDOWN(n)       (2U << ((n) * 2))
#define PIN_AFIO_AF(n, v)           ((v##U) << ((n % 8) * 4))

/*
 * GPIOA setup:
*/

#define VAL_GPIOA_MODER             (PIN_MODE_ALTERNATE(GPIOA_UART4_TX) |    \
                                     PIN_MODE_ALTERNATE(GPIOA_UART4_RX) |\
                                     PIN_MODE_ANALOG(GPIOA2_BATT_SENSE) |\
                                     PIN_MODE_ALTERNATE(GPIOA_USB_OTG_HS_D0) | \
                                     PIN_MODE_OUTPUT(GPIOA_4) |           \
                                     PIN_MODE_ALTERNATE(GPIOA_USB_OTG_HS_CK) |           \
                                     PIN_MODE_OUTPUT(GPIOA_BT_RESET) |           \
                                     PIN_MODE_OUTPUT(GPIOA_BT_AUTORUN) |\
                                     PIN_MODE_OUTPUT(GPIOA_8) |  \
                                     PIN_MODE_ALTERNATE(GPIOA_OTG_FS_VBUS) |    \
                                     PIN_MODE_ALTERNATE(GPIOA_PIN10) |  \
                                     PIN_MODE_ALTERNATE(GPIOA_OTG_FS_DM) |  \
                                     PIN_MODE_ALTERNATE(GPIOA_OTG_FS_DP) |  \
                                     PIN_MODE_ALTERNATE(GPIOA_SWDIO) |   \
                                     PIN_MODE_ALTERNATE(GPIOA_SWDCLK) |   \
                                     PIN_MODE_OUTPUT(GPIOA_BT_MODE))
#define VAL_GPIOA_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOA_UART4_TX) |\
                                     PIN_OTYPE_PUSHPULL(GPIOA_UART4_RX) |\
                                     PIN_OTYPE_OPENDRAIN(GPIOA2_BATT_SENSE) |\
                                     PIN_OTYPE_PUSHPULL(GPIOA_USB_OTG_HS_D0) |\
                                     PIN_OTYPE_PUSHPULL(GPIOA_4) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOA_USB_OTG_HS_CK) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOA_BT_RESET) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOA_BT_AUTORUN) |\
                                     PIN_OTYPE_PUSHPULL(GPIOA_8) |\
                                     PIN_OTYPE_PUSHPULL(GPIOA_OTG_FS_VBUS) |\
                                     PIN_OTYPE_PUSHPULL(GPIOA_PIN10) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOA_OTG_FS_DM) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOA_OTG_FS_DP) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOA_SWDIO) |   \
                                     PIN_OTYPE_PUSHPULL(GPIOA_SWDCLK) |   \
                                     PIN_OTYPE_PUSHPULL(GPIOA_BT_MODE))
#define VAL_GPIOA_OSPEEDR           (PIN_OSPEED_100M(GPIOA_UART4_TX) |   \
                                     PIN_OSPEED_100M(GPIOA_UART4_RX) |\
                                     PIN_OSPEED_100M(GPIOA2_BATT_SENSE) | \
                                     PIN_OSPEED_100M(GPIOA_USB_OTG_HS_D0) |\
                                     PIN_OSPEED_100M(GPIOA_4) |          \
                                     PIN_OSPEED_100M(GPIOA_USB_OTG_HS_CK) |          \
                                     PIN_OSPEED_100M(GPIOA_BT_RESET) |          \
                                     PIN_OSPEED_100M(GPIOA_BT_AUTORUN) |\
                                     PIN_OSPEED_100M(GPIOA_8) |  \
                                     PIN_OSPEED_100M(GPIOA_OTG_FS_VBUS) |   \
                                     PIN_OSPEED_100M(GPIOA_PIN10) |     \
                                     PIN_OSPEED_100M(GPIOA_OTG_FS_DM) |     \
                                     PIN_OSPEED_100M(GPIOA_OTG_FS_DP) |     \
                                     PIN_OSPEED_100M(GPIOA_SWDIO) |      \
                                     PIN_OSPEED_100M(GPIOA_SWDCLK) |      \
                                     PIN_OSPEED_100M(GPIOA_BT_MODE))
#define VAL_GPIOA_PUPDR             (PIN_PUPDR_FLOATING(GPIOA_UART4_TX) |\
                                     PIN_PUPDR_FLOATING(GPIOA_UART4_RX) |\
                                     PIN_PUPDR_FLOATING(GPIOA2_BATT_SENSE) |\
                                     PIN_PUPDR_FLOATING(GPIOA_USB_OTG_HS_D0) |\
                                     PIN_PUPDR_PULLUP(GPIOA_4) |         \
                                     PIN_PUPDR_PULLUP(GPIOA_USB_OTG_HS_CK) |         \
                                     PIN_PUPDR_PULLUP(GPIOA_BT_RESET) |         \
                                     PIN_PUPDR_FLOATING(GPIOA_BT_AUTORUN) |\
                                     PIN_PUPDR_FLOATING(GPIOA_8) |\
                                     PIN_PUPDR_PULLDOWN(GPIOA_OTG_FS_VBUS) |\
                                     PIN_PUPDR_FLOATING(GPIOA_PIN10) |  \
                                     PIN_PUPDR_FLOATING(GPIOA_OTG_FS_DM) |  \
                                     PIN_PUPDR_FLOATING(GPIOA_OTG_FS_DP) |  \
                                     PIN_PUPDR_FLOATING(GPIOA_SWDIO) |   \
                                     PIN_PUPDR_PULLDOWN(GPIOA_SWDCLK) |   \
                                     PIN_PUPDR_PULLUP(GPIOA_BT_MODE))
#define VAL_GPIOA_ODR               (PIN_ODR_LOW(GPIOA_UART4_TX) |      \
                                     PIN_ODR_HIGH(GPIOA_UART4_RX) | \
                                     PIN_ODR_HIGH(GPIOA2_BATT_SENSE) |    \
                                     PIN_ODR_HIGH(GPIOA_USB_OTG_HS_D0) |   \
                                     PIN_ODR_LOW(GPIOA_4) |             \
                                     PIN_ODR_HIGH(GPIOA_USB_OTG_HS_CK) |             \
                                     PIN_ODR_LOW(GPIOA_BT_RESET) |             \
                                     PIN_ODR_LOW(GPIOA_BT_AUTORUN) |  \
                                     PIN_ODR_HIGH(GPIOA_8) |      \
                                     PIN_ODR_HIGH(GPIOA_OTG_FS_VBUS) |      \
                                     PIN_ODR_HIGH(GPIOA_PIN10) |        \
                                     PIN_ODR_HIGH(GPIOA_OTG_FS_DM) |        \
                                     PIN_ODR_HIGH(GPIOA_OTG_FS_DP) |        \
                                     PIN_ODR_HIGH(GPIOA_SWDIO) |         \
                                     PIN_ODR_HIGH(GPIOA_SWDCLK) |         \
                                     PIN_ODR_HIGH(GPIOA_BT_MODE))
#define VAL_GPIOA_AFRL              (PIN_AFIO_AF(GPIOA_UART4_TX, 8) |    \
                                     PIN_AFIO_AF(GPIOA_UART4_RX, 8) |\
                                     PIN_AFIO_AF(GPIOA2_BATT_SENSE, 0) | \
                                     PIN_AFIO_AF(GPIOA_USB_OTG_HS_D0, 10) | \
                                     PIN_AFIO_AF(GPIOA_4, 0) |           \
                                     PIN_AFIO_AF(GPIOA_USB_OTG_HS_CK, 10) |           \
                                     PIN_AFIO_AF(GPIOA_BT_RESET, 0) |           \
                                     PIN_AFIO_AF(GPIOA_BT_AUTORUN, 0))
#define VAL_GPIOA_AFRH              (PIN_AFIO_AF(GPIOA_8, 0) |   \
                                     PIN_AFIO_AF(GPIOA_OTG_FS_VBUS, 10) |    \
                                     PIN_AFIO_AF(GPIOA_PIN10, 10) |     \
                                     PIN_AFIO_AF(GPIOA_OTG_FS_DM, 10) |     \
                                     PIN_AFIO_AF(GPIOA_OTG_FS_DP, 10) |     \
                                     PIN_AFIO_AF(GPIOA_SWDIO, 0) |       \
                                     PIN_AFIO_AF(GPIOA_SWDCLK, 0) |       \
                                     PIN_AFIO_AF(GPIOA_BT_MODE, 0))

/*
 * GPIOB setup:
 */
#define VAL_GPIOB_MODER             (PIN_MODE_ALTERNATE(GPIOB_USB_OTG_HS_D1) |  \
                                     PIN_MODE_ALTERNATE(GPIOB_USB_OTG_HS_D2) |   \
                                     PIN_MODE_INPUT(GPIOB2_USB_DETECT) |          \
                                     PIN_MODE_ALTERNATE(GPIOB_SWO) |   \
                                     PIN_MODE_OUTPUT(GPIOB_SD_INDICATE) |  \
                                     PIN_MODE_ALTERNATE(GPIOB_USB_OTG_HS_D7) |           \
                                     PIN_MODE_ALTERNATE(GPIOB_UART1_TX) |           \
                                     PIN_MODE_ALTERNATE(GPIOB_UART1_RX) |           \
                                     PIN_MODE_OUTPUT(GPIOB_I2C_ENABLE) |   \
                                     PIN_MODE_OUTPUT(GPIOB_I2C_INDICATE) |   \
                                     PIN_MODE_ALTERNATE(GPIOB_USB_OTG_HS_D3) |   \
                                     PIN_MODE_ALTERNATE(GPIOB_USB_OTG_HS_D4) |          \
                                     PIN_MODE_ALTERNATE(GPIOB_USB_OTG_HS_D5) |  \
                                     PIN_MODE_ALTERNATE(GPIOB_USB_OTG_HS_D6) |    \
                                     PIN_MODE_OUTPUT(GPIOB_14_EXT) |  \
                                     PIN_MODE_OUTPUT(GPIOB_15_EXT))
#define VAL_GPIOB_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOB_USB_OTG_HS_D1) |\
                                     PIN_OTYPE_PUSHPULL(GPIOB_USB_OTG_HS_D2) |\
                                     PIN_OTYPE_OPENDRAIN(GPIOB2_USB_DETECT) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOB_SWO) |   \
                                     PIN_OTYPE_PUSHPULL(GPIOB_SD_INDICATE) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOB_USB_OTG_HS_D7) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOB_UART1_TX) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOB_UART1_RX) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOB_I2C_ENABLE) |  \
                                     PIN_OTYPE_OPENDRAIN(GPIOB_I2C_INDICATE) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOB_USB_OTG_HS_D3) |   \
                                     PIN_OTYPE_PUSHPULL(GPIOB_USB_OTG_HS_D4) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOB_USB_OTG_HS_D5) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOB_USB_OTG_HS_D6) |\
                                     PIN_OTYPE_PUSHPULL(GPIOB_14_EXT) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOB_15_EXT))
#define VAL_GPIOB_OSPEEDR           (PIN_OSPEED_100M(GPIOB_USB_OTG_HS_D1) |  \
                                     PIN_OSPEED_100M(GPIOB_USB_OTG_HS_D2) |  \
                                     PIN_OSPEED_100M(GPIOB2_USB_DETECT) |         \
                                     PIN_OSPEED_100M(GPIOB_SWO) |      \
                                     PIN_OSPEED_100M(GPIOB_SD_INDICATE) |     \
                                     PIN_OSPEED_100M(GPIOB_USB_OTG_HS_D7) |          \
                                     PIN_OSPEED_100M(GPIOB_UART1_TX) |          \
                                     PIN_OSPEED_100M(GPIOB_UART1_RX) |          \
                                     PIN_OSPEED_100M(GPIOB_I2C_ENABLE) |      \
                                     PIN_OSPEED_100M(GPIOB_I2C_INDICATE) |      \
                                     PIN_OSPEED_100M(GPIOB_USB_OTG_HS_D3) |      \
                                     PIN_OSPEED_100M(GPIOB_USB_OTG_HS_D4) |         \
                                     PIN_OSPEED_100M(GPIOB_USB_OTG_HS_D5) |     \
                                     PIN_OSPEED_100M(GPIOB_USB_OTG_HS_D6) |   \
                                     PIN_OSPEED_100M(GPIOB_14_EXT) |     \
                                     PIN_OSPEED_100M(GPIOB_15_EXT))
#define VAL_GPIOB_PUPDR             (PIN_PUPDR_FLOATING(GPIOB_USB_OTG_HS_D1) |\
                                     PIN_PUPDR_FLOATING(GPIOB_USB_OTG_HS_D2) |\
                                     PIN_PUPDR_FLOATING(GPIOB2_USB_DETECT) |      \
                                     PIN_PUPDR_FLOATING(GPIOB_SWO) |   \
                                     PIN_PUPDR_FLOATING(GPIOB_SD_INDICATE) |  \
                                     PIN_PUPDR_PULLUP(GPIOB_USB_OTG_HS_D7) |         \
                                     PIN_PUPDR_PULLUP(GPIOB_UART1_TX) |         \
                                     PIN_PUPDR_PULLUP(GPIOB_UART1_RX) |         \
                                     PIN_PUPDR_FLOATING(GPIOB_I2C_ENABLE) |   \
                                     PIN_PUPDR_FLOATING(GPIOB_I2C_INDICATE) |   \
                                     PIN_PUPDR_FLOATING(GPIOB_USB_OTG_HS_D3) |   \
                                     PIN_PUPDR_PULLUP(GPIOB_USB_OTG_HS_D4) |        \
                                     PIN_PUPDR_FLOATING(GPIOB_USB_OTG_HS_D5) |  \
                                     PIN_PUPDR_PULLDOWN(GPIOB_USB_OTG_HS_D6) |\
                                     PIN_PUPDR_FLOATING(GPIOB_14_EXT) |  \
                                     PIN_PUPDR_FLOATING(GPIOB_15_EXT))
#define VAL_GPIOB_ODR               (PIN_ODR_HIGH(GPIOB_USB_OTG_HS_D1) |      \
                                     PIN_ODR_HIGH(GPIOB_USB_OTG_HS_D2) |     \
                                     PIN_ODR_HIGH(GPIOB2_USB_DETECT) |            \
                                     PIN_ODR_HIGH(GPIOB_SWO) |         \
                                     PIN_ODR_HIGH(GPIOB_SD_INDICATE) |        \
                                     PIN_ODR_HIGH(GPIOB_USB_OTG_HS_D7) |             \
                                     PIN_ODR_HIGH(GPIOB_UART1_TX) |             \
                                     PIN_ODR_HIGH(GPIOB_UART1_RX) |             \
                                     PIN_ODR_HIGH(GPIOB_I2C_ENABLE) |         \
                                     PIN_ODR_HIGH(GPIOB_I2C_INDICATE) |         \
                                     PIN_ODR_HIGH(GPIOB_USB_OTG_HS_D3) |         \
                                     PIN_ODR_HIGH(GPIOB_USB_OTG_HS_D4) |            \
                                     PIN_ODR_HIGH(GPIOB_USB_OTG_HS_D5) |        \
                                     PIN_ODR_HIGH(GPIOB_USB_OTG_HS_D6) |      \
                                     PIN_ODR_HIGH(GPIOB_14_EXT) |        \
                                     PIN_ODR_HIGH(GPIOB_15_EXT))
#define VAL_GPIOB_AFRL              (PIN_AFIO_AF(GPIOB_USB_OTG_HS_D1, 10) |   \
                                     PIN_AFIO_AF(GPIOB_USB_OTG_HS_D2, 10) |   \
                                     PIN_AFIO_AF(GPIOB2_USB_DETECT, 0) |          \
                                     PIN_AFIO_AF(GPIOB_SWO, 0) |       \
                                     PIN_AFIO_AF(GPIOB_SD_INDICATE, 0) |      \
                                     PIN_AFIO_AF(GPIOB_USB_OTG_HS_D7, 10) |           \
                                     PIN_AFIO_AF(GPIOB_UART1_TX, 7) |           \
                                     PIN_AFIO_AF(GPIOB_UART1_RX, 7))
#define VAL_GPIOB_AFRH              (PIN_AFIO_AF(GPIOB_I2C_ENABLE, 0) |       \
                                     PIN_AFIO_AF(GPIOB_I2C_INDICATE, 0) |       \
                                     PIN_AFIO_AF(GPIOB_USB_OTG_HS_D3, 10) |       \
                                     PIN_AFIO_AF(GPIOB_USB_OTG_HS_D4, 10) |          \
                                     PIN_AFIO_AF(GPIOB_USB_OTG_HS_D5, 10) |     \
                                     PIN_AFIO_AF(GPIOB_USB_OTG_HS_D6, 10) |    \
                                     PIN_AFIO_AF(GPIOB_14_EXT, 0) |     \
                                     PIN_AFIO_AF(GPIOB_15_EXT, 12))

/*
 * GPIOC setup:
 */
#define VAL_GPIOC_MODER             (PIN_MODE_ALTERNATE(GPIOC_USB_OTG_HS_STP) |           \
                                     PIN_MODE_OUTPUT(GPIOC_USB_INDICATE) |              \
                                     PIN_MODE_ALTERNATE(GPIOC_USB_OTG_HS_DIR) |  \
                                     PIN_MODE_ALTERNATE(GPIOC_USB_OTG_HS_NXT) |  \
                                     PIN_MODE_OUTPUT(GPIOC_USB_ENABLE) |\
                                     PIN_MODE_OUTPUT(GPIOC_USB_RESET) |\
                                     PIN_MODE_INPUT(GPIOC_1W_TX) |  \
                                     PIN_MODE_INPUT(GPIOC_1W_RX) |  \
                                     PIN_MODE_ALTERNATE(GPIOC_SD_D0) |      \
                                     PIN_MODE_ALTERNATE(GPIOC_SD_D1) |      \
                                     PIN_MODE_ALTERNATE(GPIOC_SD_D2) |      \
                                     PIN_MODE_ALTERNATE(GPIOC_SD_D3) |      \
                                     PIN_MODE_ALTERNATE(GPIOC_SD_CLK) |     \
                                     PIN_MODE_OUTPUT(GPIOC_SD_ENABLE) |           \
                                     PIN_MODE_INPUT(GPIOC_OSC32_IN) |       \
                                     PIN_MODE_INPUT(GPIOC_OSC32_OUT))
#define VAL_GPIOC_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOC_USB_OTG_HS_STP) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_USB_INDICATE) |\
                                     PIN_OTYPE_PUSHPULL(GPIOC_USB_OTG_HS_DIR) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOC_USB_OTG_HS_NXT) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOC_USB_ENABLE) |\
                                     PIN_OTYPE_PUSHPULL(GPIOC_USB_RESET) |\
                                     PIN_OTYPE_PUSHPULL(GPIOC_1W_TX) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOC_1W_RX) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOC_SD_D0) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOC_SD_D1) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOC_SD_D2) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOC_SD_D3) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOC_SD_CLK) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOC_SD_ENABLE) |        \
                                     PIN_OTYPE_PUSHPULL(GPIOC_OSC32_IN) |   \
                                     PIN_OTYPE_PUSHPULL(GPIOC_OSC32_OUT))
#define VAL_GPIOC_OSPEEDR           (PIN_OSPEED_100M(GPIOC_USB_OTG_HS_STP) |          \
                                     PIN_OSPEED_100M(GPIOC_USB_INDICATE) |  \
                                     PIN_OSPEED_100M(GPIOC_USB_OTG_HS_DIR) |     \
                                     PIN_OSPEED_100M(GPIOC_USB_OTG_HS_NXT) |     \
                                     PIN_OSPEED_100M(GPIOC_USB_ENABLE) | \
                                     PIN_OSPEED_100M(GPIOC_USB_RESET) | \
                                     PIN_OSPEED_100M(GPIOC_1W_TX) |     \
                                     PIN_OSPEED_100M(GPIOC_1W_RX) |     \
                                     PIN_OSPEED_100M(GPIOC_SD_D0) |         \
                                     PIN_OSPEED_100M(GPIOC_SD_D1) |         \
                                     PIN_OSPEED_100M(GPIOC_SD_D2) |         \
                                     PIN_OSPEED_100M(GPIOC_SD_D3) |         \
                                     PIN_OSPEED_100M(GPIOC_SD_CLK) |        \
                                     PIN_OSPEED_100M(GPIOC_SD_ENABLE) |           \
                                     PIN_OSPEED_100M(GPIOC_OSC32_IN) |      \
                                     PIN_OSPEED_100M(GPIOC_OSC32_OUT))
#define VAL_GPIOC_PUPDR             (PIN_PUPDR_PULLUP(GPIOC_USB_OTG_HS_STP) |         \
                                     PIN_PUPDR_FLOATING(GPIOC_USB_INDICATE) |\
                                     PIN_PUPDR_FLOATING(GPIOC_USB_OTG_HS_DIR) |  \
                                     PIN_PUPDR_FLOATING(GPIOC_USB_OTG_HS_NXT) |  \
                                     PIN_PUPDR_FLOATING(GPIOC_USB_ENABLE) |\
                                     PIN_PUPDR_FLOATING(GPIOC_USB_RESET) |\
                                     PIN_PUPDR_FLOATING(GPIOC_1W_TX) |  \
                                     PIN_PUPDR_FLOATING(GPIOC_1W_RX) |  \
                                     PIN_PUPDR_PULLUP(GPIOC_SD_D0) |      \
                                     PIN_PUPDR_PULLUP(GPIOC_SD_D1) |      \
                                     PIN_PUPDR_PULLUP(GPIOC_SD_D2) |      \
                                     PIN_PUPDR_PULLUP(GPIOC_SD_D3) |      \
                                     PIN_PUPDR_PULLDOWN(GPIOC_SD_CLK) |     \
                                     PIN_PUPDR_FLOATING(GPIOC_SD_ENABLE) |        \
                                     PIN_PUPDR_FLOATING(GPIOC_OSC32_IN) |   \
                                     PIN_PUPDR_FLOATING(GPIOC_OSC32_OUT))
#define VAL_GPIOC_ODR               (PIN_ODR_HIGH(GPIOC_USB_OTG_HS_STP) |             \
                                     PIN_ODR_HIGH(GPIOC_USB_INDICATE) |     \
                                     PIN_ODR_HIGH(GPIOC_USB_OTG_HS_DIR) |        \
                                     PIN_ODR_HIGH(GPIOC_USB_OTG_HS_NXT) |        \
                                     PIN_ODR_HIGH(GPIOC_USB_ENABLE) |    \
                                     PIN_ODR_HIGH(GPIOC_USB_RESET) |    \
                                     PIN_ODR_HIGH(GPIOC_1W_TX) |        \
                                     PIN_ODR_HIGH(GPIOC_1W_RX) |        \
                                     PIN_ODR_HIGH(GPIOC_SD_D0) |            \
                                     PIN_ODR_HIGH(GPIOC_SD_D1) |            \
                                     PIN_ODR_HIGH(GPIOC_SD_D2) |            \
                                     PIN_ODR_HIGH(GPIOC_SD_D3) |            \
                                     PIN_ODR_HIGH(GPIOC_SD_CLK) |           \
                                     PIN_ODR_HIGH(GPIOC_SD_ENABLE) |              \
                                     PIN_ODR_HIGH(GPIOC_OSC32_IN) |         \
                                     PIN_ODR_HIGH(GPIOC_OSC32_OUT))
#define VAL_GPIOC_AFRL              (PIN_AFIO_AF(GPIOC_USB_OTG_HS_STP, 10) |           \
                                     PIN_AFIO_AF(GPIOC_USB_INDICATE, 0) |  \
                                     PIN_AFIO_AF(GPIOC_USB_OTG_HS_DIR, 10) |      \
                                     PIN_AFIO_AF(GPIOC_USB_OTG_HS_NXT, 10) |      \
                                     PIN_AFIO_AF(GPIOC_USB_ENABLE, 0) | \
                                     PIN_AFIO_AF(GPIOC_USB_RESET, 0) | \
                                     PIN_AFIO_AF(GPIOC_1W_TX, 8) |      \
                                     PIN_AFIO_AF(GPIOC_1W_RX, 8))
#define VAL_GPIOC_AFRH              (PIN_AFIO_AF(GPIOC_SD_D0, 12) |         \
                                     PIN_AFIO_AF(GPIOC_SD_D1, 12) |         \
                                     PIN_AFIO_AF(GPIOC_SD_D2, 12) |         \
                                     PIN_AFIO_AF(GPIOC_SD_D3, 12) |         \
                                     PIN_AFIO_AF(GPIOC_SD_CLK, 12) |        \
                                     PIN_AFIO_AF(GPIOC_SD_ENABLE, 0) |            \
                                     PIN_AFIO_AF(GPIOC_OSC32_IN, 0) |       \
                                     PIN_AFIO_AF(GPIOC_OSC32_OUT, 0))

/*
 * GPIOD setup:
 */
#define VAL_GPIOD_MODER             (PIN_MODE_ALTERNATE(GPIOD_CAN1_RX) |           \
                                     PIN_MODE_ALTERNATE(GPIOD_CAN1_TX) |           \
                                     PIN_MODE_ALTERNATE(GPIOD_SD_CMD) |     \
                                     PIN_MODE_OUTPUT(GPIOD_CAN_ENABLE) |           \
                                     PIN_MODE_OUTPUT(GPIOD_USART2_INDICATE) |           \
                                     PIN_MODE_ALTERNATE(GPIOD_USART2_TX) |           \
                                     PIN_MODE_ALTERNATE(GPIOD_USART2_RX) |           \
                                     PIN_MODE_OUTPUT(GPIOD_USART2_ENABLE) |           \
                                     PIN_MODE_OUTPUT(GPIOD_PIN8) |           \
                                     PIN_MODE_INPUT(GPIOD_PIN9) |           \
                                     PIN_MODE_OUTPUT(GPIOD_PIN10) |          \
                                     PIN_MODE_OUTPUT(GPIOD_PIN11) |          \
                                     PIN_MODE_OUTPUT(GPIOD_PIN12) |          \
                                     PIN_MODE_OUTPUT(GPIOD_PIN13) |          \
                                     PIN_MODE_OUTPUT(GPIOD_PIN14) |          \
                                     PIN_MODE_INPUT(GPIOD_BTN))
#define VAL_GPIOD_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOD_CAN1_RX) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOD_CAN1_TX) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOD_SD_CMD) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOD_CAN_ENABLE) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOD_USART2_INDICATE) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOD_USART2_TX) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOD_USART2_RX) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOD_USART2_ENABLE) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN8) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN9) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN10) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN11) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN12) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN13) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN14) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOD_BTN))
#define VAL_GPIOD_OSPEEDR           (PIN_OSPEED_100M(GPIOD_CAN1_RX) |          \
                                     PIN_OSPEED_100M(GPIOD_CAN1_TX) |          \
                                     PIN_OSPEED_100M(GPIOD_SD_CMD) |        \
                                     PIN_OSPEED_100M(GPIOD_CAN_ENABLE) |          \
                                     PIN_OSPEED_100M(GPIOD_USART2_INDICATE) |          \
                                     PIN_OSPEED_100M(GPIOD_USART2_TX) |          \
                                     PIN_OSPEED_100M(GPIOD_USART2_RX) |          \
                                     PIN_OSPEED_100M(GPIOD_USART2_ENABLE) |          \
                                     PIN_OSPEED_100M(GPIOD_PIN8) |          \
                                     PIN_OSPEED_100M(GPIOD_PIN9) |          \
                                     PIN_OSPEED_100M(GPIOD_PIN10) |         \
                                     PIN_OSPEED_100M(GPIOD_PIN11) |         \
                                     PIN_OSPEED_100M(GPIOD_PIN12) |         \
                                     PIN_OSPEED_100M(GPIOD_PIN13) |         \
                                     PIN_OSPEED_100M(GPIOD_PIN14) |         \
                                     PIN_OSPEED_100M(GPIOD_BTN))
#define VAL_GPIOD_PUPDR             (PIN_PUPDR_PULLUP(GPIOD_CAN1_RX) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_CAN1_TX) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_SD_CMD) |     \
                                     PIN_PUPDR_PULLUP(GPIOD_CAN_ENABLE) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_USART2_INDICATE) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_USART2_TX) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_USART2_RX) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_USART2_ENABLE) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN8) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN9) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN10) |        \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN11) |        \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN12) |        \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN13) |        \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN14) |        \
                                     PIN_PUPDR_PULLUP(GPIOD_BTN))
#define VAL_GPIOD_ODR               (PIN_ODR_HIGH(GPIOD_CAN1_RX) |             \
                                     PIN_ODR_HIGH(GPIOD_CAN1_TX) |             \
                                     PIN_ODR_HIGH(GPIOD_SD_CMD) |           \
                                     PIN_ODR_HIGH(GPIOD_CAN_ENABLE) |             \
                                     PIN_ODR_HIGH(GPIOD_USART2_INDICATE) |             \
                                     PIN_ODR_HIGH(GPIOD_USART2_TX) |             \
                                     PIN_ODR_HIGH(GPIOD_USART2_RX) |             \
                                     PIN_ODR_HIGH(GPIOD_USART2_ENABLE) |             \
                                     PIN_ODR_HIGH(GPIOD_PIN8) |             \
                                     PIN_ODR_HIGH(GPIOD_PIN9) |             \
                                     PIN_ODR_HIGH(GPIOD_PIN10) |            \
                                     PIN_ODR_HIGH(GPIOD_PIN11) |            \
                                     PIN_ODR_HIGH(GPIOD_PIN12) |            \
                                     PIN_ODR_HIGH(GPIOD_PIN13) |            \
                                     PIN_ODR_HIGH(GPIOD_PIN14) |            \
                                     PIN_ODR_HIGH(GPIOD_BTN))
#define VAL_GPIOD_AFRL              (PIN_AFIO_AF(GPIOD_CAN1_RX, 9) |           \
                                     PIN_AFIO_AF(GPIOD_CAN1_TX, 9) |           \
                                     PIN_AFIO_AF(GPIOD_SD_CMD, 12) |        \
                                     PIN_AFIO_AF(GPIOD_CAN_ENABLE, 0) |           \
                                     PIN_AFIO_AF(GPIOD_USART2_INDICATE, 0) |           \
                                     PIN_AFIO_AF(GPIOD_USART2_TX, 7) |           \
                                     PIN_AFIO_AF(GPIOD_USART2_RX, 7) |           \
                                     PIN_AFIO_AF(GPIOD_USART2_ENABLE, 0))
#define VAL_GPIOD_AFRH              (PIN_AFIO_AF(GPIOD_PIN8, 0) |           \
                                     PIN_AFIO_AF(GPIOD_PIN9, 0) |           \
                                     PIN_AFIO_AF(GPIOD_PIN10, 0) |          \
                                     PIN_AFIO_AF(GPIOD_PIN11, 0) |          \
                                     PIN_AFIO_AF(GPIOD_PIN12, 0) |          \
                                     PIN_AFIO_AF(GPIOD_PIN13, 0) |          \
                                     PIN_AFIO_AF(GPIOD_PIN14, 0) |          \
                                     PIN_AFIO_AF(GPIOD_BTN, 0))

/*
 * GPIOE setup:
 */
#define VAL_GPIOE_MODER             (PIN_MODE_OUTPUT(GPIOE_1W_ENABLE) |           \
                                     PIN_MODE_OUTPUT(GPIOE_1W_INDICATE) |           \
                                     PIN_MODE_OUTPUT(GPIOE2_RED_LED) |           \
                                     PIN_MODE_OUTPUT(GPIOE3_YEL_LED) |           \
                                     PIN_MODE_OUTPUT(GPIOE4_GREEN_LED) |           \
                                     PIN_MODE_OUTPUT(GPIOE_PIN5) |           \
                                     PIN_MODE_OUTPUT(GPIOE_PIN6) |           \
                                     PIN_MODE_OUTPUT(GPIOE_PIN7) |           \
                                     PIN_MODE_OUTPUT(GPIOE_PIN8) |           \
                                     PIN_MODE_OUTPUT(GPIOE_PIN9) |           \
                                     PIN_MODE_OUTPUT(GPIOE_PIN10) |          \
                                     PIN_MODE_OUTPUT(GPIOE_PIN11) |          \
                                     PIN_MODE_OUTPUT(GPIOE_PIN12) |          \
                                     PIN_MODE_OUTPUT(GPIOE_PIN13) |          \
                                     PIN_MODE_OUTPUT(GPIOE_PIN14) |          \
                                     PIN_MODE_OUTPUT(GPIOE_PIN15))
#define VAL_GPIOE_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOE_1W_ENABLE) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_1W_INDICATE) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE2_RED_LED) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE3_YEL_LED) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE4_GREEN_LED) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN5) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN6) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN7) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN8) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN9) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN10) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN11) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN12) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN13) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN14) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN15))
#define VAL_GPIOE_OSPEEDR           (PIN_OSPEED_100M(GPIOE_1W_ENABLE) |          \
                                     PIN_OSPEED_100M(GPIOE_1W_INDICATE) |          \
                                     PIN_OSPEED_100M(GPIOE2_RED_LED) |          \
                                     PIN_OSPEED_100M(GPIOE3_YEL_LED) |          \
                                     PIN_OSPEED_100M(GPIOE4_GREEN_LED) |          \
                                     PIN_OSPEED_100M(GPIOE_PIN5) |          \
                                     PIN_OSPEED_100M(GPIOE_PIN6) |          \
                                     PIN_OSPEED_100M(GPIOE_PIN7) |          \
                                     PIN_OSPEED_100M(GPIOE_PIN8) |          \
                                     PIN_OSPEED_100M(GPIOE_PIN9) |          \
                                     PIN_OSPEED_100M(GPIOE_PIN10) |         \
                                     PIN_OSPEED_100M(GPIOE_PIN11) |         \
                                     PIN_OSPEED_100M(GPIOE_PIN12) |         \
                                     PIN_OSPEED_100M(GPIOE_PIN13) |         \
                                     PIN_OSPEED_100M(GPIOE_PIN14) |         \
                                     PIN_OSPEED_100M(GPIOE_PIN15))
#define VAL_GPIOE_PUPDR             (PIN_PUPDR_PULLUP(GPIOE_1W_ENABLE) |         \
                                     PIN_PUPDR_PULLUP(GPIOE_1W_INDICATE) |         \
                                     PIN_PUPDR_PULLUP(GPIOE2_RED_LED) |         \
                                     PIN_PUPDR_PULLUP(GPIOE3_YEL_LED) |         \
                                     PIN_PUPDR_PULLUP(GPIOE4_GREEN_LED) |         \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN5) |         \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN6) |         \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN7) |         \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN8) |         \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN9) |         \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN10) |        \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN11) |        \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN12) |        \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN13) |        \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN14) |        \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN15))
#define VAL_GPIOE_ODR               (PIN_ODR_HIGH(GPIOE_1W_ENABLE) |             \
                                     PIN_ODR_HIGH(GPIOE_1W_INDICATE) |             \
                                     PIN_ODR_HIGH(GPIOE2_RED_LED) |             \
                                     PIN_ODR_HIGH(GPIOE3_YEL_LED) |             \
                                     PIN_ODR_HIGH(GPIOE4_GREEN_LED) |             \
                                     PIN_ODR_HIGH(GPIOE_PIN5) |             \
                                     PIN_ODR_HIGH(GPIOE_PIN6) |             \
                                     PIN_ODR_HIGH(GPIOE_PIN7) |             \
                                     PIN_ODR_HIGH(GPIOE_PIN8) |             \
                                     PIN_ODR_LOW(GPIOE_PIN9) |             \
                                     PIN_ODR_LOW(GPIOE_PIN10) |            \
                                     PIN_ODR_LOW(GPIOE_PIN11) |            \
                                     PIN_ODR_LOW(GPIOE_PIN12) |            \
                                     PIN_ODR_LOW(GPIOE_PIN13) |            \
                                     PIN_ODR_LOW(GPIOE_PIN14) |            \
                                     PIN_ODR_LOW(GPIOE_PIN15))
#define VAL_GPIOE_AFRL              (PIN_AFIO_AF(GPIOE_1W_ENABLE, 0) |           \
                                     PIN_AFIO_AF(GPIOE_1W_INDICATE, 0) |           \
                                     PIN_AFIO_AF(GPIOE2_RED_LED, 0) |           \
                                     PIN_AFIO_AF(GPIOE3_YEL_LED, 0) |           \
                                     PIN_AFIO_AF(GPIOE4_GREEN_LED, 0) |           \
                                     PIN_AFIO_AF(GPIOE_PIN5, 0) |           \
                                     PIN_AFIO_AF(GPIOE_PIN6, 0) |           \
                                     PIN_AFIO_AF(GPIOE_PIN7, 0))
#define VAL_GPIOE_AFRH              (PIN_AFIO_AF(GPIOE_PIN8, 0) |           \
                                     PIN_AFIO_AF(GPIOE_PIN9, 0) |           \
                                     PIN_AFIO_AF(GPIOE_PIN10, 0) |          \
                                     PIN_AFIO_AF(GPIOE_PIN11, 0) |          \
                                     PIN_AFIO_AF(GPIOE_PIN12, 0) |          \
                                     PIN_AFIO_AF(GPIOE_PIN13, 0) |          \
                                     PIN_AFIO_AF(GPIOE_PIN14, 0) |          \
                                     PIN_AFIO_AF(GPIOE_PIN15, 0))

/*
 * GPIOF setup:
 *
 */
#define VAL_GPIOF_MODER             (PIN_MODE_OUTPUT(GPIOF_PIN0) |           \
                                     PIN_MODE_OUTPUT(GPIOF_PIN1) |           \
                                     PIN_MODE_OUTPUT(GPIOF_PIN2) |           \
                                     PIN_MODE_OUTPUT(GPIOF_PIN3) |           \
                                     PIN_MODE_OUTPUT(GPIOF_PIN4) |           \
                                     PIN_MODE_OUTPUT(GPIOF_PIN5) |           \
                                     PIN_MODE_OUTPUT(GPIOF_PIN6) |           \
                                     PIN_MODE_OUTPUT(GPIOF_PIN7) |           \
                                     PIN_MODE_OUTPUT(GPIOF_PIN8) |           \
                                     PIN_MODE_OUTPUT(GPIOF_PIN9) |           \
                                     PIN_MODE_OUTPUT(GPIOF_PIN10) |          \
                                     PIN_MODE_OUTPUT(GPIOF_PIN11) |   \
                                     PIN_MODE_OUTPUT(GPIOF_PIN12) |          \
                                     PIN_MODE_OUTPUT(GPIOF_PIN13) |          \
                                     PIN_MODE_OUTPUT(GPIOF_PIN14) |          \
                                     PIN_MODE_OUTPUT(GPIOF_PIN15))
#define VAL_GPIOF_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOF_PIN0) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN1) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN2) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN3) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN4) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN5) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN6) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN7) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN8) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN9) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN10) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN11) |\
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN12) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN13) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN14) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN15))
#define VAL_GPIOF_OSPEEDR           (PIN_OSPEED_100M(GPIOF_PIN0) |          \
                                     PIN_OSPEED_100M(GPIOF_PIN1) |          \
                                     PIN_OSPEED_100M(GPIOF_PIN2) |          \
                                     PIN_OSPEED_100M(GPIOF_PIN3) |          \
                                     PIN_OSPEED_100M(GPIOF_PIN4) |          \
                                     PIN_OSPEED_100M(GPIOF_PIN5) |          \
                                     PIN_OSPEED_100M(GPIOF_PIN6) |          \
                                     PIN_OSPEED_100M(GPIOF_PIN7) |          \
                                     PIN_OSPEED_100M(GPIOF_PIN8) |          \
                                     PIN_OSPEED_100M(GPIOF_PIN9) |          \
                                     PIN_OSPEED_100M(GPIOF_PIN10) |         \
                                     PIN_OSPEED_100M(GPIOF_PIN11) |  \
                                     PIN_OSPEED_100M(GPIOF_PIN12) |         \
                                     PIN_OSPEED_100M(GPIOF_PIN13) |         \
                                     PIN_OSPEED_100M(GPIOF_PIN14) |         \
                                     PIN_OSPEED_100M(GPIOF_PIN15))
#define VAL_GPIOF_PUPDR             (PIN_PUPDR_PULLUP(GPIOF_PIN0) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN1) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN2) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN3) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN4) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN5) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN6) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN7) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN8) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN9) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN10) |        \
                                     PIN_PUPDR_FLOATING(GPIOF_PIN11) |\
                                     PIN_PUPDR_PULLUP(GPIOF_PIN12) |        \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN13) |        \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN14) |        \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN15))
#define VAL_GPIOF_ODR               (PIN_ODR_HIGH(GPIOF_PIN0) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN1) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN2) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN3) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN4) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN5) |             \
                                     PIN_ODR_LOW(GPIOF_PIN6) |             \
                                     PIN_ODR_LOW(GPIOF_PIN7)  |             \
                                     PIN_ODR_LOW(GPIOF_PIN8) |             \
                                     PIN_ODR_LOW(GPIOF_PIN9) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN10) |            \
                                     PIN_ODR_HIGH(GPIOF_PIN11) |     \
                                     PIN_ODR_HIGH(GPIOF_PIN12) |            \
                                     PIN_ODR_HIGH(GPIOF_PIN13) |            \
                                     PIN_ODR_HIGH(GPIOF_PIN14) |            \
                                     PIN_ODR_HIGH(GPIOF_PIN15))
#define VAL_GPIOF_AFRL              (PIN_AFIO_AF(GPIOF_PIN0, 0) |           \
                                     PIN_AFIO_AF(GPIOF_PIN1, 0) |           \
                                     PIN_AFIO_AF(GPIOF_PIN2, 0) |           \
                                     PIN_AFIO_AF(GPIOF_PIN3, 0) |           \
                                     PIN_AFIO_AF(GPIOF_PIN4, 0) |           \
                                     PIN_AFIO_AF(GPIOF_PIN5, 0) |           \
                                     PIN_AFIO_AF(GPIOF_PIN6, 0) |           \
                                     PIN_AFIO_AF(GPIOF_PIN7, 0))
#define VAL_GPIOF_AFRH              (PIN_AFIO_AF(GPIOF_PIN8, 0) |           \
                                     PIN_AFIO_AF(GPIOF_PIN9, 0) |           \
                                     PIN_AFIO_AF(GPIOF_PIN10, 0) |          \
                                     PIN_AFIO_AF(GPIOF_PIN11, 0) |   \
                                     PIN_AFIO_AF(GPIOF_PIN12, 0) |          \
                                     PIN_AFIO_AF(GPIOF_PIN13, 0) |          \
                                     PIN_AFIO_AF(GPIOF_PIN14, 0) |          \
                                     PIN_AFIO_AF(GPIOF_PIN15, 0))

/*
 * GPIOG setup:
 *
 * PG0  - PIN0                      (input pullup).
 * PG1  - PIN1                      (input pullup).
 * PG2  - PIN2                      (input pullup).
 * PG3  - PIN3                      (input pullup).
 * PG4  - PIN4                      (input pullup).
 * PG5  - PIN5                      (input pullup).
 * PG6  - PIN6                      (input pullup).
 * PG7  - PIN7                      (input pullup).
 * PG8  - PIN8                      (input pullup).
 * PG9  - PIN9                      (input pullup).
 * PG10 - SPI2_CS                   (output pushpull maximum).
 * PG11 - ETH_RMII_TXEN             (alternate 11).
 * PG12 - PIN12                     (input pullup).
 * PG13 - ETH_RMII_TXD0             (alternate 11).
 * PG14 - ETH_RMII_TXD1             (alternate 11).
 * PG15 - PIN15                     (input pullup).
 */
#define VAL_GPIOG_MODER             (PIN_MODE_INPUT(GPIOG_PIN0) |           \
                                     PIN_MODE_INPUT(GPIOG_PIN1) |           \
                                     PIN_MODE_INPUT(GPIOG_PIN2) |           \
                                     PIN_MODE_INPUT(GPIOG_PIN3) |           \
                                     PIN_MODE_INPUT(GPIOG_PIN4) |           \
                                     PIN_MODE_INPUT(GPIOG_PIN5) |           \
                                     PIN_MODE_INPUT(GPIOG_PIN6) |           \
                                     PIN_MODE_INPUT(GPIOG_PIN7) |           \
                                     PIN_MODE_INPUT(GPIOG_PIN8) |           \
                                     PIN_MODE_INPUT(GPIOG_PIN9) |           \
                                     PIN_MODE_OUTPUT(GPIOG_PIN10) |       \
                                     PIN_MODE_OUTPUT(GPIOG_PIN11) |\
                                     PIN_MODE_OUTPUT(GPIOG_PIN12) |          \
                                     PIN_MODE_OUTPUT(GPIOG_PIN13) |\
                                     PIN_MODE_OUTPUT(GPIOG_PIN14) |\
                                     PIN_MODE_INPUT(GPIOG_PIN15))
#define VAL_GPIOG_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOG_PIN0) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN1) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN2) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN3) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN4) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN5) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN6) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN7) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN8) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN9) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN10) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN11) |\
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN12) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN13) |\
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN14) |\
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN15))
#define VAL_GPIOG_OSPEEDR           (PIN_OSPEED_100M(GPIOG_PIN0) |          \
                                     PIN_OSPEED_100M(GPIOG_PIN1) |          \
                                     PIN_OSPEED_100M(GPIOG_PIN2) |          \
                                     PIN_OSPEED_100M(GPIOG_PIN3) |          \
                                     PIN_OSPEED_100M(GPIOG_PIN4) |          \
                                     PIN_OSPEED_100M(GPIOG_PIN5) |          \
                                     PIN_OSPEED_100M(GPIOG_PIN6) |          \
                                     PIN_OSPEED_100M(GPIOG_PIN7) |          \
                                     PIN_OSPEED_100M(GPIOG_PIN8) |          \
                                     PIN_OSPEED_100M(GPIOG_PIN9) |          \
                                     PIN_OSPEED_100M(GPIOG_PIN10) |       \
                                     PIN_OSPEED_100M(GPIOG_PIN11) | \
                                     PIN_OSPEED_100M(GPIOG_PIN12) |         \
                                     PIN_OSPEED_100M(GPIOG_PIN13) | \
                                     PIN_OSPEED_100M(GPIOG_PIN14) | \
                                     PIN_OSPEED_100M(GPIOG_PIN15))
#define VAL_GPIOG_PUPDR             (PIN_PUPDR_PULLUP(GPIOG_PIN0) |         \
                                     PIN_PUPDR_PULLUP(GPIOG_PIN1) |         \
                                     PIN_PUPDR_PULLUP(GPIOG_PIN2) |         \
                                     PIN_PUPDR_PULLUP(GPIOG_PIN3) |         \
                                     PIN_PUPDR_PULLUP(GPIOG_PIN4) |         \
                                     PIN_PUPDR_PULLUP(GPIOG_PIN5) |         \
                                     PIN_PUPDR_PULLUP(GPIOG_PIN6) |         \
                                     PIN_PUPDR_PULLUP(GPIOG_PIN7) |         \
                                     PIN_PUPDR_PULLUP(GPIOG_PIN8) |         \
                                     PIN_PUPDR_PULLUP(GPIOG_PIN9) |         \
                                     PIN_PUPDR_FLOATING(GPIOG_PIN10) |    \
                                     PIN_PUPDR_FLOATING(GPIOG_PIN11) |\
                                     PIN_PUPDR_PULLUP(GPIOG_PIN12) |        \
                                     PIN_PUPDR_FLOATING(GPIOG_PIN13) |\
                                     PIN_PUPDR_FLOATING(GPIOG_PIN14) |\
                                     PIN_PUPDR_PULLUP(GPIOG_PIN15))
#define VAL_GPIOG_ODR               (PIN_ODR_HIGH(GPIOG_PIN0) |             \
                                     PIN_ODR_HIGH(GPIOG_PIN1) |             \
                                     PIN_ODR_HIGH(GPIOG_PIN2) |             \
                                     PIN_ODR_HIGH(GPIOG_PIN3) |             \
                                     PIN_ODR_HIGH(GPIOG_PIN4) |             \
                                     PIN_ODR_HIGH(GPIOG_PIN5) |             \
                                     PIN_ODR_HIGH(GPIOG_PIN6) |             \
                                     PIN_ODR_HIGH(GPIOG_PIN7) |             \
                                     PIN_ODR_HIGH(GPIOG_PIN8) |             \
                                     PIN_ODR_HIGH(GPIOG_PIN9) |             \
                                     PIN_ODR_HIGH(GPIOG_PIN10) |          \
                                     PIN_ODR_HIGH(GPIOG_PIN11) |    \
                                     PIN_ODR_HIGH(GPIOG_PIN12) |            \
                                     PIN_ODR_HIGH(GPIOG_PIN13) |    \
                                     PIN_ODR_HIGH(GPIOG_PIN14) |    \
                                     PIN_ODR_HIGH(GPIOG_PIN15))
#define VAL_GPIOG_AFRL              (PIN_AFIO_AF(GPIOG_PIN0, 0) |           \
                                     PIN_AFIO_AF(GPIOG_PIN1, 0) |           \
                                     PIN_AFIO_AF(GPIOG_PIN2, 0) |           \
                                     PIN_AFIO_AF(GPIOG_PIN3, 0) |           \
                                     PIN_AFIO_AF(GPIOG_PIN4, 0) |           \
                                     PIN_AFIO_AF(GPIOG_PIN5, 0) |           \
                                     PIN_AFIO_AF(GPIOG_PIN6, 0) |           \
                                     PIN_AFIO_AF(GPIOG_PIN7, 0))
#define VAL_GPIOG_AFRH              (PIN_AFIO_AF(GPIOG_PIN8, 0) |           \
                                     PIN_AFIO_AF(GPIOG_PIN9, 0) |           \
                                     PIN_AFIO_AF(GPIOG_PIN10, 0) |        \
                                     PIN_AFIO_AF(GPIOG_PIN11, 0) | \
                                     PIN_AFIO_AF(GPIOG_PIN12, 0) |          \
                                     PIN_AFIO_AF(GPIOG_PIN13, 0) | \
                                     PIN_AFIO_AF(GPIOG_PIN14, 0) | \
                                     PIN_AFIO_AF(GPIOG_PIN15, 0))

/*
 * GPIOH setup:
 *
 * PH0  - OSC_IN                    (input floating).
 * PH1  - OSC_OUT                   (input floating).
 * PH2  - PIN2                      (input pullup).
 * PH3  - PIN3                      (input pullup).
 * PH4  - PIN4                      (input pullup).
 * PH5  - PIN5                      (input pullup).
 * PH6  - PIN6                      (input pullup).
 * PH7  - PIN7                      (input pullup).
 * PH8  - PIN8                      (input pullup).
 * PH9  - PIN9                      (input pullup).
 * PH10 - PIN10                     (input pullup).
 * PH11 - PIN11                     (input pullup).
 * PH12 - PIN12                     (input pullup).
 * PH13 - PIN13                     (input pullup).
 * PH14 - PIN14                     (input pullup).
 * PH15 - PIN15                     (input pullup).
 */
#define VAL_GPIOH_MODER             (PIN_MODE_INPUT(GPIOH_OSC_IN) |         \
                                     PIN_MODE_INPUT(GPIOH_OSC_OUT) |        \
                                     PIN_MODE_INPUT(GPIOH_PIN2) |           \
                                     PIN_MODE_INPUT(GPIOH_PIN3) |           \
                                     PIN_MODE_INPUT(GPIOH_PIN4) |           \
                                     PIN_MODE_INPUT(GPIOH_PIN5) |           \
                                     PIN_MODE_INPUT(GPIOH_PIN6) |           \
                                     PIN_MODE_INPUT(GPIOH_PIN7) |           \
                                     PIN_MODE_INPUT(GPIOH_PIN8) |           \
                                     PIN_MODE_INPUT(GPIOH_PIN9) |           \
                                     PIN_MODE_INPUT(GPIOH_PIN10) |          \
                                     PIN_MODE_INPUT(GPIOH_PIN11) |          \
                                     PIN_MODE_INPUT(GPIOH_PIN12) |          \
                                     PIN_MODE_INPUT(GPIOH_PIN13) |          \
                                     PIN_MODE_INPUT(GPIOH_PIN14) |          \
                                     PIN_MODE_INPUT(GPIOH_PIN15))
#define VAL_GPIOH_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOH_OSC_IN) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOH_OSC_OUT) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN2) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN3) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN4) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN5) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN6) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN7) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN8) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN9) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN10) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN11) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN12) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN13) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN14) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN15))
#define VAL_GPIOH_OSPEEDR           (PIN_OSPEED_100M(GPIOH_OSC_IN) |        \
                                     PIN_OSPEED_100M(GPIOH_OSC_OUT) |       \
                                     PIN_OSPEED_100M(GPIOH_PIN2) |          \
                                     PIN_OSPEED_100M(GPIOH_PIN3) |          \
                                     PIN_OSPEED_100M(GPIOH_PIN4) |          \
                                     PIN_OSPEED_100M(GPIOH_PIN5) |          \
                                     PIN_OSPEED_100M(GPIOH_PIN6) |          \
                                     PIN_OSPEED_100M(GPIOH_PIN7) |          \
                                     PIN_OSPEED_100M(GPIOH_PIN8) |          \
                                     PIN_OSPEED_100M(GPIOH_PIN9) |          \
                                     PIN_OSPEED_100M(GPIOH_PIN10) |         \
                                     PIN_OSPEED_100M(GPIOH_PIN11) |         \
                                     PIN_OSPEED_100M(GPIOH_PIN12) |         \
                                     PIN_OSPEED_100M(GPIOH_PIN13) |         \
                                     PIN_OSPEED_100M(GPIOH_PIN14) |         \
                                     PIN_OSPEED_100M(GPIOH_PIN15))
#define VAL_GPIOH_PUPDR             (PIN_PUPDR_FLOATING(GPIOH_OSC_IN) |     \
                                     PIN_PUPDR_FLOATING(GPIOH_OSC_OUT) |    \
                                     PIN_PUPDR_PULLUP(GPIOH_PIN2) |         \
                                     PIN_PUPDR_PULLUP(GPIOH_PIN3) |         \
                                     PIN_PUPDR_PULLUP(GPIOH_PIN4) |         \
                                     PIN_PUPDR_PULLUP(GPIOH_PIN5) |         \
                                     PIN_PUPDR_PULLUP(GPIOH_PIN6) |         \
                                     PIN_PUPDR_PULLUP(GPIOH_PIN7) |         \
                                     PIN_PUPDR_PULLUP(GPIOH_PIN8) |         \
                                     PIN_PUPDR_PULLUP(GPIOH_PIN9) |         \
                                     PIN_PUPDR_PULLUP(GPIOH_PIN10) |        \
                                     PIN_PUPDR_PULLUP(GPIOH_PIN11) |        \
                                     PIN_PUPDR_PULLUP(GPIOH_PIN12) |        \
                                     PIN_PUPDR_PULLUP(GPIOH_PIN13) |        \
                                     PIN_PUPDR_PULLUP(GPIOH_PIN14) |        \
                                     PIN_PUPDR_PULLUP(GPIOH_PIN15))
#define VAL_GPIOH_ODR               (PIN_ODR_HIGH(GPIOH_OSC_IN) |           \
                                     PIN_ODR_HIGH(GPIOH_OSC_OUT) |          \
                                     PIN_ODR_HIGH(GPIOH_PIN2) |             \
                                     PIN_ODR_HIGH(GPIOH_PIN3) |             \
                                     PIN_ODR_HIGH(GPIOH_PIN4) |             \
                                     PIN_ODR_HIGH(GPIOH_PIN5) |             \
                                     PIN_ODR_HIGH(GPIOH_PIN6) |             \
                                     PIN_ODR_HIGH(GPIOH_PIN7) |             \
                                     PIN_ODR_HIGH(GPIOH_PIN8) |             \
                                     PIN_ODR_HIGH(GPIOH_PIN9) |             \
                                     PIN_ODR_HIGH(GPIOH_PIN10) |            \
                                     PIN_ODR_HIGH(GPIOH_PIN11) |            \
                                     PIN_ODR_HIGH(GPIOH_PIN12) |            \
                                     PIN_ODR_HIGH(GPIOH_PIN13) |            \
                                     PIN_ODR_HIGH(GPIOH_PIN14) |            \
                                     PIN_ODR_HIGH(GPIOH_PIN15))
#define VAL_GPIOH_AFRL              (PIN_AFIO_AF(GPIOH_OSC_IN, 0) |         \
                                     PIN_AFIO_AF(GPIOH_OSC_OUT, 0) |        \
                                     PIN_AFIO_AF(GPIOH_PIN2, 0) |           \
                                     PIN_AFIO_AF(GPIOH_PIN3, 0) |           \
                                     PIN_AFIO_AF(GPIOH_PIN4, 0) |           \
                                     PIN_AFIO_AF(GPIOH_PIN5, 0) |           \
                                     PIN_AFIO_AF(GPIOH_PIN6, 0) |           \
                                     PIN_AFIO_AF(GPIOH_PIN7, 0))
#define VAL_GPIOH_AFRH              (PIN_AFIO_AF(GPIOH_PIN8, 0) |           \
                                     PIN_AFIO_AF(GPIOH_PIN9, 0) |           \
                                     PIN_AFIO_AF(GPIOH_PIN10, 0) |          \
                                     PIN_AFIO_AF(GPIOH_PIN11, 0) |          \
                                     PIN_AFIO_AF(GPIOH_PIN12, 0) |          \
                                     PIN_AFIO_AF(GPIOH_PIN13, 0) |          \
                                     PIN_AFIO_AF(GPIOH_PIN14, 0) |          \
                                     PIN_AFIO_AF(GPIOH_PIN15, 0))

/*
 * GPIOI setup:
 *
 * PI0  - PIN0                      (input pullup).
 * PI1  - PIN1                      (input pullup).
 * PI2  - PIN2                      (input pullup).
 * PI3  - PIN3                      (input pullup).
 * PI4  - PIN4                      (input pullup).
 * PI5  - PIN5                      (input pullup).
 * PI6  - PIN6                      (input pullup).
 * PI7  - PIN7                      (input pullup).
 * PI8  - PIN8                      (input pullup).
 * PI9  - PIN9                      (input pullup).
 * PI10 - PIN10                     (input pullup).
 * PI11 - PIN11                     (input pullup).
 * PI12 - PIN12                     (input pullup).
 * PI13 - PIN13                     (input pullup).
 * PI14 - PIN14                     (input pullup).
 * PI15 - PIN15                     (input pullup).
 */
#define VAL_GPIOI_MODER             (PIN_MODE_INPUT(GPIOI_PIN0) |           \
                                     PIN_MODE_INPUT(GPIOI_PIN1) |           \
                                     PIN_MODE_INPUT(GPIOI_PIN2) |           \
                                     PIN_MODE_INPUT(GPIOI_PIN3) |           \
                                     PIN_MODE_INPUT(GPIOI_PIN4) |           \
                                     PIN_MODE_INPUT(GPIOI_PIN5) |           \
                                     PIN_MODE_INPUT(GPIOI_PIN6) |           \
                                     PIN_MODE_INPUT(GPIOI_PIN7) |           \
                                     PIN_MODE_INPUT(GPIOI_PIN8) |           \
                                     PIN_MODE_INPUT(GPIOI_PIN9) |           \
                                     PIN_MODE_INPUT(GPIOI_PIN10) |          \
                                     PIN_MODE_INPUT(GPIOI_PIN11) |          \
                                     PIN_MODE_INPUT(GPIOI_PIN12) |          \
                                     PIN_MODE_INPUT(GPIOI_PIN13) |          \
                                     PIN_MODE_INPUT(GPIOI_PIN14) |          \
                                     PIN_MODE_INPUT(GPIOI_PIN15))
#define VAL_GPIOI_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOI_PIN0) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN1) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN2) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN3) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN4) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN5) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN6) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN7) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN8) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN9) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN10) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN11) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN12) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN13) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN14) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN15))
#define VAL_GPIOI_OSPEEDR           (PIN_OSPEED_100M(GPIOI_PIN0) |          \
                                     PIN_OSPEED_100M(GPIOI_PIN1) |          \
                                     PIN_OSPEED_100M(GPIOI_PIN2) |          \
                                     PIN_OSPEED_100M(GPIOI_PIN3) |          \
                                     PIN_OSPEED_100M(GPIOI_PIN4) |          \
                                     PIN_OSPEED_100M(GPIOI_PIN5) |          \
                                     PIN_OSPEED_100M(GPIOI_PIN6) |          \
                                     PIN_OSPEED_100M(GPIOI_PIN7) |          \
                                     PIN_OSPEED_100M(GPIOI_PIN8) |          \
                                     PIN_OSPEED_100M(GPIOI_PIN9) |          \
                                     PIN_OSPEED_100M(GPIOI_PIN10) |         \
                                     PIN_OSPEED_100M(GPIOI_PIN11) |         \
                                     PIN_OSPEED_100M(GPIOI_PIN12) |         \
                                     PIN_OSPEED_100M(GPIOI_PIN13) |         \
                                     PIN_OSPEED_100M(GPIOI_PIN14) |         \
                                     PIN_OSPEED_100M(GPIOI_PIN15))
#define VAL_GPIOI_PUPDR             (PIN_PUPDR_PULLUP(GPIOI_PIN0) |         \
                                     PIN_PUPDR_PULLUP(GPIOI_PIN1) |         \
                                     PIN_PUPDR_PULLUP(GPIOI_PIN2) |         \
                                     PIN_PUPDR_PULLUP(GPIOI_PIN3) |         \
                                     PIN_PUPDR_PULLUP(GPIOI_PIN4) |         \
                                     PIN_PUPDR_PULLUP(GPIOI_PIN5) |         \
                                     PIN_PUPDR_PULLUP(GPIOI_PIN6) |         \
                                     PIN_PUPDR_PULLUP(GPIOI_PIN7) |         \
                                     PIN_PUPDR_PULLUP(GPIOI_PIN8) |         \
                                     PIN_PUPDR_PULLUP(GPIOI_PIN9) |         \
                                     PIN_PUPDR_PULLUP(GPIOI_PIN10) |        \
                                     PIN_PUPDR_PULLUP(GPIOI_PIN11) |        \
                                     PIN_PUPDR_PULLUP(GPIOI_PIN12) |        \
                                     PIN_PUPDR_PULLUP(GPIOI_PIN13) |        \
                                     PIN_PUPDR_PULLUP(GPIOI_PIN14) |        \
                                     PIN_PUPDR_PULLUP(GPIOI_PIN15))
#define VAL_GPIOI_ODR               (PIN_ODR_HIGH(GPIOI_PIN0) |             \
                                     PIN_ODR_HIGH(GPIOI_PIN1) |             \
                                     PIN_ODR_HIGH(GPIOI_PIN2) |             \
                                     PIN_ODR_HIGH(GPIOI_PIN3) |             \
                                     PIN_ODR_HIGH(GPIOI_PIN4) |             \
                                     PIN_ODR_HIGH(GPIOI_PIN5) |             \
                                     PIN_ODR_HIGH(GPIOI_PIN6) |             \
                                     PIN_ODR_HIGH(GPIOI_PIN7) |             \
                                     PIN_ODR_HIGH(GPIOI_PIN8) |             \
                                     PIN_ODR_HIGH(GPIOI_PIN9) |             \
                                     PIN_ODR_HIGH(GPIOI_PIN10) |            \
                                     PIN_ODR_HIGH(GPIOI_PIN11) |            \
                                     PIN_ODR_HIGH(GPIOI_PIN12) |            \
                                     PIN_ODR_HIGH(GPIOI_PIN13) |            \
                                     PIN_ODR_HIGH(GPIOI_PIN14) |            \
                                     PIN_ODR_HIGH(GPIOI_PIN15))
#define VAL_GPIOI_AFRL              (PIN_AFIO_AF(GPIOI_PIN0, 0) |           \
                                     PIN_AFIO_AF(GPIOI_PIN1, 0) |           \
                                     PIN_AFIO_AF(GPIOI_PIN2, 0) |           \
                                     PIN_AFIO_AF(GPIOI_PIN3, 0) |           \
                                     PIN_AFIO_AF(GPIOI_PIN4, 0) |           \
                                     PIN_AFIO_AF(GPIOI_PIN5, 0) |           \
                                     PIN_AFIO_AF(GPIOI_PIN6, 0) |           \
                                     PIN_AFIO_AF(GPIOI_PIN7, 0))
#define VAL_GPIOI_AFRH              (PIN_AFIO_AF(GPIOI_PIN8, 0) |           \
                                     PIN_AFIO_AF(GPIOI_PIN9, 0) |           \
                                     PIN_AFIO_AF(GPIOI_PIN10, 0) |          \
                                     PIN_AFIO_AF(GPIOI_PIN11, 0) |          \
                                     PIN_AFIO_AF(GPIOI_PIN12, 0) |          \
                                     PIN_AFIO_AF(GPIOI_PIN13, 0) |          \
                                     PIN_AFIO_AF(GPIOI_PIN14, 0) |          \
                                     PIN_AFIO_AF(GPIOI_PIN15, 0))


#if !defined(_FROM_ASM_)
#ifdef __cplusplus
extern "C" {
#endif
  void boardInit(void);
#ifdef __cplusplus
}
#endif
#endif /* _FROM_ASM_ */

#endif /* _BOARD_H_ */
