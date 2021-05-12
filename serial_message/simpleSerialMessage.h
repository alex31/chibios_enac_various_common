#pragma once

#include "ch.h"
#include "hal.h"

#if (!defined SIMPLE_MESSAGE_API_STREAM) && \
    (!defined SIMPLE_MESSAGE_API_SERIAL) && \
    (!defined SIMPLE_MESSAGE_API_UART)
#define SIMPLE_MESSAGE_API_STREAM 1
#endif

#if (!defined SIMPLE_MESSAGE_API_STREAM)
#define SIMPLE_MESSAGE_API_STREAM 0
#endif

#if  (!defined SIMPLE_MESSAGE_API_SERIAL)
#define SIMPLE_MESSAGE_API_SERIAL 0
#endif

#if (!defined SIMPLE_MESSAGE_API_UART)
#define SIMPLE_MESSAGE_API_UART 0
#endif

#if (SIMPLE_MESSAGE_API_STREAM)
#define SIMPLE_MESSAGE_API_STREAM 1
#endif

#if  (SIMPLE_MESSAGE_API_SERIAL)
#define SIMPLE_MESSAGE_API_SERIAL 1
#endif

#if (SIMPLE_MESSAGE_API_UART)
#define SIMPLE_MESSAGE_API_UART 1
#endif




#if (SIMPLE_MESSAGE_API_STREAM + SIMPLE_MESSAGE_API_SERIAL + SIMPLE_MESSAGE_API_UART) != 1
#error "one and only one of theses macros should be defined to TRUE : \n"
       "SIMPLE_MESSAGE_API_STREAM, SIMPLE_MESSAGE_API_SERIAL , SIMPLE_MESSAGE_API_UART"
#endif

#if SIMPLE_MESSAGE_API_STREAM
#define SSM_STREAM_TYPE BaseSequentialStream
#define SSM_STREAM_WRITE(C,B,S) streamWrite(C,B,S)
#define SSM_STREAM_READ(C,B,S) streamRead(C,B,S)
#define SSM_STREAM_GET(C) streamGet(C)
#elif SIMPLE_MESSAGE_API_SERIAL
#define SSM_STREAM_TYPE SerialDriver
#define SSM_STREAM_WRITE(C,B,S) sdWrite(C,B,S)
#define SSM_STREAM_READ(C,B,S) sdRead(C,B,S)
#define SSM_STREAM_GET(C) sdGet(C)
#elif SIMPLE_MESSAGE_API_UART
#define SSM_STREAM_TYPE UARTDriver
#define SSM_STREAM_WRITE(C,B,S) uartWriteWrapper(C,B,S)
#define SSM_STREAM_READ(C,B,S) uartReadWrapper(C,B,S)
#define SSM_STREAM_GET(C) uartGetWrapper(C)
#endif


/*
  protocole de com :
  2 octets entête   : #FEED
  1 octet  longueur de payload
  payload
  chk_A, chk_B 
  where checksum is computed over length and payload:
     
 */

// callback fonction type
#ifdef __cplusplus
extern "C" {
#endif

  typedef void (*MsgCallBack)(const uint8_t *buffer, const size_t len,  void * const userData);
  typedef void (*ChkErrCallBack)(const uint32_t recCrc, const uint32_t calcCrc);

  // just a wrapper to send message
  bool simpleMsgSend (SSM_STREAM_TYPE * const channel, const uint8_t *buffer,
		      const size_t len);
  
  // launch a thread which read and segment message then call callback when a message is complete
  thread_t * simpleMsgBind (SSM_STREAM_TYPE *channel,
			  const MsgCallBack callback, const ChkErrCallBack errCallback,
			  void * const userData);
#ifdef __cplusplus
}
#endif
