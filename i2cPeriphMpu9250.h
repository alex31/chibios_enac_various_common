////////////////////////////////////////////////////////////////////////////
//
//  This file is sinspired from RTIMULib-Arduino
//
//  Copyright (c) 2014-2015, richards-tech
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of
//  this software and associated documentation files (the "Software"), to deal in
//  the Software without restriction, including without limitation the rights to use,
//  copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
//  Software, and to permit persons to whom the Software is furnished to do so,
//  subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
//  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
//  PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
//  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
//  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


/*
#         _                  _
#        | |                | |
#        | |_     ___     __| |    ___
#        | __|   / _ \   / _` |   / _ \
#        \ |_   | (_) | | (_| |  | (_) |
#         \__|   \___/   \__,_|   \___/


° structure et fonctions pour le 9250
° structure et fonctions pour le AK8963
° structure de passage de param : mapping registre entre 9250 et 8963 partagé entre les deux objets
    mapping : [adr AK <-> adr 9250 <-> adr cache] + len
    fonction de recherche par adr magneto donne un ptr sur l'objet
° sur le AK8963 : api de recuperation de valeurs qui fait l'abstraction du mode (esclave ou transparent)
° le 9250 contient un tableau de structure de mapping et des fonctions pour retourner un pointeur sur
  un mapping en fonction de son index [0-4]
° le AK8963 a un ptr sur le 9250 et l'index de mapping
  fonction de mapping imu9250AssociateSlave (IV9250* iv, index, AK8363* ak) (faudra la même pour le baro ...)




 */

#if I2C_USE_IMU9250

#pragma once

#include "ch.h"
#include "hal.h"
#include "i2cMaster.h"

//  MPU9250 I2C Slave Addresses

#define MPU9250_ADDRESS0            0x68
#define MPU9250_ADDRESS1            0x69
#define MPU9250_ID                  0x71



//  Register map

#define MPU9250_SMPRT_DIV           0x19 // 25
#define MPU9250_GYRO_LPF            0x1a // 26
#define MPU9250_GYRO_CONFIG         0x1b // 27
#define MPU9250_ACCEL_CONFIG        0x1c // 28
#define MPU9250_ACCEL_LPF           0x1d // 29
#define MPU9250_FIFO_EN             0x23 
#define MPU9250_I2C_MST_CTRL        0x24
#define MPU9250_I2C_SLV0_ADDR       0x25
#define MPU9250_I2C_SLV0_REG        0x26
#define MPU9250_I2C_SLV0_CTRL       0x27
#define MPU9250_I2C_SLV1_ADDR       0x28
#define MPU9250_I2C_SLV1_REG        0x29
#define MPU9250_I2C_SLV1_CTRL       0x2a
#define MPU9250_I2C_SLV2_ADDR       0x2b
#define MPU9250_I2C_SLV2_REG        0x2c
#define MPU9250_I2C_SLV2_CTRL       0x2d
#define MPU9250_I2C_SLV4_ADDR       0x31
#define MPU9250_I2C_SLV4_REG        0x32
#define MPU9250_I2C_SLV4_DO         0x33
#define MPU9250_I2C_SLV4_CTRL       0x34
#define MPU9250_I2C_SLV4_DI         0x35
#define MPU9250_INT_PIN_CFG         0x37
#define MPU9250_INT_ENABLE          0x38
#define MPU9250_INT_STATUS          0x3a
#define MPU9250_ACCEL_XOUT_H        0x3b
#define MPU9250_GYRO_XOUT_H         0x43
#define MPU9250_GYRO_ZOUT_L         0x48
#define MPU9250_EXT_SENS_DATA_00    0x49
#define MPU9250_EXT_SENS_DATA_23    0x60
#define MPU9250_I2C_SLV0_DO         0x63
#define MPU9250_I2C_SLV1_DO         0x64
#define MPU9250_I2C_SLV2_DO         0x65
#define MPU9250_I2C_SLV3_DO         0x66
#define MPU9250_I2C_MST_DELAY_CTRL  0x67 // 103
#define MPU9250_USER_CTRL           0x6a
#define MPU9250_PWR_MGMT_1          0x6b
#define MPU9250_PWR_MGMT_2          0x6c
#define MPU9250_FIFO_COUNT_H        0x72
#define MPU9250_FIFO_R_W            0x74
#define MPU9250_WHO_AM_I            0x75

