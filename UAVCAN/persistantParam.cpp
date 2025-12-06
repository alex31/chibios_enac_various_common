/**
 * @file persistantParam.cpp
 * @brief Runtime helpers for flat-buffer parameter storage: init, clamp, (de)serialize.
 *
 * This file binds the constexpr layout from persistantParam.hpp to actual runtime logic:
 *  - populateDefaults() constructs all defaults in-place into the flat buffer,
 *  - enforceMinMax() clamps numeric values using ParamDefault ranges,
 *  - UAVCAN conversion helpers operate on StoredValue proxies,
 *  - serialization/deserialization uses the same on-wire format as before.
 */
#include "persistantParam.hpp"
#include <cstring>
#include <algorithm>
#include <memory>

#if defined(__x86_64__) && defined(__linux__)
#define TARGET_LINUX_X86_64
#include <cassert>
#define chDbgAssert(c,m) assert(c && m)
#elif defined(__arm__) || defined(__aarch64__) || defined(__ARM_ARCH)
#define TARGET_ARM_BARE_METAL
#include "ch.h"
#else
#error "Unsupported architecture!"
#endif

namespace Persistant {
  /**
   * @brief Compute a CRC over the compile-time parameter table.
   * @return Two's complement CRC32 of `params_list`.
   * @details Used to detect mismatches between persisted data and the compiled defaults.
   */
  consteval std::int64_t computeParamsListCRC()
  {
    std::uint32_t crc = 0xFFFFFFFFu;
    for (auto&& entry : params_list) {
      crc = hashParamDefaultEntry(crc, entry);
    }
    return ~crc;
  }

