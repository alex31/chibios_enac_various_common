////////////////////////////////////////////////////////////////////////////
//
//  This file is inspired from RTIMULib-Arduino
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


 */

#if I2C_USE_IMU9250

#pragma once

#include "ch.h"
#include "hal.h"
#include "i2cMaster.h"

//  MPU9250 I2C Slave Addresses

#define MPU9250_ADDRESS0            0x68U
#define MPU9250_ADDRESS1            0x69U
#define MPU9250_ID                  0x71U



//  Register map

#define MPU9250_SMPRT_DIV           0x19U // 25
#define MPU9250_GYRO_LPF            0x1aU // 26
#define MPU9250_GYRO_CONFIG         0x1bU // 27
#define MPU9250_ACCEL_CONFIG        0x1cU // 28
#define MPU9250_ACCEL_LPF           0x1dU // 29
#define MPU9250_ACCEL_ODR	    0x1eU // 30
#define MPU9250_ACCEL_WOM_THRESHOLD 0x1fU // 31
#define MPU9250_FIFO_EN             0x23U 
#define MPU9250_I2C_MST_CTRL        0x24U
#define MPU9250_I2C_SLV0_ADDR       0x25U
#define MPU9250_I2C_SLV0_REG        0x26U
#define MPU9250_I2C_SLV0_CTRL       0x27U
#define MPU9250_I2C_SLV1_ADDR       0x28U
#define MPU9250_I2C_SLV1_REG        0x29U
#define MPU9250_I2C_SLV1_CTRL       0x2aU
#define MPU9250_I2C_SLV2_ADDR       0x2bU
#define MPU9250_I2C_SLV2_REG        0x2cU
#define MPU9250_I2C_SLV2_CTRL       0x2dU
#define MPU9250_I2C_SLV4_ADDR       0x31U
#define MPU9250_I2C_SLV4_REG        0x32U
#define MPU9250_I2C_SLV4_DO         0x33U
#define MPU9250_I2C_SLV4_CTRL       0x34U
#define MPU9250_I2C_SLV4_DI         0x35U
#define MPU9250_INT_PIN_CFG         0x37U
#define MPU9250_INT_ENABLE          0x38U
#define MPU9250_INT_STATUS          0x3aU
#define MPU9250_ACCEL_XOUT_H        0x3bU
#define MPU9250_GYRO_XOUT_H         0x43U
#define MPU9250_GYRO_ZOUT_L         0x48U
#define MPU9250_EXT_SENS_DATA_00    0x49U
#define MPU9250_EXT_SENS_DATA_23    0x60U
#define MPU9250_I2C_SLV0_DO         0x63U
#define MPU9250_I2C_SLV1_DO         0x64U
#define MPU9250_I2C_SLV2_DO         0x65U
#define MPU9250_I2C_SLV3_DO         0x66U
#define MPU9250_I2C_MST_DELAY_CTRL  0x67U // 103
#define MPU9250_ACCEL_ITR_CTRL      0x69U // 105
#define MPU9250_USER_CTRL           0x6aU
#define MPU9250_PWR_MGMT_1          0x6bU
#define MPU9250_PWR_MGMT_2          0x6cU
#define MPU9250_FIFO_COUNT_H        0x72U
#define MPU9250_FIFO_R_W            0x74U
#define MPU9250_WHO_AM_I            0x75U

#define  MPU9250_REGISTER_BASE      MPU9250_ACCEL_XOUT_H
#define  MPU9250_JUSTIMU_LAST	    MPU9250_GYRO_ZOUT_L
#define  MPU9250_REGISTER_LAST	    MPU9250_EXT_SENS_DATA_23
//  sample rate defines (applies to gyros and accels, not mags)

#define MPU9250_SAMPLERATE_MIN      5U                       // 5 samples per second is the lowest
#define MPU9250_SAMPLERATE_MAX      1000U                    // 1000 samples per second is the absolUte maximum

//  compass rate defines

#define MPU9250_COMPASSRATE_MIN     1U                       // 1 samples per second is the lowest
#define MPU9250_COMPASSRATE_MAX     100U                     // 100 samples per second is maximum

