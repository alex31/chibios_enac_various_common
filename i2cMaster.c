#include <ch.h>
#include <hal.h>
#include "globalVar.h"
#include "printf.h"
#include "stdutil.h"
#include "i2cMaster.h"
#include <alloca.h>


/*
  ° separer lib i2cMaster et lib de composants : objet capteur
    contenant l'adresse de driver i2c
  ° prevoir pour le baro et le magnéto init sur i2c ou via 6050
  ° oversampling en param à l'init de tous les capteurs
*/


/*
  Beware that all functions here need to be reeentrant
  no static variables unless they are const.
*/




static bool_t chkErrorI2cMaster (I2CDriver *i2cd);
static const I2cMasterConfig * getMasterConfigFromDriver (I2CDriver *i2cd);

#ifdef I2C_USE_HMC5883L
static msg_t  i2cMasterControlForHMC5883L_SLV2 (I2CDriver *i2cd);
#endif

#ifdef I2C_USE_MPL3115A2
static msg_t  i2cMasterControlForMPL3115A2_SLV_RW_01 (I2CDriver *i2cd);
#endif

static bool_t i2cMasterResetBus (I2CDriver *i2cd);
static bool_t i2cMasterUnhangBus (I2CDriver *i2cd);
static void   i2cMasterSetModePeriphI2c (I2CDriver *i2cd);
static msg_t  i2cMasterWriteBit (I2CDriver *i2cd, const uint8_t slaveAdr,  
				const uint8_t regAdr, const uint8_t mask, 
				bool_t enable);

/*      
i2cReleaseBus(i2cd);			\  
chMtxInit(&i2cd->mutex);		\
*/


#define STATUS_TEST_WRITE(i2cd,array)  { \
    if (status != RDY_OK) {						\
      DebugTrace ("I2C error write " #array                             \
		  " on " #i2cd                                          \
		  " status =%s", status == RDY_RESET ?			\
		  "RDY_RESET" : "RDY_TIMEOUT");				\
      chkErrorI2cMaster (i2cd);						\
      i2cReleaseBus(i2cd);						\
      return status; }							\
  }

#define STATUS_TEST_READ_WRITE(i2cd,r_array,w_array)  { \
    if (status != RDY_OK) {						\
      DebugTrace ("I2C error read  " #r_array  " write " #w_array " status =%s", \
		  status == RDY_RESET ? "RDY_RESET" : "RDY_TIMEOUT");	\
      chkErrorI2cMaster (i2cd);						\
      i2cReleaseBus(i2cd);						\
      return status; }							\
  }

#define I2C_WRITE(i2cd,adr,array)   {					\
    status = i2cMasterTransmitTimeout(i2cd, adr, array, sizeof(array),	\
				      NULL, 0, 100) ;			\
    STATUS_TEST_WRITE(i2cd,array)}

#define I2C_WRITE_REGISTERS(i2cd,adr,regAdr,...)   {			\
    const uint8_t array[] = {regAdr, __VA_ARGS__};			\
    status = i2cMasterTransmitTimeout(i2cd, adr, array, sizeof(array),	\
				      NULL, 0, 100) ;			\
    STATUS_TEST_WRITE(i2cd,array)}

#define I2C_WRITELEN(i2cd,adr,w_array,w_size)   {			\
    status = i2cMasterTransmitTimeout(i2cd, adr, w_array, w_size,	\
				      NULL, 0, 100) ;			\
    STATUS_TEST_WRITE(i2cd,array)}

#define I2C_READ_WRITE(i2cd,adr,r_array,w_array,w_size)   {			\
    status = i2cMasterTransmitTimeout(i2cd, adr, r_array, sizeof(r_array),	\
				      w_array, w_size, 100) ;			\
    STATUS_TEST_READ_WRITE(i2cd,r_array,w_array) }

#define I2C_READ_REGISTERS(i2cd,adr,regAdr,w_array)   {				\
    const uint8_t r_array[] = {regAdr};						\
    status = i2cMasterTransmitTimeout(i2cd, adr, r_array, sizeof(r_array),	\
				      w_array, sizeof(w_array), 100) ;		\
    STATUS_TEST_READ_WRITE(i2cd,r_array,w_array) }

#define I2C_READLEN_REGISTERS(i2cd,adr,regAdr,w_array,w_len)   {		\
    const uint8_t r_array[] = {regAdr};						\
    status = i2cMasterTransmitTimeout(i2cd, adr, r_array, sizeof(r_array),	\
				      w_array, w_len, 100) ;			\
    STATUS_TEST_READ_WRITE(i2cd,r_array,w_array) }

#define I2C_READ_REGISTER(i2cd,adr,regAdr,w_val)   {				\
    const uint8_t r_array[] = {regAdr};						\
    status = i2cMasterTransmitTimeout(i2cd, adr, r_array, sizeof(r_array),	\
				      w_val, sizeof(*w_val), 100) ;		\
    STATUS_TEST_READ_WRITE(i2cd,r_array,w_val) }



