#include "ch.h"
#include "hal.h"

#include "stdutil.h"
#include "cryptoSerialMessage.h"
#include "rtcAccess.h"
#include "mbedtls/aes.h"
#include "aes_key.h"
#include <string.h>

//#define ANTI_REPLAY_ON_DOWNLINK 1
/*

 * anti replay algo 
   ° 4 bytes : window of possible replay attack : 1/100eme second, max mission duration 497 Days
   ° 3 bytes : window of possible replay attack : 1/10eme second, max mission duration 19 Days
   ° 2 bytes : window of possible replay attack : 1/10eme second, max mission duration less than two hours
   ° should probably not be used for downlink message
 * 

*/

#define MAX_CLEAR_LEN 255
typedef enum {WAIT_FOR_SYNC, WAIT_FOR_LEN, WAIT_FOR_PAYLOAD, WAIT_FOR_CHECKSUM} SerialCmdState ;
typedef enum {CLK_OK, NOT_INIT, REPLAY, NOT_SYNC} RemoteClockState; 

typedef struct {
  uint32_t init;
  uint32_t last;
  uint32_t diff;
} RemoteClock;


static size_t padWithZeroes (uint8_t *input, const size_t inputSize,
			     const size_t payloadLen, const size_t blockSize);
static bool simpleMsgBufferEncapsulate (uint8_t *outBuffer, const uint8_t *inBuffer,
					const size_t outBufferSize, const size_t payloadLen);

static size_t simpleMsgBufferCypher (uint8_t *outBuffer, const uint8_t *inBuffer,
				   const size_t outBufferSize, const size_t payloadLen);

static size_t simpleMsgBufferDecapsulate (const uint8_t *inBuffer, uint8_t **payload);

static bool simpleMsgBufferDecypher (uint8_t *outBuffer, const uint8_t *inBuffer,
				     const size_t outBufferSize, const size_t msgLen);
static uint32_t getTimeFractionSeconds(void);

static void  initRtcWithCompileTime(void);

static void             remoteClockReset (RemoteClock *rc);
static RemoteClockState remoteClockGetState (const RemoteClock *rc);
static RemoteClockState remoteClockIsReplay (const RemoteClock *rc, const uint32_t remoteTime);
static void	        remoteClockInit (RemoteClock *rc, const uint32_t localTime,
					const uint32_t remoteTime);
static RemoteClockState remoteClockSet (RemoteClock *rc, const uint32_t localTime,
					const uint32_t remoteTime);
static void rtcHasBeenResetCB (int delta);


typedef struct {
  uint8_t sync;
  uint8_t len;
} __attribute__ ((__packed__))  MsgHeader;

_Static_assert(sizeof(MsgHeader) == 2, "MsgHeader struct is not packed");


typedef struct {
  BaseSequentialStream *channel;
  MsgCallBack callback;
  void *userData;
} MsgBindParams;

typedef struct {
  uint8_t payload[MAX_CLEAR_LEN] __attribute__((aligned(4)));
  uint8_t len;
  uint16_t crc;
  SerialCmdState state;
} CmdMesgState;

typedef struct {
  struct {
    uint32_t remoteClk;
    uint8_t len;
    uint8_t payload[MAX_CLEAR_LEN]; // crc is add at end of payload
    // CRC is here after payload
  };
} EncapsulatedUpFrame;

typedef struct {
  struct {
#ifdef ANTI_REPLAY_ON_DOWNLINK
    uint32_t remoteClk;
#endif
    uint8_t len;
    uint8_t payload[MAX_CLEAR_LEN]; // crc is add at end of payload
    // CRC is here after payload
  };
} EncapsulatedDownFrame;
#define ENCAP_OVH ((sizeof(EncapsulatedDownFrame)-MAX_CLEAR_LEN)+2/*CRC*/)



static uint16_t fletcher16WithLen (uint8_t const *data, size_t bytes);
static MUTEX_DECL(sendMtx);
static bool doCypher = false;

#ifdef ANTI_REPLAY_ON_DOWNLINK
#define RTC_RST_ENCAP_ACK 0b01
#define RTC_RST_DECAP_ACK  0b10
#define RTC_RST_RESET    0b00
#define RTC_RST_SHOULD_RESET  0b11
static uint32_t rtcHasBeenReset=0;
#else
static bool rtcHasBeenReset=false;
#endif

static void readAndProcessChannel(void *arg);



