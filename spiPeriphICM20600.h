/*
#         _                  _
#        | |                | |
#        | |_     ___     __| |    ___
#        | __|   / _ \   / _` |   / _ \
#        \ |_   | (_) | | (_| |  | (_) |
#         \__|   \___/   \__,_|   \___/


 */

#pragma once

#include "ch.h"
#include "hal.h"
#include "stdutil.h"

/***************************************************************
 ICM20600  Register Table
 ***************************************************************/
typedef enum {ICM20600_XG_OFFS_TC_H           = 0x04,     
	      ICM20600_XG_OFFS_TC_L           = 0x05,
	      ICM20600_YG_OFFS_TC_H           = 0x07,
	      ICM20600_YG_OFFS_TC_L           = 0x08,
	      ICM20600_ZG_OFFS_TC_H           = 0x0a,
	      ICM20600_ZG_OFFS_TC_L           = 0x0b,
	      ICM20600_SELF_TEST_X_ACCEL      = 0x0d,
	      ICM20600_SELF_TEST_Y_ACCEL      = 0x0e,
	      ICM20600_SELF_TEST_Z_ACCEL      = 0x0f,
	      ICM20600_XG_OFFS_USRH           = 0x13,
	      ICM20600_XG_OFFS_USRL           = 0x14,
	      ICM20600_YG_OFFS_USRH           = 0x15,
	      ICM20600_YG_OFFS_USRL           = 0x16,
	      ICM20600_ZG_OFFS_USRH           = 0x17,
	      ICM20600_ZG_OFFS_USRL           = 0x18,
	      ICM20600_SMPLRT_DIV             = 0x19,
	      ICM20600_CONFIG                 = 0x1a,
	      ICM20600_GYRO_CONFIG            = 0x1b,
	      ICM20600_ACCEL_CONFIG           = 0x1c,
	      ICM20600_ACCEL_CONFIG2          = 0x1d,
	      ICM20600_GYRO_LP_MODE_CFG       = 0x1e,
	      ICM20600_ACCEL_WOM_X_THR        = 0x20,
	      ICM20600_ACCEL_WOM_Y_THR        = 0x21,
	      ICM20600_ACCEL_WOM_Z_THR        = 0x22,
	      ICM20600_FIFO_EN                = 0x23,
	      ICM20600_FSYNC_INT              = 0x36,
	      ICM20600_INT_PIN_CFG            = 0x37,
	      ICM20600_INT_ENABLE             = 0x38,
	      ICM20600_FIFO_WM_INT_STATUS     = 0x39,
	      ICM20600_INT_STATUS             = 0x3a,
	      ICM20600_ACCEL_XOUT_H           = 0x3b,
	      ICM20600_ACCEL_XOUT_L           = 0x3c,
	      ICM20600_ACCEL_YOUT_H           = 0x3d,
	      ICM20600_ACCEL_YOUT_L           = 0x3e,
	      ICM20600_ACCEL_ZOUT_H           = 0x3f,
	      ICM20600_ACCEL_ZOUT_L           = 0x40,
	      ICM20600_TEMP_OUT_H             = 0x41,
	      ICM20600_TEMP_OUT_L             = 0x42,
	      ICM20600_GYRO_XOUT_H            = 0x43,
	      ICM20600_GYRO_XOUT_L            = 0x44,
	      ICM20600_GYRO_YOUT_H            = 0x45,
	      ICM20600_GYRO_YOUT_L            = 0x46,
	      ICM20600_GYRO_ZOUT_H            = 0x47,
	      ICM20600_GYRO_ZOUT_L            = 0x48,
	      ICM20600_SELF_TEST_X_GYRO       = 0x50,
	      ICM20600_SELF_TEST_Y_GYRO       = 0x51,
	      ICM20600_SELF_TEST_Z_GYRO       = 0x52,
	      ICM20600_FIFO_WM_TH1            = 0x60,
	      ICM20600_FIFO_WM_TH2            = 0x61,
	      ICM20600_SIGNAL_PATH_RESET      = 0x68,
	      ICM20600_ACCEL_INTEL_CTRL       = 0x69,
	      ICM20600_USER_CTRL              = 0x6A,
	      ICM20600_PWR_MGMT_1             = 0x6b,
	      ICM20600_PWR_MGMT_2             = 0x6c,
	      ICM20600_I2C_IF                 = 0x70,
	      ICM20600_FIFO_COUNTH            = 0x72,
	      ICM20600_FIFO_COUNTL            = 0x73,
	      ICM20600_FIFO_R_W               = 0x74,
	      ICM20600_WHO_AM_I               = 0x75,
	      ICM20600_WHO_AM_I_CONTENT       = 0x11,
	      ICM20600_XA_OFFSET_H            = 0x77,
	      ICM20600_XA_OFFSET_L            = 0x78,
	      ICM20600_YA_OFFSET_H            = 0x7a,
	      ICM20600_YA_OFFSET_L            = 0x7b,
	      ICM20600_ZA_OFFSET_H            = 0x7d,
	      ICM20600_ZA_OFFSET_L            = 0x7e
} Icm20600_RegisterMap;