// this is formely not const, but in fact it's written only once
#if STM32_I2C_USE_I2C1
__attribute__((section(".data")))
static const I2cMasterConfig const *i2c1=NULL;
#endif
#if STM32_I2C_USE_I2C2
__attribute__((section(".data")))
static const I2cMasterConfig const *i2c2=NULL;
#endif
#if STM32_I2C_USE_I2C3
__attribute__((section(".data")))
static const I2cMasterConfig const *i2c3=NULL;
#endif

bool_t initI2cDriver (const I2cMasterConfig *mconf)
{
#if STM32_I2C_USE_I2C1
  if (mconf->driver == &I2CD1) {
    i2c1 = mconf;
  }
#endif

#if STM32_I2C_USE_I2C2
  if (mconf->driver == &I2CD2) {
    i2c2 = mconf;
  }
#endif

#if STM32_I2C_USE_I2C3
  if (mconf->driver == &I2CD3) {
    i2c3 = mconf;
  }
#endif

  if (!i2cMasterUnhangBus (mconf->driver))
    goto error;
  
  i2cMasterSetModePeriphI2c (mconf->driver);

  i2cStart(mconf->driver, mconf->i2ccfg);
  return TRUE;
  
 error:
  chkErrorI2cMaster (mconf->driver);
  return FALSE;
}

static const I2cMasterConfig * getMasterConfigFromDriver (I2CDriver *i2cd)
{
#if STM32_I2C_USE_I2C1
  if (i2cd == &I2CD1) 
    return i2c1 ;
#endif

#if STM32_I2C_USE_I2C2
  if (i2cd == &I2CD2) 
    return i2c2 ;
#endif

#if STM32_I2C_USE_I2C3
  if (i2cd == &I2CD3) 
    return i2c3 ;
#endif

  return NULL;
}
 

/*
#                                              _                                
#                                             | |                               
#                  __ _    ___    ___    ___  | |    ___   _ __    ___          
#                 / _` |  / __|  / __|  / _ \ | |   / _ \ | '__|  / _ \         
#                | (_| | | (__  | (__  |  __/ | |  |  __/ | |    | (_) |        
#                 \__,_|  \___|  \___|  \___| |_|   \___| |_|     \___/         
*/

/*
       adxl345_write_reg(0x2D, 0x08);        // measurement mode (sinon X=Y=Z=0)
       adxl345_write_reg(0x2C, 0x0F);        // Data rate = 3200Hz - Bandwidth = 1600Hz
       adxl345_write_reg(0x2C, 0x07);        // Data rate = 12.5Hz - Bandwidth = 6.25Hz

       adxl345_write_reg(0x31, 0x08);        // full resolution / +/-2g
       adxl345_write_reg(0x31, 0x09);        // full resolution / +/-4g
       adxl345_write_reg(0x31, 0x0B);        // full resolution / +/-16g
       adxl345_write_reg(0x31, 0x03);        // 10 bits resolution / +/-16g

*/

#ifdef I2C_USE_ADXL345
const uint8_t adxlAdr =  0x1d;
msg_t i2cInitAcc_ADXL345 (I2CDriver *i2cd)
{
  msg_t status;
  // first byte of each array is register address of i2c slave
  const uint8_t dataRateADXL[] = {0x2c, 0x0E, 0x08};
  const uint8_t resolution[] = {0x31, 0x0B}; // full resolution / +/-16g

  i2cAcquireBus(i2cd);
  I2C_WRITE(i2cd, adxlAdr, dataRateADXL);
  I2C_WRITE(i2cd, adxlAdr, resolution);
  i2cReleaseBus(i2cd);
  return status;
}


msg_t i2cGetAcc_ADXL345_DevId (I2CDriver *i2cd, uint8_t  *devid)
{
  const uint8_t txbuf[] = {0};
  uint8_t rxbuf[2];
  msg_t status;

  i2cAcquireBus(i2cd);
  I2C_READ_WRITE(i2cd, adxlAdr, txbuf, rxbuf, sizeof(rxbuf));
  *devid = rxbuf[0];
  i2cReleaseBus(i2cd);
  return status;
}


msg_t i2cGetAcc_ADXL345_Val (I2CDriver *i2cd, int16_t  rawBuf[3], float acc[3])
{
  msg_t status;
  const uint8_t txbuf[] = {0x32};
  int16_t  *rawB = rawBuf != NULL ? rawBuf : alloca(sizeof(int16_t)*3);

  i2cAcquireBus(i2cd);
  I2C_READ_WRITE(i2cd, adxlAdr, txbuf, (uint8_t *) rawB, sizeof(int16_t)*3);
  i2cReleaseBus(i2cd);

  acc[0] = rawB[0] * 0.004f;
  acc[1] = rawB[1] * 0.004f;
  acc[2] = rawB[2] * 0.004f;
  return status;
}
#endif


/*
  #                  __ _   _   _                         
  #                 / _` | | | | |                        
  #                | (_| | | |_| |   _ __    ___          
  #                 \__, |  \__, |  | '__|  / _ \         
  #                  __/ |   __/ |  | |    | (_) |        
  #                 |___/   |___/   |_|     \___/         
*/

