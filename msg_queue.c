#include "msg_queue.h"
#include "stdutil.h"
#include "tlsf_malloc.h"
#include <string.h>


typedef union {
  struct {
    uint16_t ptrOfst;
    uint16_t len;
  };
  msg_t msg_ptr_len;
} MsgPtrLen;





/*
  goal : dynamically initialise memory. should be used to workaround a bug if queue is located to ccmram
         not mandatory is queue is not in ccmram
  parameters : queue object
  return value: no
 */
void		msgqueue_init   (MsgQueue* que,
				 void *mp_base, const size_t mp_size,
				 msg_t *mb_buf, const cnt_t mb_size)
{
#if CH_DBG_ENABLE_ASSERTS
  if (mp_size > 65535) {
    chSysHalt("memory pool size > 65535");
  }
#endif
  chMBObjectInit (&que->mb, mb_buf, mb_size);
  que->mp_base = mp_base;
  que->mp_size = mp_size;
  memset (mp_base, 0, mp_size);
  memset (mb_buf, 0, mb_size*sizeof(msg_t *));
  init_memory_pool(mp_size, mp_base);
  que->mp_initialy_used = get_used_size (que->mp_base);
}

/*
  goal : test if queue is full

  parameters : queue object
  return value: TRUE if queue if full, FALSE otherwise
 */
bool		msgqueue_is_full   (MsgQueue* que)
{
  chSysLock();
  const bool queue_full = chMBGetFreeCountI(&que->mb) <= 0;
  chSysUnlock();

  return queue_full || (que->mp_size - get_used_size (que->mp_base)) < 32;	
}

/*
  goal : test if queue is empty

  parameters : queue object
  return value:  TRUE if queue if empty, FALSE otherwise
 */
bool 		msgqueue_is_empty  (MsgQueue* que)
{
  chSysLock();
  bool queue_empty = (chMBGetUsedCountI(&que->mb) <= 0);	
  chSysUnlock();
  
  return queue_empty;
}
  

/*
  goal :  return used size in memory pool,
         does take into account used mailbox slots

  parameters : queue object
  return value: size in byte of used memory
 */
size_t 	msgqueue_get_used_size (MsgQueue* que)
{
  return get_used_size (que->mp_base) - que->mp_initialy_used;
}

/*
  goal : return free size in memory pool,
         does take into account used mailbox slots

  parameters : queue object
  return value: size in byte of free memory
 */
size_t 	msgqueue_get_free_size (MsgQueue* que)
{
  return que->mp_size - get_used_size (que->mp_base);	
}


/*
  goal : (zero copy api)
         reserve a chunk of memory on the memory pool to be filled eventually

  parameters : queue object, reserved buffer length
  return value: if NULL => error
                if non NULL : pointer to memory
 */
void *		msgqueue_malloc_before_send (MsgQueue* que, const uint16_t msgLen)
{
  return malloc_ex (msgLen, que->mp_base);
}

/*
  goal : (zero copy api)
         send a chunk of memory prevoiously reserved and filled as a message

  parameters : queue object, pointer to reserved memory
               urgency  regular=queued at end of fifo or out_of_band queued at start of fifo
  return value: if > 0 : requested length
                if < 0 : error status (see errors at begining of this header file)
 */
int32_t		msgqueue_send (MsgQueue* que, void *msg, const uint16_t msgLen,
			       const MsgQueueUrgency urgency)
{
  return msgqueue_send_timeout (que, msg, msgLen, urgency, TIME_IMMEDIATE);
}


/*
  goal : (zero copy api)
         send a chunk of memory prevoiously reserved and filled as a message

  parameters : queue object, pointer to reserved memory
               urgency  regular=queued at end of fifo or out_of_band queued at start of fifo
  return value: if > 0 : requested length
                if < 0 : error status (see errors at begining of this header file)
 */
