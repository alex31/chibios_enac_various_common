#include "frskyFport_fsm.h"
#include <string.h>
#include <math.h>

#define FPORT_START_STOP 0x7EU
#define FPORT_ESCAPE 0x7DU
#define FPORT_ESCAPE_DIFF 0x20U
#define FPORT_DOWNLINK_LEN 0x08U
#define FPORT_TYPE_BYTE_INDEX 0U
#define FPORT_FLAGS_BYTE_INDEX 23U
#define FPORT_RSSI_BYTE_INDEX 24U

#define FPORT_CONTROL_AND_DOWNLINK_FRAME_LENGTH (FPORT_CONTROL_LEN + FPORT_DOWNLINK_LEN + 8U)
#define FPORT_DOWNLINK_FRAME_LENGTH  (FPORT_DOWNLINK_LEN + 4U)


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
  FportTelemetryValue v;
} __attribute__ ((__packed__)) Fport_DownlinkFrame;

typedef struct {
  uint8_t len;
  FPORT_FrameType type;
  FPORT_FramePrime prime;
  fportAppId appId;
  FportTelemetryValue v;
  uint8_t crc;
} __attribute__ ((__packed__)) Fport_UplinkFrame;


static FportErrorMask fportFeedFsmOneByte(FportFsmContext *context, uint8_t byte);
static FportErrorMask fportFeedFsmManageReceivedMsg(FportFsmContext *context);
static FportErrorMask fportFeedFsmManageReceivedControlMsg(FportFsmContext *context);
static FportErrorMask fportFeedFsmManageReceivedDownlinkMsg(FportFsmContext *context);
static uint8_t calc_checksum8(uint8_t* buffer, size_t size);
static bool is_checksum8_ok(uint8_t* buffer, size_t size);
static void decodeFportControlMsg (const uint8_t *src,  Fport_ControlFrame *frm);
#if FPORT_MAX_DATAIDS > 0
static void decodeFportDownlinkMsg (const uint8_t *src,  Fport_DownlinkFrame *frm);
static size_t makeFportUplinkMsgWithStuffing(Fport_UplinkFrame *frm,
					     uint8_t *dest, size_t destSize);
static size_t fportTelemetryGetAppIdIdx(FportFsmContext *context, const fportAppId appId,
					 const FportMultiplexData nbMplx);
static fportAppIdPair fportTelemetryUpGetNextPair(FportFsmContext *context);
#endif

static FportErrorMask errorMsk = FPORT_OK;

void	        fportContextInit(FportFsmContext *context, const FportFsmContextConfig *cfg)
{
  memset(context, 0, sizeof(*context));
  context->config = cfg;
  static Fport_UplinkFrame aliveUfrm = {
    .type = TYPE_UPLINK,
    .prime = PRIM_NOT_READY,
    .appId = FPORT_DATAID_NULL,
    .v = {.u32 = 0}};
  context->aliveLen = makeFportUplinkMsgWithStuffing(&aliveUfrm, context->aliveBuf,
						    sizeof(context->aliveBuf));
  context->appIdsSize = 0;
  #if FPORT_MAX_DATAIDS > 0  
  for(size_t i=0; i<FPORT_MAX_DATAIDS; i++) {
    context->appIds[i].key = FPORT_DATAID_UNDEF;
    context->appIds[i].value.u32 = 0; 
  }
#endif

}


FportErrorMask fportFeedFsm(FportFsmContext *context, const void *buffer, size_t len)
{
  for (size_t i=0; i<len; i++)
    errorMsk |= fportFeedFsmOneByte(context, ((const uint8_t *)buffer)[i]);
  return errorMsk;
}