#define  MPU9250_REGISTER_BASE      MPU9250_ACCEL_XOUT_H
#define  MPU9250_JUSTIMU_LAST	    MPU9250_GYRO_ZOUT_L
#define  MPU9250_REGISTER_LAST	    MPU9250_EXT_SENS_DATA_23
//  sample rate defines (applies to gyros and accels, not mags)

#define MPU9250_SAMPLERATE_MIN      5                       // 5 samples per second is the lowest
#define MPU9250_SAMPLERATE_MAX      1000                    // 1000 samples per second is the absolute maximum

//  compass rate defines

#define MPU9250_COMPASSRATE_MIN     1                       // 1 samples per second is the lowest
#define MPU9250_COMPASSRATE_MAX     100                     // 100 samples per second is maximum

//  Gyro LPF options

#define MPU9250_GYRO_LPF_8800       0x11                    // 8800Hz, 0.64mS delay
#define MPU9250_GYRO_LPF_3600       0x10                    // 3600Hz, 0.11mS delay
#define MPU9250_GYRO_LPF_250        0x00                    // 250Hz, 0.97mS delay
#define MPU9250_GYRO_LPF_184        0x01                    // 184Hz, 2.9mS delay
#define MPU9250_GYRO_LPF_92         0x02                    // 92Hz, 3.9mS delay
#define MPU9250_GYRO_LPF_41         0x03                    // 41Hz, 5.9mS delay
#define MPU9250_GYRO_LPF_20         0x04                    // 20Hz, 9.9mS delay
#define MPU9250_GYRO_LPF_10         0x05                    // 10Hz, 17.85mS delay
#define MPU9250_GYRO_LPF_5          0x06                    // 5Hz, 33.48mS delay

//  Gyro FSR options

#define MPU9250_GYROFSR_250         0                       // +/- 250 degrees per second
#define MPU9250_GYROFSR_500         8                       // +/- 500 degrees per second
#define MPU9250_GYROFSR_1000        0x10                    // +/- 1000 degrees per second
#define MPU9250_GYROFSR_2000        0x18                    // +/- 2000 degrees per second

//  Accel FSR options

#define MPU9250_ACCELFSR_2          0                       // +/- 2g
#define MPU9250_ACCELFSR_4          8                       // +/- 4g
#define MPU9250_ACCELFSR_8          0x10                    // +/- 8g
#define MPU9250_ACCELFSR_16         0x18                    // +/- 16g

//  Accel LPF options

#define MPU9250_ACCEL_LPF_1130      0x08                    // 1130Hz, 0.75mS delay
#define MPU9250_ACCEL_LPF_460       0x00                    // 460Hz, 1.94mS delay
#define MPU9250_ACCEL_LPF_184       0x01                    // 184Hz, 5.80mS delay
#define MPU9250_ACCEL_LPF_92        0x02                    // 92Hz, 7.80mS delay
#define MPU9250_ACCEL_LPF_41        0x03                    // 41Hz, 11.80mS delay
#define MPU9250_ACCEL_LPF_20        0x04                    // 20Hz, 19.80mS delay
#define MPU9250_ACCEL_LPF_10        0x05                    // 10Hz, 35.70mS delay
#define MPU9250_ACCEL_LPF_5         0x06                    // 5Hz, 66.96mS delay

// I2C MASTER TO SLAVE options

#define I2C_SLV_RNW		    (1<<7)
#define I2C_ID_MSK		    0b01111111
#define I2C_SLV_EN		    (1<<7)
#define I2C_SLV_BYTE_SW		    (1<<6)
#define I2C_SLV_REG_DIS		    (1<<5)
#define I2C_SLV_GRP		    (1<<4)
#define I2C_SLV_LENG_MSK	    0b00001111


//  AK8963 compass registers

#define AK8963_ADDRESS              0x0c
#define AK8963_WHO_AM_I             0x00
#define AK8963_ID                   0x48

