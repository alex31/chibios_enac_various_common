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



#ifndef I2C_USE_IMU20600
#define I2C_USE_IMU20600 0
#endif

#if I2C_USE_IMU20600

#pragma once

#include "ch.h"
#include "hal.h"
#include "i2cMaster.h"

//  MPU20600 I2C Slave Addresses

#define MPU20600_ADDRESS0            0x68U
#define MPU20600_ADDRESS1            0x69U


//  Register map

#define MPU20600_XG_OFFS_TC_H        0x04     
#define MPU20600_XG_OFFS_TC_L        0x05
#define MPU20600_YG_OFFS_TC_H        0x07
#define MPU20600_YG_OFFS_TC_L        0x08
#define MPU20600_ZG_OFFS_TC_H        0x0a
#define MPU20600_ZG_OFFS_TC_L        0x0b
#define MPU20600_SELF_TEST_X_ACCEL   0x0d
#define MPU20600_SELF_TEST_Y_ACCEL   0x0e
#define MPU20600_SELF_TEST_Z_ACCEL   0x0f
#define MPU20600_XG_OFFS_USRH        0x13
#define MPU20600_XG_OFFS_USRL        0x14
#define MPU20600_YG_OFFS_USRH        0x15
#define MPU20600_YG_OFFS_USRL        0x16
#define MPU20600_ZG_OFFS_USRH        0x17
#define MPU20600_ZG_OFFS_USRL        0x18
#define MPU20600_SMPLRT_DIV          0x19U // 25
#define MPU20600_GYRO_LPF	     0x1aU // 26
#define MPU20600_GYRO_CONFIG         0x1bU // 27
#define MPU20600_ACCEL_CONFIG        0x1cU // 28
#define MPU20600_ACCEL_CONFIG2           0x1dU // 29
#define MPU20600_ACCEL_ODR	     0x1eU // 30
#define MPU20600_ACCEL_WOM_THRESHOLD 0x1fU // 31
#define MPU20600_FIFO_EN             0x23U 
#define MPU20600_I2C_MST_CTRL        0x24U
#define MPU20600_INT_PIN_CFG         0x37U
#define MPU20600_INT_ENABLE          0x38U
#define MPU20600_INT_STATUS          0x3aU
#define MPU20600_ACCEL_XOUT_H        0x3bU
#define MPU20600_GYRO_XOUT_H         0x43U
#define MPU20600_GYRO_ZOUT_L         0x48U
#define MPU20600_EXT_SENS_DATA_00    0x49U
#define MPU20600_EXT_SENS_DATA_23    0x60U
#define MPU20600_I2C_SLV0_DO         0x63U
#define MPU20600_I2C_SLV1_DO         0x64U
#define MPU20600_I2C_SLV2_DO         0x65U
#define MPU20600_I2C_SLV3_DO         0x66U
#define MPU20600_I2C_MST_DELAY_CTRL  0x67U // 103
#define MPU20600_ACCEL_ITR_CTRL      0x69U // 105
#define MPU20600_USER_CTRL           0x6aU
#define MPU20600_PWR_MGMT_1          0x6bU
#define MPU20600_PWR_MGMT_2          0x6cU
#define MPU20600_FIFO_COUNT_H        0x72U
#define MPU20600_FIFO_R_W            0x74U
#define MPU20600_WHO_AM_I            0x75U
#define ICM20600_WHO_AM_I_CONTENT    0x11U

#define  MPU20600_REGISTER_BASE      MPU20600_ACCEL_XOUT_H
#define  MPU20600_JUSTIMU_LAST	     MPU20600_GYRO_ZOUT_L
#define  MPU20600_REGISTER_LAST	    MPU20600_EXT_SENS_DATA_23
//  sample rate defines (applies to gyros and accels, not mags)

#define MPU20600_SAMPLERATE_MIN      5U                       // 5 samples per second is the lowest
#define MPU20600_SAMPLERATE_MAX      1000U                    // 1000 samples per second is the absolUte maximum