size_t	 fportGetNextReadLen(const FportFsmContext *context)
{
  size_t retLen;
  
  switch (context->state.step) {
  case FPORT_FSM_WAIT_HEADER: {
    retLen = context->lastLen == FPORT_CONTROL_LEN ?
      FPORT_DOWNLINK_FRAME_LENGTH :
      FPORT_CONTROL_AND_DOWNLINK_FRAME_LENGTH;
    break;
  }
    
  case FPORT_FSM_WAIT_LEN: {
    retLen = context->lastLen == FPORT_CONTROL_LEN ?
      FPORT_DOWNLINK_FRAME_LENGTH -1U  :
      FPORT_CONTROL_AND_DOWNLINK_FRAME_LENGTH - 1U;
    break;
  }
    
  case FPORT_FSM_WAIT_MESSAGE: {
    retLen = context->lastLen == FPORT_CONTROL_LEN ?
      FPORT_CONTROL_AND_DOWNLINK_FRAME_LENGTH - 1U - context->state.writeIdx :
      FPORT_DOWNLINK_FRAME_LENGTH -1U - context->state.writeIdx;
    break;
  }
    
  default:
    retLen = 1U;
  }

  return retLen ?
    retLen <= FPORT_CONTROL_AND_DOWNLINK_FRAME_LENGTH ?
       retLen
       : FPORT_CONTROL_AND_DOWNLINK_FRAME_LENGTH
    : 1U;
}





/*
#                 _ __           _                    _                   
#                | '_ \         (_)                  | |                  
#                | |_) |  _ __   _   __   __   __ _  | |_     ___         
#                | .__/  | '__| | |  \ \ / /  / _` | | __|   / _ \        
#                | |     | |    | |   \ V /  | (_| | \ |_   |  __/        
#                |_|     |_|    |_|    \_/    \__,_|  \__|   \___|        
*/
static FportErrorMask fportFeedFsmOneByte(FportFsmContext *context, uint8_t byte)
{
  switch (context->state.step) {
    /*
      #                 _                          _                        
      #                | |                        | |                       
      #                | |__     ___    __ _    __| |    ___   _ __         
      #                | '_ \   / _ \  / _` |  / _` |   / _ \ | '__|        
      #                | | | | |  __/ | (_| | | (_| |  |  __/ | |           
      #                |_| |_|  \___|  \__,_|  \__,_|   \___| |_|           
    */
  case FPORT_FSM_WAIT_HEADER: {
    if (byte == FPORT_START_STOP) {
      context->state = (FportFsmState) {
	.step = FPORT_FSM_WAIT_LEN,
	.writeIdx = 0,
	.escaped = false
      };
    }
    return errorMsk;
  }
    
    /*
      #                 _                         
      #                | |                        
      #                | |    ___   _ __          
      #                | |   / _ \ | '_ \         
      #                | |  |  __/ | | | |        
      #                |_|   \___| |_| |_|        
    */
  case FPORT_FSM_WAIT_LEN: {
    switch (byte) {
    case FPORT_START_STOP : break;
    case FPORT_CONTROL_LEN  :  context->state.step = FPORT_FSM_WAIT_MESSAGE;
      break;
#if FPORT_MAX_DATAIDS == 0
      // ignore downlink message and do not answer to theses
    case FPORT_DOWNLINK_LEN : context->state.step = FPORT_FSM_WAIT_HEADER;
      break;
#else
    case FPORT_DOWNLINK_LEN : context->state.step = FPORT_FSM_WAIT_MESSAGE;
      break;
#endif      
    default: {
      context->state.step = FPORT_FSM_WAIT_HEADER;
      return FPORT_MALFORMED_FRAME;
    }

    }
    context->lastLen = context->msgBuf[context->state.writeIdx++] = byte;
    return errorMsk;
  }

    /*
      #                                                           __ _                
      #                                                          / _` |               
      #                 _ __ ___     ___   ___    ___     __ _  | (_| |   ___         
      #                | '_ ` _ \   / _ \ / __|  / __|   / _` |  \__, |  / _ \        
      #                | | | | | | |  __/ \__ \  \__ \  | (_| |   __/ | |  __/        
      #                |_| |_| |_|  \___| |___/  |___/   \__,_|  |___/   \___|        
    */
  case FPORT_FSM_WAIT_MESSAGE: {
    if (context->state.escaped) {
      context->state.escaped = false;
      const uint8_t unescaped = byte ^ FPORT_ESCAPE_DIFF;
      if ((unescaped != FPORT_START_STOP) && (unescaped != FPORT_ESCAPE)) {
	context->state.step = FPORT_FSM_WAIT_HEADER;
	return FPORT_MALFORMED_FRAME;
      }
      context->msgBuf[context->state.writeIdx++] = unescaped;
      return errorMsk;
    }
    
    if (byte == FPORT_ESCAPE) {
      context->state.escaped = true;
      return errorMsk;
    }
    if (byte == FPORT_START_STOP) {
      context->state.step = FPORT_FSM_WAIT_HEADER;
      return fportFeedFsmManageReceivedMsg(context);
    }

    context->msgBuf[context->state.writeIdx++] = byte;
    if (context->state.writeIdx > sizeof(context->msgBuf)) {
      context->state.step = FPORT_FSM_WAIT_HEADER;
      return FPORT_MALFORMED_FRAME;
    } else {
      return errorMsk;
    }
  }
    


  default: return FPORT_INTERNAL_ERROR;
  }
}

