#include "ch.h"
#include "hal.h"
#include "globalVar.h"
#include "stdutil.h"
#include "cryptoSerialMessage.h"
#include "rtcAccess.h"
#include "mbedtls/aes.h"
#include "aes_key.h"
#include <string.h>


/*

 * anti replay algo 
   ° 4 bytes : window of possible replay attack : 1/100eme second, max mission duration 497 Days
   ° 3 bytes : window of possible replay attack : 1/10eme second, max mission duration 19 Days
   ° 2 bytes : window of possible replay attack : 1/10eme second, max mission duration less than two hours
   ° should probably not be used for downlink message
 * 

*/

#define MAX_CLEAR_LEN 255


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

typedef enum  {WAIT_FOR_SYNC, WAIT_FOR_LEN, WAIT_FOR_PAYLOAD, WAIT_FOR_CHECKSUM} SerialCmdState ;


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
    time_t hos;
    uint8_t len;
    uint8_t payload[MAX_CLEAR_LEN]; // crc is add at end of payload
    // CRC is here after payload
  };
} EncapsulatedFrame;
#define ENCAP_OVH ((sizeof(EncapsulatedFrame)-MAX_CLEAR_LEN)+2/*CRC*/)



static uint16_t fletcher16WithLen (uint8_t const *data, size_t bytes);
static MUTEX_DECL(sendMtx);
static bool doCypher = false;

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
#define MAX_CLOCK_DRIFT (120 * 1000 / TIME_TICK_MS) // 120 seconds exprimed in fractionseconds
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
  EncapsulatedFrame *ec = (EncapsulatedFrame *) outBuffer;
  if ((payloadLen+ENCAP_OVH) > outBufferSize)
    return false;

  static int lastClock=0;
  int ts = getTimeFractionSeconds();

  // if burst messages are sent in the same hundredth of second,
  //  we need to cheat to defeat anti replay algo
  // this won't work is data is continuelsy flooded, but is ok to absorb burst of data
  if (lastClock == 0) {
    lastClock = ts;
  } else if (lastClock >= ts) {
    ts = lastClock+1;
  }
  
  ec->hos = lastClock = ts;
  ec->len = payloadLen;
  memcpy (ec->payload, inBuffer, payloadLen);
  // put the crc after payload
  *((uint16_t *) (&ec->payload[payloadLen])) = fletcher16WithLen (ec->payload, payloadLen);

  return true;
}

static size_t simpleMsgBufferDecapsulate (const uint8_t *inBuffer, uint8_t **payload)
{
  EncapsulatedFrame *ec = (EncapsulatedFrame *) inBuffer;
  static time_t lastClock=0; // to detect attack based on replay
  static time_t diffClock=0; // to detect attack based on flowding until crc match
  const  time_t localTime = getTimeFractionSeconds();
  
  uint16_t crc =  fletcher16WithLen (ec->payload, ec->len);
  // test against embedded crc which is after payload
  if (crc !=   *((uint16_t *) (&ec->payload[ec->len]))) {
    DebugTrace ("decypher CRC error");
    goto fail;
  }
  
  // we trust the first message
  if (lastClock == 0) {
    lastClock = ec->hos;
    diffClock = localTime - ec->hos;
    
    // received clock has to grow forever (2^32 hundreds of seconds = 497 days max)
    // if we want to flight more, we have to manage clock wrapping
  } else if (ec->hos <= lastClock) {
    DebugTrace ("Replay Detected : diff=%d", lastClock - ec->hos);
    goto fail;
  } else {
    const time_t  estimatedRemoteTime = (localTime - diffClock);
    const time_t  diff = ec->hos-estimatedRemoteTime;
    //    DebugTrace ("diff = %d", diff);
    if (ABS(diff) > MAX_CLOCK_DRIFT) {
      DebugTrace ("flood Replay Detected");
      goto fail;
    }
    lastClock = ec->hos;
    diffClock = localTime - ec->hos;
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
  
  return (getTimeUnixMillisec() - (EPOCHTS*1000ULL)) / TIME_TICK_MS;
}


static void  initRtcWithCompileTime(void)
{
  // if the RTC date is less than date of compilation, obviously RTC is not accurate
  // so let set to date of compilation.
  // Since we send (RTC - EPOCHTS) as anti replay mecanism, RTC-EPOCHTS should alway be positive
  // if afterward RTC is set to accurate (GPS) time, it will go further forward and that will
  // not affect anti replay algorithm
  if (getTimeUnixSec() < EPOCHTS) {
    setTimeUnixSec (EPOCHTS);
  }
}


