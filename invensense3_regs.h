/*
 * Copyright (C) 2022 Freek van Tienen <freek.v.tienen@gmail.com>
 *
 * This file is part of paparazzi.
 *
 * paparazzi is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * paparazzi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with paparazzi; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * @file peripherals/invensense3_regs.h
 *
 * Register and address definitions for the Invensense V3 from ardupilot.
 */

#pragma once
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifndef INVENSENSE3_USE_FIFO
#define INVENSENSE3_USE_FIFO 0
#endif

typedef enum {
  INV3_BANK0, INV3_BANK1, INV3_BANK2, INV3_BANK3, INV3_BANK4
} Inv3Bank;

#define INV3REG(b, r)      ((((uint16_t)b) << 8)|(r))
#define INV3_REG(br) (br & 0xff)
#define INV3_BANK(br) ((br >> 8) & 0xff)
#define INV3_READ_FLAG     0x80

//Register Map
#define INV3REG_DEVICE_CONFIG         INV3REG(INV3_BANK0,0x11U)
typedef enum  {
  BIT_DEVICE_CONFIG_SOFT_RESET_CONFIG =         0x01,
  BIT_DEVICE_CONFIG_SPI_MODE_1_2 =              0x10,
  BIT_DEVICE_CONFIG_SPI_MODE_0_3 =              0x0
} Inv3DeviceConfig;

#define INV3REG_INT_CONFIG         INV3REG(INV3_BANK0,0x14U)
typedef enum  {  
  INT1_ACTIVE_HIGH =				1<<0,
  INT1_ACTIVE_LOW =				0<<0,
  INT1_PUSHPULL =				1<<1,
  INT1_OPENDRAIN =				0<<1,
  INT1_LATCHED =				1<<2,
  INT1_PULSED =					0<<2,
  INT2_ACTIVE_HIGH =				1<<3,
  INT2_ACTIVE_LOW =				0<<3,
  INT2_PUSHPULL =				1<<4,
  INT2_OPENDRAIN =				0<<4,
  INT2_LATCHED =				1<<5,
  INT2_PULSED =					0<<5,
} Inv3InterruptConfig;

#define INV3REG_FIFO_CONFIG           INV3REG(INV3_BANK0,0x16U)
typedef enum {
  FIFO_CONFIG_MODE_BYPASS          =            0x00 << 6,
  FIFO_CONFIG_MODE_STREAM_TO_FIFO  =            0x01 << 6,
  FIFO_CONFIG_MODE_STOP_ON_FULL    =            0x02 << 6
} Inv3FifoConfig;

#define INV3REG_TEMP_DATA1            INV3REG(INV3_BANK0,0x1DU)
#define INV3REG_ACCEL_DATA_X1         INV3REG(INV3_BANK0,0x1FU)
#define INV3REG_INT_STATUS            INV3REG(INV3_BANK0,0x2DU)
#define INV3REG_FIFO_COUNTH           INV3REG(INV3_BANK0,0x2EU)
#define INV3REG_FIFO_COUNTL           INV3REG(INV3_BANK0,0x2FU)
#define INV3REG_FIFO_DATA             INV3REG(INV3_BANK0,0x30U)
#define INV3REG_SIGNAL_PATH_RESET     INV3REG(INV3_BANK0,0x4BU)
typedef enum {
  BIT_SIGNAL_PATH_RESET_FIFO_FLUSH       =      0x02,
  BIT_SIGNAL_PATH_RESET_TMST_STROBE      =      0x04,
  BIT_SIGNAL_PATH_RESET_ABORT_AND_RESET  =      0x08,
  BIT_SIGNAL_PATH_RESET_DMP_MEM_RESET_EN =      0x20,
  BIT_SIGNAL_PATH_RESET_DMP_INIT_EN      =      0x40
} Inv3SignalPathReset;

#define INV3REG_INTF_CONFIG0          INV3REG(INV3_BANK0,0x4CU)
typedef enum {
  UI_SIFS_CFG_SPI_DIS     =                     0x02,
  UI_SIFS_CFG_I2C_DIS     =                     0x03,
  UI_SIFS_CFG_SHIFT       =                     0x00,
  SENSOR_DATA_LITTLE_ENDIAN  =                  0x0,
  SENSOR_DATA_BIG_ENDIAN  =                     0x10,
  FIFO_COUNT_LITTLE_ENDIAN   =                  0x0,
  FIFO_COUNT_BIG_ENDIAN   =                     0x20,
  FIFO_COUNT_REC          =                     0x40,
  FIFO_HOLD_LAST_DATA_EN  =                     0x80
} Inv3IntFConfig0;

