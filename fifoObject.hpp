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

  T&     takeObject(const sysinterval_t timeout=TIME_INFINITE);
  T&     takeObjectS(const sysinterval_t timeout=TIME_INFINITE);
  T&     takeObjectI(void);

  void   sendObject(T& obj);
  void   sendObjectS(T& obj);
  void   sendObjectI(T& obj);

  msg_t  receiveObject(T* &objpp, sysinterval_t timeout=TIME_INFINITE);
  msg_t  receiveObjectS(T* &objpp, sysinterval_t timeout=TIME_INFINITE);
  msg_t  receiveObjectI(T* &objpp);

  void   returnObject(T* obj);
  void   returnObjectI(T* obj);

  size_t fifoLen(void) {return FIFO_SIZE;};
private:
 T msg_pool[FIFO_SIZE];
  msg_t msg_fifo[FIFO_SIZE];
  objects_fifo_t fifo;
};

/*
#                 _            _    _            
#                (_)          (_)  | |           
#                 _    _ __    _   | |_          
#                | |  | '_ \  | |  | __|         
#                | |  | | | | | |  \ |_          
#                |_|  |_| |_| |_|   \__|         
*/
template <typename T, size_t FIFO_SIZE>
ObjectFifo<T, FIFO_SIZE>::ObjectFifo (void)
{
  chFifoObjectInit (&fifo, sizeof(T),  FIFO_SIZE,  std::alignment_of<T>::value,
		    msg_pool, msg_fifo);
}


/*
#                 _              _                   
#                | |            | |                  
#                | |_     __ _  | | _    ___         
#                | __|   / _` | | |/ /  / _ \        
#                \ |_   | (_| | |   <  |  __/        
#                 \__|   \__,_| |_|\_\  \___|        
*/
template <typename T, size_t FIFO_SIZE>
T& ObjectFifo<T, FIFO_SIZE>::takeObject(const sysinterval_t timeout)
{
  const auto ptrToObj = static_cast<T *> (chFifoTakeObjectTimeout(&fifo, timeout));
  return *ptrToObj;
}

template <typename T, size_t FIFO_SIZE>
T& ObjectFifo<T, FIFO_SIZE>::takeObjectS(const sysinterval_t timeout)
{
  const auto ptrToObj = static_cast<T *> (chFifoTakeObjectTimeoutS(&fifo, timeout));
  return *ptrToObj;
}

template <typename T, size_t FIFO_SIZE>
T& ObjectFifo<T, FIFO_SIZE>::takeObjectI(void)
{
  const auto ptrToObj = static_cast<T *> (chFifoTakeObjectI(&fifo));
  return *ptrToObj;
}

/*
#                                           _          
#                                          | |         
#                 ___     ___   _ __     __| |         
#                / __|   / _ \ | '_ \   / _` |         
#                \__ \  |  __/ | | | | | (_| |         
#                |___/   \___| |_| |_|  \__,_|         
*/
template <typename T, size_t FIFO_SIZE>
void ObjectFifo<T, FIFO_SIZE>::sendObject(T& obj)
{
  chFifoSendObject(&fifo, &obj);
}

template <typename T, size_t FIFO_SIZE>
void ObjectFifo<T, FIFO_SIZE>::sendObjectS(T& obj)
{
  chFifoSendObjectS(&fifo, &obj);
}

template <typename T, size_t FIFO_SIZE>
void ObjectFifo<T, FIFO_SIZE>::sendObjectI(T& obj)
{
  chFifoSendObjectI(&fifo, &obj);
}


/*
#                                             _                         
#                                            (_)                        
#                 _ __    ___    ___    ___   _   __   __   ___         
#                | '__|  / _ \  / __|  / _ \ | |  \ \ / /  / _ \        
#                | |    |  __/ | (__  |  __/ | |   \ V /  |  __/        
#                |_|     \___|  \___|  \___| |_|    \_/    \___|        
*/
template <typename T, size_t FIFO_SIZE>
msg_t ObjectFifo<T, FIFO_SIZE>::receiveObject(T* &obj, sysinterval_t timeout)
{
  return chFifoReceiveObjectTimeout(&fifo,
				    reinterpret_cast<void **>(&obj),
				    timeout);
}

template <typename T, size_t FIFO_SIZE>
msg_t ObjectFifo<T, FIFO_SIZE>::receiveObjectS(T* &obj, sysinterval_t timeout)
{
  return chFifoReceiveObjectTimeoutS(&fifo,
				    reinterpret_cast<void **>(&obj),
				    timeout);
}

template <typename T, size_t FIFO_SIZE>
msg_t ObjectFifo<T, FIFO_SIZE>::receiveObjectI(T* &obj)
{
  return chFifoReceiveObjectI(&fifo,
			      reinterpret_cast<void **>(&obj));
}

/*
#                               _                                   
#                              | |                                  
#                 _ __    ___  | |_    _   _   _ __   _ __          
#                | '__|  / _ \ | __|  | | | | | '__| | '_ \         
#                | |    |  __/ \ |_   | |_| | | |    | | | |        
#                |_|     \___|  \__|   \__,_| |_|    |_| |_|        
*/
template <typename T, size_t FIFO_SIZE>
void ObjectFifo<T, FIFO_SIZE>::returnObject(T* obj)
{
  chFifoReturnObject(&fifo, obj);
}

template <typename T, size_t FIFO_SIZE>
void ObjectFifo<T, FIFO_SIZE>::returnObjectI(T* obj)
{
  chFifoReturnObjectI(&fifo, obj);
}
