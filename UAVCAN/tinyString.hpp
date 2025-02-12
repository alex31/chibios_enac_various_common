#pragma once

// Just because etl::string overhead is insane : 40 bytes for each string.
// Wish I could find a more standard way to have a low overhead string impl.

#include <iostream>
#include <cstring>  // For memcpy, strcpy, memcmp
#include <type_traits>


#include <iostream>
#include <cstring>  // For memcpy, strcpy, memcmp
#include <cassert>  
#include <type_traits>
#include "customAllocator.hpp"

template <size_t STO_SIZE, size_t STR_SIZE>
class TinyString {
    static_assert(STR_SIZE > 0, "TinyString size must be greater than 0");

private:
    char data[STR_SIZE]{};
    uint8_t len = 0;  // Only 1-byte overhead instead of 40+ bytes

    //  Ensure struct size is a multiple of 4 for alignment
    static constexpr size_t paddedSize = (sizeof(data) + sizeof(len) + 3) & ~3;
    alignas(4) std::byte padding[paddedSize - (sizeof(data) + sizeof(len))]{};

    //  Helper function to copy strings safely
    constexpr void copyFrom(const char* str) {
        len = std::strlen(str);
        if (len >= STR_SIZE) len = STR_SIZE - 1;  // Avoid buffer overflow
        std::memcpy(data, str, len);
        data[len] = '\0';
    }
  static SimpleMemoryPool<STO_SIZE> memPool; 
public:
   public:
    static void* operator new(std::size_t size) {
      //  Safe: Returns nullptr if out of memory
      return memPool.allocate(size);  
    }
    
    static void operator delete(void*, std::size_t) {
      assert( 0 && "delete should never be called by TinyString");
    }

  constexpr TinyString() = default;

    //  Copy constructor (factorized)
    constexpr TinyString(const TinyString& other) : len(other.len) {
        std::memcpy(data, other.data, len);
        data[len] = '\0';
    }

    //  Constructor from C-string (factorized)
    constexpr TinyString(const char* str) {
        copyFrom(str);
    }

    constexpr size_t size() const { return len; }
    constexpr size_t capacity() const { return STR_SIZE - 1; }
    constexpr const char* c_str() const { return data; }

    constexpr void clear() {
        len = 0;
        data[0] = '\0';
    }

    //  Append function (factorized)
    constexpr void append(const char* str) {
        const size_t str_len = std::strlen(str);
        const size_t copy_len = (len + str_len < STR_SIZE) ? str_len : (STR_SIZE - 1 - len);
        std::memcpy(data + len, str, copy_len);
        len += copy_len;
        data[len] = '\0';
    }

    //  Operator += (factorized)
    constexpr TinyString& operator+=(const char* str) {
        append(str);
        return *this;
    }

    constexpr TinyString& operator+=(const TinyString& other) {
        append(other.c_str());
        return *this;
    }

    //  Operator = (factorized)
    constexpr TinyString& operator=(const TinyString& other) {
        if (this != &other) {
            len = other.len;
            std::memcpy(data, other.data, len);
            data[len] = '\0';
        }
        return *this;
    }

    constexpr TinyString& operator=(const char* str) {
        copyFrom(str);
        return *this;
    }

    //  Comparison operators (factorized)
    constexpr bool operator==(const TinyString& other) const {
        return (len == other.len) && (std::memcmp(data, other.data, len) == 0);
    }

    constexpr bool operator==(const char* str) const {
        const size_t str_len = std::strlen(str);
        return (len == str_len) && (std::memcmp(data, str, len) == 0);
    }
};

template <size_t STO_SIZE, size_t STR_SIZE>
SimpleMemoryPool<STO_SIZE> TinyString<STO_SIZE, STR_SIZE>::memPool;