bool simpleMsgSend (BaseSequentialStream * const channel, uint8_t *inBuffer,
		    const size_t slen)
{
  if (slen > MAX_CLEAR_LEN) 
    return false;

  size_t len = slen;
  uint8_t cypherBuffer[doCypher ? len+ENCAP_OVH+16 : 0];
  const uint8_t *buffer = doCypher ? cypherBuffer : inBuffer;

  if (doCypher) {
    len = simpleMsgBufferEncapsulateAndCypher (cypherBuffer, inBuffer, slen+ENCAP_OVH+16, slen);
    
    if (len == 0) {
      return false;
    }
  }
  
  const MsgHeader msgHeader = {.sync = 0x99,
			       .len = len & 0xff};
  uint16_t crc =  fletcher16WithLen (buffer, len);
  
  chMtxLock (&sendMtx);
  if (streamWrite (channel, (uint8_t *) &msgHeader, 
			       sizeof(MsgHeader)) != sizeof(MsgHeader))
    goto exitFail;
  if (streamWrite (channel, buffer, len) != len)
    goto exitFail;
  if (streamWrite (channel, (uint8_t *) &crc, 
			       sizeof(crc)) != sizeof(crc))
    goto exitFail;
  
  chMtxUnlock(&sendMtx);
  return true;
  
 exitFail:
  chMtxUnlock(&sendMtx);
  return false;
}

thread_t * simpleMsgBind (BaseSequentialStream *channel, const MsgCallBack callback, 
			void * const userData)
{
  // will be freed when readAndProcessChannel thread will exit
  MsgBindParams *mbp = malloc_m (sizeof (mbp)); 
  mbp->channel = channel;
  mbp->callback = callback;
  mbp->userData = userData;

  thread_t *tp = chThdCreateFromHeap(NULL,  THD_WORKING_AREA_SIZE(1024),
				     "readAndProcessChannel", NORMALPRIO, 
  				   readAndProcessChannel, mbp);


  /* thread_t *tp = chThdCreateStatic(waMsgBind, sizeof(waMsgBind), NORMALPRIO, */
  /* 				   readAndProcessChannel, &mbp); */
  if (tp == NULL) {
    chprintf(chp, "simpleMsgBind : out of memory\r\n");
    return NULL;
  }
  
  return tp;
}

/* static inline uint16_t udiff (const uint16_t a, const uint16_t b) { */
/*   return (b>a) ? b-a : (65535-a)+b; */
/* } */

  
 
static uint16_t fletcher16WithLen (uint8_t const *data, size_t bytes)
{
  uint16_t sum1 = 0xff, sum2 = 0xff;
  
  sum1 = (uint16_t) (sum1 + bytes);
  sum2 = (uint16_t) (sum2 + sum1);
  sum1 = (uint16_t) ((sum1 & 0xff) + (sum1 >> 8));
  sum2 = (uint16_t) ((sum2 & 0xff) + (sum2 >> 8));

  while (bytes) {
    size_t tlen = bytes > 20 ? 20 : bytes;
    bytes -= tlen;
    do {
      sum1 = (uint16_t) (sum1 + *data++);
      sum2 =  (uint16_t) (sum2 + sum1);
    } while (--tlen);
    sum1 = (uint16_t) ((sum1 & 0xff) + (sum1 >> 8));
    sum2 = (uint16_t) ((sum2 & 0xff) + (sum2 >> 8));
  }
  /* Second reduction step to reduce sums to 8 bits */
  sum1 = (uint16_t) ((sum1 & 0xff) + (sum1 >> 8));
  sum2 = (uint16_t) ((sum2 & 0xff) + (sum2 >> 8));
   return (uint16_t) ((sum2 % 0xff) << 8) | (sum1 % 0xff);
}