static FportErrorMask fportFeedFsmManageReceivedMsg(FportFsmContext *context)
{
   if (context->lastLen == FPORT_CONTROL_LEN) {
     return fportFeedFsmManageReceivedControlMsg(context);
   } else {
     return fportFeedFsmManageReceivedDownlinkMsg(context);
   }
}


static FportErrorMask fportFeedFsmManageReceivedControlMsg(FportFsmContext *context)
{
  if (is_checksum8_ok(context->msgBuf, FPORT_CONTROL_LEN + 2)) {
    decodeFportControlMsg(context->msgBuf + 1, &context->lastFrame);
    if (context->config->ctrlReceiveCb != NULL)
      context->config->ctrlReceiveCb(context);
    if (context->lastFrame.flags & (FPORT_FLAG_FRAME_LOST | FPORT_FLAG_FAILSAFE)) {
      if (context->lastFrame.flags & FPORT_FLAG_FRAME_LOST)
	errorMsk |= FPORT_RADIO_LINK_LOST;
      if (context->lastFrame.flags & FPORT_FLAG_FAILSAFE)
	errorMsk |= FPORT_FAILSAFE;
    } else {
      errorMsk = FPORT_OK;
    }
  } else {
    errorMsk |= FPORT_CRC_ERROR;
  }
  return errorMsk;
}

