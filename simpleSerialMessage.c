#include "ch.h"
#include "hal.h"
#include "globalVar.h"
#include "stdutil.h"
#include "simpleSerialMessage.h"
#include <string.h>
#include "mbedtls/aes.h"

#if (CH_KERNEL_MAJOR > 3)
#define chSequentialStreamWrite  streamWrite
#define chSequentialStreamRead   streamRead
#define chSequentialStreamGet    streamGet
#endif

#define MAX_CLEAR_LEN 255
#define MAX_CYPHER_LEN 240

static size_t padWithZeroes (uint8_t *input, const size_t inputSize,
			     const size_t payloadLen, const size_t blockSize);
static bool simpleMsgBufferEncapsulate (uint8_t *outBuffer, const uint8_t *inBuffer,
					const size_t outBufferSize, const size_t payloadLen);

static size_t simpleMsgBufferCypher (uint8_t *outBuffer, const uint8_t *inBuffer,
				   const size_t outBufferSize, const size_t payloadLen);

static size_t simpleMsgBufferDecapsulate (const uint8_t *inBuffer, uint8_t **payload);

static bool simpleMsgBufferDecypher (uint8_t *outBuffer, const uint8_t *inBuffer,
				     const size_t outBufferSize, const size_t msgLen);

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
    uint8_t len;
    uint16_t crc;
    uint8_t payload[MAX_CLEAR_LEN];
  };
} EncapsulatedFrame;
#define ENCAP_OVH (sizeof(EncapsulatedFrame)-MAX_CLEAR_LEN)



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

  if ((payloadLen+ENCAP_OVH) > outBufferSize)
    return false;
  
  ec->len = payloadLen;
  memcpy (ec->payload, inBuffer, payloadLen);
  ec->crc = fletcher16WithLen (ec->payload, payloadLen);

  return true;
}

static size_t simpleMsgBufferCypher (uint8_t *outBuffer, const uint8_t *inBuffer,
			    const size_t outBufferSize, const size_t payloadLen)
{
  uint8_t iv[sizeof(ivSource)];

  const size_t paddedLen = padWithZeroes (outBuffer, outBufferSize, payloadLen, 16);
  if (paddedLen > outBufferSize) 
    return 0;

  memcpy (iv, ivSource, sizeof(iv));
  mbedtls_aes_crypt_cbc (&aesEnc, MBEDTLS_AES_ENCRYPT, paddedLen, iv, inBuffer, outBuffer);
  //  memcpy (ec->payload, inBuffer, payloadLen);
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
    
    if (len == 0)
      return false;
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


static size_t simpleMsgBufferDecapsulate (const uint8_t *inBuffer, uint8_t **payload)
{
  EncapsulatedFrame *ec = (EncapsulatedFrame *) inBuffer;

  uint16_t crc =  fletcher16WithLen (ec->payload, ec->len);
  if (crc != ec->crc)
    goto fail;

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
      // DebugTrace ("WAIT_FOR_SYNC");
      messState.payload[0] = (uint8_t) chSequentialStreamGet (mbp->channel);
      /* if ( (*((uint16_t *) &messState.payload[0]) & 0xffff) == 0xFEED) { */
      /* 	messState.state = WAIT_FOR_LEN; */
      /* }  */
      if (messState.payload[0]  == 0x99) {
	messState.state = WAIT_FOR_LEN;
      } 
      break;
      
    case WAIT_FOR_LEN :
      // DebugTrace ("WAIT_FOR_LEN");
      messState.len = (uint8_t) chSequentialStreamGet (mbp->channel);
      // DebugTrace ("LEN = %d", messState.len);
      messState.state = WAIT_FOR_PAYLOAD;
      break;

    case WAIT_FOR_PAYLOAD :
      // DebugTrace ("WAIT_FOR_PAYLOAD");
      chSequentialStreamRead (mbp->channel, messState.payload,  messState.len);
      messState.state = WAIT_FOR_CHECKSUM;
      break;

    case WAIT_FOR_CHECKSUM :
      // DebugTrace ("WAIT_FOR_CHECKSUM");
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
	// DebugTrace ("CRC ERROR : calculated 0x%x != in message 0x%x", calculatedCrc, 
	//	    messState.crc);
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
    return 0;
  }
}

