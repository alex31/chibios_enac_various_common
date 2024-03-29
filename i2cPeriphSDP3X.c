#include "i2cPeriphSDP3X.h"




typedef struct __attribute__((packed)) {
  uint8_t data[2];
  uint8_t crc;
}  Sdp3xDataAtom ;

typedef struct __attribute__((packed)) {
  Sdp3xDataAtom press;
  Sdp3xDataAtom temp;
  Sdp3xDataAtom scale;
} Sdp3xMeasure ;


#define SN_SIZE 2
#define PN_SIZE 4
#define SNPN_SIZE (SN_SIZE+PN_SIZE)
typedef struct __attribute__((packed)) {
  Sdp3xDataAtom snpn[6]; // SN then PN
} Sdp3xRawIdent ;


static inline uint8_t crc8_poly31_calc (const uint8_t data[], const size_t len);
static bool atomCheck(const Sdp3xDataAtom *atom);

/**
 * @brief   send command to sensor
 * @details mostly internal function, in API to permit advanced use, like entering sleeping mode
 *
 * @param[in] sdpp      pointer to the @p initialized Sdp3xDriver object
 * @param[in] cmd       command, see reference manuel
 *
 *
 * @return              The operation status.
 * @retval MSG_OK       if the function succeeded.
 * @retval MSG_RESET    if one or more I2C errors occurred, the errors can
 *                      be retrieved using @p i2cGetErrors().
 * @retval MSG_TIMEOUT  if a timeout occurred before operation end.
 *
 * @notapi
 */
msg_t sdp3xSend(const Sdp3xDriver *sdpp, const Sdp3xCommand cmd);

/* After the start measurement command is sent:-the first measurement result 
   is available after 8ms;.-small accuracy deviations (few %of reading) 
   can occur during the next 12ms. */
#define FIRST_READ_TIMOUT_MS 10U
#define I2C_TIMOUT_100MS TIME_MS2I(100U)

/*
#                         _ __    _          
#                        | '_ \  (_)         
#                  __ _  | |_) |  _          
#                 / _` | | .__/  | |         
#                | (_| | | |     | |         
#                 \__,_| |_|     |_|         
*/


void sdp3xStart(Sdp3xDriver *sdpp, I2CDriver *i2cp,
		const Sdp3xAddress addr)
{
  sdpp->i2cp = i2cp;
  sdpp->slaveAddr = addr;
}

msg_t  sdp3xRequest(Sdp3xDriver *sdpp, const Sdp3xRequest request)
{
  Sdp3xMeasure CACHE_ALIGNED(meas);
  Sdp3xCommand CACHE_ALIGNED(cmd);
  msg_t status;
  
  switch (request) {
  case SDP3X_massflow :
  case SDP3X_massflow_temp :
    cmd = SDP3X_CONTINUOUS_MASSFLOW_AVERAGE;
    break;
    
  case SDP3X_pressure :
  case SDP3X_pressure_temp :
    cmd = SDP3X_CONTINUOUS_DIFFPRESS_AVERAGE;
    break;

  case SDP3X_massflow_temp_oneshot:
    cmd = SDP3X_ONESHOT_MASSFLOW;
    break;

  case SDP3X_pressure_temp_scale_oneshot:
    cmd = SDP3X_ONESHOT_DIFFPRESS;
    break;
    
    
  default:
    cmd = SDP3X_NOT_INITIALIZED;
  }

  //  DebugTrace ("restart cmd = 0x%x", cmd);
  if ((status  = sdp3xSend(sdpp, cmd)) != MSG_OK) 
    return status;
  
  chThdSleepMilliseconds(FIRST_READ_TIMOUT_MS);

#if I2C_USE_MUTUAL_EXCLUSION
  i2cAcquireBus(sdpp->i2cp);
#endif
  status = i2cMasterCacheReceiveTimeout(sdpp->i2cp, sdpp->slaveAddr,
				   (uint8_t *) &meas, sizeof(meas),
				   I2C_TIMOUT_100MS);
  
  if (status != MSG_OK) {
    restartI2c(sdpp->i2cp);
#if I2C_USE_MUTUAL_EXCLUSION
    i2cReleaseBus(sdpp->i2cp);
#endif
   return status;
  }
#if I2C_USE_MUTUAL_EXCLUSION
  i2cReleaseBus(sdpp->i2cp);
#endif
  
  
  if (!(atomCheck(&meas.press) && atomCheck(&meas.temp) && atomCheck(&meas.scale))) {
    return MSG_RESET;
  }
  
  const int16_t pressOrFlow = meas.press.data[1] | meas.press.data[0] << 8;
  const int16_t temp = meas.temp.data[1] | meas.temp.data[0] << 8;
  const int16_t scale =  meas.scale.data[1] | meas.scale.data[0] << 8;
  
  sdpp->scale = scale;
  sdpp->temp = temp / SDP3X_TEMP_SCALE;
  sdpp->pressure = pressOrFlow / sdpp->scale;
  
  return MSG_OK;
}
  

