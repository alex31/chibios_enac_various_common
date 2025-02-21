#pragma once

#include <cstddef>
#include <cstdint>
#include "persistantParam.hpp"

/*
  TODO :

  * une methode restoreFromAlternate : parcours tous les paramètres de frozenparameters
  et essaye de chercher les valeurs par nom dans l'ancienne table pour les mettre
  dans les paramètres actuels

 */
namespace Persistant {
  
/**
 * @brief Provides access to stored parameter values.
 */
  using EepromStoreWriteFn = bool (*) (uint8_t fileIndex, size_t recordIndex,
				       const void *data, size_t size);
  using EepromStoreReadFn = bool (*)  (uint8_t fileIndex, size_t recordIndex,
				      void *data, size_t& size);
  using EepromGetLen =    size_t (*)  (uint8_t fileIndex);
  
  struct EepromStoreHandle {
    EepromStoreWriteFn	writeFn;
    EepromStoreReadFn	readFn;
    EepromGetLen	getLen;
  };


  class Storage {
  public:
    Storage(const EepromStoreHandle& _handle);
    bool store(size_t index);
    bool restore(uint8_t bank, size_t index);
    bool storeAll();
    bool restoreAll();
    
  private:
    EepromStoreHandle handle;
    uint8_t getActive() {return activeBank;}
    uint8_t getAlternate() {return 1U - activeBank;}
    void swapActive() {activeBank = 1U - activeBank;}
    bool store(size_t index, const StoreSerializeBuffer& buffer);
    bool restore(uint8_t bank, size_t index, StoreSerializeBuffer& buffer);
    ssize_t binarySearch(uint8_t bank, const frozen::string& str);
    void partialRestoreFromAlternate();
    StoredValue get(uint8_t bank, const frozen::string& name);
    uint8_t activeBank = 0;
  };

  
}
