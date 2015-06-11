#include "ch.h"
#include "hal.h"
#include "globalVar.h"
#include "stdutil.h"
#include "simpleSerialMessage.h"

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
  //  std::array<uint8_t, 255> payload __attribute__((aligned(4)));
  uint8_t payload[255] __attribute__((aligned(4)));
  uint8_t len;
  uint16_t crc;
  SerialCmdState state;
} CmdMesgState;

static msg_t readAndProcessChannel(void *arg);
static uint16_t fletcher16WithLen (uint8_t const *data, size_t bytes);
static MUTEX_DECL(sendMtx);



bool_t simpleMsgSend (BaseSequentialStream * const channel, const uint8_t *buffer,
		      const size_t len)
{
  if (len > 255) 
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
  
  chMtxUnlock();
  return true;
  
 exitFail:
  chMtxUnlock();
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

  Thread *tp = chThdCreateFromHeap(NULL,  THD_WA_SIZE(1024), NORMALPRIO,
  				   readAndProcessChannel, mbp);

  /* Thread *tp = chThdCreateStatic(waMsgBind, sizeof(waMsgBind), NORMALPRIO, */
  /* 				   readAndProcessChannel, &mbp); */
  if (tp == NULL) {
    chprintf(chp, "simpleMsgBind : out of memory\r\n");
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
  return (uint16_t) (sum2 << 8) | sum1;
}




static msg_t readAndProcessChannel(void *arg)
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
      if ( (*((uint16_t *) &messState.payload[0]) & 0xffff) == 0xFEED) {
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
  return RDY_OK;
}