static void readAndProcessChannel(void *arg)
{
  MsgBindParams *mbp = (MsgBindParams *) arg;

  chRegSetThreadName("readAndProcessChannel");
  CmdMesgState messState = {.payload = {0},
			    .len = 0,
			    .crc = 0,
			    .state = WAIT_FOR_SYNC};


  while (!chThdShouldTerminateX()) {
    switch (messState.state) {
      
    case WAIT_FOR_SYNC :
      //DebugTrace ("WAIT_FOR_SYNC");
      messState.payload[0] = (uint8_t) streamGet (mbp->channel);
      /* if ( (*((uint16_t *) &messState.payload[0]) & 0xffff) == 0xFEED) { */
      /* 	messState.state = WAIT_FOR_LEN; */
      /* }  */
      if (messState.payload[0]  == 0x99) {
	messState.state = WAIT_FOR_LEN;
      } 
      break;
      
    case WAIT_FOR_LEN :
      //DebugTrace ("WAIT_FOR_LEN");
      messState.len = (uint8_t) streamGet (mbp->channel);
      // DebugTrace ("LEN = %d", messState.len);
      messState.state = WAIT_FOR_PAYLOAD;
      break;

    case WAIT_FOR_PAYLOAD :
      //DebugTrace ("WAIT_FOR_PAYLOAD");
      streamRead (mbp->channel, messState.payload,  messState.len);
      messState.state = WAIT_FOR_CHECKSUM;
      break;

    case WAIT_FOR_CHECKSUM :
      //DebugTrace ("WAIT_FOR_CHECKSUM");
      streamRead (mbp->channel, (uint8_t *) &messState.crc,  sizeof(messState.crc));
      const uint16_t calculatedCrc = fletcher16WithLen (messState.payload, messState.len);
      if (calculatedCrc == messState.crc) {
	if (doCypher) {
	  uint8_t clearTextBuf[messState.len];
	  uint8_t *clearText;
	  const size_t len = simpleMsgBufferDecypherAndDecapsulate (clearTextBuf, messState.payload,
								    messState.len, messState.len,
								    &clearText);
	  if (len) {
	    mbp->callback (clearText, len, mbp->userData);
	  }
	} else {
	  mbp->callback (messState.payload, messState.len, mbp->userData);
	}
      } else {
	 DebugTrace ("CRC ERROR : calculated 0x%x != in message 0x%x", calculatedCrc, 
		     messState.crc);
      }
      messState.state = WAIT_FOR_SYNC;
      break;
    }

  }
  
  free_m (mbp); // was allocated by simpleMsgBind
}



/*
#                               _   _    _ __    _                    
#                              | | | |  | '_ \  | |                   
#                  ___   _ __  | |_| |  | |_) | | |_     ___          
#                 / __| | '__|  \__, |  | .__/  | __|   / _ \         
#                | (__  | |      __/ |  | |     \ |_   | (_) |        
#                 \___| |_|     |___/   |_|      \__|   \___/         
*/

#define MAX_CYPHER_LEN (MAX_CLEAR_LEN-16-ENCAP_OVH)
#define TIME_TICK_MS 10
#define MAX_CLOCK_DRIFT (120 * (1000 / TIME_TICK_MS)) // 120 seconds exprimed in fractionseconds
static  mbedtls_aes_context aesEnc, aesDec;


bool simpleMsgCypherInit (void)
{

  initRtcWithCompileTime();
  
  mbedtls_aes_init (&aesEnc);
  mbedtls_aes_init (&aesDec);
  mbedtls_aes_setkey_enc(&aesEnc, aes_key, sizeof(aes_key) * 8);
  mbedtls_aes_setkey_dec(&aesDec, aes_key, sizeof(aes_key) * 8);

  doCypher = true;
  return true;  
}


static bool simpleMsgBufferEncapsulate (uint8_t *outBuffer, const uint8_t *inBuffer,
				 const size_t outBufferSize, const size_t payloadLen)
{
  EncapsulatedDownFrame *ec = (EncapsulatedDownFrame *) outBuffer;
  if ((payloadLen+ENCAP_OVH) > outBufferSize)
    return false;

#ifdef ANTI_REPLAY_ON_DOWNLINK
  uint32_t ts = getTimeFractionSeconds();
  static uint32_t lastClock=0;
  // if local rtc has been reset we should reinit local data in use for detecting
  // intrusion. we will send a special timestamp value to remote to let him knows that
  // next value is valid
  if (rtcHasBeenReset &  RTC_RST_ENCAP_ACK) {
    ts=lastClock=0;
    rtcHasBeenReset &= ~RTC_RST_ENCAP_ACK;

  // if burst messages are sent in the same hundredth of second,
  //  we need to cheat to defeat anti replay algo
  // this won't work is data is continuelsy flooded, but is ok to absorb burst of data
  } else  if (lastClock == 0) {
    lastClock = ts;
  } else if (lastClock >= ts) {
    ts = lastClock+1;
  }
  ec->remoteClk = lastClock = ts;
#endif
  

  ec->len = payloadLen;
  memcpy (ec->payload, inBuffer, payloadLen);
  // put the crc after payload
  *((uint16_t *) (&ec->payload[payloadLen])) = fletcher16WithLen (ec->payload, payloadLen);

  return true;
}

