#pragma once

#include "ch.h"
#include "hal.h"




/*
  protocole de com :
  2 octets entête   : #FEED
  1 octet  longueur de payload
  payload
  chk_A, chk_B 
  where checksum is computed over length and payload:
     
 */

// callback fonction type
typedef void (*MsgCallBack)(const uint8_t *buffer, const size_t len,  void * const userData);

// just a wrapper to send message
bool_t simpleMsgSend (BaseSequentialStream * const channel, const uint8_t *buffer,
		      const size_t len);

// launch a thread which read and segment message then call callback when a message is complete
Thread * simpleMsgBind (BaseSequentialStream *channel, const MsgCallBack callback, 
			 void * const userData);