/*
  void init_ITG3200(void)
  {
  sur LitePilot, adresse escalve ITG-3200 (7bits) = 0x69
 
  Register 21 – Sample Rate Divider
  write_slave_1byte(0x69, 0x15, 0x00);        // SMPLRT_DIV = 0 => Fsample = Finternal / (0 + 1)
  write_slave_1byte(0x69, 0x15, 0x03);        // SMPLRT_DIV = 3 => Fsample = Finternal / (3 + 1)
 
  Register 22 – DLPF, Full Scale
  write_slave_1byte(0x69, 0x16, 0x1B);        // FS_SEL = 3        => Gyro Full-Scale Range = ±2000°/sec (pas le choix...)
  DLPF_CFG = 3        => Low Pass Filter Bandwidth = 42Hz + Internal Sample Rate = 1kHz
  write_slave_1byte(0x69, 0x16, 0x18);        // FS_SEL = 3        => Gyro Full-Scale Range = ±2000°/sec (pas le choix...)
  DLPF_CFG = 0        => Low Pass Filter Bandwidth = 256Hz + Internal Sample Rate = 8kHz    
  }
*/
#ifdef I2C_USE_ITG3200
const uint8_t itgAdr =  0x68; // 0x68 quand AD0 est sur Gnd,  0x69 si sur Vcc
msg_t i2cInitGyro_ITG3200 (I2CDriver *i2cd)
{
  msg_t status;

  // first byte of each array is register address of i2c slave
  const uint8_t dataRateScale[] = {0x15, 0x00, 0x19}; // Bandwidth = 188Hz 
						      // Internal Sample Rate = 1kHz 

  i2cAcquireBus(i2cd);
  I2C_WRITE(i2cd, itgAdr, dataRateScale);
  i2cReleaseBus(i2cd);
  return status;
}


msg_t i2cGetGyro_ITG3200_DevId (I2CDriver *i2cd, uint8_t  *devid)
{
  const uint8_t txbuf[] = {0x0};
  uint8_t rxbuf[4];
  msg_t status;

  i2cAcquireBus(i2cd);
  I2C_READ_WRITE(i2cd, itgAdr, txbuf, rxbuf, 2);
  *devid = rxbuf[0];
  i2cReleaseBus(i2cd);
  return status;
}


msg_t i2cGetGyro_ITG3200_Val (I2CDriver *i2cd, int16_t  rawBuf[4],
			      float *temp, float gyro[3])
{
  const uint8_t txbuf[] = {0x1b};
  msg_t status;
  int16_t  *rawB = rawBuf != NULL ? rawBuf : alloca(sizeof(int16_t)*4);

  
  i2cAcquireBus(i2cd);
  I2C_READ_WRITE(i2cd, itgAdr, txbuf, (uint8_t *) rawB, sizeof(int16_t)*4);
  i2cReleaseBus(i2cd);

  *temp = (SWAP_ENDIAN16(rawB[0]) +23000) / 280.0f;
  gyro[0] =  SWAP_ENDIAN16(rawB[1]) / 14.375f;
  gyro[1] =  SWAP_ENDIAN16(rawB[2]) / 14.375f;
  gyro[2] =  SWAP_ENDIAN16(rawB[3]) / 14.375f;

  return status;
}
#endif

/*
  #                                      __ _                  _                    
  #                                     / _` |                | |                   
  #                 _ __ ___     __ _  | (_| |  _ __     ___  | |_     ___          
  #                | '_ ` _ \   / _` |  \__, | | '_ \   / _ \ | __|   / _ \         
  #                | | | | | | | (_| |   __/ | | | | | |  __/ \ |_   | (_) |        
  #                |_| |_| |_|  \__,_|  |___/  |_| |_|  \___|  \__|   \___/         
*/
#ifdef I2C_USE_HMC5883L
const uint8_t hmcAdr =  0x1e;
msg_t i2cInitCompass_HMC5883L (I2CDriver *i2cd)
{
  // first byte of each array is register address of i2c slave
  msg_t status;

  //  const uint8_t dataRate[] = {0x0, 0x6c, 0x20, 0x0}; // sample average 8, gain (+/-  1.30 G)
  const uint8_t dataRateHMC[] = {0x0, 0x18, 0x00, 0x0}; // no sample average, max gain (+/- 0.88 G)
  
  i2cAcquireBus(i2cd);
  I2C_WRITE (i2cd, hmcAdr, dataRateHMC);
  i2cReleaseBus(i2cd);
  return status;
}


msg_t i2cGetCompass_HMC5883L_DevId (I2CDriver *i2cd, uint8_t  devid[3])
{
  const uint8_t txbuf[] = {0x0a};
  msg_t status;
  // 
  i2cAcquireBus(i2cd);
  I2C_READ_WRITE(i2cd, hmcAdr, txbuf, devid, 3);
  i2cReleaseBus(i2cd);
  return status;
}


