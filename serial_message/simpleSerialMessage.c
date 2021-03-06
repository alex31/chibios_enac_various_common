#include "ch.h"
#include "hal.h"
#include "stdutil.h"
#include "simpleSerialMessage.h"

/* #if (CH_KERNEL_MAJOR > 3) */
/* #define chSequentialStreamWrite  streamWrite */
/* #define chSequentialStreamRead   streamRead */
/* #define chSequentialStreamGet    streamGet */
/* #endif */

#if SIMPLE_MESSAGE_API_UART
static inline msg_t uartGetWrapper(UARTDriver *uartp);
static inline size_t uartReadWrapper(UARTDriver *uartp,	uint8_t *bp, const size_t n);
static inline size_t uartWriteWrapper(UARTDriver *uartp,	const uint8_t *bp, const size_t n);
#endif

#if SIMPLE_MESSAGE_PPRZ_STX
#define SYNC_LEN 1
#define SYNC_INIT {0x99}
#define MSG_HEADER_SIZE 2U
#else
#define SYNC_LEN 2
#define SYNC_INIT {0xED, 0xFE}
#define MSG_HEADER_SIZE 3U
#endif

typedef enum  {WAIT_FOR_SYNC, WAIT_FOR_LEN, WAIT_FOR_PAYLOAD, WAIT_FOR_CHECKSUM} SerialCmdState ;


typedef struct {
  uint8_t sync[SYNC_LEN];
  uint8_t len;
} __attribute__ ((__packed__))  MsgHeader;

_Static_assert(sizeof(MsgHeader) == MSG_HEADER_SIZE, "MsgHeader struct is not packed");


typedef struct {
  SSM_STREAM_TYPE *channel;
  MsgCallBack callback;
  ChkErrCallBack errCallback;
  void *userData;
} MsgBindParams;

typedef struct {
  //  std::array<uint8_t, 255> payload __attribute__((aligned(4)));
  uint8_t payload[255] __attribute__((aligned(4)));
  uint8_t len;
  uint16_t crc;
  SerialCmdState state;
} CmdMesgState;



static uint16_t fletcher16WithLen (uint8_t const *data, size_t bytes);
static MUTEX_DECL(sendMtx);


#if (CH_KERNEL_MAJOR == 2)
static msg_t readAndProcessChannel(void *arg);
#define chMtxUnlock(x) chMtxUnlock()
#else
static void readAndProcessChannel(void *arg);
#endif

bool simpleMsgSend (SSM_STREAM_TYPE * const channel, const uint8_t *buffer,
		      const size_t len)
{
  if (len > 255) 
    return false;
  
  const MsgHeader msgHeader = {.sync = SYNC_INIT,
			       .len = len & 0xff};
  uint16_t crc =  fletcher16WithLen (buffer, len);
  //  DebugTrace ("len = %u, crc =0x%x", len, crc);
  chMtxLock (&sendMtx);
  if (SSM_STREAM_WRITE(channel, (uint8_t *) &msgHeader, 
			       sizeof(MsgHeader)) != sizeof(MsgHeader))
    goto exitFail;
  if (SSM_STREAM_WRITE(channel, buffer, len) != len)
    goto exitFail;
  if (SSM_STREAM_WRITE(channel, (uint8_t *) &crc, 
			       sizeof(crc)) != sizeof(crc))
    goto exitFail;
  
  chMtxUnlock(&sendMtx);
  return true;
  
 exitFail:
  chMtxUnlock(&sendMtx);
  return false;
}