msg_t  sdp3xStop(Sdp3xDriver *sdpp)
{
  return sdp3xSend(sdpp, SDP3X_STOP_CONTINUOUS);
}

msg_t  sdp3xSleep(Sdp3xDriver *sdpp)
{
  return sdp3xSend(sdpp, SDP3X_SLEEP);
}

msg_t  sdp3xWakeup(Sdp3xDriver *sdpp)
{
#if I2C_USE_MUTUAL_EXCLUSION
  i2cAcquireBus(sdpp->i2cp);
#endif
  static const uint8_t dummy[] = {0};
  // chibios does not permit to just send 1 adress bytes with nothing
  // so we send 1 byte, sensor will no ack : status will be error
  msg_t status =  i2cMasterTransmitTimeout(sdpp->i2cp, sdpp->slaveAddr,
					   dummy, sizeof(dummy),
					   NULL, 0, I2C_TIMOUT_100MS) ;
  if (status != MSG_OK) {
    restartI2c(sdpp->i2cp);
  }

#if I2C_USE_MUTUAL_EXCLUSION
  i2cReleaseBus(sdpp->i2cp);
#endif

  // we need to send somme dummy operation until it works
  // after several try without success, we give up
  int try = 5;
  do {
    status = sdp3xStop(sdpp);
    if (try)
      chThdSleepMilliseconds(10);
  } while (try-- && (status != MSG_OK));

  return status;
}
  
msg_t  sdp3xGeneralReset(I2CDriver *i2cp)
{
  static const uint8_t CACHE_ALIGNED(command[]) = {SDP3X_GENERAL_RESET_COMMAND};
  
#if I2C_USE_MUTUAL_EXCLUSION
  i2cAcquireBus(i2cp);
#endif

  msg_t status = i2cMasterCacheTransmitTimeout(i2cp, SDP3X_GENERAL_RESET_ADDRESS,
					  command, sizeof(command),
					  NULL, 0, I2C_TIMOUT_100MS) ;
  if (status != MSG_OK) {
    restartI2c(i2cp);
  }
#if I2C_USE_MUTUAL_EXCLUSION
    i2cReleaseBus(i2cp);
#endif
  return status;
}


msg_t  sdp3xFetch(Sdp3xDriver *sdpp, const Sdp3xRequest request)
{
  msg_t status;
  size_t len;
  Sdp3xMeasure CACHE_ALIGNED(meas);
  
  switch (request) {
  case SDP3X_massflow :
    len = 3;
    break;
  case SDP3X_massflow_temp :
    len = 6;
   break;
  case SDP3X_pressure :
    len = 3;
    break;
  case SDP3X_pressure_temp :
    len = 6;
    break;

  default:
    len = 0;
  }

  if (len == 0)
    return MSG_RESET;

#if I2C_USE_MUTUAL_EXCLUSION
  i2cAcquireBus(sdpp->i2cp);
#endif
  status = i2cMasterCacheReceiveTimeout(sdpp->i2cp, sdpp->slaveAddr,
				   (uint8_t *) &meas, len,
				   I2C_TIMOUT_100MS);
  
  if (status != MSG_OK) {
    restartI2c(sdpp->i2cp);
#if I2C_USE_MUTUAL_EXCLUSION
    i2cReleaseBus(sdpp->i2cp);
#endif
    return status;
  }
#if I2C_USE_MUTUAL_EXCLUSION
  i2cReleaseBus(sdpp->i2cp);
#endif
  
  bool crcOk = true;
  
  switch (len) {
  case 9:
    crcOk |= atomCheck(&meas.scale);
    const int16_t scale =  meas.scale.data[1] | meas.scale.data[0] << 8;
    sdpp->scale = scale;
    /* FALLTHRU */
  case 6:
    crcOk |= atomCheck(&meas.temp);
    const int16_t temp = meas.temp.data[1] | meas.temp.data[0] << 8;
    sdpp->temp = temp / SDP3X_TEMP_SCALE;
    /* FALLTHRU */
  case 3:
    crcOk |= atomCheck(&meas.press);
    const int16_t pressOrFlow = meas.press.data[1] | meas.press.data[0] << 8;
    sdpp->pressure = pressOrFlow / sdpp->scale;
  }
  
  if (!crcOk)
    return MSG_RESET;
  
  return MSG_OK;
}