//  Gyro LPF options

#define MPU9250_GYRO_LPF_8800       0x11U                    // 8800Hz, 0.64mS delay
#define MPU9250_GYRO_LPF_3600       0x10U                    // 3600Hz, 0.11mS delay
#define MPU9250_GYRO_LPF_250        0x00U                    // 250Hz, 0.97mS delay
#define MPU9250_GYRO_LPF_184        0x01U                    // 184Hz, 2.9mS delay
#define MPU9250_GYRO_LPF_92         0x02U                    // 92Hz, 3.9mS delay
#define MPU9250_GYRO_LPF_41         0x03U                    // 41Hz, 5.9mS delay
#define MPU9250_GYRO_LPF_20         0x04U                    // 20Hz, 9.9mS delay
#define MPU9250_GYRO_LPF_10         0x05U                    // 10Hz, 17.85mS delay
#define MPU9250_GYRO_LPF_5          0x06U                    // 5Hz, 33.48mS delay

//  Gyro FSR options

#define MPU9250_GYROFSR_250         0U                       // +/- 250 degrees per second
#define MPU9250_GYROFSR_500         8U                       // +/- 500 degrees per second
#define MPU9250_GYROFSR_1000        0x10U                    // +/- 1000 degrees per second
#define MPU9250_GYROFSR_2000        0x18U                    // +/- 2000 degrees per second

//  Accel FSR options

#define MPU9250_ACCELFSR_2          0U                       // +/- 2g
#define MPU9250_ACCELFSR_4          8U                       // +/- 4g
#define MPU9250_ACCELFSR_8          0x10U                    // +/- 8g
#define MPU9250_ACCELFSR_16         0x18U                    // +/- 16g

//  Accel LPF options

#define MPU9250_ACCEL_LPF_1130      0x08U                    // 1130Hz, 0.75mS delay
#define MPU9250_ACCEL_LPF_460       0x00U                    // 460Hz, 1.94mS delay
#define MPU9250_ACCEL_LPF_184       0x01U                    // 184Hz, 5.80mS delay
#define MPU9250_ACCEL_LPF_92        0x02U                    // 92Hz, 7.80mS delay
#define MPU9250_ACCEL_LPF_41        0x03U                    // 41Hz, 11.80mS delay
#define MPU9250_ACCEL_LPF_20        0x04U                    // 20Hz, 19.80mS delay
#define MPU9250_ACCEL_LPF_10        0x05U                    // 10Hz, 35.70mS delay
#define MPU9250_ACCEL_LPF_5         0x06U                    // 5Hz, 66.96mS delay

// I2C MASTER TO SLAVE options

#define I2C_SLV_RNW		    (1U<<7)
#define I2C_ID_MSK		    0b01111111U
#define I2C_SLV_EN		    (1U<<7)
#define I2C_SLV_BYTE_SW		    (1U<<6)
#define I2C_SLV_REG_DIS		    (1U<<5)
#define I2C_SLV_GRP		    (1U<<4)
#define I2C_SLV_LENG_MSK	    0b00001111U


//  AK8963 compass registers

#define AK8963_ADDRESS              0x0cU
#define AK8963_WHO_AM_I             0x00U
#define AK8963_ID                   0x48U

#define AK8963_ST1                  0x02U                    // statUs 1
#define AK8963_HXL                  0x03U                    // values (3 x 2 bytes)
#define AK8963_HZH                  0x08U                    // last value
#define AK8963_ST2                  0x09U                    // control reg
#define AK8963_CNTL1                0x0aU                    // control reg
#define AK8963_CNTL2                0x0bU                    // control reg
#define AK8963_I2CDIS               0x0fU                    // control reg
#define AK8963_ASAX                 0x10U                    // start of the fuse ROM data
#define AK8963_REGISTER_BASE        AK8963_ST1
#define AK8963_REGISTER_LAST	    AK8963_ST2

