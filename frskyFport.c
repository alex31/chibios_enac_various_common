#include <ch.h>
#include <hal.h>
#include <string.h>
#include "frskyFport.h"
#include "stdutil.h"
#include "math.h"

#define FPORT_START_STOP 0x7E
#define FPORT_ESCAPE 0x7D
#define FPORT_ESCAPE_DIFF 0x20
#define FPORT_CONTROL_LEN 0x19
#define FPORT_DOWNLINK_LEN 0x08
#define FPORT_UPLINK_LEN 0x08
#define FPORT_TYPE_BYTE_INDEX 0U
#define FPORT_FLAGS_BYTE_INDEX 23U
#define FPORT_RSSI_BYTE_INDEX 24U
// delay before answering to command frame, must no exceed 1ms, but must
// leave time for the frsky receiver to reconfigure RXTX pin in input mode
#define FPORT_ANSWER_DELAY_US 350U
#define WORKING_AREA_SIZE     1024U

typedef enum __attribute__ ((__packed__)) {
  PRIM_NOT_READY=0x0, PRIM_DATA=0x10, PRIM_READ=0x30,
  PRIM_WRITE=0x31, PRIM_RESPONSE=0x32
} FPORT_FramePrime;

typedef enum __attribute__ ((__packed__)) {
  TYPE_CONTROL = 0x0, TYPE_DOWNLINK = 0x01, TYPE_UPLINK = 0x81
} FPORT_FrameType;


typedef struct {
  FPORT_FrameType type;
  FPORT_FramePrime prime;
  fportAppId appId;
  fportTelemetryValue v;
} __attribute__ ((__packed__)) FPORT_DownlinkFrame;

typedef struct {
  uint8_t len;
  FPORT_FrameType type;
  FPORT_FramePrime prime;
  fportAppId appId;
  fportTelemetryValue v;
  uint8_t crc;
} __attribute__ ((__packed__)) FPORT_UplinkFrame;

static void receivingLoopThread (void *arg);
static uint8_t calc_checksum8(uint8_t* buffer, size_t size);
static bool is_checksum8_ok(uint8_t* buffer, size_t size);
static void decodeFportControlMsg (const uint8_t *src,  FPORT_ControlFrame *frm);
#if FPORT_MAX_DATAIDS > 0
static void decodeFportDownlinkMsg (const uint8_t *src,  FPORT_DownlinkFrame *frm);
static size_t sendFportUplinkMsgWithStuffing(SerialDriver *sd,
					     uint8_t *buffer, size_t len);
static void sendFportUplinkMsg (FPORTDriver *fportp, FPORT_UplinkFrame *frm);
static size_t fportTelemetryGetAppIdIdx(FPORTDriver *fportp, const fportAppId appId,
					 const FPORT_MultiplexData nbMplx);
static fportAppIdPair fportTelemetryUpGetNextPair(FPORTDriver *fportp);
#endif

static SerialConfig fportSerialConfig = {
  .speed = 115200,
  .cr1 = 0, // 8 bits, no parity, one stop bit
  .cr2 = USART_CR2_STOP1_BITS, 
  //  .cr3 = USART_CR3_HDSEL // half duplex
};

/*
#                         _         _                 _      _____           _    _            
#                        | |       (_)               | |    |_   _|         (_)  | |           
#                  ___   | |__     | |   ___    ___  | |_     | |    _ __    _   | |_          
#                 / _ \  | '_ \    | |  / _ \  / __| | __|    | |   | '_ \  | |  | __|         
#                | (_) | | |_) |  _/ | |  __/ | (__  \ |_    _| |_  | | | | | |  \ |_          
#                 \___/  |_.__/  |__/   \___|  \___|  \__|  |_____| |_| |_| |_|   \__|         
*/
void fportObjectInit(FPORTDriver *fportp)
{
  memset(fportp, 0, sizeof(FPORTDriver));
}


/*
#                        _                     _            
#                       | |                   | |           
#                 ___   | |_     __ _   _ __  | |_          
#                / __|  | __|   / _` | | '__| | __|         
#                \__ \  \ |_   | (_| | | |    \ |_          
#                |___/   \__|   \__,_| |_|     \__|         
*/
void fportStart(FPORTDriver *fportp, const FPORTConfig *configp)
{
  fportp->config = configp;
  fportp->appIdsSize = 0;
  fportp->errorMsk = FPORT_OK;

#if FPORT_MAX_DATAIDS > 0  
  for(size_t i=0; i<FPORT_MAX_DATAIDS; i++) {
    fportp->appIds[i].key = FPORT_DATAID_UNDEF;
    fportp->appIds[i].value.u32 = 0; 
  }
#endif
#ifndef USART_CR2_RXINV
  // USARTv1 without level inversion capability
  // signal must have been inverted by external device
  chDbgAssert(configp->driverShouldInvert == false,
	      "fonction not available on UARTv1 device (F4xx)");
#else
  if (configp->driverShouldInvert == true)
    fportSerialConfig.cr2 |= (USART_CR2_RXINV | USART_CR2_TXINV);
#endif

  sdStart(configp->sd, &fportSerialConfig);
}