msg_t  sdp3xGetIdent(Sdp3xDriver *sdpp, Sdp3xIdent *id)
{
  Sdp3xRawIdent CACHE_ALIGNED(rid);
  id->sn = id->pn = 0U;
  
  
  sdp3xSend(sdpp, SDP3X_READ_PRODUCT_ID1);
  chThdSleepMilliseconds(1);
  sdp3xSend(sdpp, SDP3X_READ_PRODUCT_ID2);
  
#if I2C_USE_MUTUAL_EXCLUSION
  i2cAcquireBus(sdpp->i2cp);
#endif
  msg_t status = i2cMasterCacheReceiveTimeout(sdpp->i2cp, sdpp->slaveAddr,
					 (uint8_t *) &rid, sizeof(rid),
					 I2C_TIMOUT_100MS);
  if (status != MSG_OK) {
    restartI2c(sdpp->i2cp);
#if I2C_USE_MUTUAL_EXCLUSION
    i2cReleaseBus(sdpp->i2cp);
#endif
    return status;
  }
#if I2C_USE_MUTUAL_EXCLUSION
  i2cReleaseBus(sdpp->i2cp);
#endif
  
  for (size_t i=0; i<SNPN_SIZE; i++) {
    if (!(atomCheck(&rid.snpn[i])))
      return  MSG_RESET;
  }

  for (size_t i=0; i<SN_SIZE*2; i++) {
    id->pn <<= 8;
    id->pn |= rid.snpn[i/2].data[i%2];
  }
 
  for (size_t i=SN_SIZE*2; i<SNPN_SIZE*2; i++) {
    id->sn <<= 8;
    id->sn |= rid.snpn[i/2].data[i%2];
  }
  
  return MSG_OK;
}

msg_t sdp3xSend(const Sdp3xDriver *sdpp, const Sdp3xCommand cmd)
{
#if I2C_USE_MUTUAL_EXCLUSION
  i2cAcquireBus(sdpp->i2cp);
#endif
#if defined(__DCACHE_PRESENT) && __DCACHE_PRESENT != 0
  const Sdp3xCommand CACHE_ALIGNED(alCmd) = cmd;
  msg_t status = i2cMasterCacheTransmitTimeout(sdpp->i2cp, sdpp->slaveAddr,
					       (uint8_t *) &alCmd, sizeof(alCmd),
					       NULL, 0, I2C_TIMOUT_100MS) ;
#else
  msg_t status = i2cMasterCacheTransmitTimeout(sdpp->i2cp, sdpp->slaveAddr,
					       (uint8_t *) &cmd, sizeof(cmd),
					       NULL, 0, I2C_TIMOUT_100MS) ;
#endif
  if (status != MSG_OK) 
    restartI2c(sdpp->i2cp);
  
#if I2C_USE_MUTUAL_EXCLUSION
  i2cReleaseBus(sdpp->i2cp);
#endif
  
  return status;
}


/*
#                 _ __           _                    _                   
#                | '_ \         (_)                  | |                  
#                | |_) |  _ __   _   __   __   __ _  | |_     ___         
#                | .__/  | '__| | |  \ \ / /  / _` | | __|   / _ \        
#                | |     | |    | |   \ V /  | (_| | \ |_   |  __/        
#                |_|     |_|    |_|    \_/    \__,_|  \__|   \___|        
*/