//  compass rate defines

//  Gyro LPF options

#define MPU20600_GYRO_LPF_3281       0x07U                    // 3600Hz, 0.11mS delay
#define MPU20600_GYRO_LPF_250        0x00U                    // 250Hz, 0.97mS delay
#define MPU20600_GYRO_LPF_176        0x01U                    // 184Hz, 2.9mS delay
#define MPU20600_GYRO_LPF_92         0x02U                    // 92Hz, 3.9mS delay
#define MPU20600_GYRO_LPF_41         0x03U                    // 41Hz, 5.9mS delay
#define MPU20600_GYRO_LPF_20         0x04U                    // 20Hz, 9.9mS delay
#define MPU20600_GYRO_LPF_10         0x05U                    // 10Hz, 17.85mS delay
#define MPU20600_GYRO_LPF_5          0x06U                    // 5Hz, 33.48mS delay


//  Gyro FSR options

#define MPU20600_GYROFSR_250         (0b00 << 3)              // +/- 250 degrees per second
#define MPU20600_GYROFSR_500         (0b01 << 3)              // +/- 500 degrees per second
#define MPU20600_GYROFSR_1000        (0b10 << 3)              // +/- 1000 degrees per second
#define MPU20600_GYROFSR_2000        (0b11 << 3)              // +/- 2000 degrees per second

//  Accel FSR options

#define MPU20600_ACCELFSR_2          (0b00 << 3)              // +/- 2g
#define MPU20600_ACCELFSR_4          (0b01 << 3)              // +/- 4g
#define MPU20600_ACCELFSR_8          (0b10 << 3)              // +/- 8g
#define MPU20600_ACCELFSR_16         (0b11 << 3)              // +/- 16g

//  Accel LPF options

#define MPU20600_ACCEL_LPF_1046      0x08U                    // 1130Hz, 0.75mS delay
#define MPU20600_ACCEL_LPF_218       0x01U                    // 184Hz, 5.80mS delay
#define MPU20600_ACCEL_LPF_99        0x02U                    // 92Hz, 7.80mS delay
#define MPU20600_ACCEL_LPF_44        0x03U                    // 41Hz, 11.80mS delay
#define MPU20600_ACCEL_LPF_21        0x04U                    // 20Hz, 19.80mS delay
#define MPU20600_ACCEL_LPF_10        0x05U                    // 10Hz, 35.70mS delay
#define MPU20600_ACCEL_LPF_5         0x06U                    // 5Hz, 66.96mS delay
#define MPU20600_ACCEL_LPF_420       0x07U                    // 460Hz, 1.94mS delay



// power management option
#define MPU20600_PWRM2_DISABLE_ACC  0b00111000
#define MPU20600_PWRM2_DISABLE_GYRO 0b00000111
#define MPU20600_PWRM1_HRESET	   (1<<7)
#define MPU20600_PWRM1_SLEEP	   (1<<6)
#define MPU20600_PWRM1_CYCLE	   (1<<5)
#define MPU20600_PWRM1_GYROSTANDBY  (1<<4)
#define MPU20600_PWRM1_PDPTAT	   (1<<3)

// motion detection option
#define MPU20600_INTEL_ENABLE	   (1<<7)
#define MPU20600_INTEL_DISABLE	   (0<<7)
#define MPU20600_INTEL_MODE_COMPARE (1<<6)

#define MPU20600_INT_PIN_CFG_ACTIVE_LOW		(1<<7)
#define MPU20600_INT_PIN_CFG_ACTIVE_HIGH		(0<<7)

#define MPU20600_INT_PIN_CFG_OPENDRAIN 		(1<<6)
#define MPU20600_INT_PIN_CFG_PUSHPULL 		(0<<6)

#define MPU20600_INT_PIN_CFG_LATCH_UNTIL_CLR  	(1<<5)
#define MPU20600_INT_PIN_CFG_PULSE_50_US  	(0<<5)