#define INV3REG_INTF_CONFIG1          INV3REG(INV3_BANK0,0x4DU)
typedef enum {
  CLKSEL_INTERNAL_RC =				0b00,
  CLKSEL_INTERNAL_PLL =				0b01,
  CLKSEL_INTERNAL_DISABLE =			0b11,
  CLKSEL_EXTERNAL_RTC =				0b1<<2,
  ACCEL_LP_CLK_SEL_DEFAULT =			0,
  ACCEL_LP_CLK_SEL_RC =				0b1<<3,
  CLOCKSEL_DEFAULT =				CLKSEL_INTERNAL_PLL			
} Inv3IntFConfig1;

#define INV3REG_SELF_TEST_CONFIG          INV3REG(INV3_BANK0,0x70U)
typedef enum {
  SELFTEST_DISABLE = 0,
  SELFTEST_ENABLE_GYRO = 0x7, SELFTEST_ENABLE_ACCEL = 0x78
} Inv3SelfTestConfig;

#define INV3REG_INTF_CONFIG5          INV3REG(INV3_BANK1,0x7BU)
typedef enum {
  PIN9_INT2 =					0b00 << 1,
  PIN9_FSYNC =					0b01 << 1,
  PIN9_CLKIN =					0b10 << 1,
} Inv3IntFConfig5;


#define INV3REG_PWR_MGMT0             INV3REG(INV3_BANK0,0x4EU)
typedef enum {
  ACCEL_MODE_OFF            =                   0x00,
  ACCEL_MODE_LN             =			0x03,
  GYRO_MODE_OFF             =                   0x00,
  GYRO_MODE_LN              =                   0x03 << 2,
  BIT_PWR_MGMT_IDLE         =                   0x08,
  BIT_PWM_MGMT_TEMP_DIS     =                   0x10
} Inv3PwrMgmt0;

#define INV3REG_GYRO_CONFIG0          INV3REG(INV3_BANK0,0x4FU)
typedef enum  {
  COMMON_ODR_MASK		=		  0x0f,
  COMMON_ODR_32KHZ              =                 0x01,
  COMMON_ODR_16KHZ              =                 0x02,
  COMMON_ODR_8KHZ               =                 0x03,
  COMMON_ODR_4KHZ               =                 0x04,
  COMMON_ODR_2KHZ               =                 0x05,
  COMMON_ODR_1KHZ               =                 0x06,
  COMMON_ODR_500HZ              =                 0x0F,
  COMMON_ODR_200HZ              =	          0x07,
  COMMON_ODR_100HZ              =                 0x08,
  COMMON_ODR_50HZ               =                 0x09,
  COMMON_ODR_25HZ               =                 0x0A,
  COMMON_ODR_12_5HZ             =                 0x0B,
  ACCEL_ODR_6_25HZ              =                 0x0C,
  ACCEL_ODR_3_125HZ             =                 0x0D,
  ACCEL_ODR_1_5625HZ            =                 0x0E,
} Inv3Odr;

typedef enum  {
  GYRO_FS_SHIFT		      =			5,
  GYRO_FS_MASK		      =			0x07 << 5,
  GYRO_FS_SEL_2000DPS         =                 0x00 << 5,
  GYRO_FS_SEL_1000DPS         =                 0x01 << 5,
  GYRO_FS_SEL_500DPS          =                 0x02 << 5,
  GYRO_FS_SEL_250DPS          =                 0x03 << 5,
  GYRO_FS_SEL_125DPS          =                 0x04 << 5,
  GYRO_FS_SEL_62_5DPS         =                 0x05 << 5,
  GYRO_FS_SEL_31_25DPS        =                 0x06 << 5,
  GYRO_FS_SEL_15_625DPS       =                 0x07 << 5
} Inv3GyroScale;

#define INV3REG_ACCEL_CONFIG0         INV3REG(INV3_BANK0,0x50U)
typedef enum {
  ACCEL_FS_SHIFT		=		5,
  ACCEL_FS_MASK		        =		0x07 << 5,
  ACCEL_FS_SEL_16G              =               0x00 << 5,
  ACCEL_FS_SEL_8G               =               0x01 << 5,
  ACCEL_FS_SEL_4G               =               0x02 << 5,
  ACCEL_FS_SEL_2G               =               0x03 << 5
} Inv3AccelScale;

