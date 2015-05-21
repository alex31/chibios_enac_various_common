#if I2C_USE_MPL3115A2

#include "i2cPeriphMPL3115A2.h"

/* /\* typedef struct   */
/* { */
/*   I2CDriver *i2cd; */
/*   Mpu9250MasterConfig_0_to_3 *mstConfig; */
/*   uint32_t cacheTimestamp;  */
/*   uint32_t sampleInterval;                       // interval between samples in tick */
/*   uint8_t rawCache[MPL3115A2_OUT_P_LSB-MPL3115A2_OUT_P_MSB+1]; */
/*   uint8_t cntl1;                                  // down or single or continuous mode */
/* } MPL3115A2Data; */


msg_t MPL3115A2_init (MPL3115A2Data *baro, I2CDriver *i2cd, uint8_t oversampling)
{
  msg_t status = RDY_OK;

  baro->cacheTimestamp = halGetCounterValue();
  baro->i2cd = i2cd;
  baro->mstConfig = NULL;
  baro->oversampling = oversampling;
 
  switch (oversampling) {
  case MPL3115A2_SAMPLE_PERIOD_006MS : baro->sampleInterval = 6; break;
  case MPL3115A2_SAMPLE_PERIOD_010MS : baro->sampleInterval = 10; break;
  case MPL3115A2_SAMPLE_PERIOD_018MS : baro->sampleInterval = 18; break;
  case MPL3115A2_SAMPLE_PERIOD_034MS : baro->sampleInterval = 34; break;
  case MPL3115A2_SAMPLE_PERIOD_066MS : baro->sampleInterval = 66; break;
  case MPL3115A2_SAMPLE_PERIOD_130MS : baro->sampleInterval = 130; break;
  case MPL3115A2_SAMPLE_PERIOD_258MS : baro->sampleInterval = 258; break;
  case MPL3115A2_SAMPLE_PERIOD_512MS : baro->sampleInterval = 512; break;
  }
  
  i2cAcquireBus(baro->i2cd);
  I2C_WRITE_REGISTERS(baro->i2cd, MPL3115A2_ADDRESS, MPL3115A2_PT_DATA_CFG, 
		      MPL3115A2_DATA_READY_EVENT_MODE | MPL3115A2_DATA_READY_PRESSURE);
  I2C_WRITE_REGISTERS(baro->i2cd, MPL3115A2_ADDRESS, MPL3115A2_CTRL_REG1,
		      MPL3115A2_ONESHOT_MODE | oversampling);
  i2cReleaseBus(baro->i2cd);
  
  return status;
}


msg_t MPL3115A2_getDevid (MPL3115A2Data *baro, uint8_t *devid)
{
  msg_t status;

  i2cAcquireBus(baro->i2cd);
  I2C_READ_REGISTER(baro->i2cd, MPL3115A2_ADDRESS, MPL3115A2_WHO_AM_I, devid);
  i2cReleaseBus(baro->i2cd);

 if (*devid !=  MPL3115A2_ID)
   status = I2C_BADID;
 return status;
}


msg_t MPL3115A2_setOversampling (MPL3115A2Data *baro, uint8_t oversampling)
{
  baro->oversampling = oversampling;
  return RDY_OK;
}


msg_t MPL3115A2_cacheVal  (MPL3115A2Data *baro)
{
  msg_t status = RDY_OK;
  uint8_t baroStatus;
  bool_t  notReady;

 if (baro->mstConfig == NULL) {
   i2cAcquireBus(baro->i2cd);
   
   I2C_WRITE_REGISTERS(baro->i2cd, MPL3115A2_ADDRESS, MPL3115A2_CTRL_REG1,
		       MPL3115A2_ONESHOT_INIT | baro->oversampling);
   do {
     I2C_READ_REGISTER(baro->i2cd, MPL3115A2_ADDRESS, MPL3115A2_STATUS, &baroStatus);
     notReady = !(baroStatus & MPL3115A2_PRESSURE_AVAIL);
     if (notReady) {
       i2cReleaseBus(baro->i2cd);
       chThdSleepMilliseconds(2);
       i2cAcquireBus(baro->i2cd);
     }
   } while (notReady);
   
   I2C_READ_REGISTERS(baro->i2cd, MPL3115A2_ADDRESS, MPL3115A2_OUT_P_MSB, baro->rawCache);
   i2cReleaseBus(baro->i2cd);
 } else {
   status = mpu9250_cacheVal (baro->mstConfig->mpu);
 }  
 
 return status;
}


msg_t MPL3115A2_getVal  (MPL3115A2Data *baro, float *pressure)
{
  msg_t status = RDY_OK;
  
  // alias 
  const uint32_t  *rawB =  (uint32_t  *) ((baro->mstConfig == NULL) ? baro->rawCache :
					baro->mstConfig->cacheAdr);
  
  if (!halIsCounterWithin(baro->cacheTimestamp, 
			  baro->cacheTimestamp + baro->sampleInterval)) {
    baro->cacheTimestamp = halGetCounterValue();
    status =  MPL3115A2_cacheVal (baro);
  }
  
  if (status != RDY_OK) {
    *pressure = 0.0f;
    return status;
  }
  
  const uint32_t swapVal = (SWAP_ENDIAN32(*rawB<<8)) ;
  *pressure = (float) swapVal / 6400.0f;
  
  return status;
}


/* msg_t mpu9250AddSlv_MPL3115A2 (Mpu9250Data *imu, MPL3115A2Data *baro) */
/* { */
/*   msg_t status = RDY_OK; */

/*   return status; */
/* } */

#endif