#define MPU20600_INT_PIN_CFG_CLR_ON_READ		(1<<4)

#define MPU20600_INT_PIN_CFG_FSYNC  		(1<<3)
#define MPU20600_INT_PIN_CFG_FSYNC_INT	 	(1<<2)
#define MPU20600_INT_PIN_CFG_BYPASS_EN  		(1<<1)

#define MPU20600_INT_ENABLE_WAKE_ON_MOTION	(1<<6)
#define MPU20600_INT_DISABLE_WAKE_ON_MOTION	(0<<6)
#define MPU20600_INT_ENABLE_FIFO_OVERFLOW	(1<<4)
#define MPU20600_INT_ENABLE_FSYNC		(1<<3)
#define MPU20600_INT_ENABLE_RAW_READY		(1<<0)

#define MPU20600_INT_WOM_X			(1<<7)
#define MPU20600_INT_WOM_Y			(1<<6)
#define MPU20600_INT_WOM_Z			(1<<5)
#define MPU20600_INT_FIFO_OFLOW			(1<<4)
#define MPU20600_INT_GDRIVE			(1<<2)
#define MPU20600_INT_DATA_RDY			(1<<0)




// bitmask, obiousvly cannot have enabled and disabled bit at same time


typedef enum {MPU20600_LOW_POWER_ACC_ODR_0_DOT_24_HZ=0,	
	      MPU20600_LOW_POWER_ACC_ODR_0_DOT_49_HZ,	
	      MPU20600_LOW_POWER_ACC_ODR_1_HZ,	
	      MPU20600_LOW_POWER_ACC_ODR_2_HZ,	
	      MPU20600_LOW_POWER_ACC_ODR_4_HZ,	
	      MPU20600_LOW_POWER_ACC_ODR_8_HZ,	
	      MPU20600_LOW_POWER_ACC_ODR_15_HZ,	
	      MPU20600_LOW_POWER_ACC_ODR_31_HZ,	
	      MPU20600_LOW_POWER_ACC_ODR_62_HZ,	
	      MPU20600_LOW_POWER_ACC_ODR_125_HZ,
	      MPU20600_LOW_POWER_ACC_ODR_250_HZ,
	      MPU20600_LOW_POWER_ACC_ODR_500_HZ,
	      MPU20600_NORMAL_POWER_ODR}  Mpu20600_LowPowerAccelerometerFrequencyCycle;



  

/*
#                 __  __ _____  _    _      ___   ___    __   ___   ___  
#                |  \/  |  __ \| |  | |    |__ \ / _ \  / /  / _ \ / _ \ 
#                | \  / | |__) | |  | |______ ) | | | |/ /_ | | | | | | |
#                | |\/| |  ___/| |  | |______/ /| | | | '_ \| | | | | | |
#                | |  | | |    | |__| |     / /_| |_| | (_) | |_| | |_| |
#                |_|  |_|_|     \____/     |____|\___/ \___/ \___/ \___/ 
#                                                                        
#                                                                        
*/

struct _Mpu20600Data;
typedef struct _Mpu20600Data  Mpu20600Data;


typedef union 
{
  struct {
    int16_t acc[3];
    int16_t temperature;
    int16_t gyr[3];
  };
  int16_t raw[7]; // usefull to swap bytes (endianness) of words in a loop
} Mpu20600FifoData;
static_assert(sizeof(Mpu20600FifoData) == 14);

struct _Mpu20600Data
{
  I2CDriver		*i2cd;

  uint32_t	sampleRate;     // imu sample rate in Hz
  uint32_t	sampleInterval; // interval between samples in tick
  uint32_t	cacheTimestamp;	// time stamp for raw data cache
  float		gyroScale;
  float		accelScale;

  bool		accOnly;
  uint8_t			nextSlvFreeSlot;	// next slot for i2c slv managing
  uint8_t			slaveAddr;	// I2C address of MPU20600
  
