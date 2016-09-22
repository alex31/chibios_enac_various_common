#pragma once

#include "ch.h"
#include "hal.h"
#include "portage.h"



/*
  protocole de com :
  INNER FRAME (and only one if encryption is not used)
  2 octets entête   : #FEED
  1 octets  longueur de payload (if encrypted, max len is 255-5 = 250)
  payload
  chk_A, chk_B 
  where checksum is computed over length and payload:
    
  if encrypted :
  OUTER FRAME :
  2 octets entête   : #FEED
  1 octets  longueur de payload
  payload => which is previous INNER FRAME, encrypted
  chk_A, chk_B 
  where checksum is computed over length and payload:
  
 
 */

// callback fonction type
typedef void (*MsgCallBack)(const uint8_t *buffer, const size_t len,  void * const userData);

// just a wrapper to send message
bool simpleMsgSend (BaseSequentialStream * const channel, uint8_t *inBuffer,
		      const size_t len);

// launch a thread which read and segment message then call callback when a message is complete
Thread * simpleMsgBind (BaseSequentialStream *channel, const MsgCallBack callback, 
			 void * const userData);

bool simpleMsgCypherInit (void);

size_t simpleMsgBufferEncapsulateAndCypher (uint8_t *outBuffer, const uint8_t *inBuffer,
					  const size_t outBufferSize, const size_t payloadLen);

size_t simpleMsgBufferDecypherAndDecapsulate (uint8_t *outBuffer, const uint8_t *inBuffer,
					      const size_t outBufferSize, const size_t msgLen,
					      uint8_t **payload);