/***************************************************************
 ICM20600 bitmask
 ***************************************************************/
typedef enum {	      
  ICM20600_GYRO_RATE_8K_BW_250	= (0 << 0),
  ICM20600_GYRO_RATE_1K_BW_176	= (1 << 0),
  ICM20600_GYRO_RATE_1K_BW_92	= (2 << 0),
  ICM20600_GYRO_RATE_1K_BW_41	= (3 << 0),
  ICM20600_GYRO_RATE_1K_BW_20	= (4 << 0),
  ICM20600_GYRO_RATE_1K_BW_10	= (5 << 0),
  ICM20600_GYRO_RATE_1K_BW_5	= (6 << 0),
  ICM20600_GYRO_RATE_8K_BW_3281 = (7 << 0),
  ICM20600_FIFO_OVERWRITE       = (1 << 6),
  ICM20600_FIFO_BLOCK           = (0 << 6),
} Icm20600_config;


/***************************************************************
Gyroscope scale range
 ***************************************************************/
typedef enum 
{
  ICM20600_FCHOICE_RATE_32K_BW_8173 = (1 << 0),
  ICM20600_FCHOICE_RATE_32K_BW_3281 = (2 << 0),
  ICM20600_RANGE_250_DPS	    = (0 << 3),
  ICM20600_RANGE_500_DPS	    = (1 << 3),
  ICM20600_RANGE_1K_DPS		    = (2 << 3),
  ICM20600_RANGE_2K_DPS		    = (3 << 3),
  ICM20600_RANGE_GYRO_MASK	    = (3 << 3),
  ICM20600_Z_GYRO_SELFTEST	    = (1 << 5),
  ICM20600_Y_GYRO_SELFTEST	    = (1 << 6),
  ICM20600_X_GYRO_SELFTEST	    = (1 << 7),
} Icm20600_gyroConf;

/***************************************************************
Accelerometer scale range
 ***************************************************************/
typedef enum 
{
  ICM20600_RANGE_2G	    = (0 << 3),
  ICM20600_RANGE_4G	    = (1 << 3),
  ICM20600_RANGE_8G	    = (2 << 3),
  ICM20600_RANGE_16G	    = (3 << 3),
  ICM20600_RANGE_ACCEL_MASK = (3 << 3),
  ICM20600_Z_ACCEL_SELFTEST = (1 << 5),
  ICM20600_Y_ACCEL_SELFTEST = (1 << 6),
  ICM20600_X_ACCEL_SELFTEST = (1 << 7),
} Icm20600_accelConf;

typedef enum 
{
  ICM20600_ACC_RATE_4K_BW_1046	 = (8 << 0),
  ICM20600_ACC_RATE_1K_BW_218	 = (1 << 0),
  ICM20600_ACC_RATE_1K_BW_99	 = (2 << 0),
  ICM20600_ACC_RATE_1K_BW_44	 = (3 << 0),
  ICM20600_ACC_RATE_1K_BW_21	 = (4 << 0),
  ICM20600_ACC_RATE_1K_BW_10	 = (5 << 0),
  ICM20600_ACC_RATE_1K_BW_5	 = (6 << 0),
  ICM20600_ACC_RATE_1K_BW_420	 = (7 << 0),
  ICM20600_ACC_LOWPOW_AVERAGE_4	 = (0 << 4),
  ICM20600_ACC_LOWPOW_AVERAGE_8	 = (1 << 4),
  ICM20600_ACC_LOWPOW_AVERAGE_16 = (2 << 4),
  ICM20600_ACC_LOWPOW_AVERAGE_32 = (3 << 4)
} Icm20600_accelConf2;



/***************************************************************
Averaging filter configuration for low-power gyroscope mode
 ***************************************************************/