#define AK8963_ST1                  0x02                    // status 1
#define AK8963_HXL                  0x03                    // values (3 x 2 bytes)
#define AK8963_HZH                  0x08                    // last value
#define AK8963_ST2                  0x09                    // control reg
#define AK8963_CNTL                 0x0a                    // control reg
#define AK8963_ASAX                 0x10                    // start of the fuse ROM data
#define AK8963_REGISTER_BASE        AK8963_ST1
#define AK8963_REGISTER_LAST	    AK8963_ST2

// Compass option
#define AK8963_POWERDOWN	    0b0000
#define AK8963_SINGLE_MESURE	    0b0001	    
#define AK8963_CONTINUOUS_8HZ	    0b0010
#define AK8963_CONTINUOUS_100HZ	    0b0110	    
#define AK8963_FUSE_ROM	            0b1111
#define AK8963_14BITS		    0b00000	    
#define AK8963_16BITS		    0b10000	    
#define AK8963_ST1_DATAREADY	    0b01	    
#define AK8963_ST1_OVERRUN	    0b10	    
#define AK8963_ST2_OVERFLOW	    0b1000

//  FIFO transfer size

#define MPU9250_FIFO_CHUNK_SIZE     12                      // gyro and accels take 12 bytes

typedef enum {IMU_MASTER,  IMU_BYPASS} PassThroughMode;
typedef enum {IMU_TRANSFER_WRITE=0, IMU_TRANSFER_READ=I2C_SLV_RNW} TransferWay;
typedef enum {IMU_NO_SWAP=0, IMU_SWAP_0_1=0b01000000, IMU_SWAP_1_2=0b01010000} TransferSwapMode;



/*
#                 __  __   _____    _    _              ___    ___    _____    ___          
#                |  \/  | |  __ \  | |  | |            / _ \  |__ \  | ____|  / _ \         
#                | \  / | | |__) | | |  | |           | (_) |    ) | | |__   | | | |        
#                | |\/| | |  ___/  | |  | |            \__, |   / /  |___ \  | | | |        
#                | |  | | | |      | |__| |  ______      / /   / /_   ___) | | |_| |        
#                |_|  |_| |_|       \____/  |______|    /_/   |____| |____/   \___/         
*/

struct _Mpu9250Data;
typedef struct _Mpu9250Data  Mpu9250Data;

typedef struct  
{
  Mpu9250Data *		mpu;
  const uint8_t *	cacheAdr;
  uint8_t		slvI2cAdr;
  uint8_t		slvRegStart;
  uint8_t		mapLen;
  uint8_t		slvDo; // I2C_SLV4_Dx : data to be written to slave x if way is WRITE
  TransferWay		way;
  TransferSwapMode	swapMode;
  bool_t		useMstDlyPrev;
} Mpu9250MasterConfig_0_to_3;

typedef struct  
{
  Mpu9250Data *		mpu;
  uint8_t		slvI2cAdr;
  uint8_t		slvReg;
  bool_t		enable;
  uint8_t		slvDo; // I2C_SLV4_DO : data to be written to slave 4 if enabled
  uint8_t		slvDi; // I2C_SLV4_DI : data read from i2c slave 4
  uint8_t		mstDlay; // master sample slave every (1+mstDlay) samples 
  TransferWay		way;
  TransferSwapMode	swapMode;
} Mpu9250MasterConfig_4;


typedef struct 
{
  Mpu9250MasterConfig_0_to_3	mc03[4];
  //  Mpu9250MasterConfig_4		mc4; /* not yet implemented */
} Mpu9250MasterConfig;

struct _Mpu9250Data
{
  Mpu9250MasterConfig mc;
  I2CDriver *i2cd;

  int32_t sampleRate;                             // imu sample rate in Hz
  uint32_t auxSampleRate;			  // sample rate on  auxiliary i2c bus
  uint32_t sampleInterval;                       // interval between samples in tick
  uint32_t cacheTimestamp;			  // time stamp for raw data cache
  float gyroScale;
  float accelScale;
  uint8_t rawCache[MPU9250_REGISTER_LAST-MPU9250_REGISTER_BASE]; // can cache all of readable segment

  PassThroughMode byPass;			  // pathrough mode or bypass mode
  uint8_t nextSlvFreeSlot;			 // next slot for i2c slv managing
  uint8_t slaveAddr;                             // I2C address of MPU9250
  
