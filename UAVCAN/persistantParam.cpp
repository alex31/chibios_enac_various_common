#include "persistantParam.hpp"
#include <bit>  // For std::bit_cast

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
  consteval std::int64_t computeParamsListCRC()
  {
    std::uint32_t crc = 0xFFFFFFFFu; // initial
    for (auto&& entry : params_list) {
      crc = hashParamDefaultEntry(crc, entry);
    }
    return ~crc; // final XOR
  }

  // does not use heap, custom allocator will use statically
  // defined memory area
  struct Overload {
    void operator()(StoredValue& sv, NoValue n) const {
      sv = n;
    }
    void operator()(StoredValue& sv, Integer i) const {
      sv = i;
    }
    void operator()(StoredValue& sv, bool b) const {
      sv = b;
    }
    void operator()(StoredValue& sv, float f) const {
      sv = f;
    }
    void operator()(StoredValue& sv, const frozen::string& s) const {
      sv = new StoredString (s.data()); // no heap harmed during this allocation
    }
  };


  void Parameter::populateDefaults()
  {
    size_t index = 0;
    for (const auto& [_, variant] : frozenParameters) {  
      variant.v.visit([&](const auto& value) {
	Overload{}(storedParamsList[index++], value);  
      });
    }
    const auto& crc32 = Persistant::Parameter::cfind("CONST.PARAMETERS.CRC32");
    set(crc32, computeParamsListCRC());
  }

 struct EnforceMinMax {
   void operator()(StoredValue&, NoValue,  NoValue) const {
    }
   void operator()(StoredValue& sv, NoValue,  Integer max) const {
     sv = std::min(max, std::get<Integer>(sv));
    }
   void operator()(StoredValue& sv, NoValue,  float max) const {
     sv = std::min(max, std::get<float>(sv));
    }
   void operator()(StoredValue& sv, Integer min, NoValue) const {
     sv = std::max(min, std::get<Integer>(sv));
    }
   void operator()(StoredValue& sv, float min , NoValue) const {
      sv = std::max(min, std::get<float>(sv));
    }
   void operator()(StoredValue&, float, Integer) const {
     chDbgAssert(false,  "internal fault");
    }
   void operator()(StoredValue&, Integer, float) const {
     chDbgAssert(false,  "internal fault");
    }
   void operator()(StoredValue& sv, Integer min, Integer max) const {
     sv = std::max(min, std::min(std::get<Integer>(sv), max));
    }
   void operator()(StoredValue& sv, float min, float max) const {
     sv = std::max(min, std::min(std::get<float>(sv), max));
   }
 };

  void Parameter::enforceMinMax(size_t index)
  {
    const auto& variant = std::next(frozenParameters.begin(), index)->second;
    std::visit([&](const auto& min, const auto& max) {
      EnforceMinMax{}(storedParamsList[index], min, max);  
    }, variant.min, variant.max);
  }

  void Parameter::enforceMinMax()
  {
    for (size_t index = 0; index < params_list_len; index++) {
      enforceMinMax(index);
    }

    //   size_t index = 0;
    // for (const auto& [_, variant] : frozenParameters) {  
    //   std::visit([&](const auto& min, const auto& max) {
    // 	EnforceMinMax{}(storedParamsList[index++], min, max);  
    //   }, variant.min, variant.max);
    // }
  }

  /**
   * @brief Converts a StoredValue (std::variant) into a UAVCAN Value structure.
   *
   * This function takes a `StoredValue`, which is a `std::variant` containing different types,
   * and converts it into the corresponding `uavcan_protocol_param_Value` structure.
   *
   * @param storedValue The `StoredValue` to convert.
   * @param[out] uavcanValue The resulting UAVCAN structure.
   */
  void toUavcan(const StoredValue& storedValue, uavcan_protocol_param_Value& uavcanValue)
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
      } else if constexpr (std::is_same_v<T, StoredString*>) {
	uavcanValue.union_tag = UAVCAN_PROTOCOL_PARAM_VALUE_STRING_VALUE;
	if (val) {
	  uavcanValue.string_value.len = std::min<uint8_t>(val->size(), 128);
	  std::memcpy(uavcanValue.string_value.data, val->c_str(), uavcanValue.string_value.len);
	} else {
	  uavcanValue.string_value.len = 0;
	}
      }
    });
  }



  /**
   * @brief Converts a UAVCAN Value structure into a StoredValue (std::variant).
   *
   * This function takes a `uavcan_protocol_param_Value` structure and converts it into a `StoredValue`.
   * If the `StoredValue` is a string, it modifies the existing `StoredString` in place.
   *
   * @param uavcanValue The UAVCAN structure to convert.
   * @param[out] storedValue The resulting `StoredValue`, updated in place.
   */
  bool fromUavcan(const uavcan_protocol_param_Value& uavcanValue, StoredValue& storedValue)
  {
    switch (uavcanValue.union_tag) {
    case UAVCAN_PROTOCOL_PARAM_VALUE_EMPTY:
      storedValue = NoValue{};
      break;
    case UAVCAN_PROTOCOL_PARAM_VALUE_INTEGER_VALUE:
      if (std::holds_alternative<Integer>(storedValue)) {
	storedValue = uavcanValue.integer_value;
      } else {
	return false;
      }
      break;
    case UAVCAN_PROTOCOL_PARAM_VALUE_REAL_VALUE:
      if (std::holds_alternative<float>(storedValue)) {
      storedValue = uavcanValue.real_value;
      } else {
	return false;
      }
      break;
    case UAVCAN_PROTOCOL_PARAM_VALUE_BOOLEAN_VALUE:
      if (std::holds_alternative<bool>(storedValue)) {
	storedValue = static_cast<bool>(uavcanValue.boolean_value);
      } else {
	return false;
      }
      break;
    case UAVCAN_PROTOCOL_PARAM_VALUE_STRING_VALUE:
      if (std::holds_alternative<StoredString*>(storedValue)) {
        StoredString* strPtr = std::get<StoredString*>(storedValue);
	if (!strPtr) {
	  	return false;
	}
	strPtr->assign(reinterpret_cast<const char*>(uavcanValue.string_value.data), 
		       uavcanValue.string_value.len);
      } else {
	return false;
      }
      break;
    default:
      return false;
    }
    return true;
  }

  /**
   * @brief Converts a NumericValue (std::variant) into a UAVCAN NumericValue structure.
   * @param numericValue The NumericValue to convert.
   * @param uavcanValue The resulting UAVCAN structure.
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
   * @brief Converts a UAVCAN NumericValue structure into a NumericValue (std::variant).
   * @param uavcanValue The UAVCAN structure to convert.
   * @param numericValue The resulting NumericValue.
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
      numericValue = NoValue{}; // Fallback to NoValue in case of an invalid type
      return false;
    }
    return true;
  }

  /*
    Get or set a parameter by name or by index.
    Note that access by index should only be used to retrieve the list of parameters; it is highly
    discouraged to use it for anything else, because persistent ordering is not guaranteed.


    implementors may choose to make parameter set operations be immediately persistent, or can choose
    to make them temporary, requiring a ExecuteOpcode with OPCODE_SAVE to put into persistent storage


    Index of the parameter starting from 0; ignored if name is nonempty.
    Use index only to retrieve the list of parameters.
    Parameter ordering must be well defined (e.g. alphabetical, or any other stable ordering),
    in order for the index access to work.


    If set - parameter will be assigned this value, then the new value will be returned.
    If not set - current parameter value will be returned.
    Refer to the definition of Value for details.




    Actual parameter value.

    For set requests, it should contain the actual parameter value after the set request was
    executed. The objective is to let the client know if the value could not be updated, e.g.
    due to its range violation, etc.

    Empty value (and/or empty name) indicates that there is no such parameter.
  */

  StoreData getSetResponse(const uavcan_protocol_param_GetSetRequest &req,
			   uavcan_protocol_param_GetSetResponse& resp)
  {
    // request by name
    StoreData ret = {-1L, {}};
    uint16_t index = req.index;
    
    if (req.name.len != 0) {
      memcpy(&resp.name, &req.name, sizeof(req.name));
      // it's by name let's find the corresponding index
      // don't know if string is null terminated, so make it
      resp.name.len = req.name.len;
      resp.name.data[std::min<uint16_t>(resp.name.len, sizeof(resp.name.data) -1U)] = 0;
      index = Parameter::findIndex(resp.name.data);
      if (index == 0) {
	resp.value.union_tag = 
	  resp.default_value.union_tag = UAVCAN_PROTOCOL_PARAM_VALUE_EMPTY;
	resp.max_value.union_tag = 
	  resp.min_value.union_tag = UAVCAN_PROTOCOL_PARAM_NUMERICVALUE_EMPTY;
	return ret;
      }
      // if index is found, indicate it in response
    } else {
      // request by index
      const auto& paramName =  std::next(frozenParameters.begin(), index)->first;
      resp.name.len = paramName.size();
      memcpy(resp.name.data, paramName.data(),
	     std::min<uint16_t>(sizeof(resp.name.data), resp.name.len + 1U));
    }
    // now we use index either found by name or directly given in message
    // is it a set_and_request or just request ?
    const auto& [stored, deflt] = Parameter::find(index);
    if (req.value.union_tag != UAVCAN_PROTOCOL_PARAM_VALUE_EMPTY) {
      fromUavcan(req.value, stored);
      Parameter::enforceMinMax(index);
      ret = {index, stored}; // responsability to the caller to make the permanent store
    }
    // fill all fields of response
    toUavcan(stored, resp.value);
    toUavcan(deflt.v, resp.default_value);
    toUavcan(deflt.min, resp.min_value);
    toUavcan(deflt.max, resp.max_value);

    return ret;
  }


