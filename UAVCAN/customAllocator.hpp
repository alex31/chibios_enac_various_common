#pragma once

#include <iostream>
#include <cstddef>  // For std::size_t
#include <array>
#include <cstdio>

template<size_t STORAGE_SIZE>
class SimpleMemoryPool {
private:
  static constexpr std::size_t ALIGNMENT = 4;       // 4-byte boundary alignment
  std::array<std::byte, STORAGE_SIZE> storage = {};
  std::size_t offset = 0;  // Next free memory location
  
public:
  //  Allocate memory from pool (no exceptions)
  void* allocate(std::size_t size);
};


template<size_t STORAGE_SIZE>
void* SimpleMemoryPool<STORAGE_SIZE>::allocate(std::size_t size) {
  size = size == 1 ? 0 : size;
  std::size_t alignedOffset = (offset + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1);
  
  if (alignedOffset + size > STORAGE_SIZE) {
    return nullptr;  // ❌ Out of memory, return null pointer
  }
  
  void* ptr = storage.data() + alignedOffset;
  offset = alignedOffset + size;  // Move offset forward
  return ptr;
}