msg_t i2cGetCompass_HMC5883L_Val (I2CDriver *i2cd, int16_t  rawBuf[3],
				  float mag[3])
{
  const uint8_t txbuf[] = {0x03};
  msg_t status;
  int16_t  *rawB = rawBuf != NULL ? rawBuf : alloca(sizeof(int16_t)*3);

  i2cAcquireBus(i2cd);
  I2C_READ_WRITE(i2cd, hmcAdr, txbuf,(uint8_t *) rawB, (sizeof(int16_t)*3));
  i2cReleaseBus(i2cd);

  mag[0] = SWAP_ENDIAN16(rawB[0]) / 1.37f;
  mag[2] = SWAP_ENDIAN16(rawB[1]) / 1.37f; // HMC5883L returns values in x,z,y order
  mag[1] = SWAP_ENDIAN16(rawB[2]) / 1.37f;
  return status;
}
#endif

#ifdef I2C_USE_MPL3115A2
// first byte of each array is register address of i2c slave
//#define  OVERSAMPLING (0x2 << 3) // oversampling ratio = 4 : conv in 18ms
//#define  OVERSAMPLING (0x7 << 3) // oversampling ratio = 128 : conv in 512ms
#define  OVERSAMPLING (0x0 << 3) // no oversampling conv in 6 ms
const uint8_t activeMode[] = {0x26, 0x01 | OVERSAMPLING};
const uint8_t standbyMode[] = {0x26, 0x0 | OVERSAMPLING};
const uint8_t oneShotMode[] = {0x26, 0x2 | OVERSAMPLING};
const uint8_t continuousMode[] = {0x26, 0x2 | OVERSAMPLING};
const uint8_t enableEvent[] = {0x13, 0x07};
const uint8_t statusReg[] = {0x00};
const uint8_t pressureReg[] = {0x01};

const uint8_t mplAdr =  0x60;
msg_t i2cInitBaro_MPL3115A2 (I2CDriver *i2cd, MPL3115A2_Mode mode)
{
  msg_t status;
  (void) mode;

  i2cAcquireBus(i2cd);
  I2C_WRITE (i2cd, mplAdr, enableEvent);
  I2C_WRITE (i2cd, mplAdr, standbyMode);
  i2cReleaseBus(i2cd);

  return status;
}


msg_t i2cGetBaro_MPL3115A2_Val (I2CDriver *i2cd, int32_t  *rawBuf, float *pressure)
{
  uint8_t rxbuf[4];
  bool_t  notReady;
  msg_t status;
  int32_t  *rawB = rawBuf != NULL ? rawBuf : alloca(sizeof(int32_t));

  i2cAcquireBus(i2cd);
  
  I2C_WRITE(i2cd, mplAdr, oneShotMode);
  do {
    I2C_READ_WRITE(i2cd, mplAdr, statusReg,rxbuf, 2);
    notReady = !(rxbuf[0] & 1<<2);
    if (notReady) {
      //      i2cReleaseBus(i2cd);
      chThdSleepMilliseconds(2);
      //      i2cAcquireBus(i2cd);
    }
  } while (notReady);
  
  I2C_READ_WRITE(i2cd, mplAdr, pressureReg, (uint8_t *)rawB, 3);
  i2cReleaseBus(i2cd);

  const uint32_t swapVal = (SWAP_ENDIAN32(*rawB<<8)) ;
  *pressure = swapVal/6400.0f;

  return  RDY_OK;
}

msg_t i2cGetBaro_MPL3115A2_DevId (I2CDriver *i2cd, uint8_t  *devid)
{
  const uint8_t txbuf[] = {0x0c};
  uint8_t rxbuf[4];
  msg_t status;

  i2cAcquireBus(i2cd);
  I2C_READ_WRITE(i2cd, mplAdr, txbuf, rxbuf, 2);
  *devid = rxbuf[0];
  i2cReleaseBus(i2cd);
  return status;
}
#endif

/*
  #                 _                              
  #                (_)                             
  #                 _    _ __ ___    _   _         
  #                | |  | '_ ` _ \  | | | |        
  #                | |  | | | | | | | |_| |        
  #                |_|  |_| |_| |_|  \__,_|        
*/


const uint8_t imuAdr =  0x69;
msg_t i2cGetIMU_MPU6050_DevId (I2CDriver *i2cd, uint8_t devid[3])
{
  const uint8_t txbuf[] = {0x75};
  uint8_t rxbuf[4];
  msg_t status;
  
  i2cAcquireBus(i2cd);
  I2C_READ_WRITE(i2cd, imuAdr, txbuf, rxbuf, 2);
  *devid = rxbuf[0];

  i2cReleaseBus(i2cd);
  return status;
}



