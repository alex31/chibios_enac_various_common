#pragma once

#include <ch.h>
#include <hal.h>
#include <utility>
#include <optional>
#include <functional>


enum class FifoPriority {Regular, High};

template <typename T, size_t FIFO_SIZE>
class ObjectFifo
{
  using OptRef = std::optional<std::reference_wrapper<T>>;

  
public:
  ObjectFifo ();

  OptRef      	takeObject(const sysinterval_t timeout=TIME_INFINITE);
  OptRef      	takeObjectS(const sysinterval_t timeout=TIME_INFINITE);
  OptRef      	takeObjectI();

  void		sendObject(T& obj, const FifoPriority p = FifoPriority::Regular);
  void   	sendObjectS(T& obj, const FifoPriority p = FifoPriority::Regular);
  void   	sendObjectI(T& obj, const FifoPriority p = FifoPriority::Regular);

  OptRef  	receiveObject(sysinterval_t timeout=TIME_INFINITE);
  OptRef  	receiveObjectS(sysinterval_t timeout=TIME_INFINITE);
  OptRef  	receiveObjectI();

  void   	returnObject(T& obj);
  void   	returnObjectI(T& obj);

  constexpr size_t fifoLen();
  size_t	fifoUsedI();
  size_t	fifoFreeI();
  size_t	fifoUsed();
  size_t	fifoFree();
  
private:
  T msg_pool[FIFO_SIZE];
  msg_t msg_fifo[FIFO_SIZE];
  objects_fifo_t fifo;
};

/*
#                        _                           
#                       | |                          
#                  ___  | |_     ___    _ __         
#                 / __| | __|   / _ \  | '__|        
#                | (__  \ |_   | (_) | | |           
#                 \___|  \__|   \___/  |_|           
*/
template <typename T, size_t FIFO_SIZE>
ObjectFifo<T, FIFO_SIZE>::ObjectFifo()
{
  chFifoObjectInit(&fifo, sizeof(T),  FIFO_SIZE, msg_pool, msg_fifo);
  // static_assert(std::is_copy_constructible<T>::value == false,
  // 		"type T should no be copy constructible to impose return by reference");
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
ObjectFifo<T, FIFO_SIZE>::OptRef ObjectFifo<T, FIFO_SIZE>::takeObject(const sysinterval_t timeout)
{
  const auto ptrToObj = static_cast<T *> (chFifoTakeObjectTimeout(&fifo, timeout));
  if (ptrToObj == nullptr) {
    return std::nullopt;
  }
  return std::ref(*ptrToObj);
}

template <typename T, size_t FIFO_SIZE>
ObjectFifo<T, FIFO_SIZE>::OptRef ObjectFifo<T, FIFO_SIZE>::takeObjectS(const sysinterval_t timeout)
{
  const auto ptrToObj = static_cast<T *> (chFifoTakeObjectTimeoutS(&fifo, timeout));
  if (ptrToObj == nullptr) {
    return std::nullopt;
  }
  return std::ref(*ptrToObj);
}

template <typename T, size_t FIFO_SIZE>
ObjectFifo<T, FIFO_SIZE>::OptRef ObjectFifo<T, FIFO_SIZE>::takeObjectI()
{
  const auto ptrToObj = static_cast<T *> (chFifoTakeObjectI(&fifo));
  if (ptrToObj == nullptr) {
    return std::nullopt;
  }
  return std::ref(*ptrToObj);
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
void ObjectFifo<T, FIFO_SIZE>::sendObject(T& obj, const FifoPriority p)
{
  if (p == FifoPriority::Regular) {
    chFifoSendObject(&fifo, &obj);
  } else {
    chFifoSendObjectAhead(&fifo, &obj);
  }
}

template <typename T, size_t FIFO_SIZE>
void ObjectFifo<T, FIFO_SIZE>::sendObjectS(T& obj, const FifoPriority p)
{
 if (p == FifoPriority::Regular) {
   chFifoSendObjectS(&fifo, &obj);
 } else {
  chFifoSendObjectSAhead(&fifo, &obj);
 }
}

template <typename T, size_t FIFO_SIZE>
void ObjectFifo<T, FIFO_SIZE>::sendObjectI(T& obj, const FifoPriority p)
{
 if (p == FifoPriority::Regular) {
   chFifoSendObjectI(&fifo, &obj);
 } else {
   chFifoSendObjectAheadI(&fifo, &obj);
 }
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
ObjectFifo<T, FIFO_SIZE>::OptRef ObjectFifo<T, FIFO_SIZE>::receiveObject(sysinterval_t timeout)
{
  T *ptr = nullptr;
  const msg_t status = chFifoReceiveObjectTimeout(&fifo,
				    reinterpret_cast<void **>(&ptr),
				    timeout);
  if ((status != MSG_OK) || (ptr == nullptr)) {
    return std::nullopt;
  }
  return std::ref(*ptr);
}

template <typename T, size_t FIFO_SIZE>
ObjectFifo<T, FIFO_SIZE>::OptRef ObjectFifo<T, FIFO_SIZE>::receiveObjectS(sysinterval_t timeout)
{
  T *ptr;
  const msg_t status = chFifoReceiveObjectTimeoutS(&fifo,
				    reinterpret_cast<void **>(&ptr),
				    timeout);
  if ((status != MSG_OK) || (ptr == nullptr)) {
    return std::nullopt;
  }
  return std::ref(*ptr);
}

template <typename T, size_t FIFO_SIZE>
ObjectFifo<T, FIFO_SIZE>::OptRef ObjectFifo<T, FIFO_SIZE>::receiveObjectI()
{
  T *ptr;
  const msg_t status = chFifoReceiveObjectI(&fifo,
				    reinterpret_cast<void **>(&ptr));
  if ((status != MSG_OK) || (ptr == nullptr)) {
    return std::nullopt;
  }
  return std::ref(*ptr);
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
void ObjectFifo<T, FIFO_SIZE>::returnObject(T& obj)
{
  chFifoReturnObject(&fifo, &obj);
}

template <typename T, size_t FIFO_SIZE>
void ObjectFifo<T, FIFO_SIZE>::returnObjectI(T& obj)
{
  chFifoReturnObjectI(&fifo, &obj);
}


/*
#                  __                     __                     _ 
#                 / _|                   / /                    | |
#                | |_ _ __ ___  ___     / /   _   _ ___  ___  __| |
#                |  _| '__/ _ \/ _ \   / /   | | | / __|/ _ \/ _` |
#                | | | | |  __/  __/  / /    | |_| \__ \  __/ (_| |
#                |_| |_|  \___|\___| /_/      \__,_|___/\___|\__,_|
#                                                                  
#                                                                  
*/


template <typename T, size_t FIFO_SIZE>
size_t	ObjectFifo<T, FIFO_SIZE>::fifoFreeI() {return chMBGetFreeCountI(&fifo.mbx);}

template <typename T, size_t FIFO_SIZE>
size_t	ObjectFifo<T, FIFO_SIZE>::fifoUsedI() {return chMBGetUsedCountI(&fifo.mbx);}

template <typename T, size_t FIFO_SIZE>
size_t	ObjectFifo<T, FIFO_SIZE>::fifoUsed() {
  chSysLock();
  auto ret = fifoUsedI();
  chSysUnlock();
  return ret;
}

template <typename T, size_t FIFO_SIZE>
size_t	ObjectFifo<T, FIFO_SIZE>::fifoFree() {
  chSysLock();
  auto ret = fifoFreeI();
  chSysUnlock();
  return ret;
}
