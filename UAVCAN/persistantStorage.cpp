#include "persistantStorage.hpp"

#include <span>
#include <cstdint>
#include <cstring>
#include <ranges>
#include <algorithm>

#if defined(__x86_64__) && defined(__linux__)
#define TARGET_LINUX_X86_64
#define DebugTrace(fmt, ...) printf (fmt "\r\n", ## __VA_ARGS__ )
#define chDbgAssert(c,m) assert(c && m)
#elif defined(__arm__) || defined(__aarch64__) || defined(__ARM_ARCH)
#define TARGET_ARM_CHIBIOS
#include "ch.h"
#include "stdutil.h"
#include "stdutil++.hpp"
#else
#error "Unsupported architecture!"
#endif


namespace {

  consteval auto operator""_u(const char* str, std::size_t len) {
    std::array<uint8_t, 128> buffer{};  // Change 8 to fit your needs
    std::size_t n = std::min(len, buffer.size()); // Avoid overflow

    for (std::size_t i = 0; i < n; ++i) {
      buffer[i] = static_cast<uint8_t>(str[i]);
    }
    return buffer;
  }
  
  constexpr std::strong_ordering compareStrSpan(
						std::span<const uint8_t> lhs, 
						std::span<const uint8_t> rhs, 
						bool use_min_length = false) 
  {
    const size_t strLhsLen = strnlen(reinterpret_cast<const char *>(lhs.data()), lhs.size());
    const size_t strRhsLen = strnlen(reinterpret_cast<const char *>(rhs.data()), rhs.size());
    // Determine the maximum comparison length
    std::size_t sublen = use_min_length ?
      std::min(strLhsLen, strRhsLen) :
      std::max(strLhsLen, strRhsLen);
    
    for (std::size_t i = 0; i < sublen; ++i) {
      // If one string runs out of characters first, it is smaller
      if (i >= lhs.size()) return std::strong_ordering::less;
      if (i >= rhs.size()) return std::strong_ordering::greater;
      
      // Compare character-by-character
      if (auto cmp = lhs[i] <=> rhs[i]; cmp != std::strong_ordering::equal) {
	return cmp;
      }
    }
    
    return std::strong_ordering::equal;  // If all compared characters are equal
  }




  
}

namespace Persistant {
  Storage::Storage(const EepromStoreHandle& _handle) : handle(_handle)
  {
    chDbgAssert(handle.writeFn != nullptr, "handle.writeFn is null");
    chDbgAssert(handle.readFn != nullptr, "handle.readFn is null");
    chDbgAssert(handle.getLen != nullptr, "handle.getLen is null");
    chDbgAssert(handle.eraseFn != nullptr, "handle.eraseFn is null");
    
  }

  bool Storage::start()
  {
    Parameter::populateDefaults();
    bool restoreStatus = restoreAll();
    if (not restoreStatus) {
      DebugTrace("DBG> total restore failed");
      partialRestore();
      Parameter::enforceMinMax();
      eraseAll();
      restoreStatus = storeAll();
    } 
    return restoreStatus;
  }
  
  bool Storage::eraseAll()
  {
#ifdef TARGET_ARM_CHIBIOS
    MutexGuard guard(mtx);
#endif
    return handle.eraseFn();
  }
  
  bool Storage::store(size_t index, const StoreSerializeBuffer& lbuffer)
  {
    return handle.writeFn(index, buffer.data(), lbuffer.size());
  }
  
  bool Storage::store(size_t index)
  {
#ifdef TARGET_ARM_CHIBIOS
    MutexGuard guard(mtx);
#endif
    Parameter::serializeStoredValue(index, buffer);
    return store(index, buffer);
  }

  bool Storage::erase(size_t index)
  {
#ifdef TARGET_ARM_CHIBIOS
    MutexGuard guard(mtx);
#endif
    return handle.eraseRecordFn(index);
  }
 
