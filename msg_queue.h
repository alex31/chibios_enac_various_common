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


/**
 * @brief	initialise MsgQueue
 * @details	init sdQueue objet, tie memory pool buffer, tie mailbox buffer 
 *              initialize mailbox
 * @param[out] 	que:		opaque object to be initialized
 * @param[in]	mp_base:	address of memory pool used by tlsf allocator
 * @param[in]	mp_size:	size of previous memory pool in bytes
 * @param[in]	mb_buff:	internal buffer used by MailBox (see Chibios Doc)
 * @param[in]	mb_size:	size of previous buffer (length of MailBox queue)
 */
void		msgqueue_init   (MsgQueue* que,
				 void *mp_base, const size_t mp_size,
				 msg_t *mb_buf, const cnt_t mb_size);

/**
 * @brief	test if queue is full
 * @param[in] 	que:		pointer to opaque MsgQueue object 
 * @return	true if full, false otherwise
 */
bool		msgqueue_is_full   (MsgQueue* que);

/**
 * @brief	test if queue is empty
 * @param[in] 	que:	pointer to opaque MsgQueue object 
 * @return	true if empty, false otherwise
 */
bool 		msgqueue_is_empty  (MsgQueue* que);

/**
 * @brief	return used size in memory pool
 * @param[in] 	que:	pointer to opaque MsgQueue object 
 * @return	size in byte of used memory
 */
size_t 	msgqueue_get_used_size (MsgQueue* que);

/**
 * @brief	return free size in memory pool
 * @param[in] 	que:	pointer to opaque MsgQueue object 
 * @return	size in byte of free memory
 */
size_t 	msgqueue_get_free_size (MsgQueue* que);


/**
 * @brief	allocate a buffer prior to send it
 * @details	to limit the number of buffer copies, this api is provided and
 *		should be used like this
 *		ptr = msgqueue_malloc_before_send
 *		if (ptr) { fill  ptr }
 *		msgqueue_sendxxx ()
 *		deallocation is done by the caching thread which actually write data to sd card
 *		from the sender point of view, ptr should be considered invalid after beeing sent
 *
 * @param[in] 	que:	pointer to opaque MsgQueue object 
 * @param[in] 	msgLen:	length of requested buffer
 * @return	if NULL => error	       
 *		if non NULL : pointer to memory
 */
void *	 msgqueue_malloc_before_send (MsgQueue* que, const uint16_t msgLen);

/**
 * @brief	send a buffer previously allocated by msgqueue_malloc_before_send
 * @details	deallocation is done by the caching thread which actually write data to sd card
 *		from the sender point of view, ptr should be considered invalid after beeing sent.
 *		Even if msgqueue_send fail, deallocation is done
 *		Non blocking, if queue is full, report error and immediately return
 * @param[in] 	que:	pointer to opaque MsgQueue object 
 * @param[in] 	msg:	pointer to buffer given by msgqueue_malloc_before_send
 * @param[in] 	msgLen:	length of buffer (the same param that has been 
 *		given to msgqueue_malloc_before_send)
 * @param[in] 	urgency  regular=queued at end of fifo or out_of_band queued at start of fifo
 * @return	if > 0 : requested length
 *		if < 0 : error status (see errors at begining of this header file)
 */
int32_t	 msgqueue_send (MsgQueue* que, void *msg, const uint16_t msgLen,
			const MsgQueueUrgency urgency);


/**
 * @brief	send a buffer previously allocated by msgqueue_malloc_before_send
 * @details	deallocation is done by the caching thread which actually write data to sd card
 *		from the sender point of view, ptr should be considered invalid after beeing sent.
 *		Even if msgqueue_send fail, deallocation is done
 * @param[in] 	que:	pointer to opaque MsgQueue object 
 * @param[in] 	msg:	pointer to buffer given by msgqueue_malloc_before_send
 * @param[in] 	msgLen:	length of buffer (the same param that has been 
 *		given to msgqueue_malloc_before_send)
 * @param[in] 	urgency  regular=queued at end of fifo or out_of_band queued at start of fifo
 * @param[in] 	timout : time to wait for MailBox avaibility (can be TIME_INFINITE or TIME_IMMEDIATE)
 * @return	if > 0 : requested length
 *		if < 0 : error status (see errors at begining of this header file)
 */
int32_t	 msgqueue_send_timeout (MsgQueue* que, void *msg, const uint16_t msgLen,
				const MsgQueueUrgency urgency, const systime_t timout);