typedef enum 
{
  ICM20600_GYRO_AVERAGE_1	  = (0 << 4),
  ICM20600_GYRO_AVERAGE_2	  = (1 << 4),
  ICM20600_GYRO_AVERAGE_4	  = (2 << 4),
  ICM20600_GYRO_AVERAGE_8	  = (3 << 4),
  ICM20600_GYRO_AVERAGE_16	  = (4 << 4),
  ICM20600_GYRO_AVERAGE_32	  = (5 << 4),
  ICM20600_GYRO_AVERAGE_64	  = (6 << 4),
  ICM20600_GYRO_AVERAGE_128	  = (7 << 4),
  ICM20600_GYRO_LOWPOWER_ENABLED  = (1 << 7),
  ICM20600_GYRO_LOWPOWER_DISABLED = (1 << 7)
} Icm20600_gyroLowPower;

/***************************************************************
Fifo configuration
 ***************************************************************/
typedef enum 
{
  ICM20600_ACCEL_FIFO_ENABLE  = (1 << 3),
  ICM20600_ACCEL_FIFO_DISABLE = (0 << 3),
  ICM20600_GYRO_FIFO_ENABLE   = (1 << 4),
  ICM20600_GYRO_FIFO_DISABLE  = (0 << 4),
} Icm20600_fifoConfig;

/***************************************************************
ICM20600 power mode
 ***************************************************************/
typedef enum 
{
  ICM20600_CLKSEL_20MHZ = (0 << 0),
  ICM20600_CLKSEL_AUTO	= (1 << 0),
  ICM20600_CLKSEL_STOP	= (7 << 0),
  ICM20600_TEMP_DISABLE = (1 << 3),
  ICM20600_GYRO_STANDBY = (1 << 4),
  ICM20600_CYCLE_ENABLE = (1 << 5),
  ICM20600_ENTER_SLEEP	= (1 << 6),
  ICM20600_RESET	= (1 << 7)
} Icm20600_powerMgmt1;

typedef enum 
{
  ICM20600_DISABLE_Z_GYRO  = (1 << 0),
  ICM20600_DISABLE_Y_GYRO  = (1 << 1),
  ICM20600_DISABLE_X_GYRO  = (1 << 2),
  ICM20600_DISABLE_Z_ACCEL = (1 << 3),
  ICM20600_DISABLE_Y_ACCEL = (1 << 4),
  ICM20600_DISABLE_X_ACCEL = (1 << 5),
  ICM20600_DISABLE_GYRO  = (7 << 0),
  ICM20600_DISABLE_ACCEL  = (7 << 3)
} Icm20600_powerMgmt2;

/***************************************************************
ICM20600 bus interface
 ***************************************************************/
typedef enum 
{
  ICM20600_SPI_MODE = (1 << 6),
  ICM20600_I2C_MODE = (0 << 6)
} Icm20600_Interface;

/***************************************************************
ICM20600 interrupt and ready pin control
 ***************************************************************/
typedef enum 
{
  ICM20600_INT2_ENABLE		 = (1 << 0),
  ICM20600_INT2_DISABLE		 = (1 << 0),
  ICM20600_FSYNC_ITR_ENABLE	 = (1 << 2),
  ICM20600_FSYNC_ITR_DISABLE	 = (0 << 2),
  ICM20600_FSYNC_ACTIVE_LOW	 = (1 << 3),
  ICM20600_FSYNC_ACTIVE_HIGH	 = (0 << 3),
  ICM20600_CLEAR_ITR_READ_ANY	 = (1 << 4),
  ICM20600_CLEAR_ITR_READ_STATUS = (0 << 4),
  ICM20600_LATCH_UNTIL_CLEAR	 = (1 << 5),
  ICM20600_PULSE_50_US		 = (0 << 5),
  ICM20600_OPENDRAIN		 = (1 << 6),
  ICM20600_PUSHPULL		 = (0 << 6),
  ICM20600_ACTIVE_LOW		 = (1 << 7),
  ICM20600_ACTIVE_HIGH		 = (0 << 7),
} Icm20600_pinControl;

/***************************************************************
ICM20600 accelerometer intelligence control
 ***************************************************************/
typedef enum 
{
  ICM20600_ONE_AXIS_REACHES_THRESHOLD = (0 << 0),
  ICM20600_ALL_AXIS_REACH_THRESHOLD   = (1 << 0),
  ICM20600_OUTPUT_LIMIT_ENABLED	      = (1 << 1),
  ICM20600_OUTPUT_LIMIT_DISABLED      = (0 << 1),
  ICM20600_WAKE_ON_MOTION_ENABLE      = (3 << 6),
  ICM20600_WAKE_ON_MOTION_DISABLE     = (0 << 6)
} Icm20600_wakeOnMotion;

/***************************************************************
ICM20600 user control
 ***************************************************************/
typedef enum 
{
  ICM20600_COLD_RESET	= (1 << 0),
  ICM20600_FIFO_RESET	= (1 << 2),
  ICM20600_FIFO_ENABLE	= (1 << 6),
  ICM20600_FIFO_DISABLE = (0 << 6)
} Icm20600_userControl;


