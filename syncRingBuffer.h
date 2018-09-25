#pragma once
#include "ringBuffer.h"
#include "ch.h"
#include "hal.h"
/*

TODO:


*/

#ifdef __cplusplus
extern "C" {
#endif


typedef struct
{
  CircularBuffer *cb;
  mutex_t	       mtx;
  condition_variable_t cv_empty; 
  condition_variable_t cv_full; 
} SyncCircularBuffer;



void			syncRingBufferInit(SyncCircularBuffer* sque, CircularBuffer *_cb);
static inline bool	syncRingBufferIsFull(const SyncCircularBuffer* sque)
{return ringBufferIsFull(sque->cb);};
static inline bool 	syncRingBufferIsEmpty(const SyncCircularBuffer* sque)
{return ringBufferIsEmpty(sque->cb);};  
static inline ssize_t 	syncRingBufferUsedSize(const SyncCircularBuffer* sque)
{return ringBufferUsedSize(sque->cb);};   
static inline ssize_t 	syncRingBufferFreeSize(const SyncCircularBuffer* sque)
{return ringBufferFreeSize(sque->cb);}; 
ssize_t			syncRingBufferEnque(SyncCircularBuffer* sque, const uint8_t* pK,
						  size_t len);
ssize_t			syncRingBufferDeque(SyncCircularBuffer* sque, uint8_t* pK,
						  size_t len);

#ifdef __cplusplus
}
#endif