#ifdef I2C_USE_MPU6050
msg_t i2cGetIMU_MPU6050_Val (I2CDriver *i2cd,  int8_t  *rawBuf, float *temp, 
			     float gyro[3], float acc[3], float *baro, float mag[3])
{
  const uint8_t getVal[] = {0x3b};
  uint8_t i;
  msg_t status = RDY_OK;
  uint8_t  *rawB = rawBuf != NULL ? rawBuf : alloca(sizeof(int8_t)*23);

  //  DebugTrace ("In GetIMU_MPU6050");

  i2cAcquireBus(i2cd);
  I2C_READ_WRITE(i2cd, imuAdr, getVal, rawB, sizeof(int8_t)*23);
  i2cReleaseBus(i2cd);

  *temp = ( ((int16_t) ((rawB[6]<<8) | rawB[7])) / 340.0f) + 36.53f;
  
  for (i=0; i< 3; i++) {
    acc[i] =  ((int16_t) ((rawB[i*2]<<8) | rawB[(i*2)+1])) / 16384.0f; // POUR MODE 2G
  }

  for (i=0; i< 3; i++) {
    gyro[i] =  ((int16_t) ((rawB[(i*2)+8]<<8) | rawB[(i*2)+9])) / 131.0f; // POUR MODE 250°/s
  }

  uint32_t val =  SWAP_ENDIAN32_BY_8(rawB[16], rawB[15], rawB[14], 0);
  *baro = val/6400.0f;

  const int16_t magx = SWAP_ENDIAN16_BY_8(rawB[18], rawB[17]);
  const int16_t magz = SWAP_ENDIAN16_BY_8(rawB[20], rawB[19]);
  const int16_t magy = SWAP_ENDIAN16_BY_8(rawB[22], rawB[21]);
  mag[0] = magx / 1.37f;
  mag[1] = magy / 1.37f;
  mag[2] = magz / 1.37f;

  return status;
}



msg_t i2cInitIMU_MPU6050 (I2CDriver *i2cd, MPU6050_Mode mode)
{
  msg_t status = RDY_OK;
  i2cAcquireBus(i2cd);
  // 0x6b, 0x1 : sortir du mode sleep
  // 0x19, 0x7  : diviseur par 8 sur la frequence d'echantillonage : 8khz/8 = 1khz
  // 0x1a, 0x3  : filtrage bp accelero 44hz, 42hz pour le gyro
  //       OU
  // 0x1a, 0x0  : pas de filtrage bp accelero 260hz, 256hz pour le gyro
  // 0x67, 0x0  : pas d'esclave autorisés, ce sont les fonctions d'init
  //		  par esclave qui doivent renseigner ces bits en fonction de leur
  //		  numéro d'esclave respectifs
  const uint8_t modeActive[][2] = {{0x6b, 0x1}, {0x19, 0x7}, {0x1a, 0x0}};
  const uint8_t initBypass[][2] = {{0x37, 0x2}, {0x6a, 0x0}};
  const uint8_t initMaster[][2] = {{0x37, 0x0}, {0x6a, 0x20}, {0x24, 0x0d}, {0x67, 0x0}};

  I2C_WRITE (i2cd, imuAdr, modeActive[0]);
  I2C_WRITE (i2cd, imuAdr, modeActive[1]);
  I2C_WRITE (i2cd, imuAdr, modeActive[2]);

  if (mode == MPU6050_Bypass) {
    I2C_WRITE (i2cd, imuAdr, initBypass[0]);
    I2C_WRITE (i2cd, imuAdr, initBypass[1]);
  } else {
    I2C_WRITE (i2cd, imuAdr, initMaster[0]);
    I2C_WRITE (i2cd, imuAdr, initMaster[1]);
    I2C_WRITE (i2cd, imuAdr, initMaster[2]);
    I2C_WRITE (i2cd, imuAdr, initMaster[3]);

#ifdef I2C_USE_MPL3115A2
    status = i2cMasterControlForMPL3115A2_SLV_RW_01 (i2cd);
#endif

#ifdef I2C_USE_HMC5883L
    status = i2cMasterControlForHMC5883L_SLV2 (i2cd);
#endif

  }

  //  if (status == RDY_OK) 
  i2cReleaseBus(i2cd);
  
  return status;
}
#endif

#ifdef I2C_USE_HMC5883L
static msg_t   i2cMasterControlForHMC5883L_SLV2 (I2CDriver *i2cd)
{
  msg_t status = RDY_OK;

  // adresse du registre I2C_SLV2_ADDR 0x2b, 
  // (1<<7)  : lecture de l'esclave
  // 0x1e : adresse I2C de l'esclave
  // 0x03 : adresse du registre à lire dans l'esclave (pression baro)
  // (1<<7) : enable slave0 pour transfert de données
  //  0x3   : nombre d'octets à lire dans l'esclave
  const uint8_t i2cMasterControl1[] = {0x2b, 
				       (0x1<<7)|0x1e, 
				       0x03, 
				       (0x1<<7)|0x6};
  
  I2C_WRITE (i2cd, imuAdr, i2cMasterControl1);
  // enable l' esclave 2(read)
  i2cMasterWriteBit (i2cd, imuAdr, 0x67,  0x4, TRUE);
  return status;
}
#endif