/**
 * @brief	send a buffer *NOT* previously allocated
 * @details	buffer is copied before beeing sent, buffer is still valid after this function
 *		returns. Less effective that zero copy alternatives which has been to be preferably
 *              used
 *		Non blocking, if queue is full, report error and immediately return
 * @param[in] 	que:	pointer to opaque MsgQueue object 
 * @param[in] 	msg:	pointer to buffer
 * @param[in] 	msgLen:	length of buffer (the same param that has been 
 *		given to msgqueue_malloc_before_send)
 * @param[in] 	urgency  regular=queued at end of fifo or out_of_band queued at start of fifo
 * @return	if > 0 : requested length
 *		if < 0 : error status (see errors at begining of this header file)
 */
int32_t	 msgqueue_copy_send (MsgQueue* que, const void *msg, const uint16_t msgLen,
			     const MsgQueueUrgency urgency);


/**
 * @brief	send a buffer *NOT* previously allocated
 * @details	buffer is copied before beeing sent, buffer is still valid after this function
 *		returns. Less effective that zero copy alternatives which has been to be preferably
 *              used
 * @param[in] 	que:	pointer to opaque MsgQueue object 
 * @param[in] 	msg:	pointer to buffer
 * @param[in] 	msgLen:	length of buffer (the same param that has been 
 *		given to msgqueue_malloc_before_send)
 * @param[in] 	urgency  regular=queued at end of fifo or out_of_band queued at start of fifo
 * @param[in] 	timout : time to wait for MailBox avaibility (can be TIME_INFINITE or TIME_IMMEDIATE)
 * @return	if > 0 : requested length
 *		if < 0 : error status (see errors at begining of this header file)
 */
int32_t	 msgqueue_copy_send_timeout (MsgQueue* que, const void *msg, const uint16_t msgLen,
				     const MsgQueueUrgency urgency, const systime_t timout);



  /*
  goal : (zero copy api)
  get a message to be processed without copy (blocking)

  parameters : INOUT queue object
	       OUT msg pointer
  return value: 
                
 */

/**
 * @brief	wait then receive message
 * @details	this is zero copy api, blocking call
 *		usage : struct MyStruct *msg
 *                      msgqueue_pop (&que, void (void **) &msg);
 *                      use msg->myField etc etc
                        msgqueue_free_after_pop (&que, msg);
 * @param[in] 	que:	pointer to opaque MsgQueue object 
 * @param[out] 	msgPtr:	pointer to pointer to buffer
 * @return	if > 0 : length of received msg					  
 *		if < 0 : error status (see errors at begining of this header file)
 */
int32_t	msgqueue_pop (MsgQueue* que, void **msgPtr);

/**
 * @brief	receive message specifying timeout
 * @details	this is zero copy api, blocking call
 *		usage : struct MyStruct *msg
 *                      msgqueue_pop (&que, void (void **) &msg);
 *                      use msg->myField etc etc
                        msgqueue_free_after_pop (&que, msg);
 * @param[in] 	que:	pointer to opaque MsgQueue object 
 * @param[out] 	msgPtr:	pointer to pointer to buffer
 * @param[in] 	timout : time to wait for MailBox avaibility (can be TIME_INFINITE or TIME_IMMEDIATE)
 * @return	if > 0 : length of received msg					  
 *		if < 0 : error status (see errors at begining of this header file)
 */
int32_t	msgqueue_pop_timeout (MsgQueue* que, void **msgPtr, const systime_t timout);


/*
  goal : (zero copy api)
  free memory of a precedently given message by PopChunk

  parameters : INOUT queue object
               IN    ChunkBufferRO pointer
 */
/**
 * @brief	free message after using it to give back memory to dynamic allocator
 * @details	this is zero copy api
 * @param[in] 	que:	pointer to opaque MsgQueue object 
 * @param[in] 	msg:	pointer to buffer to free
 */
void	msgqueue_free_after_pop (MsgQueue* que, void *msg);



/**
 * @brief	test and debug api
 * @details	test that when mailbox is empty, memory pool is unused too
 * @param[in] 	que:	pointer to opaque MsgQueue object 
 * @return	MsgQueue_OK or MsgQueue_MAILBOX_NOT_EMPTY or MsgQueue_MAILBOX_FAIL in case of internal error
 */
MsgQueueStatus	msgqueue_test_integrity_if_empty(MsgQueue* que);

/**
 * @brief	debug api
 * @details	give ascii string corresponding to the given errno
 * @param[in] 	errno : staus
 * @return	pointer to error string
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
  mutex_t	mtx;
  mailbox_t	mb;
  void *	mp_base;
  size_t	mp_size;
  size_t	mp_initialy_used;
} ;


#ifdef __cplusplus
}
#endif
