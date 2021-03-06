#include "msg_queue.h"
#include "stdutil.h"
#include "tlsf_malloc.h"
#include "revportage.h"
#include <string.h>

#define MSGQ_HEAP HEAP_CCM


typedef union {
  struct {
    uint16_t ptrOfst;
    uint16_t len;
  };
  msg_t msg_ptr_len;
} MsgPtrLen;


#if CH_KERNEL_MAJOR  >= 5
#define chMBPost chMBPostTimeout
#define chMBPostAhead chMBPostAheadTimeout
#define chMBFetch  chMBFetchTimeout
#endif


void		msgqueue_init   (MsgQueue* que, tlsf_memory_heap_t *heap,
				 msg_t *mb_buf, const cnt_t mb_size)
{
  chMBObjectInit (&que->mb, mb_buf, mb_size);
  memset (mb_buf, 0, mb_size*sizeof(msg_t));
  que->heap = heap;
}

bool		msgqueue_is_full   (MsgQueue* que)
{
  chSysLock();
  const bool queue_full = chMBGetFreeCountI(&que->mb) <= 0;
  chSysUnlock();

  return queue_full;	
}

bool 		msgqueue_is_empty  (MsgQueue* que)
{
  chSysLock();
  bool queue_empty = (chMBGetUsedCountI(&que->mb) <= 0);	
  chSysUnlock();
  
  return queue_empty;
}
  

int32_t		msgqueue_send (MsgQueue* que, void *msg, const uint16_t msgLen,
			       const MsgQueueUrgency urgency)
{
  return msgqueue_send_timeout (que, msg, msgLen, urgency, TIME_IMMEDIATE);
}


int32_t		msgqueue_send_timeout (MsgQueue* que, void *msg, const uint16_t msgLen,
				       const MsgQueueUrgency urgency, const systime_t timout)
{
  const MsgPtrLen mpl = {{.ptrOfst = (uint32_t) msg - (uint32_t) tlsf_get_heap_addr(&MSGQ_HEAP),
			  .len = msgLen}};

#if CH_DBG_ENABLE_CHECKS
  if (((uint32_t) msg < (uint32_t) tlsf_get_heap_addr(&MSGQ_HEAP)) ||
      ((uint32_t) msg >= ((uint32_t) tlsf_get_heap_addr(&MSGQ_HEAP) + 65535))) {
#if CH_DBG_ENABLE_ASSERTS
    chSysHalt("MsgQueue_INVALID_PTR");
#else
    return MsgQueue_INVALID_PTR;
#endif
  }
#endif
  
  if (urgency == MsgQueue_REGULAR) {
    if (chMBPost (&que->mb, mpl.msg_ptr_len, timout) != MSG_OK)
      goto fail;
  } else {
    if (chMBPostAhead (&que->mb, mpl.msg_ptr_len, timout) != MSG_OK)
      goto fail;
  }
  return msgLen;
  
 fail:
 
  tlsf_free_r (que->heap, msg);
  
  return  MsgQueue_MAILBOX_FULL;
}

int32_t		msgqueue_copy_send_timeout (MsgQueue* que, const void *msg, const uint16_t msgLen,
					    const MsgQueueUrgency urgency, const systime_t timout)
{
  void *dst = tlsf_malloc_r(&MSGQ_HEAP, msgLen);

  if (dst == NULL) {
    return MsgQueue_MAILBOX_FULL;
  }

  memcpy (dst, msg, msgLen);
  return msgqueue_send_timeout (que, dst, msgLen, urgency, timout);
}

int32_t		msgqueue_copy_send (MsgQueue* que, const void *msg, const uint16_t msgLen,
					    const MsgQueueUrgency urgency)
{
  return msgqueue_copy_send_timeout (que, msg, msgLen, urgency, TIME_IMMEDIATE);
}


int32_t	msgqueue_pop (MsgQueue* que, void **msgPtr)
{
  return msgqueue_pop_timeout (que, msgPtr, TIME_INFINITE);
}

int32_t	msgqueue_pop_timeout (MsgQueue* que, void **msgPtr, const systime_t timout)
{
  MsgPtrLen mpl = {.ptrOfst = 0, .len = 0};
  
  msg_t status;
  do  {
    status = chMBFetch (&que->mb,  (msg_t *) &mpl.msg_ptr_len, timout);
  } while (status == MSG_RESET);
  
  if (status != MSG_OK)
    return MsgQueue_MAILBOX_TIMEOUT;
  
  *msgPtr = (void *) (mpl.ptrOfst + (uint32_t) tlsf_get_heap_addr(&MSGQ_HEAP));
  return mpl.len;
}




static const char* _strerror[] = {"MsgQueue_MAILBOX_FULL", 
				 "MsgQueue_MAILBOX_FAIL", 	  
				 "MsgQueue_MAILBOX_TIMEOUT",	  
				 "MsgQueue_MAILBOX_NOT_EMPTY",   
				 "MsgQueue_OK",
				 "MsgQueue_INVALID_PTR", 
				  "MsgQueue_INVALID_ERRNO"};

const char*     msgqueue_strerror (const MsgQueueStatus _errno)
{
  size_t indice = _errno - MsgQueue_MAILBOX_FULL;
  indice = MIN(indice, ARRAY_LEN(_strerror)-1);
  return _strerror[indice];
}




