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
#include "invensense3_regs.h"

typedef struct {
  SPIDriver		*spid;
  Inv3Odr		commonOdr;
  Inv3GyroScale		gyroScale;
  Inv3AccelScale	accelScale;
  bool			externClockRef;
#if INVENSENSE3_USE_FIFO
  uint8_t		watermarkPercent;
  Inv3Packet3FifoBuffer *fifoBuffer;
#endif  
} Inv3Config;


typedef struct {
  const Inv3Config *config;
  Inv3Bank   currentBank;
  float accScale;
  float gyroScale;
#if INVENSENSE3_USE_FIFO
  uint8_t watermarkNbPacket;
#endif
} Inv3Driver;


typedef struct {
  uint8_t factory; // score : 0 is defective unit, 100 is perfect unit
  bool	  bias;    // true is ok
  bool	  passed;  // true if all test are ok
} Inv3TestResult;

#ifdef __cplusplus
extern "C" {
#endif

  bool inv3RunSelfTest(SPIDriver *spid,
		       Vec3f *accelDiff, Vec3f *gyroDiff,
		       Vec3f *accelRatio, Vec3f *gyroRatio);
  
  bool inv3Init (Inv3Driver *inv3d, const Inv3Config* cfg);
  void inv3SetOdr (Inv3Driver *inv3d, const Inv3Odr odr);
  void inv3SetAccelScale (Inv3Driver *inv3d, const Inv3AccelScale scale);
  void inv3SetGyroScale (Inv3Driver *inv3d, const Inv3GyroScale scale);
  void inv3SetSampleRate (Inv3Driver *inv3d, const uint32_t rate);
  
  // temp in celcius degree, gyro in rad/s, accel in m/s², 
  void inv3GetVal (Inv3Driver *inv3d, float *temp, 
		   Vec3f *gyro, Vec3f *acc);
#if INVENSENSE3_USE_FIFO
  void inv3GetAverageVal (Inv3Driver *inv3d, float *temp, 
			  Vec3f *gyro, Vec3f *acc);
  uint16_t inv3PopFifo (Inv3Driver *inv3d);
#endif
  //Inv3_interruptStatus inv3_getItrStatus  (Inv3Driver *inv3d);
  Inv3SensorType inv3GetSensorType (Inv3Driver *inv3d);
  uint8_t inv3GetStatus(Inv3Driver *inv3d);
  uint8_t inv3GetClockDiv(Inv3Driver *inv3d);
  
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

  inv3_setModeAccOnly (...);
  inv3_activateMotionDetect (...);
  inv3_setModeAutoWake (...);

  if order to restore normal (without compass) operation :
  inv3_init (...);
 */



// run self test and return results on each axis :
// a zero value means selftest passed, otherwise bit in bitfield indicate which
// axis is doomed
Inv3TestResult inv3_runSelfTests (Inv3Driver *inv3d);

#ifdef __cplusplus
}
#endif


#ifdef __cplusplus
//constexpr static inline  Inv3_gyroConf operator|(Inv3_gyroConf a, Inv3_gyroConf b)
//{return static_cast<Inv3_gyroConf>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));}
#endif