#ifdef I2C_USE_MPL3115A2
static msg_t   i2cMasterControlForMPL3115A2_SLV_RW_01 (I2CDriver *i2cd)
{
  msg_t status = RDY_OK;


  // adresse du registre I2C_SLV0_ADDR 0x25, 
  // (0<<7)  : ecriture de l'esclave
  // 0x60 : adresse I2C de l'esclave
  // 0x26 : adresse du registre à ecrire dans l'esclave (demande de conversion)
  // (1<<7) : enable slave0 pour transfert de données
  // 0x1   : nombre d'octets à ecrire dans l'esclave
  // 
  const uint8_t i2cMasterControlWrite1[] = {0x25, 
					    (0x0<<7)|0x60, 
					    0x26,
					    (0x1<<7)|0x1};

  const uint8_t i2cMasterControlWrite2[] = {0x63, 0x2 | OVERSAMPLING};

  // adresse du registre I2C_SLV1_ADDR 0x28, 
  // (1<<7)  : lecture de l'esclave
  // 0x60 : adresse I2C de l'esclave
  // 0x01 : adresse du registre à lire dans l'esclave (pression baro)
  // (1<<7) : enable slave0 pour transfert de données
  //  0x3   : nombre d'octets à lire dans l'esclave
  const uint8_t i2cMasterControlRead1[] = {0x28, 
					   (0x1<<7)|0x60, 
					   0x01,
					   (0x1<<7)|0x3};

  // 0x34 : diviseur pour la frequence à laquelle l'esclave est échantillonné
  // division par 1+valeur, dans notre cas on divise 1khz par 8 : echantillonnage de
  // l'esclave à 125hz
  const uint8_t i2cMasterControl2[] = {0x34, 7};
  // enable du diviseur de frequence d'echantillonage de l'esclave

  I2C_WRITE (i2cd, imuAdr, i2cMasterControlWrite1);
  I2C_WRITE (i2cd, imuAdr, i2cMasterControlWrite2);
  I2C_WRITE (i2cd, imuAdr, i2cMasterControlRead1);
 
  I2C_WRITE (i2cd, imuAdr, i2cMasterControl2);

  // enable les esclave 0(write) et 1(read)
  i2cMasterWriteBit (i2cd, imuAdr, 0x67,  0x3, TRUE);

  return status;
}
#endif

#ifdef I2C_USE_MCP23008
const uint8_t gpio23008Adr =  0x20;
msg_t i2cInitIO_MCP23008 (I2CDriver *i2cd)
{
  const uint8_t initOutput[] = {0x00, 0x00};
  msg_t status;

  i2cAcquireBus(i2cd);
  I2C_WRITE (i2cd, gpio23008Adr, initOutput);
  i2cReleaseBus(i2cd);
 
  return status;
}

msg_t i2cSetIO_MCP23008 (I2CDriver *i2cd, uint8_t ioVal)
{
  const uint8_t gpioOut[] = {0x0a, ioVal};
  msg_t status;

  i2cAcquireBus(i2cd);
  I2C_WRITE (i2cd, gpioAdr, gpioOut);
  i2cReleaseBus(i2cd);
   
  return status;
}
#endif

#ifdef I2C_USE_PCF8574
const uint8_t gpio8574Adr =  0x38;
msg_t i2cSetIO_PCF8574 (I2CDriver *i2cd, uint8_t ioVal)
{
  const uint8_t gpioOut[] = {ioVal};
  msg_t status;

  i2cAcquireBus(i2cd);
  I2C_WRITE (i2cd, gpio8574Adr, gpioOut);
  i2cReleaseBus(i2cd);
   
  return status;
}

msg_t i2cGetIO_PCF8574 (I2CDriver *i2cd, uint8_t *ioVal)
{
  uint8_t gpioIn[1];
  const uint8_t nullBuf[0];
  msg_t status;

  i2cAcquireBus(i2cd);
  I2C_READ_WRITE (i2cd, gpio8574Adr, nullBuf, gpioIn, sizeof(gpioIn));
  i2cReleaseBus(i2cd);

  *ioVal = gpioIn[0];
  
  return status;
}
#endif




/*
  i2cd		: I2C driver
  adrOffset	: pin selectionnable i2c address offset
  bitmask       : mask of channel to be sampled
  useExt_VRef	: use external ref (1) or internal 2.5v ref (0)
  percent	: pointer to an array of 8 float values
 */
#ifdef I2C_USE_ADS7828
const uint8_t adcAdrBase =  0x48;
const uint8_t chanSel[] = {0x80, 0xC0, 0x90, 0xD0, 0xA0, 0xE0, 0xB0, 0xF0};
const uint8_t powerDownExtRef = 0b0100;
const uint8_t powerDownIntRef = 0b1100;
msg_t i2cGetADC_ADS7828_Val (I2CDriver *i2cd, const uint8_t adrOffset, 
			     const uint8_t bitmask, const bool useExt_VRef, 
			     float *percent)
{
  msg_t status = RDY_OK;
  uint8_t rawVal[2];
  //  uint8_t zeroSizeArray[0];
  uint8_t fcount=0;
  const uint8_t adcAdr = adcAdrBase+adrOffset;
  const uint8_t powerDown = useExt_VRef ? powerDownExtRef : powerDownIntRef;

  i2cAcquireBus(i2cd);
  for (uint8_t i=0; i<8; i++) {
    if (bitmask & (1 << i)) {
      const uint8_t command[1] = {chanSel[i]|powerDown};
      I2C_READ_WRITE (i2cd, adcAdr, command, rawVal, sizeof(rawVal));
      //      I2C_WRITE (i2cd, adcAdr, command);
      //      I2C_READ_WRITE (i2cd, adcAdr, zeroSizeArray, rawVal, sizeof(rawVal));
      percent[fcount++]=((rawVal[0]*256)+rawVal[1]) / 4096.0f;
    }
  }
  i2cReleaseBus(i2cd);
  return status;
}
#endif