/*
#                        _              _ __          
#                       | |            | '_ \         
#                 ___   | |_     ___   | |_) |        
#                / __|  | __|   / _ \  | .__/         
#                \__ \  \ |_   | (_) | | |            
#                |___/   \__|   \___/  |_|            
*/
void fportStop(FPORTDriver *fportp)
{
  sdStop(fportp->config->sd);
  fportObjectInit(fportp);
}


/*
#                        _                     _            
#                       | |                   | |           
#                 ___   | |_     __ _   _ __  | |_          
#                / __|  | __|   / _` | | '__| | __|         
#                \__ \  \ |_   | (_| | | |    \ |_          
#                |___/   \__|   \__,_| |_|     \__|         
#                 _____                         _                         
#                |  __ \                       (_)                        
#                | |__) |   ___    ___    ___   _   __   __   ___         
#                |  _  /   / _ \  / __|  / _ \ | |  \ \ / /  / _ \        
#                | | \ \  |  __/ | (__  |  __/ | |   \ V /  |  __/        
#                |_|  \_\  \___|  \___|  \___| |_|    \_/    \___|        
*/
void fportStartReceive(FPORTDriver *fportp)
{
  chDbgAssert(fportp->wth == NULL, "already in active state");
  fportp->wth = chThdCreateFromHeap (NULL, WORKING_AREA_SIZE, "fport_loop", 
				    NORMALPRIO, receivingLoopThread, fportp);
}


/*
#                        _              _ __          
#                       | |            | '_ \         
#                 ___   | |_     ___   | |_) |        
#                / __|  | __|   / _ \  | .__/         
#                \__ \  \ |_   | (_) | | |            
#                |___/   \__|   \___/  |_|            
#                 _____                         _                         
#                |  __ \                       (_)                        
#                | |__) |   ___    ___    ___   _   __   __   ___         
#                |  _  /   / _ \  / __|  / _ \ | |  \ \ / /  / _ \        
#                | | \ \  |  __/ | (__  |  __/ | |   \ V /  |  __/        
#                |_|  \_\  \___|  \___|  \___| |_|    \_/    \___|        
*/
void fportStopReceive(FPORTDriver *fportp)
{
  if (fportp->wth != NULL) {
    chThdTerminate(fportp->wth);
    chThdWait(fportp->wth);
    fportp->wth = NULL;
  }
}

#if FPORT_MAX_DATAIDS > 0
void fportTelemetryUpData(FPORTDriver *fportp, const fportAppId appId, 
			      const fportTelemetryValue v)
{
  const size_t idx = fportTelemetryGetAppIdIdx(fportp, appId, MULTIPLEX_1DATA);

  fportp->appIds[idx].value = v; // atomic write, no need for mutex
}

void fportTelemetryUpDataMultiplexed(FPORTDriver *fportp, const fportAppId appId, 
			      const fportTelemetryValue *v, const FPORT_MultiplexData nbMplx)
{
  chDbgAssert(nbMplx <= MULTIPLEX_4DATAS, "multiplex maximum of 4 datas");
  const size_t idx = fportTelemetryGetAppIdIdx(fportp, appId, nbMplx);

  for (size_t i = 0; i < nbMplx; i++) {
    fportp->appIds[idx+i].value = v[i]; // atomic write, no need for mutex
  }
}
#endif

/*
#                 _                    _ __          
#                | |                  | '_ \         
#                | |    ___     ___   | |_) |        
#                | |   / _ \   / _ \  | .__/         
#                | |  | (_) | | (_) | | |            
#                |_|   \___/   \___/  |_|            
*/
static void receivingLoopThread (void *arg)
{
  FPORTDriver *fportp = (FPORTDriver *) arg;

#if FPORT_MAX_DATAIDS > 0
  FPORT_DownlinkFrame fportDL;
  FPORT_UplinkFrame ufrm;
#endif
  
  while(true) {
     sdGet(fportp->config->sd);

 }
}