// Compass option
#define AK8963_POWERDOWN	    0b0000U
#define AK8963_SINGLE_MESURE	    0b0001U	    
#define AK8963_CONTINUOUS_8HZ	    0b0010u
#define AK8963_CONTINUOUS_100HZ	    0b0110U	    
#define AK8963_FUSE_ROM	            0b1111U
#define AK8963_14BITS		    0b00000U	    
#define AK8963_16BITS		    0b10000U	    
#define AK8963_ST1_DATAREADY	    0b01U	    
#define AK8963_ST1_OVERRUN	    0b10U	    
#define AK8963_ST2_OVERFLOW	    0b1000U

// power management option
#define MPU9250_PWRM2_DISABLE_ACC  0b00111000
#define MPU9250_PWRM2_DISABLE_GYRO 0b00000111
#define MPU9250_PWRM1_HRESET	   (1<<7)
#define MPU9250_PWRM1_SLEEP	   (1<<6)
#define MPU9250_PWRM1_CYCLE	   (1<<5)
#define MPU9250_PWRM1_GYROSTANDBY  (1<<4)
#define MPU9250_PWRM1_PDPTAT	   (1<<3)
#define AK8963_I2CDIS_DISABLE	   0b00011011

// motion detection option
#define MPU9250_INTEL_ENABLE	   (1<<7)
#define MPU9250_INTEL_MODE_COMPARE (1<<6)

#define MPU9250_INT_PIN_CFG_ACTIVE_LOW		(1<<7)
#define MPU9250_INT_PIN_CFG_ACTIVE_HIGH		(0<<7)
#define MPU9250_INT_PIN_CFG_OPENDRAIN 		(1<<6)
#define MPU9250_INT_PIN_CFG_PUSHPULL 		(0<<6)
#define MPU9250_INT_PIN_CFG_LATCH_UNTIL_CLR  	(1<<5)
#define MPU9250_INT_PIN_CFG_PULSE_50_US  	(0<<5)
#define MPU9250_INT_PIN_CFG_CLR_ON_READ		(1<<4)

#define MPU9250_INT_PIN_CFG_FSYNC  		(1<<3)
#define MPU9250_INT_PIN_CFG_FSYNC_INT	 	(1<<2)
#define MPU9250_INT_PIN_CFG_BYPASS_EN  		(1<<1)

#define MPU9250_INT_ENABLE_WAKE_ON_MOTION	(1<<6)
#define MPU9250_INT_ENABLE_FIFO_OVERFLOW	(1<<4)
#define MPU9250_INT_ENABLE_FSYNC		(1<<3)
#define MPU9250_INT_ENABLE_RAW_READY		(1<<0)





//  FIFO transfer size
#define MPU9250_FIFO_CHUNK_SIZE     12U                      // gyro and accels take 12 bytes

// bitmask, obiousvly cannot have enabled and disabled bit at same time

typedef enum {IMU_MASTER,  IMU_BYPASS} Mpu9250_PassThroughMode;

typedef enum {IMU_TRANSFER_WRITE=0, IMU_TRANSFER_READ=I2C_SLV_RNW} Mpu9250_TransferWay;

typedef enum {IMU_NO_SWAP=0, IMU_SWAP_0_1=0b01000000, IMU_SWAP_1_2=0b01010000} Mpu9250_TransferSwapMode;