  bool Storage::restore(size_t index, StoreSerializeBuffer& lbuffer)
  {
    size_t size = buffer.capacity();
    if (handle.readFn(index, buffer.data(), size)) {
      lbuffer.uninitialized_resize(size);
      return size != 0;
    } else {
      return false;
    }
  }
  

  
  bool Storage::restore(size_t index)
  {
#ifdef TARGET_ARM_CHIBIOS
    MutexGuard guard(mtx);
#endif
    
    if (restore(index, buffer) != true) {
      return false;
    }
    const auto& name = Parameter::deserializeGetName(buffer);
    //    DebugTrace("DBG> restore name = %s", name.data());
    // for CRC32 : we don't restore, and if the value is different
    // we return false
    if (compareStrSpan(name, "const.parameters.crc32"_u) == std::strong_ordering::equal) {
      // if the crc32 is at another position something has changed
      if (strncmp(Parameter::findName(index).data(), (char *) name.data(), name.size()) != 0)
	return false;
      const auto crcp = Persistant::Parameter::find(index);
      const Integer crcBefore = Parameter::get<Integer>(crcp);
      Parameter::deserializeStoredValue(index, buffer);
      const Integer crcAfter = Parameter::get<Integer>(crcp);
      if (crcBefore != crcAfter) {
	Parameter::set(crcp, crcBefore);
	return false;
      }
    } 
    // for other parameters : we restore the ones which does
    // not begin with CONST and return true in both cases
    else if (compareStrSpan(name, "const."_u, true) != std::strong_ordering::equal) {
      Parameter::deserializeStoredValue(index, buffer);
    }
    return true;
  }
  
  bool Storage::restore(size_t frozenIndex, size_t storeIndex)
  {
    if (restore(storeIndex, buffer) != true) {
      return false;
    }
    const auto& name = Parameter::deserializeGetName(buffer);
    //    DebugTrace("DBG> restore name = %s", name.data());
    
    // discard CONST\..* parameters
    if (compareStrSpan(name, "const."_u, true) != std::strong_ordering::equal) {
      Parameter::deserializeStoredValue(frozenIndex, buffer);
    }
    return true;
  }
  
  bool Storage::storeAll()
  {
    bool success = true;
    for (size_t index=0; index < params_list_len; index++) {
      success = store(index) && success;
    }
    
    const size_t storeLen = getLen();
    for (size_t index=params_list_len; index < storeLen; index++) {
      erase(index);
    }
    
    return success;
  }

  bool Storage::restoreAll()
  {
    bool success = true;
    for (size_t index=0; index < params_list_len; index++) {
      const bool restoreSucces = restore(index);
      success = restoreSucces && success;
      if (not restoreSucces) {
	DebugTrace("*** restoreAll fail on index %u", index);
      }
    }
    return success;
  }

  size_t Storage::getLen()
  {
    return  handle.getLen();
  }
  
// Binary search function (returns index of found string or -1 if not found)
  ssize_t Storage::binarySearch(const frozen::string& ftarget)
  {
    const std::size_t total_strings = handle.getLen();
    if (total_strings == 0)
      return -1;

    std::size_t low = 0;
    std::size_t high = total_strings - 1;


    std::span<const uint8_t> target {
        reinterpret_cast<const uint8_t*>(ftarget.data()), ftarget.size()
    };
    
    while (low <= high) {
        std::size_t mid = low + (high - low) / 2;
	size_t size = buffer.capacity();
	if (not handle.readFn(mid, buffer.data(), size)) {
	  return -1;
	}
	buffer.uninitialized_resize(size);
	const auto& mid_value = Parameter::deserializeGetName(buffer);
	auto compare = compareStrSpan(mid_value, target);

        if (compare == std::strong_ordering::equal) {
            return static_cast<ssize_t>(mid);  // Found
        } else if (compare == std::strong_ordering::less) {
            low = mid + 1;  // Search right
        } else {
            high = mid - 1; // Search left
        }
    }

    return -1;  // Not found
}
  
 void Storage::partialRestore()
 {
   for (size_t idx=0; idx < params_list_len; idx++) {
     const auto& paramName = std::next(frozenParameters.begin(), idx)->first;
     const ssize_t paramIndex = binarySearch(paramName);
     if (paramIndex >= 0) {
       if (restore(idx, paramIndex) != true) {
	 DebugTrace("DBG> partial restore alt('%s') ***FAILED***", paramName.data());
       } else {
	 //	 DebugTrace("DBG> partial restore '%s' is SUCCESS", paramName.data());
       }
     } else {
       DebugTrace("DBG> %s not found in storage", paramName.data());
     }
   }
 }
  
  StoredValue Storage::get(const frozen::string& paramName)
  {
    const ssize_t index = binarySearch(paramName);
    if (index >= 0) {
      if (restore(index, buffer) != true) {
	return {};
      }
      StoredValue value;
      if (Parameter::deserializeStoredValue(value, buffer)) {
	return value;
      } else {
	return {};
      }
    } else {
      return {};
    }
  }

#ifdef TARGET_ARM_CHIBIOS
  __attribute__ ((section(DMA_SECTION), aligned(8)))
#endif
  StoreSerializeBuffer Storage::buffer;


}