  /**
   * @brief Construct all defaults in-place inside the flat buffer.
   * @note Strings are built directly into their dedicated slot; CRC param filled afterwards.
   */
  void Parameter::populateDefaults()
  {
    for (size_t index = 0; index < params_list_len; index++) {
      const auto &def = std::next(frozenParameters.begin(), index)->second.v;
      std::byte *ptr = ramStore.data() + layoutInfo.entries[index].offset;

      def.visit([&](const auto &value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, NoValue>) {
          // nothing to construct
        } else if constexpr (std::is_same_v<T, Integer>) {
          std::construct_at(reinterpret_cast<Integer *>(ptr), value);
        } else if constexpr (std::is_same_v<T, float>) {
          std::construct_at(reinterpret_cast<float *>(ptr), value);
        } else if constexpr (std::is_same_v<T, bool>) {
          std::construct_at(reinterpret_cast<bool *>(ptr), value);
        } else if constexpr (std::is_same_v<T, frozen::string>) {
          std::construct_at(reinterpret_cast<StoredString *>(ptr), value.data());
        }
      });
    }
    static_assert(Persistant::Parameter::findIndex("const.parameters.crc32") > 0);
    const auto &crc32 = Persistant::Parameter::cfind("const.parameters.crc32");
    set(crc32, computeParamsListCRC());
  }

  /**
   * @brief Clamp a single parameter against its declared min/max.
   * @param index Index into `params_list`.
   * @details No-ops for bool/string/empty kinds; integers and floats are clamped in place.
   */
  void Parameter::enforceMinMax(size_t index)
  {
    auto [value, variant] = Parameter::find(index);
    switch (layoutInfo.entries[index].kind) {
    case ValueKind::Int: {
      auto &sv = value.get<Integer>();
      if (std::holds_alternative<Integer>(variant.min)) {
        sv = std::max(sv, std::get<Integer>(variant.min));
      }
      if (std::holds_alternative<Integer>(variant.max)) {
        sv = std::min(sv, std::get<Integer>(variant.max));
      }
      break;
    }
    case ValueKind::Real: {
      auto &sv = value.get<float>();
      if (std::holds_alternative<float>(variant.min)) {
        sv = std::max(sv, std::get<float>(variant.min));
      }
      if (std::holds_alternative<float>(variant.max)) {
        sv = std::min(sv, std::get<float>(variant.max));
      }
      break;
    }
    default:
      break;
    }
  }

  /**
   * @brief Clamp every parameter; typically called after deserialization.
   */
  void Parameter::enforceMinMax()
  {
    for (size_t index = 0; index < params_list_len; index++) {
      enforceMinMax(index);
    }
  }

  /**
   * @brief Serialize an in-RAM parameter into a UAVCAN union.
   * @param storedValue Proxy bound to the flat buffer.
   * @param uavcanValue Output UAVCAN value.
   */
  void toUavcan(const StoredValue &storedValue, uavcan_protocol_param_Value &uavcanValue)
  {
    storedValue.visit([&](const auto& val) {
      using T = std::decay_t<decltype(val)>;
      if constexpr (std::is_same_v<T, NoValue>) {
        uavcanValue.union_tag = UAVCAN_PROTOCOL_PARAM_VALUE_EMPTY;
      } else if constexpr (std::is_same_v<T, Integer>) {
        uavcanValue.union_tag = UAVCAN_PROTOCOL_PARAM_VALUE_INTEGER_VALUE;
        uavcanValue.integer_value = val;
      } else if constexpr (std::is_same_v<T, float>) {
        uavcanValue.union_tag = UAVCAN_PROTOCOL_PARAM_VALUE_REAL_VALUE;
        uavcanValue.real_value = val;
      } else if constexpr (std::is_same_v<T, bool>) {
        uavcanValue.union_tag = UAVCAN_PROTOCOL_PARAM_VALUE_BOOLEAN_VALUE;
        uavcanValue.boolean_value = val;
      } else if constexpr (std::is_same_v<T, StoredString>) {
        uavcanValue.union_tag = UAVCAN_PROTOCOL_PARAM_VALUE_STRING_VALUE;
        uavcanValue.string_value.len = std::min<uint8_t>(val.size(), 128);
        std::memcpy(uavcanValue.string_value.data, val.c_str(), uavcanValue.string_value.len);
      }
    });
  }

  /**
   * @brief Deserialize a UAVCAN union into a StoredValue.
   * @param uavcanValue Incoming wire value.
   * @param storedValue Destination proxy; must have matching kind.
   * @return true on success, false on type mismatch.
   */
  bool fromUavcan(const uavcan_protocol_param_Value& uavcanValue, StoredValue& storedValue)
  {
    switch (uavcanValue.union_tag) {
    case UAVCAN_PROTOCOL_PARAM_VALUE_EMPTY:
      if (storedValue.kind() == ValueKind::None) {
        return true;
      }
      if (storedValue.kind() == ValueKind::Str) {
        storedValue.get<StoredString>().clear();
        return true;
      }
      return false;
    case UAVCAN_PROTOCOL_PARAM_VALUE_INTEGER_VALUE:
      if (storedValue.kind() == ValueKind::Int) {
        storedValue.get<Integer>() = uavcanValue.integer_value;
        return true;
      }
      return false;
    case UAVCAN_PROTOCOL_PARAM_VALUE_REAL_VALUE:
      if (storedValue.kind() == ValueKind::Real) {
        storedValue.get<float>() = uavcanValue.real_value;
        return true;
      }
      return false;
    case UAVCAN_PROTOCOL_PARAM_VALUE_BOOLEAN_VALUE:
      if (storedValue.kind() == ValueKind::Bool) {
        storedValue.get<bool>() = static_cast<bool>(uavcanValue.boolean_value);
        return true;
      }
      return false;
    case UAVCAN_PROTOCOL_PARAM_VALUE_STRING_VALUE:
      if (storedValue.kind() == ValueKind::Str) {
        auto &str = storedValue.get<StoredString>();
        uint8_t len = std::min<uint8_t>(uavcanValue.string_value.len, str.capacity());
        str.assign(reinterpret_cast<const char *>(uavcanValue.string_value.data), len);
        return true;
      }
      return false;
    default:
      return false;
    }
  }

  /**
   * @brief Serialize NumericValue (min/max) variant into UAVCAN.
   */
  void toUavcan(const NumericValue& numericValue, uavcan_protocol_param_NumericValue& uavcanValue)
  {
    numericValue.visit([&](const auto& val) {
      using T = std::decay_t<decltype(val)>;

      if constexpr (std::is_same_v<T, NoValue>) {
        uavcanValue.union_tag = UAVCAN_PROTOCOL_PARAM_NUMERICVALUE_EMPTY;
      } else if constexpr (std::is_same_v<T, Integer>) {
        uavcanValue.union_tag = UAVCAN_PROTOCOL_PARAM_NUMERICVALUE_INTEGER_VALUE;
        uavcanValue.integer_value = val;
      } else if constexpr (std::is_same_v<T, float>) {
        uavcanValue.union_tag = UAVCAN_PROTOCOL_PARAM_NUMERICVALUE_REAL_VALUE;
        uavcanValue.real_value = val;
      }
    });
  }

  /**
   * @brief Serialize a FrozenDefault into UAVCAN representation.
   */
  void toUavcan(const FrozenDefault& defaultValue, uavcan_protocol_param_Value& uavcanValue)
  {
    defaultValue.visit([&](const auto& val) {
      using T = std::decay_t<decltype(val)>;
      if constexpr (std::is_same_v<T, NoValue>) {
        uavcanValue.union_tag = UAVCAN_PROTOCOL_PARAM_VALUE_EMPTY;
      } else if constexpr (std::is_same_v<T, Integer>) {
        uavcanValue.union_tag = UAVCAN_PROTOCOL_PARAM_VALUE_INTEGER_VALUE;
        uavcanValue.integer_value = val;
      } else if constexpr (std::is_same_v<T, float>) {
        uavcanValue.union_tag = UAVCAN_PROTOCOL_PARAM_VALUE_REAL_VALUE;
        uavcanValue.real_value = val;
      } else if constexpr (std::is_same_v<T, bool>) {
        uavcanValue.union_tag = UAVCAN_PROTOCOL_PARAM_VALUE_BOOLEAN_VALUE;
        uavcanValue.boolean_value = val;
      } else if constexpr (std::is_same_v<T, frozen::string>) {
        uavcanValue.union_tag = UAVCAN_PROTOCOL_PARAM_VALUE_STRING_VALUE;
        uavcanValue.string_value.len = std::min<uint8_t>(val.size(), 128);
        std::memcpy(uavcanValue.string_value.data, val.data(), uavcanValue.string_value.len);
      }
    });
  }

  /**
   * @brief Deserialize a UAVCAN NumericValue into a NumericValue variant.
   * @return true when a recognized union tag is received.
   */
  bool fromUavcan(const uavcan_protocol_param_NumericValue& uavcanValue, NumericValue& numericValue)
  {
    switch (uavcanValue.union_tag) {
    case UAVCAN_PROTOCOL_PARAM_NUMERICVALUE_EMPTY:
      numericValue = NoValue{};
      break;
    case UAVCAN_PROTOCOL_PARAM_NUMERICVALUE_INTEGER_VALUE:
      numericValue = uavcanValue.integer_value;
      break;
    case UAVCAN_PROTOCOL_PARAM_NUMERICVALUE_REAL_VALUE:
      numericValue = uavcanValue.real_value;
      break;
    default:
      numericValue = NoValue{};
      return false;
    }
    return true;
  }

  /**
   * @brief Handle UAVCAN GetSet: resolve name/index, optionally update, and build response.
   * @param req Incoming GetSet request.
   * @param resp Outgoing response (name, current value, defaults, min/max).
   * @return {-1, {}} when no value updated, otherwise {index, StoredValue after update}.
   */
  StoreData getSetResponse(const uavcan_protocol_param_GetSetRequest &req,
                           uavcan_protocol_param_GetSetResponse& resp)
  {
    StoreData ret = {-1L, StoredValue{}};
    ssize_t index = req.index;

    if (req.name.len != 0) {
      memcpy(&resp.name, &req.name, sizeof(req.name));
      resp.name.len = req.name.len;
      resp.name.data[std::min<uint16_t>(resp.name.len, sizeof(resp.name.data) -1U)] = 0;
      index = Parameter::findIndex(resp.name.data);
    } else {
      if ((index >= 0) and (index < params_list_len)) {
        const auto& paramName =  std::next(frozenParameters.begin(), index)->first;
        resp.name.len = paramName.size();
        memcpy(resp.name.data, paramName.data(),
               std::min<uint16_t>(sizeof(resp.name.data), resp.name.len + 1U));
      }
    }

    if ((index < 0) or (index >= params_list_len)) {
      resp.name.len = req.name.len;
      memcpy(resp.name.data, req.name.data,
             std::min<uint16_t>(sizeof(resp.name.data), resp.name.len));
      resp.value.union_tag = 
        resp.default_value.union_tag = UAVCAN_PROTOCOL_PARAM_VALUE_EMPTY;
      resp.max_value.union_tag = 
        resp.min_value.union_tag = UAVCAN_PROTOCOL_PARAM_NUMERICVALUE_EMPTY;
      return ret;
    }
    auto [stored, deflt] = Parameter::find(index);
    if (req.value.union_tag != UAVCAN_PROTOCOL_PARAM_VALUE_EMPTY) {
      fromUavcan(req.value, stored);
      Parameter::enforceMinMax(index);
      ret = {index, stored};
    }
    toUavcan(stored, resp.value);
    toUavcan(deflt.v, resp.default_value);
    toUavcan(deflt.min, resp.min_value);
    toUavcan(deflt.max, resp.max_value);
    return ret;
  }

  void Parameter::serializeStoredValue(size_t index, StoreSerializeBuffer& buffer)
  {
    buffer.clear();
    const frozen::string& paramName = Parameter::findName(index);
    buffer.push_back(paramName.size() + 1);
    buffer.insert(buffer.end(), paramName.begin(),  paramName.end());
    buffer.push_back(0);
    const auto & [value, _] = find(index);
    uint8_t type_id = static_cast<uint8_t>(layoutInfo.entries[index].kind);
    buffer.push_back(type_id);

    value.visit([&](const auto& val) {
      using T = std::decay_t<decltype(val)>;
      if constexpr (std::is_same_v<T, Integer>) {
        uint8_t bytes[sizeof(Integer)];
        std::memcpy(bytes, &val, sizeof(Integer));
        buffer.insert(buffer.end(), bytes, bytes + sizeof(Integer));
      } else if constexpr (std::is_same_v<T, float>) {
        uint8_t bytes[sizeof(float)];
        std::memcpy(bytes, &val, sizeof(float));
        buffer.insert(buffer.end(), bytes, bytes + sizeof(float));
      } else if constexpr (std::is_same_v<T, bool>) {
        buffer.push_back(static_cast<uint8_t>(val));
      } else if constexpr (std::is_same_v<T, StoredString>) {
        uint8_t len = std::min<uint8_t>(val.size(), val.capacity());
        buffer.push_back(len);
        buffer.insert(buffer.end(), val.c_str(), val.c_str() + len);
      }
    });
  }

  etl::span<const uint8_t>
  Parameter::deserializeGetName(const StoreSerializeBuffer& buffer)
  {
    chDbgAssert(!buffer.empty(), "Buffer is empty!");
    const size_t paramNameLen = buffer[0];
    const etl::span<const uint8_t> paramName(std::next(buffer.begin(), 1), paramNameLen);
    return paramName;
  }

  bool
  Parameter::deserializeStoredValue(size_t index, const StoreSerializeBuffer& buffer)
  {
    if (buffer.empty()) {
      return false;
    }
    const size_t paramNameLen = buffer[0];
    size_t currentIndex = paramNameLen + 1;
    const uint8_t type_id = buffer[currentIndex++];
    const uint8_t* data = buffer.data() + currentIndex;
    const auto kind = layoutInfo.entries[index].kind;
    if (type_id != static_cast<uint8_t>(kind)) {
      return false;
    }
    auto [value, _] = Parameter::find(index);

    switch (kind) {
    case ValueKind::None:
      break;
    case ValueKind::Int: {
      chDbgAssert(buffer.size() >= 2 + paramNameLen + sizeof(Integer), "buffer size too small");
      Integer tmp;
      std::memcpy(&tmp, data, sizeof(Integer));
      value.get<Integer>() = tmp;
      break;
    }
    case ValueKind::Real: {
      chDbgAssert(buffer.size() >= 2 + paramNameLen + sizeof(float), "buffer size too small");
      float tmp;
      std::memcpy(&tmp, data, sizeof(float));
      value.get<float>() = tmp;
      break;
    }
    case ValueKind::Bool: {
      chDbgAssert(buffer.size() >= 3 + paramNameLen, "buffer size too small");
      bool tmp;
      std::memcpy(&tmp, data, sizeof(bool));
      value.get<bool>() = tmp;
      break;
    }
    case ValueKind::Str: {
      auto &str = value.get<StoredString>();
      uint8_t len = std::min<uint8_t>(data[0], str.capacity());
      str.assign(reinterpret_cast<const char*>(data + 1), len);
      break;
    }
    }
    return true;
  }

} // namespace Persistant