#ifdef I2C_USE_24AA02
#include <string.h>
const uint8_t eepromI2cAddr =  {0xA0>>1};
const uint32_t eepromPageSize = 8;

static msg_t i2cWriteInPage24AA02 (I2CDriver *i2cd, const uint8_t chipAddr, const uint8_t eepromAddr, 
				  const uint8_t *buffer, const size_t len);



msg_t i2cRead24AA02 (I2CDriver *i2cd, const uint8_t chipAddr,
		     const uint8_t _eepromAddr, uint8_t *buffer, const size_t len)
{
  msg_t status = RDY_OK;
  const uint8_t eepromAddr[1] = {_eepromAddr};

  i2cAcquireBus(i2cd);
  I2C_READ_WRITE(i2cd, eepromI2cAddr|chipAddr, eepromAddr, buffer, len);
  i2cReleaseBus(i2cd);

  return status;
}

msg_t i2cWrite24AA02 (I2CDriver *i2cd, const uint8_t chipAddr, const uint8_t _eepromAddr, 
		      const uint8_t *_buffer, const size_t _len)
{
  msg_t status = RDY_OK;

  struct PtrLen {uint16_t eepromAddr; const uint8_t *buffer; size_t len;} ;
  struct PtrLen pl = {.eepromAddr = _eepromAddr, .buffer = _buffer, .len = _len};

  msg_t writePage (struct PtrLen *ptrLen) {
    msg_t status = RDY_OK;
    const size_t maxLen = eepromPageSize - (ptrLen->eepromAddr % eepromPageSize);
    if (ptrLen->len <= maxLen) {
      status = i2cWriteInPage24AA02 (i2cd, chipAddr, ptrLen->eepromAddr, ptrLen->buffer, ptrLen->len);
      //       DebugTrace ("Final B[0x%x] => E[0x%x] for len %d",  
      //	   ptrLen->eepromAddr, ptrLen->buffer, ptrLen->len); 
       ptrLen->len = 0;
    } else {
      status = i2cWriteInPage24AA02 (i2cd, chipAddr, ptrLen->eepromAddr, ptrLen->buffer, maxLen);
      //      DebugTrace ("Intermediate B[0x%x] => E[0x%x] for len %d",
      //	  ptrLen->eepromAddr, ptrLen->buffer, maxLen);
      ptrLen->eepromAddr += maxLen;
      ptrLen->buffer += maxLen;
      ptrLen->len -= maxLen;
    }

    return status;
  };

  while ((pl.len != 0) && (status ==  RDY_OK)) {
    status = writePage (&pl);
  }
  
  return status;
}


static msg_t i2cWriteInPage24AA02 (I2CDriver *i2cd, const uint8_t chipAddr, const uint8_t eepromAddr, 
				  const uint8_t *buffer, const size_t len)
{
  msg_t status = RDY_OK;
  uint8_t chunk[9] = {[0]=eepromAddr};
  if (((eepromAddr % eepromPageSize) + len) > eepromPageSize) {
    DebugTrace ("i2cWriteInPage24AA02 ERROR writing %d byte @ 0x%x will cross page boundary",
		len, eepromAddr);
    return RDY_RESET;
  }

  memcpy (&chunk[1], buffer, len);
  i2cAcquireBus(i2cd);
  I2C_WRITELEN(i2cd, eepromI2cAddr|chipAddr, chunk, len+1);
  
  // WAIT for internal write cycle is finished
  while (i2cMasterReceiveTimeout (i2cd, eepromI2cAddr|chipAddr, chunk, 1, 1) != RDY_OK) {
    chThdSleepMilliseconds(1);
  }
  
  i2cReleaseBus(i2cd);
  return status;
}

#endif

#include "i2cPeriphMpu9250.c"

/*
#                 _ __           _                    _                   
#                | '_ \         (_)                  | |                  
#                | |_) |  _ __   _   __   __   __ _  | |_     ___         
#                | .__/  | '__| | |  \ \ / /  / _` | | __|   / _ \        
#                | |     | |    | |   \ V /  | (_| | \ |_   |  __/        
#                |_|     |_|    |_|    \_/    \__,_|  \__|   \___|        
*/