typedef enum {MPU9250_LOW_POWER_ACC_ODR_0_DOT_24_HZ=0,	
	      MPU9250_LOW_POWER_ACC_ODR_0_DOT_49_HZ,	
	      MPU9250_LOW_POWER_ACC_ODR_1_HZ,	
	      MPU9250_LOW_POWER_ACC_ODR_2_HZ,	
	      MPU9250_LOW_POWER_ACC_ODR_4_HZ,	
	      MPU9250_LOW_POWER_ACC_ODR_8_HZ,	
	      MPU9250_LOW_POWER_ACC_ODR_15_HZ,	
	      MPU9250_LOW_POWER_ACC_ODR_31_HZ,	
	      MPU9250_LOW_POWER_ACC_ODR_62_HZ,	
	      MPU9250_LOW_POWER_ACC_ODR_125_HZ,
	      MPU9250_LOW_POWER_ACC_ODR_250_HZ,
	      MPU9250_LOW_POWER_ACC_ODR_500_HZ,
	      MPU9250_NORMAL_POWER_ODR}  Mpu9250_LowPowerAccelerometerFrequencyCycle;



  

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
  Mpu9250Data *			mpu;
  const uint8_t *		cacheAdr;
  uint8_t			slvI2cAdr;
  uint8_t			slvRegStart;
  uint8_t			mapLen;
  uint8_t			slvDo; // I2C_SLV4_Dx : data to be written to slave x if way is WRITE
  Mpu9250_TransferWay		way;
  Mpu9250_TransferSwapMode	swapMode;
  bool				useMstDlyPrev;
} Mpu9250MasterConfig_0_to_3;

typedef struct  
{
  Mpu9250Data *			mpu;
  uint8_t			slvI2cAdr;
  uint8_t			slvReg;
  bool				enable;
  uint8_t			slvDo; // I2C_SLV4_DO : data to be written to slave 4 if enabled
  uint8_t			slvDi; // I2C_SLV4_DI : data read from i2c slave 4
  uint8_t			mstDlay; // master sample slave every (1+mstDlay) samples 
  Mpu9250_TransferWay		way;
  Mpu9250_TransferSwapMode	swapMode;
} Mpu9250MasterConfig_4;


typedef struct 
{
  Mpu9250MasterConfig_0_to_3	mc03[4];
  //  Mpu9250MasterConfig_4		mc4; /* not yet implemented */
} Mpu9250MasterConfig;

struct _Mpu9250Data
{
  Mpu9250MasterConfig	 mc;
  I2CDriver		*i2cd;

  uint32_t	sampleRate;     // imu sample rate in Hz
  uint32_t	auxSampleRate;	// sample rate on  auxiliary i2c bus
  uint32_t	sampleInterval; // interval between samples in tick
  uint32_t	cacheTimestamp;	// time stamp for raw data cache
  float		gyroScale;
  float		accelScale;
  uint8_t	rawCache[MPU9250_REGISTER_LAST-MPU9250_REGISTER_BASE];	// can cache all of readable segment

  Mpu9250_PassThroughMode	byPass;	// pathrough mode or bypass mode
  bool		accOnly;
  uint8_t			nextSlvFreeSlot;	// next slot for i2c slv managing
  uint8_t			slaveAddr;	// I2C address of MPU9250
  
  uint8_t	gyroLpf;        // gyro low pass filter setting
  uint8_t	accelLpf;       // accel low pass filter setting
  //  int32_t compassRate;                        // compass sample rate in Hz

  uint8_t	gyroFsr;	// gyro scale (250 to 2000 °/second)
  uint8_t	accelFsr;	// accel scale (+/- 2g to +/- 16g)
  uint8_t	registerSegmentLen;	// len of register to acquire 
  // to be put in AK8963 register map
  //  float compassAdjust[3];                             // the compass fuse ROM values converted for use
};

typedef struct  
{
  I2CDriver		*i2cd;
  Mpu9250MasterConfig    masterCfg;	// configuration of master module
  uint32_t		 sampleRate;	// imu sample rate in Hz
  uint32_t		 auxSampleRate;	// sample rate on  auxiliary i2c bus
  bool			 useAd0;	// I2C address offset pin enabled
  uint8_t		 gyroLpf;	// gyro low pass filter setting
  uint8_t		 accelLpf;	// accel low pass filter setting
  uint8_t		 gyroFsr;	// gyro scale (250 to 2000 °/second)
  uint8_t		 accelFsr;	// accel scale (+/- 2g to +/- 16g)
} Mpu9250Config;


typedef struct  
{
  I2CDriver			*i2cd;
  Mpu9250MasterConfig_0_to_3	*mstConfig;
  uint32_t			 cacheTimestamp; 
  uint32_t			 sampleInterval;	// interval between samples in tick
  ImuVec3f			 compassAdjust;	// the compass fuse ROM values converted for use
  uint8_t			 rawCache[AK8963_REGISTER_LAST-AK8963_REGISTER_BASE+1];
  uint8_t			 cntl1;	// down or single or continuous mode
} Ak8963Data;

