#pragma once

#include <cstddef>
#include <cstdint>
#include "persistantParam.hpp"
#if defined(__arm__) || defined(__aarch64__) || defined(__ARM_ARCH)
#define TARGET_ARM_CHIBIOS
#include "ch.h"
#endif
/*
  TODO :

 
 */
namespace Persistant {
  
/**
 * @brief Provides access to stored parameter values.
 */
  using EepromStoreWriteFn = bool (*) (size_t recordIndex, const void *data, size_t size);
  using EepromStoreReadFn =  bool (*) (size_t recordIndex, void *data, size_t& size);
  using EepromEraseRecord =  bool (*) (size_t recordIndex);
  using EepromErase =        bool (*) (void);
  using EepromGetLen =       size_t (*) ();
  
  struct EepromStoreHandle {
    EepromStoreWriteFn	writeFn;
    EepromStoreReadFn	readFn;
    EepromErase		eraseFn;
    EepromGetLen	getLen;
  };


  class Storage {
  public:
    Storage(const EepromStoreHandle& _handle);
    bool   start();
    bool   eraseAll();
    bool   store(size_t index);
    bool   restore(size_t index);
    bool   storeAll();
    bool   restoreAll();
    size_t getLen();
    
  private:
#ifdef TARGET_ARM_CHIBIOS
    MUTEX_DECL(mtx); // to protect shared dma StoreSerializeBuffer buffer
#endif
    EepromStoreHandle handle;
    bool store(size_t index, const StoreSerializeBuffer& buffer);
    bool restore(size_t index, StoreSerializeBuffer& buffer);
    bool restore(size_t frozenIndex, size_t storeIndex);
    ssize_t binarySearch(const frozen::string& str);
    void partialRestore();
    StoredValue get(const frozen::string& name);
    static StoreSerializeBuffer buffer;
  };

  
}
