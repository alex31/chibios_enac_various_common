#pragma once

#include <cstddef>
#include <cstdint>
#include "persistantParam.hpp"

/*
  TODO :

 
 */
namespace Persistant {
  
/**
 * @brief Provides access to stored parameter values.
 */
  using EepromStoreWriteFn = bool (*) (size_t recordIndex, const void *data, size_t size);
  using EepromStoreReadFn =  bool (*) (size_t recordIndex, void *data, size_t& size);
  using EepromGetLen =       size_t (*) ();
  
  struct EepromStoreHandle {
    EepromStoreWriteFn	writeFn;
    EepromStoreReadFn	readFn;
    EepromGetLen	getLen;
  };


  class Storage {
  public:
    Storage(const EepromStoreHandle& _handle);
    bool store(size_t index);
    bool restore(size_t index);
    bool storeAll();
    bool restoreAll();
    
  private:
    EepromStoreHandle handle;
    bool store(size_t index, const StoreSerializeBuffer& buffer);
    bool restore(size_t index, StoreSerializeBuffer& buffer);
    bool restore(size_t frozenIndex, size_t storeIndex);
    ssize_t binarySearch(const frozen::string& str);
    void partialRestore();
    StoredValue get(const frozen::string& name);
  };

  
}
