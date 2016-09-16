#include "ch.h"
#include "hal.h"
#include "globalVar.h"
#include "stdutil.h"
#include "simpleSerialMessage.h"
#include <string.h>

#if (CH_KERNEL_MAJOR > 3)
#define chSequentialStreamWrite  streamWrite
#define chSequentialStreamRead   streamRead
#define chSequentialStreamGet    streamGet
#endif

#define MAX_CLEAR_LEN 255
#define MAX_CYPHER_LEN 240


static void simpleMsgBufferEncapsulate (uint8_t *outBuffer, const uint8_t *buffer,
				   const size_t len);
static size_t simpleMsgBufferDecapsulate (const uint8_t *outBuffer, const uint8_t **buffer);


typedef enum  {WAIT_FOR_SYNC, WAIT_FOR_LEN, WAIT_FOR_PAYLOAD, WAIT_FOR_CHECKSUM} SerialCmdState ;


typedef struct {
  uint8_t sync[2];
  uint8_t len;
} __attribute__ ((__packed__))  MsgHeader;

_Static_assert(sizeof(MsgHeader) == 3, "MsgHeader struct is not packed");


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
    MsgHeader header;
    uint8_t payload[MAX_CLEAR_LEN] __attribute__((aligned(4)));
    uint16_t crc;
  };
} EncapsulatedFrame;



static uint16_t fletcher16WithLen (uint8_t const *data, size_t bytes);
static MUTEX_DECL(sendMtx);


#if (CH_KERNEL_MAJOR == 2)
static msg_t readAndProcessChannel(void *arg);
#define chMtxUnlock(x) chMtxUnlock()
#else
static void readAndProcessChannel(void *arg);
#endif

static void simpleMsgBufferEncapsulate (uint8_t *outBuffer, const uint8_t *buffer,
				   const size_t len)
{
  if (len > 250) 
    return ;
  EncapsulatedFrame *ec = (EncapsulatedFrame *) outBuffer;
  ec->header.sync[0] = 0xED;
  ec->header.sync[1] = 0xFE;
  ec->header.len = len;
  ec->crc = fletcher16WithLen (buffer, len);

  memcpy (ec->payload, buffer, len);
}

  
bool simpleMsgSend (BaseSequentialStream * const channel, const uint8_t *buffer,
		    const size_t len)
{
  if (len > MAX_CLEAR_LEN) 
    return false;
  
  const MsgHeader msgHeader = {.sync = {0xED, 0xFE},
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

static size_t simpleMsgBufferDecapsulate (const uint8_t *outBuffer, const uint8_t **buffer)
{
  EncapsulatedFrame *ec = (EncapsulatedFrame *) outBuffer;
  if ((ec->header.sync[0] != 0xED) || (ec->header.sync[1] != 0xFE))
    goto fail;
  uint16_t crc =  fletcher16WithLen (ec->payload, ec->header.len);
  if (crc != ec->crc)
    goto fail;
  *buffer = ec->payload;
  return ec->header.len;

  
 fail:
  *buffer = NULL;
  return 0;
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
      messState.payload[0] = messState.payload[1];
      messState.payload[1] = (uint8_t) chSequentialStreamGet (mbp->channel);
      /* if ( (*((uint16_t *) &messState.payload[0]) & 0xffff) == 0xFEED) { */
      /* 	messState.state = WAIT_FOR_LEN; */
      /* }  */
      if ((messState.payload[0]  == 0xED) &&  (messState.payload[1]  == 0xFE)) {
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
	mbp->callback (messState.payload, messState.len, mbp->userData);
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