/*
#                        _                     _                                  ___          
#                       | |                   | |                                / _ \         
#                  ___  | |__     ___    ___  | | _   ___    _   _   _ __ ___   | (_) |        
#                 / __| | '_ \   / _ \  / __| | |/ / / __|  | | | | | '_ ` _ \   > _ <         
#                | (__  | | | | |  __/ | (__  |   <  \__ \  | |_| | | | | | | | | (_) |        
#                 \___| |_| |_|  \___|  \___| |_|\_\ |___/   \__,_| |_| |_| |_|  \___/         
*/
static uint8_t calc_checksum8(uint8_t* buffer, size_t size)
{
    uint16_t checksum = 0;
    for (size_t i=0; i < size; i++)
     checksum += buffer[i]; 

    // done only one time
    // works because frame length is always a lot less than 255 bytes
    checksum = (checksum + (checksum >> 8U)) & 0xff; 

    return ~checksum & 0xff;
}

static bool is_checksum8_ok(uint8_t* buffer, size_t size)
{
  const bool checksumOk = calc_checksum8(buffer, size) == 0;
  return checksumOk;
}

/*
#                     _                              _                 
#                    | |                            | |                
#                  __| |    ___    ___    ___     __| |    ___         
#                 / _` |   / _ \  / __|  / _ \   / _` |   / _ \        
#                | (_| |  |  __/ | (__  | (_) | | (_| |  |  __/        
#                 \__,_|   \___|  \___|  \___/   \__,_|   \___|        
*/
static void decodeFportControlMsg (const uint8_t *src,  FPORT_ControlFrame *frm)
{
  int16_t *dst = frm->channel;
  frm->type = src[FPORT_TYPE_BYTE_INDEX];
  // decode sbus data
  dst[0]  = ((src[1]    ) | (src[2]<<8))                  & 0x07FF;
  dst[1]  = ((src[2]>>3 ) | (src[3]<<5))                  & 0x07FF;
  dst[2]  = ((src[3]>>6 ) | (src[4]<<2)  | (src[5]<<10))  & 0x07FF;
  dst[3]  = ((src[5]>>1 ) | (src[6]<<7))                  & 0x07FF;
  dst[4]  = ((src[6]>>4 ) | (src[7]<<4))                  & 0x07FF;
  dst[5]  = ((src[7]>>7 ) | (src[8]<<1 ) | (src[9]<<9))   & 0x07FF;
  dst[6]  = ((src[9]>>2 ) | (src[10]<<6))                 & 0x07FF;
  dst[7]  = ((src[10]>>5) | (src[11]<<3))                 & 0x07FF;
  dst[8]  = ((src[12]   ) | (src[13]<<8))                 & 0x07FF;
  dst[9]  = ((src[13]>>3) | (src[14]<<5))                 & 0x07FF;
  dst[10] = ((src[14]>>6) | (src[15]<<2) | (src[16]<<10)) & 0x07FF;
  dst[11] = ((src[16]>>1) | (src[17]<<7))                 & 0x07FF;
  dst[12] = ((src[17]>>4) | (src[18]<<4))                 & 0x07FF;
  dst[13] = ((src[18]>>7) | (src[19]<<1) | (src[20]<<9))  & 0x07FF;
  dst[14] = ((src[20]>>2) | (src[21]<<6))                 & 0x07FF;
  dst[15] = ((src[21]>>5) | (src[22]<<3))                 & 0x07FF;

  frm->flags = src[FPORT_FLAGS_BYTE_INDEX];
  frm->rssi = src[FPORT_RSSI_BYTE_INDEX];
}

#if FPORT_MAX_DATAIDS > 0
static void decodeFportDownlinkMsg (const uint8_t *src,  FPORT_DownlinkFrame *frm)
{
  memcpy(frm, src, sizeof(FPORT_DownlinkFrame));
}


/*
#                         _ __    _    _            _            
#                        | '_ \  | |  (_)          | |           
#                 _   _  | |_) | | |   _    _ __   | | _         
#                | | | | | .__/  | |  | |  | '_ \  | |/ /        
#                | |_| | | |     | |  | |  | | | | |   <         
#                 \__,_| |_|     |_|  |_|  |_| |_| |_|\_\        
*/
static size_t sendFportUplinkMsgWithStuffing(SerialDriver *sd, uint8_t *buffer, size_t len)
{
  size_t effectiveLen = 0;
  for (size_t c = 0; c < len; c++) {
    const uint8_t b = buffer[c];
    if ((c == FPORT_START_STOP) || (c == FPORT_ESCAPE)) {
      effectiveLen++;
      sdPut(sd, FPORT_ESCAPE);
      sdPut(sd, b ^ FPORT_ESCAPE_DIFF);
    } else {
      sdPut(sd, b);
    }
    effectiveLen++;
  }
  return effectiveLen;
}

