#include "ch.h"
#include "hal.h"
#include "stdutil.h"
#include "pprzFrame.h"


static inline msg_t uartGetWrapper(UARTDriver *uartp);
static inline size_t uartReadWrapper(UARTDriver *uartp,	uint8_t *bp, const size_t n);
static inline size_t uartWriteWrapper(UARTDriver *uartp,	const uint8_t *bp, const size_t n);


typedef enum  {WAIT_FOR_SYNC, WAIT_FOR_LEN, WAIT_FOR_PAYLOAD, WAIT_FOR_CHECKSUM} SerialCmdState ;


typedef struct {
  uint8_t sync;
  uint8_t len;
} __attribute__ ((__packed__))  MsgHeader;

_Static_assert(sizeof(MsgHeader) == 2, "MsgHeader struct is not packed");


typedef struct {
  UARTDriver *channel;
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


static void readAndProcessChannel(void *arg);

bool simpleMsgSend (UARTDriver * const channel, const uint8_t *buffer,
		      const size_t len)
{
  if (len > 255) 
    return false;
  
  const MsgHeader msgHeader = {.sync = 0x99,
			       .len = len & 0xff};
  uint16_t crc =  fletcher16WithLen (buffer, len);
  //  DebugTrace ("len = %u, crc =0x%x", len, crc);
  uartAcquireBus(channel);
  if (uartWriteWrapper(channel, (uint8_t *) &msgHeader, 
		       sizeof(MsgHeader)) != sizeof(MsgHeader))
    goto exitFail;
  if (uartWriteWrapper(channel, buffer, len) != len)
    goto exitFail;
  if (uartWriteWrapper(channel, (uint8_t *) &crc, 
		       sizeof(crc)) != sizeof(crc))
    goto exitFail;
  
  uartReleaseBus(channel);
  return true;
  
 exitFail:
  uartReleaseBus(channel);
  return false;
}

thread_t * simpleMsgBind (UARTDriver *channel,
			const MsgCallBack callback, const ChkErrCallBack errCallback,
			void * const userData)
{
  // will be freed when readAndProcessChannel thread will exit
  MsgBindParams *mbp = malloc_m (sizeof (mbp)); 
  mbp->channel = channel;
  mbp->callback = callback;
  mbp->errCallback = errCallback;
  mbp->userData = userData;

  thread_t *tp = chThdCreateFromHeap(NULL,  1024,
				     "readAndProcessChannel", NORMALPRIO, 
				     readAndProcessChannel, mbp);

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
      // DebugTrace ("WAIT_FOR_SYNC");
      messState.payload[0] = (uint8_t) uartGetWrapper (mbp->channel);

      if (messState.payload[0]  == 0x99) {
	messState.state = WAIT_FOR_LEN;
      } 
      break;
      
    case WAIT_FOR_LEN :
      // DebugTrace ("WAIT_FOR_LEN");
      messState.len = (uint8_t) uartGetWrapper (mbp->channel);
      // DebugTrace ("LEN = %d", messState.len);
      messState.state = WAIT_FOR_PAYLOAD;
      break;

    case WAIT_FOR_PAYLOAD :
      // DebugTrace ("WAIT_FOR_PAYLOAD");
      uartReadWrapper(mbp->channel, messState.payload,  messState.len);
      messState.state = WAIT_FOR_CHECKSUM;
      break;

    case WAIT_FOR_CHECKSUM :
      // DebugTrace ("WAIT_FOR_CHECKSUM");
      uartReadWrapper(mbp->channel, (uint8_t *) &messState.crc,  sizeof(messState.crc));
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
  chThdExit(MSG_OK);
}


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


