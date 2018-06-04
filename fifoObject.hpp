#pragma once

#include <ch.h>
#include <hal.h>
#include <array>
#include <type_traits>

template <typename T, size_t FIFO_SIZE>
class ObjectFifo
{
public:
  ObjectFifo (void);

  T&    takeObjectTimeout(const sysinterval_t timeout=TIME_INFINITE);
  void  sendObject(T& obj);
  msg_t receiveObjectTimeout(T* &objpp, sysinterval_t timeout=TIME_INFINITE);
  void  returnObject(T* obj);
  size_t fifoLen(void) {return FIFO_SIZE;};
private:
 T msg_pool[FIFO_SIZE];
  msg_t msg_fifo[FIFO_SIZE];
  objects_fifo_t fifo;
};



template <typename T, size_t FIFO_SIZE>
ObjectFifo<T, FIFO_SIZE>::ObjectFifo (void)
{
  chFifoObjectInit (&fifo, sizeof(T),  FIFO_SIZE,  std::alignment_of<T>::value,
		    msg_pool, msg_fifo);
}


template <typename T, size_t FIFO_SIZE>
T& ObjectFifo<T, FIFO_SIZE>::takeObjectTimeout(const sysinterval_t timeout)
{
  const auto ptrToObj = static_cast<T *> (chFifoTakeObjectTimeout(&fifo, timeout));
  return *ptrToObj;
}

template <typename T, size_t FIFO_SIZE>
void ObjectFifo<T, FIFO_SIZE>::sendObject(T& obj)
{
  chFifoSendObject(&fifo, &obj);
}

template <typename T, size_t FIFO_SIZE>
msg_t ObjectFifo<T, FIFO_SIZE>::receiveObjectTimeout(T* &obj, sysinterval_t timeout)
{
  return chFifoReceiveObjectTimeout(&fifo,
				    reinterpret_cast<void **>(&obj),
				    timeout);
}

template <typename T, size_t FIFO_SIZE>
void ObjectFifo<T, FIFO_SIZE>::returnObject(T* obj)
{
  chFifoReturnObject(&fifo, obj);
}