#define INV3REG_GYRO_CONFIG1          INV3REG(INV3_BANK0,0x51U)
#define INV3REG_GYRO_ACCEL_CONFIG0    INV3REG(INV3_BANK0,0x52U)
#define INV3REG_ACCEL_CONFIG1         INV3REG(INV3_BANK0,0x53U)

#define INV3REG_TMST_CONFIG           INV3REG(INV3_BANK0,0x54U)
typedef enum {
  BIT_TMST_CONFIG_TMST_EN       =             0x1 << 0,
  BIT_TMST_FSYNC_EN		=             0x1 << 1,
  BIT_TMST_DELTA_EN		=             0x1 << 2,
  BIT_TMST_RES			=             0x1 << 3,
  BIT_TMST_TO_REGS_EN		=             0x1 << 4
} Inv3TmstConfig;

#define INV3REG_FIFO_CONFIG1          INV3REG(INV3_BANK0,0x5FU)
typedef enum {
  BIT_FIFO_CONFIG1_ACCEL_EN          =           0x01,
  BIT_FIFO_CONFIG1_GYRO_EN           =           0x02,
  BIT_FIFO_CONFIG1_TEMP_EN           =           0x04,
  BIT_FIFO_CONFIG1_TMST_FSYNC_EN     =           0x08,
  BIT_FIFO_CONFIG1_HIRES_EN          =           0x10,
  BIT_FIFO_CONFIG1_WM_GT_TH          =           0x20,
  BIT_FIFO_CONFIG1_RESUME_PARTIAL_RD =           0x40
} Inv3FifoConfig1;

#define INV3REG_FIFO_CONFIG2          INV3REG(INV3_BANK0,0x60U)
#define INV3REG_FIFO_CONFIG3          INV3REG(INV3_BANK0,0x61U)
#define INV3REG_INT_SOURCE0           INV3REG(INV3_BANK0,0x65U)
typedef enum {
  BIT_FIFO_FULL_INT1_EN         =               0x02,
  BIT_FIFO_THS_INT1_EN          =               0x04,
  BIT_UI_DRDY_INT1_EN           =               0x08
} Inv3IntSource0;

#define INV3REG_INT_SOURCE3           INV3REG(INV3_BANK0,0x68U)
typedef enum {
  BIT_FIFO_FULL_INT2_EN         =               0x02,
  BIT_FIFO_THS_INT2_EN          =               0x04,
  BIT_UI_DRDY_INT2_EN           =               0x08
} Inv3IntSource3;


#define INV3REG_INT_CONFIG0           INV3REG(INV3_BANK0,0x63U)
typedef enum {
  FIFO_FULL_CLEAR_STATUS_AND_FIFO_READ =	0b11 << 0,
  FIFO_THRESHOLD_CLEAR_STATUS_AND_FIFO_READ =	0b11 << 2,
  DATA_READY_CLEAR_STATUS_AND_REG_READ =	0b11 << 4,
} Inv3IntConfig0;

#define INV3REG_INT_CONFIG1           INV3REG(INV3_BANK0,0x64U)
typedef enum {
  BIT_INT_ASYNC_RESET  =                        1 << 4,
  BIT_INT_TDEASSERT_ODR4KPLUS =			1 << 5,
  INT_TPULSE_DURATION_ODR4KPLUS =		1 << 6
} Inv3IntConfig1;

#define INV3REG_WHO_AM_I              INV3REG(INV3_BANK0,0x75U)

#define INV3REG_GYRO_CONFIG_STATIC2   INV3REG(INV3_BANK1,0x0BU)
typedef enum {
  BIT_GYRO_NF_DIS          =                    0x01,
  BIT_GYRO_AAF_DIS         =                    0x02
} Inv3GyroConfigStatic2;

#define INV3REG_GYRO_CONFIG_STATIC3   INV3REG(INV3_BANK1,0x0CU)
typedef enum {
  GYRO_AAF_DELT_SHIFT           =              0x00
} Inv3GyroConfigStatic3;

#define INV3REG_GYRO_CONFIG_STATIC4   INV3REG(INV3_BANK1,0x0DU)
typedef enum {
  GYRO_AAF_DELTSQR_LOW_SHIFT    =	       0x00
} Inv3GyroConfigStatic4;

#define INV3REG_GYRO_CONFIG_STATIC5   INV3REG(INV3_BANK1,0x0EU)
typedef enum {
GYRO_AAF_DELTSQR_HIGH_SHIFT     =            0x00 //[11:8]
} Inv3GyroConfigStatic5;

