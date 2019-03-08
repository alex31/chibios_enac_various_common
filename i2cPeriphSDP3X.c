#include "i2cPeriphSDP3X.h"
#include "math.h"

#include "globalVar.h"

static void resetI2c(I2CDriver *i2cp);

typedef struct __attribute__((packed)) {
  uint8_t data[2];
  uint8_t crc;
}  Spd3xDataAtom ;

typedef struct __attribute__((packed)) {
  Spd3xDataAtom press;
  Spd3xDataAtom temp;
  Spd3xDataAtom scale;
} Sdp3xMeasure ;

typedef struct __attribute__((packed)) {
  Spd3xDataAtom pn2;
  Spd3xDataAtom pn1;
  Spd3xDataAtom sn4;
  Spd3xDataAtom sn3;
  Spd3xDataAtom sn2;
  Spd3xDataAtom sn1;
} Sdp3xRawIdent ;

struct _Spd3xDriver {
  I2CDriver	 *i2cp;
  Spd3xAddress slaveAddr;
  float		scale;
  float		massflow;
  float		pressure;
  float		temp;
} ;

static uint8_t crc8_poly31_calc (const uint8_t data[], const size_t len);
static bool atomCheck(const Spd3xDataAtom *atom);

/* After the start measurement command is sent:-the first measurement result 
   is available after 8ms;.-small accuracy deviations (few %of reading) 
   can occur during the next 12ms. */
#define FIRST_READ_TIMOUT_MS 10U
#define I2C_TIMOUT_MS 100U

/*
#                         _ __    _          
#                        | '_ \  (_)         
#                  __ _  | |_) |  _          
#                 / _` | | .__/  | |         
#                | (_| | | |     | |         
#                 \__,_| |_|     |_|         
*/


msg_t sdp3xStart(Spd3xDriver *sdpp, I2CDriver *i2cp,
		 const Spd3xAddress addr, const Spd3xRequest request)
{
  sdpp->i2cp = i2cp;
  sdpp->slaveAddr = addr;
  if (request != SPD3X_none) 
    return sdp3xRestart(sdpp, request);
  else
    return MSG_OK;
}

