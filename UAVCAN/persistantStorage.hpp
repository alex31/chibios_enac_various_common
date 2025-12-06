#pragma once

/**
 * @file persistantStorage.hpp
 * @brief Lightweight EEPROM-backed persistence helpers for UAVCAN parameters.
 *
 * The `Storage` facade owns a user-provided `EepromStoreHandle` composed of four
 * function pointers (read/write/erase/getLen). It serializes parameter values
 * using `Persistant::Parameter::serializeStoredValue()` into small records,
 * and restores them back into the flat buffer on boot.
 */

#include <cstddef>
#include <cstdint>
#include "persistantParam.hpp"
#if defined(__arm__) || defined(__aarch64__) || defined(__ARM_ARCH)
#define TARGET_ARM_CHIBIOS
#include "ch.h"
#endif
namespace Persistant {
  /// Callback used to write a serialized record to non-volatile storage.
  using EepromStoreWriteFn = bool (*) (size_t recordIndex, const void *data, size_t size);
  /// Callback used to read a serialized record back into RAM.
  using EepromStoreReadFn =  bool (*) (size_t recordIndex, void *data, size_t& size);
  /// Optional callback to erase a single record.
  using EepromEraseRecord =  bool (*) (size_t recordIndex);
  /// Callback to erase the whole store (used on CRC mismatch).
  using EepromErase =        bool (*) (void);
  /// Callback returning the number of records available in storage.
  using EepromGetLen =       size_t (*) ();
  
  /**
   * @brief Bundle of EEPROM-like backend functions required by Storage.
   */
  struct EepromStoreHandle {
    EepromStoreWriteFn	writeFn;
    EepromStoreReadFn	readFn;
    EepromErase		eraseFn;
    EepromGetLen	getLen;
  };


  class Storage {
  public:
    /**
     * @brief Build a Storage helper around a backend handle.
     * @param _handle Function pointer bundle; all members must be non-null.
     */
    Storage(const EepromStoreHandle& _handle);

    /**
     * @brief Initialize RAM defaults then try to restore persisted values.
     * @return true if a full restore succeeded or partial restore + store succeeded.
     */
    bool   start();

    /**
     * @brief Erase the whole backing store.
     */
    bool   eraseAll();

    /**
     * @brief Serialize and write a single parameter at @p index.
     */
    bool   store(size_t index);

    /**
     * @brief Read and deserialize a single record from backing store.
     */
    bool   restore(size_t index);

    /**
     * @brief Serialize and write every parameter.
     * @return Aggregated success (all writes succeed).
     */
    bool   storeAll();

    /**
     * @brief Restore every record from backing store.
     * @return Aggregated success; still returns true even when CONST.* parameters are skipped.
     */
    bool   restoreAll();

    /**
     * @brief Expose number of records available in backing store.
     */
    size_t getLen();
    
  private:
#ifdef TARGET_ARM_CHIBIOS
    MUTEX_DECL(mtx); // to protect shared dma StoreSerializeBuffer buffer
#endif
    EepromStoreHandle handle;
    bool store(size_t index, const StoreSerializeBuffer& buffer);
    bool restore(size_t index, StoreSerializeBuffer& buffer);
    bool restore(size_t frozenIndex, size_t storeIndex);
    /// Binary search a serialized store by name (stored alphabetically).
    ssize_t binarySearch(const frozen::string& str);
    /// Restore only matching entries, used when a full restore fails.
    void partialRestore();
    /// Return a parameter from persistent storage without mutating the store.
    StoredValue get(const frozen::string& name);
    static StoreSerializeBuffer buffer;
  };

  
}