static void sendFportUplinkMsg (FPORTDriver *fportp, FPORT_UplinkFrame *frm)
{
  SerialDriver * const sd = fportp->config->sd;
  frm->len = sizeof(*frm) - 2U;
  frm->crc = calc_checksum8(&frm->len, sizeof(*frm) - 1U);
  size_t sentLen =
    sendFportUplinkMsgWithStuffing(sd, &frm->len, sizeof(*frm));
#if !FPORT_TX_READBACK_DOCHECK
  // empty the queue because half duplex mode
  while(sentLen--)
    sdGet(sd);
#else
  uint8_t checkBuffer[sentLen];
  const size_t nbRead = sdReadTimeout(sd, checkBuffer, sentLen, TIME_MS2I(2));
  if (nbRead != sentLen) {
    // cannot read as many byte than have been emitted : probable TX/TX conflict
    fportp->errorMsk |= FPORT_READBACK_CTRL_FAIL;
  }
#endif
}

/*
#                  __ _          _       _____   _                                      _   
#                 / _` |        | |     / ____| | |                                    | |  
#                | (_| |   ___  | |_   | |      | |__     __ _   _ __    _ __     ___  | |  
#                 \__, |  / _ \ | __|  | |      | '_ \   / _` | | '_ \  | '_ \   / _ \ | |  
#                  __/ | |  __/ \ |_   | |____  | | | | | (_| | | | | | | | | | |  __/ | |  
#                 |___/   \___|  \__|   \_____| |_| |_|  \__,_| |_| |_| |_| |_|  \___| |_|  
#                 _____       _          
#                |_   _|     | |         
#                  | |     __| |         
#                  | |    / _` |         
#                 _| |_  | (_| |         
#                |_____|  \__,_|         
*/
static size_t fportTelemetryGetAppIdIdx(FPORTDriver *fportp, const fportAppId appId,
					 const FPORT_MultiplexData nbMplx)
{
  // if already allocated, find it
  for (size_t i=0; i < fportp->appIdsSize; i++) {
    if (fportp->appIds[i].key == appId) {
      return i;
    }
  }

  // else allocate
  if (fportp->appIdsSize < ((FPORT_MAX_DATAIDS+1U) - nbMplx)) {
    chSysLock();
    for (size_t i=0; i < nbMplx; i++) {
      fportp->appIds[fportp->appIdsSize++].key = appId;
    }
    chSysUnlock();
    return fportp->appIdsSize - 1U;
  }
  
  // if table is full, in Debug mode halt the program
  // in release mode (CH_DBG_ENABLE_ASSERTS=0) , set an erro code
  chDbgAssert(false, "fportTelemetryGetAppIdIdx table is full");
  fportp->errorMsk |= FPORT_APPID_LEN_EXCEDEED;
  
  return FPORT_MAX_DATAIDS;
}

static fportAppIdPair fportTelemetryUpGetNextPair(FPORTDriver *fportp)
{
  // not found
  if (fportp->appIdsSize == 0)
    return (fportAppIdPair) {.key = FPORT_DATAID_UNDEF,
                             .value = (fportTelemetryValue) {.u32 = 0}};

  const fportAppIdPair pair = fportp->appIds[fportp->nextIdToSent];
  fportp->nextIdToSent = (fportp->nextIdToSent + 1) % fportp->appIdsSize;
  return pair;
}



/*
#                 _              _    _ __                        
#                | |            | |  | '_ \                       
#                | |__     ___  | |  | |_) |   ___   _ __         
#                | '_ \   / _ \ | |  | .__/   / _ \ | '__|        
#                | | | | |  __/ | |  | |     |  __/ | |           
#                |_| |_|  \___| |_|  |_|      \___| |_|           
*/
uint32_t transcodeLat(double lat)
{
  uint32_t tmpui = fabs(lat*1e7);  // now we have unsigned value and one bit to spare
  tmpui = (tmpui + tmpui / 2U) / 25U; // MSB=0 to code latitude
  if (lat < 0.0)
    tmpui |= 0x40000000;
  return tmpui;
}

uint32_t transcodeLong(double lon)
{
  uint32_t  tmpui = fabs(lon*1e7);  // now we have unsigned value and one bit to spare
  tmpui = ((tmpui + tmpui / 2U) / 25U) | 0x80000000; // MSB=1 to code longitude  
  if (lon < 0.0)
    tmpui |= 0x40000000;
  return tmpui;
}

#endif