static FportErrorMask fportFeedFsmManageReceivedDownlinkMsg(FportFsmContext *context)
{
  Fport_DownlinkFrame fportDL;
  Fport_UplinkFrame ufrm __attribute__((unused));

  if (is_checksum8_ok(context->msgBuf, FPORT_DOWNLINK_LEN + 2)) {
    decodeFportDownlinkMsg(context->msgBuf + 1, &fportDL);
    if (fportDL.prime == PRIM_DATA) {
      // the receiver ask for a telemetry frame
      const fportAppIdPair pair = fportTelemetryUpGetNextPair(context);
      if (pair.key == FPORT_DATAID_UNDEF)
      {
	// not data to send : send alive frame
	if (context->config->tlmSendCb != nullptr)
	  context->config->tlmSendCb(context->msgBuf, context->aliveLen, context->config->optArg);
      } else {
	ufrm = (Fport_UplinkFrame) {
	  .type = TYPE_UPLINK,
	  .prime = PRIM_DATA,
	  .appId = pair.key,
	  .v = pair.value
	};
	if (context->config->tlmSendCb != nullptr) {
	  const size_t upMsgLen = makeFportUplinkMsgWithStuffing(&ufrm, context->msgBuf,
								 sizeof(context->msgBuf));
	  context->config->tlmSendCb(context->msgBuf, upMsgLen, context->config->optArg);
	}
      }
    } else {
      // the receiver does congestion control and cannot aford to
      // manage a data frame : send alive frame
      if (context->config->tlmSendCb != nullptr)
	context->config->tlmSendCb(context->msgBuf,  context->aliveLen, context->config->optArg);
    }
  } else {
    return FPORT_CRC_ERROR;
  }
  
  return errorMsk;
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
static void decodeFportControlMsg (const uint8_t *src,  Fport_ControlFrame *frm)
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


  void fportTelemetryUpData(FportFsmContext *context, const fportAppId appId, 
			    const FportTelemetryValue v)
  {
    const size_t idx = fportTelemetryGetAppIdIdx(context, appId, MULTIPLEX_1DATA);

    context->appIds[idx].value = v; // atomic write, no need for mutex
  }



  void fportTelemetryUpDataMultiplexed(FportFsmContext *context, const fportAppId appId, 
				       const FportTelemetryValue *v,
				       const FportMultiplexData nbMplx)
  {
    if (nbMplx > MULTIPLEX_4DATAS)
      return;
    const size_t idx = fportTelemetryGetAppIdIdx(context, appId, nbMplx);

    for (size_t i = 0; i < nbMplx; i++) {
      context->appIds[idx+i].value = v[i]; // atomic write, no need for mutex
    }
    
  }


static void decodeFportDownlinkMsg (const uint8_t *src,  Fport_DownlinkFrame *frm)
{
  memcpy(frm, src, sizeof(Fport_DownlinkFrame));
}

static size_t makeFportUplinkMsgWithStuffing(Fport_UplinkFrame *frm,
					     uint8_t * const dest, size_t destSize)
{
  frm->len = sizeof(*frm) - 2U;
  frm->crc = calc_checksum8(&frm->len, sizeof(*frm) - 1U);

  const uint8_t *from = (uint8_t *) frm;
  size_t destLen = 0;
  for (size_t c = 0; c < sizeof(*frm); c++) {
    const uint8_t b = from[c];
    if ((c == FPORT_START_STOP) || (c == FPORT_ESCAPE)) {
      destLen++;
      dest[destLen++] = FPORT_ESCAPE;
      dest[destLen++] = b ^ FPORT_ESCAPE_DIFF;
    } else {
      dest[destLen++] = b;
    }
    if (destLen == destSize)
      break;
  }
  return destLen;
}

// warning : not reentrant
static size_t fportTelemetryGetAppIdIdx(FportFsmContext *context, const fportAppId appId,
					 const FportMultiplexData nbMplx)
{
  // if already allocated, find it
  for (size_t i=0; i < context->appIdsSize; i++) {
    if (context->appIds[i].key == appId) {
      return i;
    }
  }

  // else allocate
  if (context->appIdsSize < ((FPORT_MAX_DATAIDS+1U) - nbMplx)) {
    for (size_t i=0; i < nbMplx; i++) {
      context->appIds[context->appIdsSize++].key = appId;
    }
    return context->appIdsSize - 1U;
  }

  /* caller need to test if return value is equal to FPORT_MAX_DATAIDS to detect
     table too small error */
  return FPORT_MAX_DATAIDS;
}


static fportAppIdPair fportTelemetryUpGetNextPair(FportFsmContext *context)
{
  // not found
  if (context->appIdsSize == 0)
    return (fportAppIdPair) {.key = FPORT_DATAID_UNDEF,
      .value = (FportTelemetryValue) {.u32 = 0}};
  
  const fportAppIdPair pair = context->appIds[context->nextIdToSent];
  context->nextIdToSent = (context->nextIdToSent + 1) % context->appIdsSize;
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

uint32_t fportTranscodeLat(double lat)
{
  uint32_t tmpui = fabs(lat*1e7);  // now we have unsigned value and one bit to spare
  tmpui = (tmpui + tmpui / 2U) / 25U; // MSB=0 to code latitude
  if (lat < 0.0)
    tmpui |= 0x40000000;
  return tmpui;
}

uint32_t fportTranscodeLong(double lon)
{
  uint32_t  tmpui = fabs(lon*1e7);  // now we have unsigned value and one bit to spare
  tmpui = ((tmpui + tmpui / 2U) / 25U) | 0x80000000; // MSB=1 to code longitude  
  if (lon < 0.0)
    tmpui |= 0x40000000;
  return tmpui;
}


#endif