  uint8_t gyroLpf;                                // gyro low pass filter setting
  uint8_t accelLpf;                               // accel low pass filter setting
  //  int32_t compassRate;                        // compass sample rate in Hz

  uint8_t gyroFsr;				  // gyro scale (250 to 2000 °/second)
  uint8_t accelFsr;				  // accel scale (+/- 2g to +/- 16g)
  uint8_t registerSegmentLen;			  // len of register to acquire 
  // to be put in AK8963 register map
  //  float compassAdjust[3];                             // the compass fuse ROM values converted for use
};

typedef struct  
{
  I2CDriver *i2cd;
  Mpu9250MasterConfig masterCfg;		  // configuration of master module
  uint32_t sampleRate;                             // imu sample rate in Hz
  uint32_t auxSampleRate;			  // sample rate on  auxiliary i2c bus
  bool_t  useAd0;				  // I2C address offset pin enabled
  uint8_t gyroLpf;                                // gyro low pass filter setting
  uint8_t accelLpf;                               // accel low pass filter setting
  uint8_t gyroFsr;				  // gyro scale (250 to 2000 °/second)
  uint8_t accelFsr;				  // accel scale (+/- 2g to +/- 16g)
} Mpu9250Config;


typedef struct  
{
  I2CDriver *i2cd;
  Mpu9250MasterConfig_0_to_3 *mstConfig;
  uint32_t cacheTimestamp; 
  uint32_t sampleInterval;                       // interval between samples in tick
  ImuVec3f compassAdjust;                         // the compass fuse ROM values converted for use
  uint8_t rawCache[AK8963_REGISTER_LAST-AK8963_REGISTER_BASE+1];
  uint8_t cntl1;                                  // down or single or continuous mode
} Ak8963Data;

typedef struct  
{
  ImuVec3f mag;
  bool_t  dataReady;
  bool_t  overflow;
  bool_t  overrun;
} Ak8963Value;




msg_t mpu9250_init (Mpu9250Data *imu, const Mpu9250Config* initParam);
msg_t mpu9250_setGyroLpf (Mpu9250Data *imu, const uint8_t lpf);
msg_t mpu9250_setAccelLpf (Mpu9250Data *imu, const uint8_t lpf);
msg_t mpu9250_setSampleRate (Mpu9250Data *imu, const int32_t rate);
msg_t mpu9250_setAuxSampleRate (Mpu9250Data *imu, const int32_t rate);
msg_t mpu9250_setGyroFsr (Mpu9250Data *imu, const uint8_t fsr);
msg_t mpu9250_setAccelFsr (Mpu9250Data *imu, const uint8_t fsr);
msg_t mpu9250_setBypass (Mpu9250Data *imu, const PassThroughMode mode);  
msg_t mpu9250_cacheVal  (Mpu9250Data *imu);
msg_t mpu9250_getVal  (Mpu9250Data *imu, float *temp, 
		      ImuVec3f *gyro, ImuVec3f *acc);
msg_t mpu9250_getDevid (Mpu9250Data *imu, uint8_t *devid);



/*
#                  ___    _  __             ___     ___      __    ____          
#                 / _ \  | |/ /            / _ \   / _ \    / /   |___ \         
#                | |_| | | ' /            | (_) | | (_) |  / /_     __) |        
#                |  _  | |  <              > _ <   \__, | | '_ \   |__ <         
#                | | | | | . \   ______   | (_) |    / /  | (_) |  ___) |        
#                |_| |_| |_|\_\ |______|   \___/    /_/    \___/  |____/         
*/

msg_t ak8963_init (Ak8963Data *compass, I2CDriver *i2cd);
msg_t ak8963_getDevid (Ak8963Data *compass, uint8_t *devid);
msg_t ak8963_setCompassCntl (Ak8963Data *compass, const uint8_t cntl);
msg_t ak8963_cacheVal  (Ak8963Data *compass);
msg_t ak8963_getVal  (Ak8963Data *compass, Ak8963Value *mag);
msg_t mpu9250AddSlv_Ak8963 (Mpu9250Data *imu, Ak8963Data *compass);



#endif
