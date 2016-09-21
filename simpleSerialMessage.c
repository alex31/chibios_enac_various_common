#include "ch.h"
#include "hal.h"
#include "globalVar.h"
#include "stdutil.h"
#include "simpleSerialMessage.h"
#include "rtcAccess.h"
#include "mbedtls/aes.h"
#include <string.h>


#if (CH_KERNEL_MAJOR > 3)
#define chSequentialStreamWrite  streamWrite
#define chSequentialStreamRead   streamRead
#define chSequentialStreamGet    streamGet
#endif

#define MAX_CLEAR_LEN 255
#define MAX_CYPHER_LEN 240
#define NO_REPLAY_MAX_DRIFT 60

static size_t padWithZeroes (uint8_t *input, const size_t inputSize,
			     const size_t payloadLen, const size_t blockSize);
static bool simpleMsgBufferEncapsulate (uint8_t *outBuffer, const uint8_t *inBuffer,
					const size_t outBufferSize, const size_t payloadLen);

static size_t simpleMsgBufferCypher (uint8_t *outBuffer, const uint8_t *inBuffer,
				   const size_t outBufferSize, const size_t payloadLen);

static size_t simpleMsgBufferDecapsulate (const uint8_t *inBuffer, uint8_t **payload);

static bool simpleMsgBufferDecypher (uint8_t *outBuffer, const uint8_t *inBuffer,
				     const size_t outBufferSize, const size_t msgLen);
static uint32_t getTimeHundredthOfSeconds(void);

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
static  mbedtls_aes_context aesEnc, aesDec;
static  uint8_t key[16] = {0};
static  uint8_t ivSource[16] = {0};
static bool doCypher = false;

#if (CH_KERNEL_MAJOR == 2)
static msg_t readAndProcessChannel(void *arg);
#define chMtxUnlock(x) chMtxUnlock()
#else
static void readAndProcessChannel(void *arg);
#endif


bool simpleMsgCypherInit (const uint8_t *_key, const size_t keyLen, const uint8_t *iv, const size_t ivLen)
{
  if ((keyLen != sizeof(key)) || (ivLen != sizeof(ivSource)))
    return false;

  initRtcWithCompileTime();
  
  memcpy (key, _key, keyLen);
  memcpy (ivSource, iv, ivLen);
  mbedtls_aes_init (&aesEnc);
  mbedtls_aes_init (&aesDec);
  mbedtls_aes_setkey_enc(&aesEnc, key, sizeof(key) * 8);
  mbedtls_aes_setkey_dec(&aesDec, key, sizeof(key) * 8);

  doCypher = true;
  return true;  
}


static bool simpleMsgBufferEncapsulate (uint8_t *outBuffer, const uint8_t *inBuffer,
				 const size_t outBufferSize, const size_t payloadLen)
{
  EncapsulatedFrame *ec = (EncapsulatedFrame *) outBuffer;
  
  if ((payloadLen+ENCAP_OVH) > outBufferSize) {
    return false;
  }
  ec->hos = getTimeHundredthOfSeconds();
  ec->len = payloadLen;
  memcpy (ec->payload, inBuffer, payloadLen);
  // put the crc after payload
  *((uint16_t *) (&ec->payload[payloadLen])) = fletcher16WithLen (ec->payload, payloadLen);
  
  return true;
}

static size_t simpleMsgBufferCypher (uint8_t *outBuffer, const uint8_t *inBuffer,
			    const size_t outBufferSize, const size_t payloadLen)
{
  uint8_t iv[sizeof(ivSource)];

  const size_t paddedLen = padWithZeroes (outBuffer, outBufferSize, payloadLen, 16);
  if (paddedLen == 0) {
    DebugTrace ("cypher ERROR : payloadLen[%d] paddedLen[%d] > outBufferSize[%d]",
		payloadLen, paddedLen, outBufferSize);
    return 0;
  }
  memcpy (iv, ivSource, sizeof(iv));
  mbedtls_aes_crypt_cbc (&aesEnc, MBEDTLS_AES_ENCRYPT, paddedLen, iv, inBuffer, outBuffer);
  return paddedLen;
}

size_t simpleMsgBufferEncapsulateAndCypher (uint8_t *outBuffer, const uint8_t *inBuffer,
					  const size_t outBufferSize, const size_t payloadLen)
{
 uint8_t encapMsg[payloadLen+ENCAP_OVH];
  if (!simpleMsgBufferEncapsulate (encapMsg, inBuffer, payloadLen+ENCAP_OVH, payloadLen))
    return 0;
  return simpleMsgBufferCypher (outBuffer, encapMsg, outBufferSize, payloadLen+ENCAP_OVH);
}
  
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
  if (chSequentialStreamWrite (channel, (uint8_t *) &msgHeader, 
			       sizeof(MsgHeader)) != sizeof(MsgHeader))
    goto exitFail;
  if (chSequentialStreamWrite (channel, buffer, len) != len)
    goto exitFail;
  if (chSequentialStreamWrite (channel, (uint8_t *) &crc, 
			       sizeof(crc)) != sizeof(crc))
    goto exitFail;
  
  chMtxUnlock(&sendMtx);
  return true;
  
 exitFail:
  chMtxUnlock(&sendMtx);
  return false;
}

