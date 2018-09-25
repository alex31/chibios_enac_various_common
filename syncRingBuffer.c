#include "syncRingBuffer.h"
#include <string.h>

void		syncRingBufferInit(SyncCircularBuffer* sque, CircularBuffer *_cb)
{
  sque->cb = _cb;
  chMtxObjectInit(&sque->mtx);
  chCondObjectInit(&sque->cv_empty);
  chCondObjectInit(&sque->cv_full);
};


ssize_t		syncRingBufferEnque(SyncCircularBuffer* sque, const uint8_t* pK, const size_t len)
{
  chMtxLock(&sque->mtx);
  ssize_t slen = len;
  const uint8_t* ptr = pK;
  ssize_t freeSz;
  ssize_t ret=0;

  
  // si la place libredans le circular buffer n'est pas suffisante
  // on remplit au max, et on attend que de la place se libère
  while ((slen != 0) && (freeSz = syncRingBufferFreeSize(sque)) < slen) {
    const ssize_t  sret = ringBufferEnqueBuffer(sque->cb, ptr, freeSz);
    ret += sret;
    ptr += freeSz;
    slen -= freeSz;
    chCondSignal(&sque->cv_full);
    osalDbgAssert((sret == freeSz), "syncRingBuffer internal error");
    chCondWait(&sque->cv_empty);
  }
  if (slen) {
    const ssize_t sret = ringBufferEnqueBuffer(sque->cb, ptr, slen);
    ret += sret;
    chCondSignal(&sque->cv_full);
    osalDbgAssert((sret == slen), "syncRingBuffer internal error");
  }

  chMtxUnlock(&sque->mtx);
  return ret;
}


ssize_t 	syncRingBufferDeque(SyncCircularBuffer* sque, uint8_t* pK, const size_t len)
{
  chMtxLock(&sque->mtx);
  ssize_t slen = len;

  // si le nombre d'elements disponibles dans le circular buffer n'est pas suffisant,
  // on attend qu'il se remplisse
  while (syncRingBufferUsedSize(sque) < slen) {
    chCondWait(&sque->cv_full);
  }
  const ssize_t	ret =ringBufferDequeBuffer(sque->cb, pK, slen);
  osalDbgAssert((ret == slen), "syncRingBuffer internal error");
  chCondSignal(&sque->cv_empty);

  chMtxUnlock(&sque->mtx);
  return ret;
}


 