static size_t simpleMsgBufferDecapsulate (const uint8_t *inBuffer, uint8_t **payload)
{
  EncapsulatedUpFrame *ec = (EncapsulatedUpFrame *) inBuffer;
  static RemoteClock remClock = {0,0xdeadbeef,0};
  const  uint32_t localTime = getTimeFractionSeconds();
  
  uint16_t crc =  fletcher16WithLen (ec->payload, ec->len);
  // test against embedded crc which is after payload
  if (crc !=   *((uint16_t *) (&ec->payload[ec->len]))) {
    DebugTrace ("decypher CRC error");
    goto fail;
  }

  // local rtc has been reset
#ifdef ANTI_REPLAY_ON_DOWNLINK
  if (rtcHasBeenReset &  RTC_RST_DECAP_ACK) {
    remoteClockReset (&remClock);
    rtcHasBeenReset &= ~RTC_RST_DECAP_ACK;
  }
#else
 if (rtcHasBeenReset) {
   remoteClockReset (&remClock);
   rtcHasBeenReset = false;
 }
#endif
 
  if (ec->remoteClk == 0) {
    // this as a special meaning : remote RTC has been reset, so accept this message
    // and next message will give new time
    remoteClockReset (&remClock);
    // we trust the first message
  } else if (remoteClockGetState (&remClock) == NOT_INIT) {
    remoteClockInit (&remClock, localTime, ec->remoteClk);
    
    // received clock has to grow forever (2^32 hundreds of seconds = 497 days max)
    // if we want to flight more, we have to manage clock wrapping
  } else if (remoteClockIsReplay (&remClock, ec->remoteClk) == REPLAY) {
    DebugTrace ("Replay Detected : diff=%d", remClock.last - ec->remoteClk);
    goto fail;
  } else if (remoteClockSet (&remClock, localTime, ec->remoteClk) == NOT_SYNC) {
    DebugTrace ("flood Replay Detected");
    goto fail;
  }
  
  *payload = ec->payload;
  return ec->len;
  
 fail:
  *payload = NULL;
  return 0;
}

static size_t simpleMsgBufferCypher (uint8_t *outBuffer, const uint8_t *inBuffer,
			    const size_t outBufferSize, const size_t payloadLen)
{
  uint8_t iv[sizeof(aes_iv)];

  const size_t paddedLen = padWithZeroes (outBuffer, outBufferSize, payloadLen, 16);
  if (paddedLen == 0) {
    DebugTrace ("cypher ERROR : payloadLen[%d] paddedLen[%d] > outBufferSize[%d]",
		payloadLen, paddedLen, outBufferSize);
    return 0;
  }
  memcpy (iv, aes_iv, sizeof(iv));
  mbedtls_aes_crypt_cbc (&aesEnc, MBEDTLS_AES_ENCRYPT, paddedLen, iv, inBuffer, outBuffer);
  return paddedLen;
}

static bool simpleMsgBufferDecypher (uint8_t *outBuffer, const uint8_t *inBuffer,
				       const size_t outBufferSize, const size_t msgLen)
{
  uint8_t iv[sizeof(aes_iv)];

  if (msgLen > outBufferSize)
    return false;
  
  memcpy (iv, aes_iv, sizeof(iv));
  mbedtls_aes_crypt_cbc (&aesDec, MBEDTLS_AES_DECRYPT, msgLen, iv, inBuffer, outBuffer);
  return true;
}



size_t simpleMsgBufferEncapsulateAndCypher (uint8_t *outBuffer, const uint8_t *inBuffer,
					  const size_t outBufferSize, const size_t payloadLen)
{
 uint8_t encapMsg[payloadLen+ENCAP_OVH];
  if (!simpleMsgBufferEncapsulate (encapMsg, inBuffer, payloadLen+ENCAP_OVH, payloadLen))
    return 0;
  return simpleMsgBufferCypher (outBuffer, encapMsg, outBufferSize, payloadLen+ENCAP_OVH);
}
  





size_t simpleMsgBufferDecypherAndDecapsulate (uint8_t *outBuffer, const uint8_t *inBuffer,
					      const size_t outBufferSize, const size_t msgLen,
					      uint8_t **payload)
{
  if (!simpleMsgBufferDecypher (outBuffer, inBuffer, outBufferSize, msgLen)) {
    *payload = NULL;
    return 0;
  }

  return simpleMsgBufferDecapsulate (outBuffer, payload);
}