__attribute__ ((pure))
static inline uint8_t crc8_poly31_calc(const uint8_t data[], const size_t len) {
  static const uint8_t crc8_poly31[256] =
    { 0x00, 0x31, 0x62, 0x53, 0xC4, 0xF5, 0xA6, 0x97, 0xB9, 0x88, 0xDB, 0xEA, 0x7D, 0x4C, 0x1F,
      0x2E, 0x43, 0x72, 0x21, 0x10, 0x87, 0xB6, 0xE5, 0xD4, 0xFA, 0xCB, 0x98, 0xA9, 0x3E, 0x0F,
      0x5C, 0x6D, 0x86, 0xB7, 0xE4, 0xD5, 0x42, 0x73, 0x20, 0x11, 0x3F, 0x0E, 0x5D, 0x6C, 0xFB,
      0xCA, 0x99, 0xA8, 0xC5, 0xF4, 0xA7, 0x96, 0x01, 0x30, 0x63, 0x52, 0x7C, 0x4D, 0x1E, 0x2F,
      0xB8, 0x89, 0xDA, 0xEB, 0x3D, 0x0C, 0x5F, 0x6E, 0xF9, 0xC8, 0x9B, 0xAA, 0x84, 0xB5, 0xE6,
      0xD7, 0x40, 0x71, 0x22, 0x13, 0x7E, 0x4F, 0x1C, 0x2D, 0xBA, 0x8B, 0xD8, 0xE9, 0xC7, 0xF6,
      0xA5, 0x94, 0x03, 0x32, 0x61, 0x50, 0xBB, 0x8A, 0xD9, 0xE8, 0x7F, 0x4E, 0x1D, 0x2C, 0x02,
      0x33, 0x60, 0x51, 0xC6, 0xF7, 0xA4, 0x95, 0xF8, 0xC9, 0x9A, 0xAB, 0x3C, 0x0D, 0x5E, 0x6F,
      0x41, 0x70, 0x23, 0x12, 0x85, 0xB4, 0xE7, 0xD6, 0x7A, 0x4B, 0x18, 0x29, 0xBE, 0x8F, 0xDC,
      0xED, 0xC3, 0xF2, 0xA1, 0x90, 0x07, 0x36, 0x65, 0x54, 0x39, 0x08, 0x5B, 0x6A, 0xFD, 0xCC,
      0x9F, 0xAE, 0x80, 0xB1, 0xE2, 0xD3, 0x44, 0x75, 0x26, 0x17, 0xFC, 0xCD, 0x9E, 0xAF, 0x38,
      0x09, 0x5A, 0x6B, 0x45, 0x74, 0x27, 0x16, 0x81, 0xB0, 0xE3, 0xD2, 0xBF, 0x8E, 0xDD, 0xEC,
      0x7B, 0x4A, 0x19, 0x28, 0x06, 0x37, 0x64, 0x55, 0xC2, 0xF3, 0xA0, 0x91, 0x47, 0x76, 0x25,
      0x14, 0x83, 0xB2, 0xE1, 0xD0, 0xFE, 0xCF, 0x9C, 0xAD, 0x3A, 0x0B, 0x58, 0x69, 0x04, 0x35,
      0x66, 0x57, 0xC0, 0xF1, 0xA2, 0x93, 0xBD, 0x8C, 0xDF, 0xEE, 0x79, 0x48, 0x1B, 0x2A, 0xC1,
      0xF0, 0xA3, 0x92, 0x05, 0x34, 0x67, 0x56, 0x78, 0x49, 0x1A, 0x2B, 0xBC, 0x8D, 0xDE, 0xEF,
      0x82, 0xB3, 0xE0, 0xD1, 0x46, 0x77, 0x24, 0x15, 0x3B, 0x0A, 0x59, 0x68, 0xFF, 0xCE, 0x9D,
      0xAC };
  uint8_t crc = 0xff;
  
  for (size_t i=0; i<len; ++i)
    crc = crc8_poly31[crc ^ data[i]];
  
  return crc;
}

static bool atomCheck(const Sdp3xDataAtom *atom)
{
  return crc8_poly31_calc(atom->data, sizeof(atom->data)) == atom->crc;
}