typedef struct  
{
  ImuVec3f	mag;
  bool		dataReady;
  bool		overflow;
  bool		overrun;
} Ak8963Value;




msg_t mpu9250_init (Mpu9250Data *imu, const Mpu9250Config* initParam);
msg_t mpu9250_setGyroLpf (Mpu9250Data *imu, const uint8_t lpf);
msg_t mpu9250_setAccelLpf (Mpu9250Data *imu, const uint8_t lpf);
msg_t mpu9250_setSampleRate (Mpu9250Data *imu, const uint32_t rate);
msg_t mpu9250_setAuxSampleRate (Mpu9250Data *imu, const uint32_t rate);
msg_t mpu9250_setGyroFsr (Mpu9250Data *imu, const uint8_t fsr);
msg_t mpu9250_setAccelFsr (Mpu9250Data *imu, const uint8_t fsr);
msg_t mpu9250_setBypass (Mpu9250Data *imu, const Mpu9250_PassThroughMode mode);

msg_t mpu9250_cacheVal  (Mpu9250Data *imu);

// temp in celcius degree, gyro in rad/s, accel in m/s², 
msg_t mpu9250_getVal  (Mpu9250Data *imu, float *temp, 
		      ImuVec3f *gyro, ImuVec3f *acc);
msg_t mpu9250_getItrStatus  (Mpu9250Data *imu, uint8_t *itrStatus);
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

// mag in tesla
msg_t ak8963_getVal  (Ak8963Data *compass, Ak8963Value *mag);
msg_t mpu9250AddSlv_Ak8963 (Mpu9250Data *imu, Ak8963Data *compass);


/*
#                 _ __                      __  __            __ _               _            
#                | '_ \                    |  \/  |          / _` |             | |           
#                | |_) | __      __  _ __  | \  / |  _ __   | (_| |  _ __ ___   | |_          
#                | .__/  \ \ /\ / / | '__| | |\/| | | '_ \   \__, | | '_ ` _ \  | __|         
#                | |      \ V  V /  | |    | |  | | | | | |   __/ | | | | | | | \ |_          
#                |_|       \_/\_/   |_|    |_|  |_| |_| |_|  |___/  |_| |_| |_|  \__|         
*/

/*
  In order to put the mpu in low consumption motion detection, the apu function should be 
  called in this order :

  mpu9250_setModeAccOnly (...);
  mpu9250_activateMotionDetect (...);
  mpu9250_setModeAutoWake (...);

  if order to restore normal (without compass) operation :
  mpu9250_init (...);
 */



// disable gyro and mag for usual (I2C) measure. Disabling compass is not reversible
// until the mpu9250 is power cycled
msg_t mpu9250_setModeAccOnly (Mpu9250Data *imu, Ak8963Data *compass);

// Disable gyro and mag AND set imu in  low power cycle mode wich internally get acceleration
// at lpodr frequency (cf enum Mpu9250_LowPowerAccelerometerFrequencyCycle)
// useful if associated with MotionDetect
msg_t mpu9250_setModeAutoWake (Mpu9250Data *imu, 
			       Mpu9250_LowPowerAccelerometerFrequencyCycle lpodr);

// mode low power with motion detection, when motion is detected, interruption is made
// on itr pin of mpu9250 in order to awake MCU
// when the MCU is awaked, it has to reinit IMU to get actual measure.
// Motion detect can be used in normal or autowake mode
// config mask of the interrupt pin has to be given
msg_t mpu9250_activateMotionDetect (Mpu9250Data *imu, const uint32_t threadsholdInMilliG,
				    const uint8_t pinConfigMask);


// deep sleep (completely and definitly shutoff mpu, to be used again,
// to be used again, MPU has to be power cycled
msg_t mpu9250_setModeDeepSleep (Mpu9250Data *imu, Ak8963Data *compass);



#endif
