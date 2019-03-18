#pragma once

#include "ch.h"
#include "hal.h"

/*
  TODO : doxygen, reinit with NULL as config to be used in SPD3X driver
 */

typedef enum {
  LIS3_SLAVE_SA1_LOW  = 0x1C,
  LIS3_SLAVE_SA1_HIGH = 0x1E
} Lis3_NumSlave;


typedef enum  {
  LIS3_WHO_AM_I    = 0x0F,
  LIS3_WHO_AM_I_ANSWER    = 0x3D,
  
  LIS3_CTRL_REG1   = 0x20,
  LIS3_CTRL_REG2   = 0x21,
  LIS3_CTRL_REG3   = 0x22,
  LIS3_CTRL_REG4   = 0x23,
  LIS3_CTRL_REG5   = 0x24,
  
  LIS3_STATUS_REG  = 0x27,
  LIS3_OUT_X_L     = 0x28,
  LIS3_OUT_X_H     = 0x29,
  LIS3_OUT_Y_L     = 0x2A,
  LIS3_OUT_Y_H     = 0x2B,
  LIS3_OUT_Z_L     = 0x2C,
  LIS3_OUT_Z_H     = 0x2D,
  LIS3_TEMP_OUT_L  = 0x2E,
  LIS3_TEMP_OUT_H  = 0x2F,
  LIS3_INT_CFG     = 0x30,
  LIS3_INT_SRC     = 0x31,
  LIS3_INT_THS_L   = 0x32,
  LIS3_INT_THS_H   = 0x33
} Lis3_RegAddr ;

typedef enum {
  LIS3_CR1_SELFTEST_ENABLE =	(1),
  LIS3_CR1_SELFTEST_DISABLE =	(0),
  LIS3_CR1_FASTODR_ENABLE =	(1 << 1),
  LIS3_CR1_FASTODR_DISABLE =	(0 << 1),
  LIS3_CR1_DATARATE_0p6 =   (0x0 << 2),
  LIS3_CR1_DATARATE_1p2  =  (0x1 << 2),
  LIS3_CR1_DATARATE_2p52 =  (0x2 << 2),
  LIS3_CR1_DATARATE_5 =     (0x3 << 2),
  LIS3_CR1_DATARATE_10 =    (0x4 << 2),
  LIS3_CR1_DATARATE_20 =    (0x5 << 2),
  LIS3_CR1_DATARATE_40 =    (0x6 << 2),
  LIS3_CR1_DATARATE_80 =    (0x7 << 2),
  LIS3_CR1_OMXY_LOW =       (0x0 << 5),
  LIS3_CR1_OMXY_MEDIUM =    (0x1 << 5),
  LIS3_CR1_OMXY_HIGH =      (0x2 << 5),
  LIS3_CR1_OMXY_ULTRAHIGH = (0x0 << 5),
  LIS3_CR1_DATARATE_155 =   (LIS3_CR1_OMXY_ULTRAHIGH | LIS3_CR1_FASTODR_ENABLE),
  LIS3_CR1_DATARATE_300 =   (LIS3_CR1_OMXY_HIGH | LIS3_CR1_FASTODR_ENABLE),
  LIS3_CR1_DATARATE_560 =   (LIS3_CR1_OMXY_MEDIUM | LIS3_CR1_FASTODR_ENABLE),
  LIS3_CR1_DATARATE_1000 =  (LIS3_CR1_OMXY_LOW | LIS3_CR1_FASTODR_ENABLE),
  LIS3_CR1_TEMP_ENABLE =    (1 << 7),
  LIS3_CR1_TEMP_DISABLE =   (0 << 7)
  }  Lis3_Cr1;

typedef enum  {
  LIS3_CR2_SOFTRESET = (1 << 2),
  LIS3_CR2_REBOOT = (1 << 3),
  LIS3_CR2_SCALE_4_GAUSS = (0x0 << 5),
  LIS3_CR2_SCALE_8_GAUSS = (0x1 << 5),
  LIS3_CR2_SCALE_12_GAUSS = (0x2 << 5),
  LIS3_CR2_SCALE_16_GAUSS = (0x3 << 5),
} Lis3_CR2;

typedef enum  {
  LIS3_CR3_MODE_CONTINUOUS_CONV = (0),
  LIS3_CR3_MODE_SINGLE_CONV = (1),
  LIS3_CR3_MODE_POWERDOWN = (2),
  LIS3_CR3_SPI = (0 << 2),
  LIS3_CR3_SPI_3_WIRES = (1 << 2),
  LIS3_CR3_LOWPOWER  = (1 << 5)
} Lis3_Cr3;

typedef enum  {
  LIS3_CR4_LITTLE_ENDIAN = (0 << 1),
  LIS3_CR4_BIG_ENDIAN = (1 << 1),
  LIS3_CR4_OMZ_LOW = (0 << 2),
  LIS3_CR4_OMZ_MEDIUM = (1 << 2),
  LIS3_CR4_OMZ_HIGH = (2 << 2),
  LIS3_CR4_OMZ_ULTRAHIGH = (3 << 2),
} Lis3_Cr4;