/***************************************************************
ICM20600 interrupt status
 ***************************************************************/
typedef enum 
{
  ICM20600_DATA_RDY_INT	  = (1 << 0),
  ICM20600_GDRIVE_INT	  = (1 << 2),
  ICM20600_FIFO_OFLOW_INT = (1 << 4),
  ICM20600_WOM_Z_INT	  = (1 << 5),
  ICM20600_WOM_Y_INT	  = (1 << 6),
  ICM20600_WOM_X_INT	  = (1 << 7)
} Icm20600_interruptStatus;






#define ICM20600_SAMPLERATE_MIN      5U         // 5 samples per second is the lowest
#define ICM20600_SAMPLERATE_MAX      8000U      // 8000 samples per second is the absolUte maximum


/*
#                 _____    _____   __  __             ___     ___      __     ___     ___   
#                |_   _|  / ____| |  \/  |           |__ \   / _ \    / /    / _ \   / _ \  
#                  | |   | |      | \  / |              ) | | | | |  / /_   | | | | | | | | 
#                  | |   | |      | |\/| |             / /  | | | | | '_ \  | | | | | | | | 
#                 _| |_  | |____  | |  | |  ______    / /_  | |_| | | (_) | | |_| | | |_| | 
#                |_____|  \_____| |_|  |_| |______|  |____|  \___/   \___/   \___/   \___/  
*/

struct _Icm20600Data;
typedef struct _Icm20600Data  Icm20600Data;


typedef struct  
{
  SPIDriver		*spid;
  uint32_t		 sampleRate;	// imu sample rate in Hz

  Icm20600_config	config;        // gyro low pass filter setting
  Icm20600_gyroConf	gyroConfig;	// gyro scale (250 to 2000 °/second)

  Icm20600_accelConf	accelConf;	// accel scale (+/- 2g to +/- 16g)
  Icm20600_accelConf2	accelConf2;       // accel low pass filter setting
} Icm20600Config;


struct _Icm20600Data
{
  Icm20600Config *config;
  
  uint32_t	sampleInterval; // interval between samples in tick
  uint32_t	cacheTimestamp;	// time stamp for raw data cache
  float		gyroScale;
  float		accelScale;
  uint8_t	rawCache[ICM20600_GYRO_ZOUT_L - ICM20600_FIFO_WM_INT_STATUS]; // all usefull output
  bool		accOnly;
  uint8_t	registerSegmentLen;	// len of register to acquire 
};



msg_t icm20600_init (Icm20600Data *imu, Icm20600Config* initParam);
void icm20600_setGyroLpf (Icm20600Data *imu, const uint8_t lpf);
void icm20600_setAccelLpf (Icm20600Data *imu, const uint8_t lpf);
void icm20600_setSampleRate (Icm20600Data *imu, const uint32_t rate);
void icm20600_setGyroFsr (Icm20600Data *imu, const uint8_t fsr);
void icm20600_setAccelFsr (Icm20600Data *imu, const uint8_t fsr);
void icm20600_fetch(Icm20600Data *imu);

// temp in celcius degree, gyro in rad/s, accel in m/s², 
void icm20600_getVal  (Icm20600Data *imu, float *temp, 
		      Vec3f *gyro, Vec3f *acc);
Icm20600_interruptStatus icm20600_getItrStatus  (Icm20600Data *imu);
uint8_t icm20600_getDevid (Icm20600Data *imu);


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

  icm20600_setModeAccOnly (...);
  icm20600_activateMotionDetect (...);
  icm20600_setModeAutoWake (...);

  if order to restore normal (without compass) operation :
  icm20600_init (...);
 */



// disable gyro and mag for usual measure.
void icm20600_setModeAccOnly (Icm20600Data *imu);

// Disable gyro and mag AND set imu in  low power cycle mode wich internally measure acceleration
// useful if associated with MotionDetect
void icm20600_setModeAutoWake (Icm20600Data *imu, uint16_t frequency);

// mode low power with motion detection, when motion is detected, interruption is made
// on itr pin of icm20600 in order to awake MCU
// when the MCU is awaked, it has to reinit IMU to get actual measure.
// Motion detect can be used in normal or autowake mode
// config mask of the interrupt pin has to be given
void icm20600_activateMotionDetect (Icm20600Data *imu, const uint32_t threadsholdInMilliG,
				    const Icm20600_pinControl pinConfigMask);


// deep sleep (completely and definitly shutoff mpu, to be used again,
// to be used again, MPU has to be power cycled
void icm20600_setModeDeepSleep (Icm20600Data *imu);




