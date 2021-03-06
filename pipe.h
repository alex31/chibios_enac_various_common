#pragma once
#include "ringBuffer.h"
#include "ch.h"
#include "hal.h"
#include <sys/types.h>
#include <unistd.h>
/*

TODO:


*/

#ifdef __cplusplus
extern "C" {
#endif

#define _LPIPE_DATA(name, _cb)  {	        \
  .cb = _cb,					\
  .mtx = _MUTEX_DATA(name.mtx),			\
  .cv_empty = _CONDVAR_DATA(name.cv_empty),	\
  .cv_full =_CONDVAR_DATA(name.cv_full)		\
}

#define LPIPE_DECL(name, cb)		\
  Pipe name = _LPIPE_DATA(name, cb)
  
typedef struct
{
  CircularBuffer *cb;
  mutex_t	       mtx;
  condition_variable_t cv_empty; 
  condition_variable_t cv_full; 
} Pipe;



void			pipeObjectInit(Pipe* pipe, CircularBuffer *_cb);
static inline bool	pipeIsFull(const Pipe* pipe)
{return ringBufferIsFull(pipe->cb);};
static inline bool 	pipeIsEmpty(const Pipe* pipe)
{return ringBufferIsEmpty(pipe->cb);};  
static inline ssize_t 	pipeUsedSize(const Pipe* pipe)
{return ringBufferUsedSize(pipe->cb);};   
static inline ssize_t 	pipeFreeSize(const Pipe* pipe)
{return ringBufferFreeSize(pipe->cb);}; 
ssize_t			pipeWrite(Pipe* pipe, const uint8_t* pK,
						  size_t len);
ssize_t			pipeRead(Pipe* pipe, uint8_t* pK,
						  size_t len);

#ifdef __cplusplus
}
#endif


