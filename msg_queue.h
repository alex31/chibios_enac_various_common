#pragma once

#include "ch.h"
#include "hal.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef enum {MsgQueue_MAILBOX_FULL=-100, 
	      MsgQueue_MAILBOX_FAIL, 
	      MsgQueue_MAILBOX_TIMEOUT,
	      MsgQueue_MAILBOX_NOT_EMPTY, 
	      MsgQueue_INVALID_PTR, 
	      MsgQueue_OK=0} MsgQueueStatus;
  
typedef enum  {MsgQueue_REGULAR, MsgQueue_OUT_OF_BAND} MsgQueueUrgency;



typedef struct  MsgQueue MsgQueue;


/*
  goal : dynamically initialise memory. should be used to workaround a bug if queue is located to ccmram
         not mandatory is queue is not in ccmram
  parameters : queue object
  return value: no
 */
  void		msgqueue_init   (MsgQueue* que,
				 void *mp_base, const size_t mp_size,
				 msg_t *mb_buf, const cnt_t mb_size);

/*
  goal : test if queue is full

  parameters : queue object
  return value: TRUE if queue if full, FALSE otherwise
 */
bool		msgqueue_is_full   (MsgQueue* que);

/*
  goal : test if queue is empty

  parameters : queue object
  return value:  TRUE if queue if empty, FALSE otherwise
 */
bool 		msgqueue_is_empty  (MsgQueue* que);

/*
  goal :  return used size in memory pool,
         does take into account used mailbox slots

  parameters : queue object
  return value: size in byte of used memory
 */
size_t 	msgqueue_get_used_size (MsgQueue* que);

/*
  goal : return free size in memory pool,
         does take into account used mailbox slots

  parameters : queue object
  return value: size in byte of free memory
 */
size_t 	msgqueue_get_free_size (MsgQueue* que);


/*
  goal : (zero copy api)
         reserve a chunk of memory on the memory pool to be filled eventually

  parameters : queue object, reserved buffer length
  return value: if NULL => error
                if non NULL : pointer to memory
 */
void *	 msgqueue_malloc_before_send (MsgQueue* que, const uint16_t msgLen);

/*
  goal : (zero copy api)
         send a chunk of memory prevoiously reserved and filled as a message

  parameters : queue object, pointer to reserved memory
               urgency  regular=queued at end of fifo or out_of_band queued at start of fifo
  return value: if > 0 : requested length
                if < 0 : error status (see errors at begining of this header file)
 */
int32_t	 msgqueue_send (MsgQueue* que, void *msg, const uint16_t msgLen,
			const MsgQueueUrgency urgency);


int32_t	 msgqueue_send_timeout (MsgQueue* que, void *msg, const uint16_t msgLen,
				       const MsgQueueUrgency urgency, const systime_t timout);
/*
  goal : (zero copy api)
  get a message to be processed without copy (blocking)

  parameters : INOUT queue object
	       OUT msg pointer
  return value: if > 0 : length of received msg
                if < 0 : error status (see errors at begining of this header file)
 */
int32_t	msgqueue_pop (MsgQueue* que, void **msgPtr);

/*
  goal : (zero copy api)
  get a message to be processed without copy (blocking)

  parameters : INOUT queue object
	       OUT pointer to msg
	       IN  timeout (or TIME_IMMEDIATE or TIME_INFINITE)
  return value: if > 0 : length od received msg
                if < 0 : error status (see errors at begining of this header file)

 */
int32_t	msgqueue_pop_timeout (MsgQueue* que, void **msgPtr, const systime_t timout);


/*
  goal : (zero copy api)
  free memory of a precedently given message by PopChunk

  parameters : INOUT queue object
               IN    ChunkBufferRO pointer
 */
void	msgqueue_free_after_pop (MsgQueue* que, void *msg);



/*
  goal : helper function to fix queue : verify that a empty queue is in coherent state

  parameters : queue object
  return value: TRUE if OK, FALSE if ERROR
 */
MsgQueueStatus	msgqueue_test_integrity_if_empty(MsgQueue* que);

/*
  goal : give literal message from a status error code

  parameters : status error code
  return value: pointer to const message
 */
const char*     msgqueue_strerror (const MsgQueueStatus errno);

/*
#                 _____    _____    _____  __      __   ___    _______   ______         
#                |  __ \  |  __ \  |_   _| \ \    / /  / _ \  |__   __| |  ____|        
#                | |__) | | |__) |   | |    \ \  / /  | |_| |    | |    | |__           
#                |  ___/  |  _  /    | |     \ \/ /   |  _  |    | |    |  __|          
#                | |      | | \ \   _| |_     \  /    | | | |    | |    | |____         
#                |_|      |_|  \_\ |_____|     \/     |_| |_|    |_|    |______|        
*/

struct MsgQueue {
  mailbox_t	mb;
  void *	mp_base;
  size_t	mp_size;
  size_t	mp_initialy_used;
} ;


#ifdef __cplusplus
}
#endif