typedef enum  {
  LIS3_CR5_UPDATE_CONTINUOUS = (0 << 6),
  LIS3_CR5_UPDATE_BDU = (1 << 6),
  LIS3_CR5_NORMALREAD =  (0 << 7),
  LIS3_CR5_FASTREAD =  (1 << 7),
} Lis3_Cr5;


typedef enum  {
  LIS3_INT_CFG_DISABLE = 0,
  LIS3_INT_CFG_ENABLE = 1,
  LIS3_INT_CFG_REQUEST_NOT_LATCHED = (0 << 1),
  LIS3_INT_CFG_REQUEST_LATCHED = (1 << 1),
  LIS3_INT_CFG_ACTIVE_LEVEL_LOW = (0 << 2),
  LIS3_INT_CFG_ACTIVE_LEVEL_HIGH = (1 << 2),
  LIS3_INT_CFG_ENABLE_Z = (1 << 5),
  LIS3_INT_CFG_ENABLE_Y = (1 << 6),
  LIS3_INT_CFG_ENABLE_X = (1 << 7)
} Lis3_interruptCfg;

typedef enum  {
  LIS3_INT_SRC_INTERRUPT_OCCURS = (1),
  LIS3_INT_SRC_MAGNETIC_OVERFLOW = (1 << 1),
  LIS3_INT_SRC_Z_THRESHOLD_EXCEED_NEG = (1 << 2),
  LIS3_INT_SRC_Y_THRESHOLD_EXCEED_NEG = (1 << 3),
  LIS3_INT_SRC_X_THRESHOLD_EXCEED_NEG = (1 << 4),
  LIS3_INT_SRC_Z_THRESHOLD_EXCEED_POS = (1 << 5),
  LIS3_INT_SRC_Y_THRESHOLD_EXCEED_POS = (1 << 6),
  LIS3_INT_SRC_X_THRESHOLD_EXCEED_POS = (1 << 7)
} Lis3_InterruptSource;

typedef enum  {
  LIS3_STATUS_X_AVAIL = (1),
  LIS3_STATUS_Y_AVAIL = (1 << 1),
  LIS3_STATUS_Z_AVAIL = (1 << 2),
  LIS3_STATUS_ZYX_AVAIL = (1 << 3),
  LIS3_STATUS_X_OVERRUN = (1 << 4),
  LIS3_STATUS_Y_OVERRUN = (1 << 5),
  LIS3_STATUS_Z_OVERRUN = (1 << 6),
  LIS3_STATUS_ZYX_OVERRUN = (1 << 7)
} Lis3_Status;

typedef enum  {
  LIS3_TEST_PASS = 0,
  LIS3_I2C_ERROR = (1 << 0),
  LIS3_X_TEST_FAIL = (1 << 1),
  LIS3_Y_TEST_FAIL = (1 << 2),
  LIS3_Z_TEST_FAIL = (1 << 3)
} Lis3_ErrorMask;

typedef struct {
  uint8_t cr[5];
  uint8_t intCfg;
  uint16_t threshold;
} Lis3mdlConfigRegister;

typedef struct {
  I2CDriver *i2cp;
  uint8_t numSlave;
  Lis3mdlConfigRegister regs;
} Lis3mdlConfig;

typedef struct __attribute__((packed)) {
  uint8_t status;
  union __attribute__((packed)) {
    int16_t out[3];
    struct __attribute__((packed)) {
      int16_t outX;
      int16_t outY;
      int16_t outZ;
    };
  };
  int16_t temp;
  uint8_t intCfg;
  uint8_t intSrc;
} Lis3mdlRaw;

typedef struct {
  const Lis3mdlConfig *config;
  Lis3mdlRaw	raw;
} Lis3mdlDriver;




msg_t lis3mdlStart(Lis3mdlDriver *ldp, const Lis3mdlConfig *cfg);
msg_t lis3mdlWaitUntilDataReady(Lis3mdlDriver *ldp);
msg_t lis3mdlFetch(Lis3mdlDriver *ldp, const Lis3_RegAddr first,
			  const Lis3_RegAddr last);
void lis3mdlGetMag(const Lis3mdlDriver *ldp, Vec3f *mag);
void lis3mdlGetNormalizedMag(const Lis3mdlDriver *ldp, Vec3f *normMag);
float lis3mdlGetTemp(const Lis3mdlDriver *ldp);
Lis3_Status lis3mdlGetStatus(const Lis3mdlDriver *ldp);
Lis3_InterruptSource lis3mdlGetIntSource(const Lis3mdlDriver *ldp);
Lis3_ErrorMask lis3mdlLaunchTest(Lis3mdlDriver *ldp);