thread_t * simpleMsgBind (SSM_STREAM_TYPE *channel,
			const MsgCallBack callback, const ChkErrCallBack errCallback,
			void * const userData)
{
  // will be freed when readAndProcessChannel thread will exit
  MsgBindParams *mbp = malloc_m(sizeof (MsgBindParams)); 
  mbp->channel = channel;
  mbp->callback = callback;
  mbp->errCallback = errCallback;
  mbp->userData = userData;

#if (CH_KERNEL_MAJOR <= 3)
  Thread *tp = chThdCreateFromHeap(NULL, THD_WA_SIZE(1024), NORMALPRIO, 
  				   &readAndProcessChannel, mbp);
#else
  thread_t *tp = chThdCreateFromHeap(NULL, THD_WORKING_AREA_SIZE(1024), "readAndProcessChannel", NORMALPRIO, 
				   &readAndProcessChannel, mbp);
#endif


  /* thread_t *tp = chThdCreateStatic(waMsgBind, sizeof(waMsgBind), NORMALPRIO, */
  /* 				   readAndProcessChannel, &mbp); */
  if (tp == NULL) {
    //    chprintf(chp, "simpleMsgBind : out of memory\r\n");
    return NULL;
  }
  
  return tp;
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


  while (!chThdShouldTerminateX()) {
    switch (messState.state) {
      
    case WAIT_FOR_SYNC :
      // DebugTrace ("WAIT_FOR_SYNC");
#if (SIMPLE_MESSAGE_PPRZ_STX == 0)
      messState.payload[0] = messState.payload[1];
      messState.payload[1] = (uint8_t)  SSM_STREAM_GET (mbp->channel);
#else
      messState.payload[0] = (uint8_t)  SSM_STREAM_GET (mbp->channel);
#endif
      /* if ( (*((uint16_t *) &messState.payload[0]) & 0xffff) == 0xFEED) { */
      /* 	messState.state = WAIT_FOR_LEN; */
      /* }  */
#if (SIMPLE_MESSAGE_PPRZ_STX == 0)
      if ((messState.payload[0]  == 0xED) &&  (messState.payload[1]  == 0xFE)) 
#else
      if (messState.payload[0]  == 0x99) 
#endif
	{
	  messState.state = WAIT_FOR_LEN;
	} 
      break;
      
    case WAIT_FOR_LEN :
      // DebugTrace ("WAIT_FOR_LEN");
      messState.len = (uint8_t)  SSM_STREAM_GET (mbp->channel);
      // DebugTrace ("LEN = %d", messState.len);
      messState.state = WAIT_FOR_PAYLOAD;
      break;

    case WAIT_FOR_PAYLOAD :
      // DebugTrace ("WAIT_FOR_PAYLOAD");
       SSM_STREAM_READ(mbp->channel, messState.payload,  messState.len);
      messState.state = WAIT_FOR_CHECKSUM;
      break;

    case WAIT_FOR_CHECKSUM :
      // DebugTrace ("WAIT_FOR_CHECKSUM");
       SSM_STREAM_READ(mbp->channel, (uint8_t *) &messState.crc,  sizeof(messState.crc));
      const uint16_t calculatedCrc = fletcher16WithLen (messState.payload, messState.len);
      if (calculatedCrc == messState.crc) {
	mbp->callback (messState.payload, messState.len, mbp->userData);
      } else {
	// DebugTrace ("CRC ERROR : calculated 0x%x != in message 0x%x", calculatedCrc, 
	//	    messState.crc);
	if (mbp->errCallback)
	  mbp->errCallback(messState.crc, calculatedCrc);
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


#if SIMPLE_MESSAGE_API_UART
static inline msg_t uartGetWrapper(UARTDriver *uartp)
{
  uint8_t ret;
  size_t count=1;
  uartReceiveTimeout(uartp, &count, &ret, TIME_INFINITE);
  return ret;
}
static inline size_t uartReadWrapper(UARTDriver *uartp,	uint8_t *bp, const size_t n)
{
  size_t count=n;
  uartReceiveTimeout(uartp, &count, bp, TIME_INFINITE);
  return count;
}

static inline size_t uartWriteWrapper(UARTDriver *uartp, const uint8_t *bp, const size_t n)
{
  size_t count=n;
  uartSendTimeout(uartp, &count, bp, TIME_INFINITE);
  return count;
}

#endif