Thread * simpleMsgBind (BaseSequentialStream *channel, const MsgCallBack callback, 
			void * const userData)
{
  // will be freed when readAndProcessChannel thread will exit
  MsgBindParams *mbp = malloc_m (sizeof (mbp)); 
  mbp->channel = channel;
  mbp->callback = callback;
  mbp->userData = userData;

#if (CH_KERNEL_MAJOR <= 3)
  Thread *tp = chThdCreateFromHeap(NULL,  THD_WA_SIZE(1024), NORMALPRIO, 
  				   readAndProcessChannel, mbp);
#else
  Thread *tp = chThdCreateFromHeap(NULL,  THD_WA_SIZE(1024), "readAndProcessChannel", NORMALPRIO, 
  				   readAndProcessChannel, mbp);
#endif


  /* Thread *tp = chThdCreateStatic(waMsgBind, sizeof(waMsgBind), NORMALPRIO, */
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

static size_t simpleMsgBufferDecapsulate (const uint8_t *inBuffer, uint8_t **payload)
{
  EncapsulatedFrame *ec = (EncapsulatedFrame *) inBuffer;
  static time_t lastClock=0;
  
  uint16_t crc =  fletcher16WithLen (ec->payload, ec->len);
  // test against embedded crc which is after payload
  if (crc !=   *((uint16_t *) (&ec->payload[ec->len]))) {
    DebugTrace ("decypher CRC error");
    goto fail;
  }
  
  // we trust the first message
  if (lastClock == 0) {
    lastClock = ec->hos;
    // received clock has to grow forever (2^32 hundreds of seconds = 497 days max)
    // if we want to flight more, we have to manage clock wrapping
  } else if (ec->hos <= lastClock) {
    DebugTrace ("Replay Detected");
    goto fail;
  } else {
    lastClock = ec->hos;
  }

  *payload = ec->payload;
  return ec->len;
  
 fail:
  *payload = NULL;
  return 0;
}

static bool simpleMsgBufferDecypher (uint8_t *outBuffer, const uint8_t *inBuffer,
				       const size_t outBufferSize, const size_t msgLen)
{
  uint8_t iv[sizeof(ivSource)];

  if (msgLen > outBufferSize)
    return false;
  
  memcpy (iv, ivSource, sizeof(iv));
  mbedtls_aes_crypt_cbc (&aesDec, MBEDTLS_AES_DECRYPT, msgLen, iv, inBuffer, outBuffer);
  return true;
}

size_t simpleMsgBufferDecypherAndDecapsulate (uint8_t *outBuffer, const uint8_t *inBuffer,
					      const size_t outBufferSize, const size_t msgLen, uint8_t **payload)
{
  if (!simpleMsgBufferDecypher (outBuffer, inBuffer, outBufferSize, msgLen)) {
    *payload = NULL;
    return 0;
  }

  return simpleMsgBufferDecapsulate (outBuffer, payload);
}
  
 
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



#if (CH_KERNEL_MAJOR == 2)
static msg_t readAndProcessChannel(void *arg)
#define chMtxUnlock(x) chMtxUnlock()
#else
static void readAndProcessChannel(void *arg)
#endif
{
  MsgBindParams *mbp = (MsgBindParams *) arg;

  chRegSetThreadName("readAndProcessChannel");
  CmdMesgState messState = {.payload = {0},
			    .len = 0,
			    .crc = 0,
			    .state = WAIT_FOR_SYNC};


  while (!chThdShouldTerminate()) {
    switch (messState.state) {
      
    case WAIT_FOR_SYNC :
      //DebugTrace ("WAIT_FOR_SYNC");
      messState.payload[0] = (uint8_t) chSequentialStreamGet (mbp->channel);
      /* if ( (*((uint16_t *) &messState.payload[0]) & 0xffff) == 0xFEED) { */
      /* 	messState.state = WAIT_FOR_LEN; */
      /* }  */
      if (messState.payload[0]  == 0x99) {
	messState.state = WAIT_FOR_LEN;
      } 
      break;
      
    case WAIT_FOR_LEN :
      //DebugTrace ("WAIT_FOR_LEN");
      messState.len = (uint8_t) chSequentialStreamGet (mbp->channel);
      // DebugTrace ("LEN = %d", messState.len);
      messState.state = WAIT_FOR_PAYLOAD;
      break;

    case WAIT_FOR_PAYLOAD :
      //DebugTrace ("WAIT_FOR_PAYLOAD");
      chSequentialStreamRead (mbp->channel, messState.payload,  messState.len);
      messState.state = WAIT_FOR_CHECKSUM;
      break;

    case WAIT_FOR_CHECKSUM :
      //DebugTrace ("WAIT_FOR_CHECKSUM");
      chSequentialStreamRead (mbp->channel, (uint8_t *) &messState.crc,  sizeof(messState.crc));
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
#if (CH_KERNEL_MAJOR == 2)
  return RDY_OK;
#endif
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

static uint32_t getTimeHundredthOfSeconds()
{
  /* DebugTrace ("getTimeUnixMilliSec() - (EPOCHTS*1000) = %d", */
  /* 	      (uint32_t) (getTimeUnixMillisec() - ((uint64_t)(EPOCHTS)*1000ULL) / 10ULL)); */
  
   return ((getTimeUnixMillisec() - (EPOCHTS*1000ULL)) / 10ULL);
}

static void  initRtcWithCompileTime(void)
{
  // if the RTC date is less than date of compilation, obviously RTC is not accurate
  // so let set it to date of compilation which is less false RTC
  // since we sent RTC-EPOCHTS as anti replay mecanism, RTC-EPOCHTS should alway be positive
  // if afterward RTC is set to accurate (GPS) time, it will go further forward and that will
  // not affect anti replay algorithm
  if (getTimeUnixSec() < EPOCHTS) {
    setTimeUnixSec (EPOCHTS);
  }
}