int32_t		msgqueue_send_timeout (MsgQueue* que, void *msg, const uint16_t msgLen,
				       const MsgQueueUrgency urgency, const systime_t timout)
{
  const MsgPtrLen mpl = {.ptrOfst =  (uint32_t) msg - (uint32_t) que->mp_base,
			 .len = msgLen};

#if CH_DBG_ENABLE_CHECKS
  if (((uint32_t) msg < (uint32_t) que->mp_base) ||
      ((uint32_t) msg >= ((uint32_t) que->mp_base + que->mp_size))) {
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
 
  free_ex (msg, que->mp_base);
  
  return  MsgQueue_MAILBOX_FULL;
}

/*
  goal : 
         send a chunk of memory copying buffer

  parameters : queue object, pointer to reserved memory
               urgency  regular=queued at end of fifo or out_of_band queued at start of fifo
  return value: if > 0 : requested length
                if < 0 : error status (see errors at begining of this header file)
 */
int32_t		msgqueue_copy_send_timeout (MsgQueue* que, const void *msg, const uint16_t msgLen,
					    const MsgQueueUrgency urgency, const systime_t timout)
{
  void *dst = msgqueue_malloc_before_send (que, msgLen);

  if (dst == NULL) {
    return MsgQueue_MAILBOX_FULL;
  }

  memcpy (dst, msg, msgLen);
  return msgqueue_send_timeout (que, dst, msgLen, urgency, timout);
}

/*
  goal : 
         send a chunk of memory copying buffer

  parameters : queue object, pointer to reserved memory
               urgency  regular=queued at end of fifo or out_of_band queued at start of fifo
  return value: if > 0 : requested length
                if < 0 : error status (see errors at begining of this header file)
 */
int32_t		msgqueue_copy_send (MsgQueue* que, const void *msg, const uint16_t msgLen,
					    const MsgQueueUrgency urgency)
{
  return msgqueue_copy_send_timeout (que, msg, msgLen, urgency, TIME_IMMEDIATE);
}


/*
  goal : (zero copy api)
  get a message to be processed without copy (blocking)

  parameters : INOUT queue object
	       OUT msg pointer
  return value: if > 0 : length of received msg
                if < 0 : error status (see errors at begining of this header file)
 */
int32_t	msgqueue_pop (MsgQueue* que, void **msgPtr)
{
  return msgqueue_pop_timeout (que, msgPtr, TIME_INFINITE);
}

/*
  goal : (zero copy api)
  get a message to be processed without copy (blocking)

  parameters : INOUT queue object
	       OUT pointer to msg
	       IN  timeout (or TIME_IMMEDIATE or TIME_INFINITE)
  return value: if > 0 : length od received msg
                if < 0 : error status (see errors at begining of this header file)

 */
int32_t	msgqueue_pop_timeout (MsgQueue* que, void **msgPtr, const systime_t timout)
{
  MsgPtrLen mpl = {.ptrOfst = 0, .len = 0};
  
  const msg_t status = chMBFetch (&que->mb,  (msg_t *) &mpl.msg_ptr_len, timout);
  if (status != MSG_OK)
    return MsgQueue_MAILBOX_TIMEOUT;

  *msgPtr = (void *) (mpl.ptrOfst + (uint32_t) que->mp_base);
  return mpl.len;
}


/*
  goal : (zero copy api)
  free memory of a precedently given message by PopChunk

  parameters : INOUT queue object
               IN    ChunkBufferRO pointer
 */
void		msgqueue_free_after_pop (MsgQueue* que, void *msg)
{
#if CH_DBG_ENABLE_CHECKS
  if (((uint32_t) msg < (uint32_t) que->mp_base) ||
      ((uint32_t) msg >= ((uint32_t) que->mp_base + que->mp_size))) {
#if CH_DBG_ENABLE_ASSERTS
    chSysHalt("MsgQueue_INVALID_PTR");
#else
    return ;
#endif
  }
#endif
 
  free_ex (msg, que->mp_base);
}



/*
  goal : helper function to fix queue : verify that a empty queue is in coherent state

  parameters : queue object
  return value: TRUE if OK, FALSE if ERROR
 */
MsgQueueStatus msgqueue_test_integrity_if_empty(MsgQueue* que)
{
  if (msgqueue_is_empty (que)) {
    if (msgqueue_get_used_size (que) != 0) {
      return MsgQueue_MAILBOX_FAIL;
    } else {
      return MsgQueue_OK;
    }
  } else {
    return MsgQueue_MAILBOX_NOT_EMPTY;
  }
}

/*
  goal : give literal message from a status error code

  parameters : status error code
  return value: pointer to const message
 */

static const char* _strerror[] = {"MsgQueue_MAILBOX_FULL", 
				 "MsgQueue_MAILBOX_FAIL", 	  
				 "MsgQueue_MAILBOX_TIMEOUT",	  
				 "MsgQueue_MAILBOX_NOT_EMPTY",   
				 "MsgQueue_OK",
				 "MsgQueue_INVALID_ERRNO",
};

const char*     msgqueue_strerror (const MsgQueueStatus _errno)
{
  size_t indice = _errno - MsgQueue_MAILBOX_FULL;
  indice = MIN(indice, ARRAY_LEN(_strerror)-1);
  return _strerror[indice];
}