static size_t padWithZeroes (uint8_t *input, const size_t inputSize,
			   const size_t payloadLen, const size_t blockSize)
{
  const size_t padSize = (blockSize -  (payloadLen % blockSize)) % blockSize;
  if ((payloadLen+padSize) <= inputSize) {
    if (padSize) {
      memset (&input[payloadLen], 0, padSize);
    /* DebugTrace ("blockSize[%d] -  (payloadLen[%d] modulo blockSize[%d]) = %d => return %d", */
    /* 		blockSize, payloadLen, blockSize, padSize,  payloadLen + padSize); */
    }
    return payloadLen + padSize;
  } else {
    DebugTrace ("PWZ error payloadLen[%d] + padSize[%d] > inputSize[%d]",
		payloadLen, padSize,  inputSize); 
    return 0;
  }
}

static uint32_t getTimeFractionSeconds()
{
  /* DebugTrace ("getTimeUnixMilliSec() - (EPOCHTS*1000) = %d", */
  /* 	      (uint32_t) (getTimeUnixMillisec() - ((uint64_t)(EPOCHTS)*1000ULL) / 10ULL)); */
  
  return getTimeUnixMillisec() / TIME_TICK_MS;
}

// in pprz, should not be done in crypto module but in early init
static void  initRtcWithCompileTime(void)
{
  // if the RTC date is less than date of compilation, obviously RTC is not accurate
  // so let set to date of compilation.
  // Since we send (RTC - EPOCHTS) as anti replay mecanism, RTC-EPOCHTS should alway be positive
  // if afterward RTC is set to accurate (GPS) time, it will go further forward and that will
  // not affect anti replay algorithm
  registerRtcChanged (&rtcHasBeenResetCB);
  if (getTimeUnixSec() < EPOCHTS) {
    setTimeUnixSec (EPOCHTS);
  }
}


static RemoteClockState remoteClockGetState (const RemoteClock *rc)
{
  return rc->last == 0xdeadbeef ? NOT_INIT : CLK_OK;
}

static void remoteClockReset (RemoteClock *rc)
{
  rc->last = 0xdeadbeef;
}

static RemoteClockState remoteClockIsReplay (const RemoteClock *rc, const uint32_t rawRemoteTime)
{
  if (rawRemoteTime < rc->init)
    return REPLAY;
  
  const uint32_t remoteTime = rawRemoteTime - rc->init;  // shift remote time
  return remoteTime <= rc->last ? REPLAY : CLK_OK;
}

static void remoteClockInit (RemoteClock *rc, const uint32_t localTime,
					const uint32_t rawRemoteTime)
{
  rc->init = rawRemoteTime;
  const uint32_t remoteTime = rawRemoteTime - rc->init; // shift remote time
  rc->last = remoteTime;
  rc->diff = localTime > remoteTime ? localTime - remoteTime : remoteTime - localTime;
  DebugTrace ("Clock init rawRemoteTime=%u rc->init=%u remoteTime=%u rc->diff=%u",
	      rawRemoteTime,  rc->init, remoteTime, rc->diff);
	      
}

static RemoteClockState remoteClockSet (RemoteClock *rc, const uint32_t localTime,
					const uint32_t rawRemoteTime)
{
  const uint32_t remoteTime = rawRemoteTime - rc->init; // shift remote time
  const uint32_t  estimatedRemoteTime = (localTime - rc->diff);
  const uint32_t  diff = remoteTime > estimatedRemoteTime ?
    remoteTime - estimatedRemoteTime : estimatedRemoteTime - remoteTime;
  
  /* DebugTrace ("Clock set rawRemoteTime=%u remoteTime=%u estimated=%u diff=%u" , */
  /* 	      rawRemoteTime, remoteTime, estimatedRemoteTime, diff); */
  
  if (diff > MAX_CLOCK_DRIFT) {
    return NOT_SYNC;
  }
  rc->last = remoteTime;
  rc->diff = localTime > remoteTime ? localTime - remoteTime : remoteTime - localTime;
  /* DebugTrace ("Clock set rawRemoteTime=%u rc->init=%u remoteTime=%u rc->diff=%u diff=%u" , */
  /* 	      rawRemoteTime,  rc->init, remoteTime, rc->diff, diff); */
  
  return CLK_OK;
}

static void rtcHasBeenResetCB (int delta)
{
  const int deltaNorm= delta > 0 ? delta : -delta;
  if (deltaNorm > 60) {
#ifdef ANTI_REPLAY_ON_DOWNLINK
    rtcHasBeenReset = RTC_RST_SHOULD_RESET;
#else
    rtcHasBeenReset = true;
#endif
  }
}
