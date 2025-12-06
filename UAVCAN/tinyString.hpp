#pragma once

/**
 * @file tinyString.hpp
 * @brief Minimal fixed-capacity string with a tiny allocator footprint.
 *
 * Designed for MCUs where `etl::string` overhead is too large. The storage is
 * embedded (no heap) and uses a very small static `SimpleMemoryPool` when
 * placement-new is requested by the parameter subsystem.
 *
 * @tparam STO_SIZE Size of the static memory pool backing `operator new`.
 * @tparam STR_SIZE Maximum number of characters (including null terminator).
 */

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

    /**
     * @brief Fixed-capacity, POD-friendly string.
     * @details
     *  - Does not allocate dynamically; payload is inline.
     *  - Provides `operator new` overloads to integrate with the flat parameter store.
     *  - Keeps the layout trivially copyable so it can live inside a byte buffer.
     */
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
    /**
     * @brief Allocate from the static pool (used by placement-new in the parameter store).
     * @return Pointer inside the pool or nullptr if exhausted.
     */
    static void* operator new(std::size_t size) {
      //  Safe: Returns nullptr if out of memory
      return memPool.allocate(size);  
    }

    /** @brief Placement-new overload; does not touch the pool. */
    static void* operator new(std::size_t, void* ptr) noexcept {
      return ptr;
    }
    
    /** @brief Delete should never fire because storage is embedded. */
    static void operator delete(void*, std::size_t) {
      assert( 0 && "delete should never be called by TinyString");
    }
    static void operator delete(void*, void*) noexcept {}

    /** @brief Create an empty string. */
    constexpr TinyString() = default;

    /** @brief Copy constructor. */
    constexpr TinyString(const TinyString& other) : len(other.len) {
        std::memcpy(data, other.data, len);
        data[len] = '\0';
    }

    /** @brief Construct from a C-string (truncates to capacity). */
    constexpr TinyString(const char* str) {
        copyFrom(str);
    }

    /** @return Number of characters currently stored (no null terminator). */
    constexpr size_t size() const { return len; }
    /** @return Maximum characters that can be stored (excluding null). */
    constexpr size_t capacity() const { return STR_SIZE - 1; }
    /** @return Null-terminated C-string view. */
    constexpr const char* c_str() const { return data; }

  // Iterator support
    constexpr char* begin() { return data; }
    constexpr char* end() { return data + len; }
    
    constexpr const char* begin() const { return data; }
    constexpr const char* end() const { return data + len; }
    /** @brief Reset to an empty string. */
    constexpr void clear() {
        len = 0;
        data[0] = '\0';
    }

    /**
     * @brief Append a C-string, truncating if necessary.
     */
    constexpr void append(const char* str) {
        const size_t str_len = std::strlen(str);
        const size_t copy_len = (len + str_len < STR_SIZE) ? str_len : (STR_SIZE - 1 - len);
        std::memcpy(data + len, str, copy_len);
        len += copy_len;
        data[len] = '\0';
    }

    /** @brief Append a C-string (alias for append). */
    constexpr TinyString& operator+=(const char* str) {
        append(str);
        return *this;
    }

    /**
     * @brief Assign raw bytes (useful for deserialization).
     * @param str Pointer to characters (not required to be null-terminated).
     * @param _len Number of bytes to copy; truncated to capacity.
     */
    constexpr TinyString& assign(const char* str, size_t _len) {
      len = _len >= STR_SIZE ? STR_SIZE - 1 : _len;
      std::memcpy(data, str, len);
      data[len] = '\0';
      return *this;
     }

    /** @brief Append another TinyString. */
    constexpr TinyString& operator+=(const TinyString& other) {
        append(other.c_str());
        return *this;
    }

    /** @brief Copy assignment. */
    constexpr TinyString& operator=(const TinyString& other) {
        if (this != &other) {
            len = other.len;
            std::memcpy(data, other.data, len);
            data[len] = '\0';
        }
        return *this;
    }

    /** @brief Assign from null-terminated C-string (truncates to capacity). */
    constexpr TinyString& operator=(const char* str) {
        copyFrom(str);
        return *this;
    }

    /** @brief Equality comparison with another TinyString. */
    constexpr bool operator==(const TinyString& other) const {
        return (len == other.len) && (std::memcmp(data, other.data, len) == 0);
    }

    /** @brief Equality comparison with a C-string literal. */
    constexpr bool operator==(const char* str) const {
        const size_t str_len = std::strlen(str);
        return (len == str_len) && (std::memcmp(data, str, len) == 0);
    }
};

template <size_t STO_SIZE, size_t STR_SIZE>
SimpleMemoryPool<STO_SIZE> TinyString<STO_SIZE, STR_SIZE>::memPool;