msg_t  sdp3xRestart(Spd3xDriver *sdpp, const Spd3xRequest request)
{
  Sdp3xMeasure meas;
  Spd3xCommand cmd;
  msg_t status;
  
  switch (request) {
  case SPD3X_massflow :
  case SPD3X_massflow_temp :
    cmd = SPD3X_CONTINUOUS_MASSFLOW_AVERAGE;
    break;
    
  case SPD3X_pressure :
  case SPD3X_pressure_temp :
  case SPD3X_pressure_temp_scale :
    cmd = SPD3X_CONTINUOUS_DIFFPRESS_AVERAGE;
    break;

  case SPD3X_massflow_temp_oneshot:
    cmd = SPD3X_ONESHOT_MASSFLOW;
    break;

  case SPD3X_pressure_temp_scale_oneshot:
    cmd = SPD3X_ONESHOT_DIFFPRESS;
    break;
    
    
  default:
    cmd = SPD3X_NOT_INITIALIZED;
  }

  //  DebugTrace ("restart cmd = 0x%x", cmd);
  if ((status  = sdp3xSend(sdpp, cmd)) != MSG_OK) 
    return status;
  
  chThdSleepMilliseconds(FIRST_READ_TIMOUT_MS);

#if I2C_USE_MUTUAL_EXCLUSION
  i2cAcquireBus(sdpp->i2cp);
#endif
  status = i2cMasterReceiveTimeout(sdpp->i2cp, sdpp->slaveAddr,
				   (uint8_t *) &meas, sizeof(meas),
				   I2C_TIMOUT_MS);
  
  if (status != MSG_OK) {
    resetI2c(sdpp->i2cp);
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
  
  const int16_t pressOrFlow = ((int16_t) meas.press.data[1]) | ((int16_t) (meas.press.data[0]) << 8);
  const int16_t temp = ((int16_t) meas.temp.data[1]) | ((int16_t) (meas.temp.data[0]) << 8);
  const int16_t scale = ((int16_t) meas.scale.data[1]) | ((int16_t) (meas.scale.data[0]) << 8);
  
  sdpp->scale = scale;
  sdpp->temp = temp / SPD3X_TEMP_SCALE;
  if ((cmd == SPD3X_CONTINUOUS_DIFFPRESS_AVERAGE) || (cmd == SPD3X_ONESHOT_DIFFPRESS)) {
    sdpp->pressure = pressOrFlow / sdpp->scale;
    sdpp->massflow = NAN;
  } else {
    sdpp->massflow = pressOrFlow;
    sdpp->pressure = NAN;
  }
  
  return MSG_OK;
}
  

msg_t  sdp3xStop(Spd3xDriver *sdpp)
{
  return sdp3xSend(sdpp, SPD3X_STOP_CONTINUOUS);
}

msg_t  sdp3xGeneralReset(I2CDriver *i2cp)
{
  static const uint8_t command[] = {SPD3X_GENERAL_RESET_COMMAND};
  
#if I2C_USE_MUTUAL_EXCLUSION
  i2cAcquireBus(i2cp);
#endif

  msg_t status = i2cMasterTransmitTimeout(i2cp, SPD3X_GENERAL_RESET_ADDRESS,
					  command, sizeof(command),
					  NULL, 0, I2C_TIMOUT_MS) ;
  if (status != MSG_OK) {
    resetI2c(i2cp);
#if I2C_USE_MUTUAL_EXCLUSION
    i2cReleaseBus(i2cp);
#endif
  }
#if I2C_USE_MUTUAL_EXCLUSION
  i2cReleaseBus(i2cp);
#endif
  return status;
}


msg_t  sdp3xCache(Spd3xDriver *sdpp, const Spd3xRequest request)
{
  msg_t status;
  size_t len;
  Sdp3xMeasure meas;
  bool massflow = false;
  
  switch (request) {
  case SPD3X_massflow :
    len = 3;
    massflow = true;
    break;
  case SPD3X_massflow_temp :
    len = 6;
    massflow = true; 
   break;
  case SPD3X_pressure :
    len = 3;
    break;
  case SPD3X_pressure_temp :
    len = 6;
    break;
  case SPD3X_pressure_temp_scale :
    len = 9;
    break;

  default:
    len = 0;
  }

  if (len == 0)
    return MSG_RESET;

#if I2C_USE_MUTUAL_EXCLUSION
  i2cAcquireBus(sdpp->i2cp);
#endif
  status = i2cMasterReceiveTimeout(sdpp->i2cp, sdpp->slaveAddr,
				   (uint8_t *) &meas, len,
				   I2C_TIMOUT_MS);
  
  if (status != MSG_OK) {
    resetI2c(sdpp->i2cp);
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
    const int16_t scale = ((int16_t) meas.scale.data[1]) | ((int16_t) (meas.scale.data[0]) << 8);
    sdpp->scale = scale;
    /* FALLTHRU */
  case 6:
    crcOk |= atomCheck(&meas.temp);
    const int16_t temp = ((int16_t) meas.temp.data[1]) | ((int16_t) (meas.temp.data[0]) << 8);
    sdpp->temp = temp / SPD3X_TEMP_SCALE;
    /* FALLTHRU */
  case 3:
    crcOk |= atomCheck(&meas.press);
    const int16_t pressOrFlow = ((int16_t) meas.press.data[1]) | ((int16_t) (meas.press.data[0]) << 8);
    if (massflow == false) {
      sdpp->pressure = pressOrFlow / sdpp->scale;
      sdpp->massflow = NAN;
    } else {
      sdpp->massflow = pressOrFlow;
      sdpp->pressure = NAN;
    }
  }
  
  if (!crcOk)
    return MSG_RESET;
  
  return MSG_OK;
}

msg_t  sdp3xGetIdent(Spd3xDriver *sdpp, Spd3xIdent *id)
{
  Sdp3xRawIdent rid;
  
  sdp3xSend(sdpp, SPD3X_READ_PRODUCT_ID1);
  chThdSleepMilliseconds(1);
  sdp3xSend(sdpp, SPD3X_READ_PRODUCT_ID2);

#if I2C_USE_MUTUAL_EXCLUSION
  i2cAcquireBus(sdpp->i2cp);
#endif
  msg_t status = i2cMasterReceiveTimeout(sdpp->i2cp, sdpp->slaveAddr,
					 (uint8_t *) &rid, sizeof(rid),
					 I2C_TIMOUT_MS);
  if (status != MSG_OK) {
    resetI2c(sdpp->i2cp);
#if I2C_USE_MUTUAL_EXCLUSION
    i2cReleaseBus(sdpp->i2cp);
#endif
    return status;
  }
#if I2C_USE_MUTUAL_EXCLUSION
    i2cReleaseBus(sdpp->i2cp);
#endif

  if (!(atomCheck(&rid.sn1) &&
	atomCheck(&rid.sn2) &&
	atomCheck(&rid.sn3) &&
	atomCheck(&rid.sn4) &&
	atomCheck(&rid.pn1) &&
	atomCheck(&rid.pn2))) {
    return  MSG_RESET;
  }

  DebugTrace("pn2.0 pn2.1 pn1.0 pn1.1 = 0x%x:%x:%x:%x",
  	     rid.pn2.data[0], rid.pn2.data[1], rid.pn1.data[0], rid.pn1.data[1]);
  
  id->pn = SWAP_ENDIAN32_BY_8(rid.pn1.data[1], rid.pn1.data[0],
			      rid.pn2.data[1], rid.pn2.data[0]);

  const uint64_t snl = SWAP_ENDIAN32_BY_8(rid.sn3.data[1], rid.sn3.data[0],
			      rid.sn2.data[1], rid.sn2.data[0]);
  const uint64_t snm = SWAP_ENDIAN32_BY_8(rid.sn1.data[1], rid.sn1.data[0],
			      rid.sn2.data[1], rid.sn2.data[0]);
  id->sn = (snl << 32) | (snm & 0xffffffff);
  
  return MSG_OK;
}

inline float sdp3xGetPressure(Spd3xDriver *sdpp)
{
  return sdpp->pressure;
}

float sdp3xGetTemp(Spd3xDriver *sdpp)
{
  return sdpp->temp;
}

float sdp3xGetScale(Spd3xDriver *sdpp)
{
  return sdpp->scale;
}

float sdp3xGetMassflow(Spd3xDriver *sdpp)
{
  return sdpp->massflow;
}


msg_t sdp3xSend(const Spd3xDriver *sdpp, const Spd3xCommand cmd)
{
#if I2C_USE_MUTUAL_EXCLUSION
  i2cAcquireBus(sdpp->i2cp);
#endif
  
  msg_t status = i2cMasterTransmitTimeout(sdpp->i2cp, sdpp->slaveAddr,
					  (uint8_t *) &cmd, sizeof(cmd),
					  NULL, 0, I2C_TIMOUT_MS) ;
  if (status != MSG_OK) 
    resetI2c(sdpp->i2cp);

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




static uint8_t crc8_poly31_calc (const uint8_t data[], const size_t len) {
  uint8_t crc = 0xff;
  static const uint8_t polynomial = 0x31;
  //calculates 8-Bit checksum with given polynomial
  for (uint8_t byteCtr = 0; byteCtr < len; ++byteCtr) { 
    crc ^= (data[byteCtr]);
    for (uint8_t bit = 8; bit > 0; --bit) 
      crc = (crc & 0x80) ? (crc << 1) ^ polynomial : crc << 1;
  }
  return crc;
} 

static bool atomCheck(const Spd3xDataAtom *atom)
{
  return crc8_poly31_calc(atom->data, sizeof(atom->data)) == atom->crc;
}

static void resetI2c(I2CDriver *i2cp)
{
    const I2CConfig *cfg = i2cp->config;
    i2cStop(i2cp);
    chThdSleepMilliseconds(1); 
    i2cStart(i2cp, cfg);
    chThdSleepMilliseconds(1); 
}
