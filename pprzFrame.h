#pragma once

#include "ch.h"
#include "hal.h"


/*
  protocole de com :
  1 octets entête   : 0x99
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
  bool simpleMsgSend(UARTDriver * const channel, const uint8_t *buffer,
		      const size_t len);
  
  // launch a thread which read and segment message then call callback when a message is complete
  thread_t * simpleMsgBind(UARTDriver *channel,
			  const MsgCallBack callback, const ChkErrCallBack errCallback,
			  void * const userData);
#ifdef __cplusplus
}
#endif