#define INV3REG_GYRO_CONFIG_STATIC6   INV3REG(INV3_BANK1,0x0FU)
typedef enum {
  GYRO_X_NF_COSWZ_LOW_SHIFT       =             0x00 //[0:7]
} Inv3GyroConfigStatic6;

#define INV3REG_GYRO_CONFIG_STATIC7   INV3REG(INV3_BANK1,0x10U)
typedef enum {
  GYRO_Y_NF_COSWZ_LOW_SHIFT       =             0x00 //[0:7]
} Inv3GyroConfigStatic7;

#define INV3REG_GYRO_CONFIG_STATIC8   INV3REG(INV3_BANK1,0x11U)
typedef enum {
  GYRO_Z_NF_COSWZ_LOW_SHIFT       =            0x00 //[0:7]
} Inv3GyroConfigStatic8;

#define INV3REG_GYRO_CONFIG_STATIC9   INV3REG(INV3_BANK1,0x12U)
typedef enum {
  GYRO_X_NF_COSWZ_HIGH_SHIFT        =           0x00, //[8]
  GYRO_Y_NF_COSWZ_HIGH_SHIFT        =           0x01, //[8]
  GYRO_Z_NF_COSWZ_HIGH_SHIFT        =           0x02, //[8]
  GYRO_X_NF_COSWZ_SEL_SHIFT         =           0x03, //[0]
  GYRO_Y_NF_COSWZ_SEL_SHIFT         =           0x04, //[0]
  GYRO_Z_NF_COSWZ_SEL_SHIFT         =           0x05 //[0]
} Inv3GyroConfigStatic9;

#define INV3REG_GYRO_CONFIG_STATIC10  INV3REG(INV3_BANK1,0x13U)
typedef enum {
  GYRO_NF_BW_SEL_SHIFT              =           0x04
} Inv3GyroConfigStatic10;

#define INV3REG_XG_ST_DATA  INV3REG(INV3_BANK1,0x5FU)

#define INV3REG_ACCEL_CONFIG_STATIC2  INV3REG(INV3_BANK2,0x03U)
typedef enum {
  ACCEL_AAF_DIS                     =           0x01
} Inv3AccelConfigStatic2;

#define INV3REG_ACCEL_CONFIG_STATIC3  INV3REG(INV3_BANK2,0x04U)
typedef enum {
  ACCEL_AAF_DELTSQR_LOW_SHIFT       =           0x00 //[0:7]
} Inv3AccelConfigStatic3;

#define INV3REG_ACCEL_CONFIG_STATIC4  INV3REG(INV3_BANK2,0x05U)
typedef enum {
  ACCEL_AAF_DELTSQR_HIGH_SHIFT      =           0x00 //[11:8]
} Inv3AccelConfigStatic4;

#define INV3REG_XA_ST_DATA  INV3REG(INV3_BANK2,0x3BU)

#define INV3REG_CLOCKDIV  INV3REG(INV3_BANK3,0x2AU)

#define INV3REG_BANK_SEL              0x76

// WHOAMI values
typedef enum {
  INV3_WHOAMI_ICM40605  =    0x33,
  INV3_WHOAMI_ICM40609  =    0x3b,
  INV3_WHOAMI_ICM42605  =    0x42,
  INV3_WHOAMI_ICM42688  =    0x47,
  INV3_WHOAMI_IIM42652  =    0x6f,
  INV3_WHOAMI_ICM42670  =    0x67,
} Inv3SensorType;


typedef enum {INV3_NOTS=0, INV3_TS_ODR=2, INV3_TS_FSYNC=3} Inv3PacketTsType;

typedef struct  
{
  struct {
    uint8_t fifoEmpty  :1;
    uint8_t accAvail   :1;
    uint8_t gyroAvail  :1;
    uint8_t extended20 :1;
    Inv3PacketTsType tsType:2;
    uint8_t odrAccelChange :1;
    uint8_t gyroAccelChange :1;
  };
  int16_t acc[3];
  int16_t gyro[3];
  int8_t temp;
  uint16_t timeStamp;
} __attribute__ ((__packed__)) Inv3Packet3;
static_assert(sizeof(Inv3Packet3) == 16);


typedef struct {
  uint16_t	fifoCount;
  Inv3Packet3   fifoBuf[128]; // 2Kb buffer
} Inv3Packet3FifoBuffer;

#ifdef __cplusplus
}
#endif
