#if I2C_USE_MPL3115A2

#pragma once

#include "ch.h"
#include "hal.h"
#include "i2cMaster.h"
#include "i2cPeriphMpu9250.h"

#define MPL3115A2_ADDRESS 0x60 // Unshifted 7-bit I2C address for sensor

#define  MPL3115A2_STATUS     		0x00
#define  MPL3115A2_OUT_P_MSB  		0x01
#define  MPL3115A2_OUT_P_CSB  		0x02
#define  MPL3115A2_OUT_P_LSB  		0x03
#define  MPL3115A2_OUT_T_MSB  		0x04
#define  MPL3115A2_OUT_T_LSB  		0x05
#define  MPL3115A2_DR_STATUS  		0x06
#define  MPL3115A2_OUT_P_DELTA_MSB  	0x07
#define  MPL3115A2_OUT_P_DELTA_CSB  	0x08
#define  MPL3115A2_OUT_P_DELTA_LSB  	0x09
#define  MPL3115A2_OUT_T_DELTA_MSB  	0x0A
#define  MPL3115A2_OUT_T_DELTA_LSB  	0x0B
#define  MPL3115A2_WHO_AM_I   		0x0C
#define  MPL3115A2_F_STATUS   		0x0D
#define  MPL3115A2_F_DATA     		0x0E
#define  MPL3115A2_F_SETUP    		0x0F
#define  MPL3115A2_TIME_DLY   		0x10
#define  MPL3115A2_SYSMOD     		0x11
#define  MPL3115A2_INT_SOURCE 		0x12
#define  MPL3115A2_PT_DATA_CFG 		0x13
#define  MPL3115A2_BAR_IN_MSB 		0x14
#define  MPL3115A2_BAR_IN_LSB 		0x15
#define  MPL3115A2_P_TGT_MSB  		0x16
#define  MPL3115A2_P_TGT_LSB  		0x17
#define  MPL3115A2_T_TGT      		0x18
#define  MPL3115A2_P_WND_MSB  		0x19
#define  MPL3115A2_P_WND_LSB  		0x1A
#define  MPL3115A2_T_WND      		0x1B
#define  MPL3115A2_P_MIN_MSB  		0x1C
#define  MPL3115A2_P_MIN_CSB  		0x1D
#define  MPL3115A2_P_MIN_LSB  		0x1E
#define  MPL3115A2_T_MIN_MSB  		0x1F
#define  MPL3115A2_T_MIN_LSB  		0x20
#define  MPL3115A2_P_MAX_MSB  		0x21
#define  MPL3115A2_P_MAX_CSB  		0x22
#define  MPL3115A2_P_MAX_LSB  		0x23
#define  MPL3115A2_T_MAX_MSB  		0x24
#define  MPL3115A2_T_MAX_LSB  		0x25
#define  MPL3115A2_CTRL_REG1  		0x26
#define  MPL3115A2_CTRL_REG2  		0x27
#define  MPL3115A2_CTRL_REG3  		0x28
#define  MPL3115A2_CTRL_REG4  		0x29
#define  MPL3115A2_CTRL_REG5  		0x2A
#define  MPL3115A2_OFF_P      		0x2B
#define  MPL3115A2_OFF_T      		0x2C
#define  MPL3115A2_OFF_H		0x2D

#define  MPL3115A2_ID 0xC4

#define  MPL3115A2_SAMPLE_PERIOD_006MS 0b000000
#define  MPL3115A2_SAMPLE_PERIOD_010MS 0b001000
#define  MPL3115A2_SAMPLE_PERIOD_018MS 0b010000
#define  MPL3115A2_SAMPLE_PERIOD_034MS 0b011000
#define  MPL3115A2_SAMPLE_PERIOD_066MS 0b100000
#define  MPL3115A2_SAMPLE_PERIOD_130MS 0b101000
#define  MPL3115A2_SAMPLE_PERIOD_258MS 0b110000
#define  MPL3115A2_SAMPLE_PERIOD_512MS 0b111000

#define  MPL3115A2_DATA_READY_EVENT_MODE  0b100
#define  MPL3115A2_DATA_READY_PRESSURE    0b010
#define  MPL3115A2_DATA_READY_TEMPERATURE 0b001

#define  MPL3115A2_CONTINUOUS_MODE 0b1
#define  MPL3115A2_ONESHOT_MODE    0b0
#define  MPL3115A2_ONESHOT_INIT    0b10
#define  MPL3115A2_RESET	   0b100

#define  MPL3115A2_TEMPERATURE_AVAIL	0b10
#define  MPL3115A2_PRESSURE_AVAIL	0b100
#define  MPL3115A2_DATA_AVAIL		0b1000
#define  MPL3115A2_TEMPERATURE_OWRITE	0b10000
#define  MPL3115A2_PRESSURE_OWRITE	0b100000
#define  MPL3115A2_DATA_OWRITEL		0b1000000



typedef struct  
{
  I2CDriver *i2cd;
  Mpu9250MasterConfig_0_to_3 *mstConfig;
  uint32_t cacheTimestamp; 
  uint32_t sampleInterval;                       // interval between samples in tick
  uint8_t rawCache[MPL3115A2_OUT_P_LSB-MPL3115A2_OUT_P_MSB+1];
  uint8_t oversampling;
} MPL3115A2Data;


msg_t MPL3115A2_init (MPL3115A2Data *baro, I2CDriver *i2cd, uint8_t oversampling);
msg_t MPL3115A2_getDevid (MPL3115A2Data *baro, uint8_t *devid);
msg_t MPL3115A2_setOversampling (MPL3115A2Data *baro, uint8_t oversampling);
msg_t MPL3115A2_cacheVal  (MPL3115A2Data *baro);
msg_t MPL3115A2_getVal  (MPL3115A2Data *baro, float *pressure);
msg_t mpu9250AddSlv_MPL3115A2 (Mpu9250Data *imu, MPL3115A2Data *baro);

#endif