  uint8_t	gyroLpf;        // gyro low pass filter setting
  uint8_t	accelLpf;       // accel low pass filter setting

  uint8_t	gyroFsr;	// gyro scale (250 to 2000 °/second)
  uint8_t	accelFsr;	// accel scale (+/- 2g to +/- 16g)
  uint8_t	registerSegmentLen;	// len of register to acquire
  uint8_t	fifoIndex;      // next fifo element to pop
  uint8_t	fifoLen;        // number of elements in the fifo
  Mpu20600FifoData fifo[72];	// cache for fifo or once by once data read
};

typedef struct  
{
  I2CDriver		*i2cd;
  uint32_t		 sampleRate;	// imu sample rate in Hz
  bool			 useAd0;	// I2C address offset pin enabled
  bool			 fifoEnabled;   // Enable Fifo mode
  uint8_t		 gyroLpf;	// gyro low pass filter setting
  uint8_t		 accelLpf;	// accel low pass filter setting
  uint8_t		 gyroFsr;	// gyro scale (250 to 2000 °/second)
  uint8_t		 accelFsr;	// accel scale (+/- 2g to +/- 16g)
} Mpu20600Config;





msg_t mpu20600_init(Mpu20600Data *imu, const Mpu20600Config* initParam);
msg_t mpu20600_setGyroLpf(Mpu20600Data *imu, const uint8_t lpf);
msg_t mpu20600_setAccelLpf(Mpu20600Data *imu, const uint8_t lpf);
msg_t mpu20600_setSampleRate(Mpu20600Data *imu, const uint32_t rate);
msg_t mpu20600_setGyroFsr(Mpu20600Data *imu, const uint8_t fsr);
msg_t mpu20600_setAccelFsr(Mpu20600Data *imu, const uint8_t fsr);

msg_t mpu20600_cacheVal(Mpu20600Data *imu);

// temp in celcius degree, gyro in rad/s, accel in m/s², 
msg_t mpu20600_getVal(Mpu20600Data *imu, float *temp, 
		      ImuVec3f *gyro, ImuVec3f *acc);
msg_t mpu20600_getItrStatus(Mpu20600Data *imu, uint8_t *itrStatus);
msg_t mpu20600_getDevid(Mpu20600Data *imu, uint8_t *devid);
bool  mpu20600_popFifo(Mpu20600Data *imu, ImuVec3f *acc, ImuVec3f *gyro,
		       float *dt, bool *fifoFull);





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

  mpu20600_setModeAccOnly (...);
  mpu20600_activateMotionDetect (...);
  mpu20600_setModeAutoWake (...);

  if order to restore normal (without compass) operation :
  mpu20600_init (...);
 */



// disable gyro and mag for usual (I2C) measure. Disabling compass is not reversible
// until the mpu20600 is power cycled
msg_t mpu20600_setModeAccOnly (Mpu20600Data *imu);

// Disable gyro and mag AND set imu in  low power cycle mode wich internally get acceleration
// at lpodr frequency (cf enum Mpu20600_LowPowerAccelerometerFrequencyCycle)
// useful if associated with MotionDetect
msg_t mpu20600_setModeAutoWake (Mpu20600Data *imu, 
			       Mpu20600_LowPowerAccelerometerFrequencyCycle lpodr);

// mode low power with motion detection, when motion is detected, interruption is made
// on itr pin of mpu20600 in order to awake MCU
// when the MCU is awaked, it has to reinit IMU to get actual measure.
// Motion detect can be used in normal or autowake mode
// config mask of the interrupt pin has to be given
msg_t mpu20600_activateMotionDetect (Mpu20600Data *imu, const uint32_t threadsholdInMilliG,
				    const uint8_t pinConfigMask);


// deep sleep (completely and definitly shutoff mpu, to be used again,
// to be used again, MPU has to be power cycled
msg_t mpu20600_setModeDeepSleep (Mpu20600Data *imu);



#endif