static bool_t chkErrorI2cMaster (I2CDriver *i2cd)
{
  i2cflags_t errors = i2cGetErrors(i2cd);
  uint8_t retry=10;

  do {
    if (errors & I2CD_BUS_ERROR) {
      DebugTrace ("I2CD_BUS_ERROR");
    }
    
    if (errors & I2CD_ARBITRATION_LOST) {
      DebugTrace ("I2CD_ARBITRATION_LOST");
    }
    
    if (errors & I2CD_ACK_FAILURE) {
      DebugTrace ("I2CD_ACK_FAILURE");
    }
    if (errors & I2CD_OVERRUN) {
      DebugTrace ("I2CD_OVERRUN");
    }
    if (errors & I2CD_PEC_ERROR) {
      DebugTrace ("I2CD_PEC_ERROR");
    }
    if (errors & I2CD_TIMEOUT) {
      DebugTrace ("I2CD_TIMEOUT");
    }
    if (errors & I2CD_SMB_ALERT) {
      DebugTrace ("I2CD_SMB_ALERT");
    }
    
    i2cMasterResetBus (i2cd);
  } while (((errors = i2cGetErrors(i2cd)) != 0) && retry--); 
  

  return (errors != 0x0);
}


static bool_t   i2cMasterResetBus (I2CDriver *i2cd)
{
  i2cStop (i2cd);
  bool_t res = i2cMasterUnhangBus (i2cd);
  i2cStart(i2cd, getMasterConfigFromDriver(i2cd)->i2ccfg);
  return res;
}


/* static const I2cMasterConfig i2c1 = { */
/*   .driver = &I2CD1, */
/*   .sdaGpio = GPIOB,     */
/*   .sclGpio = GPIOB,   */
/*   .sdaPin = GPIOB_I2C1_SDA,	    */
/*   .sclPin = GPIOB_I2C1_SCL,	    */
/*   .sdaAlt = 4,	    */
/*   .alternateFunction = 4 */
/* }; */

static bool_t i2cMasterUnhangBus (I2CDriver *i2cd)
{
  const I2cMasterConfig* i2cMcfg = getMasterConfigFromDriver (i2cd);
  bool_t sdaReleased;
  
  palSetPadMode (i2cMcfg->sdaGpio, i2cMcfg->sdaPin, PAL_MODE_INPUT); 
  sdaReleased = (palReadPad (i2cMcfg->sdaGpio, i2cMcfg->sdaPin) == 1);
  if (sdaReleased) 
    goto end;
  
  palSetPadMode (i2cMcfg->sclGpio, i2cMcfg->sclPin, PAL_MODE_INPUT);
  bool_t currentInput = palReadPad (i2cMcfg->sclGpio, i2cMcfg->sclPin);
  if (currentInput) 
    palSetPad (i2cMcfg->sclGpio, i2cMcfg->sclPin);
  else
    palClearPad (i2cMcfg->sclGpio, i2cMcfg->sclPin);
  
  palSetPadMode (i2cMcfg->sclGpio, i2cMcfg->sclPin, PAL_MODE_OUTPUT_PUSHPULL);
  
  for (uint8_t i=0; i<=8; i++) {
    halPolledDelay (US2ST(10)) ; // 10µs : 100 khz
    palTogglePad (i2cMcfg->sclGpio, i2cMcfg->sclPin);
    halPolledDelay (US2ST(10)) ; // 10µs : 100 khz
    palTogglePad (i2cMcfg->sclGpio, i2cMcfg->sclPin);
    halPolledDelay (US2ST(10)) ; // 10µs : 100 khz
    
    sdaReleased = (palReadPad (i2cMcfg->sdaGpio, i2cMcfg->sdaPin) == 1);
    if (sdaReleased) 
      break;
  }
  
 end:
  DebugTrace ("Unhang %s", sdaReleased ? "OK" : "Fail");
  return sdaReleased; 
}


static void i2cMasterSetModePeriphI2c (I2CDriver *i2cd)
{
  const I2cMasterConfig* i2cMcfg = getMasterConfigFromDriver (i2cd);
  
  palSetPadMode (i2cMcfg->sclGpio, i2cMcfg->sclPin, 
		 PAL_MODE_ALTERNATE(i2cMcfg->alternateFunction) | PAL_STM32_OTYPE_OPENDRAIN |
		 PAL_STM32_OSPEED_HIGHEST | PAL_STM32_PUDR_PULLUP);
  
  palSetPadMode (i2cMcfg->sdaGpio, i2cMcfg->sdaPin, 
  		 PAL_MODE_ALTERNATE(i2cMcfg->alternateFunction) | PAL_STM32_OTYPE_OPENDRAIN |
		 PAL_STM32_OSPEED_HIGHEST | PAL_STM32_PUDR_PULLUP);
  
}

__attribute__((__unused__))
static msg_t i2cMasterWriteBit (I2CDriver *i2cd, const uint8_t slaveAdr,  const uint8_t regAdr,
				const uint8_t mask, bool_t enable) 
{
  msg_t status;
  const uint8_t txbuf[] = {regAdr};
  uint8_t recBuf[2]; // 

  I2C_READ_WRITE(i2cd, slaveAdr, txbuf, recBuf, 2);
  if (enable) {
    recBuf[0] |= mask;
  } else {
    recBuf[0] &= ~mask;
  }

  I2C_WRITE(i2cd, slaveAdr, recBuf);
  
  return RDY_OK;
}