#include <variant>
#include <vector>
#include <cstring>




  /**
   * @brief Serializes a StoredValue into a byte buffer.
   * @param index The index of StoredValue to serialize.
   * @param buffer The output buffer where the serialized data will be stored.
   */
  void Parameter::serializeStoredValue(size_t index, StoreSerializeBuffer& buffer)
  {
    buffer.clear();

    // store parameter name first
    const frozen::string& paramName = Parameter::findName(index);
    // store the length of the parameter name
    buffer.push_back(paramName.size() + 1);
    // then store the name
    buffer.insert(buffer.end(), paramName.begin(),  paramName.end());
    buffer.push_back(0);
    
    // then store parameter
    const auto & [value, _] = find(index);
    uint8_t type_id = static_cast<uint8_t>(value.index()); // Store type index
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
      } else if constexpr (std::is_same_v<T, StoredString*>) {
	if (val) {
	  uint8_t len = std::min<uint8_t>(val->size(), val->capacity());
	  buffer.push_back(len);
	  buffer.insert(buffer.end(), val->c_str(), val->c_str() + len);
	} else {
	  buffer.push_back(0);
	}
      }
      // NoValue does not require extra storage
    });
  }

  /**
   * @brief Deserializes a StoredValue from a byte buffer.
   * @param buffer The input buffer containing serialized data.
   * @param index The index of output StoredValue that will be reconstructed.
   */
  std::span<const uint8_t>
  Parameter::deserializeGetName(const StoreSerializeBuffer& buffer)
  {
    chDbgAssert(!buffer.empty(), "Buffer is empty!");
    // unstore parameter name first
    // unstore the length of the parameter name
    const size_t paramNameLen = buffer[0];
    const std::span<const uint8_t> paramName(std::next(buffer.begin(), 1), paramNameLen);

    return paramName;
  }

  bool
  Parameter::deserializeStoredValue(size_t index, const StoreSerializeBuffer& buffer)
  {
    if (buffer.empty()) {
      return false;
    }
    // unstore parameter name first
    // unstore the length of the parameter name
    const size_t paramNameLen = buffer[0];
    size_t currentIndex = paramNameLen + 1;
    const auto & [value, _] = find(index);
    uint8_t type_id = buffer[currentIndex++];
    const uint8_t* data = buffer.data() + currentIndex;
    
    switch (type_id) {
    case 0: // NoValue
      value = NoValue{};
      break;
    case 1: // Integer
      chDbgAssert(buffer.size() >= 2 + paramNameLen + sizeof(Integer), "buffer size to small");
      if (std::holds_alternative<Integer>(value)) {
	// hint the compiler that values are not properly aligned in the serialized store
	value = std::bit_cast<Integer>(*reinterpret_cast<const std::array<std::byte,
				       sizeof(Integer)>*>(data));
      } else {
	return false;
      }
      break;
    case 2: // float
      chDbgAssert(buffer.size() >= 2 + paramNameLen + sizeof(float), "buffer size to small");
      if (std::holds_alternative<float>(value)) {
	// hint the compiler that values are not properly aligned in the serialized store
	value = std::bit_cast<float>(*reinterpret_cast<const std::array<std::byte,
				       sizeof(float)>*>(data));
      } else {
	return false;
      }
      break;
    case 3: // bool
      chDbgAssert(buffer.size() >= 3 + paramNameLen, "buffer size to small");
       if (std::holds_alternative<bool>(value)) {
	// hint the compiler that values are not properly aligned in the serialized store
	 value = std::bit_cast<bool>(*reinterpret_cast<const std::array<std::byte,
				      sizeof(bool)>*>(data));
       } else {
	return false;
      }
      break;
    case 4: // StoredString*
      if (std::holds_alternative<StoredString*>(value)) {
	StoredString* strPtr = std::get<StoredString*>(value);
	if (strPtr) {
	  uint8_t len = std::min<uint8_t>(data[0], strPtr->capacity());
	  strPtr->assign(reinterpret_cast<const char*>(data + 1), len);
	}
      } else {
	return false;
      }
      break;
    default:
      chDbgAssert(false, "Unknown type identifier in serialization!");
      value = NoValue{};
      return false;
    }
    
    return true;
  }


  bool
  Parameter::deserializeStoredValue(StoredValue& value, const StoreSerializeBuffer& buffer)
  {
    if (buffer.empty()) {
      return false;
    }
    // unstore parameter name first
    // unstore the length of the parameter name
    const size_t paramNameLen = buffer[0];
    size_t currentIndex = paramNameLen + 1;
    uint8_t type_id = buffer[currentIndex++];
    const uint8_t* data = buffer.data() + currentIndex;

    switch (type_id) {
    case 0: // NoValue
      value = NoValue{};
      break;
    case 1: // Integer
      chDbgAssert(buffer.size() >= 2 + paramNameLen + sizeof(Integer), "buffer size to small");
      // hint the compiler that values are not properly aligned in the serialized store
      value = std::bit_cast<Integer>(*reinterpret_cast<const std::array<std::byte,
				     sizeof(Integer)>*>(data));
      break;
    case 2: // float
      chDbgAssert(buffer.size() >= 2 + paramNameLen + sizeof(float), "buffer size to small");
      // hint the compiler that values are not properly aligned in the serialized store
      value = std::bit_cast<float>(*reinterpret_cast<const std::array<std::byte,
				   sizeof(float)>*>(data));
      break;
    case 3: // bool
      chDbgAssert(buffer.size() >= 3 + paramNameLen, "buffer size to small");
	// hint the compiler that values are not properly aligned in the serialized store
      value = std::bit_cast<bool>(*reinterpret_cast<const std::array<std::byte,
				  sizeof(bool)>*>(data));
      break;
    case 4: // StoredString*
      if (std::holds_alternative<StoredString*>(value)) {
	StoredString* strPtr = std::get<StoredString*>(value);
	if (strPtr) {
	  uint8_t len = std::min<uint8_t>(data[1], strPtr->capacity());
	  strPtr->assign(reinterpret_cast<const char*>(data + 1), len);
	}
      } else {
	return false;
      }
      break;
    default:
      chDbgAssert(false, "Unknown type identifier in serialization!");
      value = NoValue{};
      return false;
    }
    
    return true;
  }


  std::array<StoredValue, params_list_len> Parameter::storedParamsList;
  //  EepromStoreHandle Parameter::eepromHandle = {};
}
