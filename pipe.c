#include "pipe.h"
#include <string.h>

void		pipeObjectInit(Pipe* pipe, CircularBuffer *_cb)
{
  pipe->cb = _cb;
  chMtxObjectInit(&pipe->mtx);
  chCondObjectInit(&pipe->cv_empty);
  chCondObjectInit(&pipe->cv_full);
};


ssize_t		pipeEnque(Pipe* pipe, const uint8_t* pK, const size_t len)
{
  chMtxLock(&pipe->mtx);
  ssize_t slen = len;
  const uint8_t* ptr = pK;
  ssize_t freeSz;
  ssize_t ret=0;

  
  // si la place libredans le circular buffer n'est pas suffisante
  // on remplit au max, et on attend que de la place se libère
  while ((slen != 0) && (freeSz = pipeFreeSize(pipe)) < slen) {
    const ssize_t  sret = ringBufferEnqueBuffer(pipe->cb, ptr, freeSz);
    ret += sret;
    ptr += freeSz;
    slen -= freeSz;
    chCondSignal(&pipe->cv_full);
    osalDbgAssert((sret == freeSz), "pipe internal error");
    chCondWait(&pipe->cv_empty);
  }
  if (slen) {
    const ssize_t sret = ringBufferEnqueBuffer(pipe->cb, ptr, slen);
    ret += sret;
    chCondSignal(&pipe->cv_full);
    osalDbgAssert((sret == slen), "pipe internal error");
  }

  chMtxUnlock(&pipe->mtx);
  return ret;
}


ssize_t 	pipeDeque(Pipe* pipe, uint8_t* pK, const size_t len)
{
  chMtxLock(&pipe->mtx);
  ssize_t slen = len;

  // si le nombre d'elements disponibles dans le circular buffer n'est pas suffisant,
  // on attend qu'il se remplisse
  while (pipeUsedSize(pipe) < slen) {
    chCondWait(&pipe->cv_full);
  }
  const ssize_t	ret =ringBufferDequeBuffer(pipe->cb, pK, slen);
  osalDbgAssert((ret == slen), "pipe internal error");
  chCondSignal(&pipe->cv_empty);

  chMtxUnlock(&pipe->mtx);
  return ret;
}


 
