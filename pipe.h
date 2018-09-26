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
} Pipe;



void			pipeInit(Pipe* pipe, CircularBuffer *_cb);
static inline bool	pipeIsFull(const Pipe* pipe)
{return ringBufferIsFull(pipe->cb);};
static inline bool 	pipeIsEmpty(const Pipe* pipe)
{return ringBufferIsEmpty(pipe->cb);};  
static inline ssize_t 	pipeUsedSize(const Pipe* pipe)
{return ringBufferUsedSize(pipe->cb);};   
static inline ssize_t 	pipeFreeSize(const Pipe* pipe)
{return ringBufferFreeSize(pipe->cb);}; 
ssize_t			pipeEnque(Pipe* pipe, const uint8_t* pK,
						  size_t len);
ssize_t			pipeDeque(Pipe* pipe, uint8_t* pK,
						  size_t len);

#ifdef __cplusplus
}
#endif


