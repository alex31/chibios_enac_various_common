#include "persistantParam.hpp"
#include <cassert>

namespace Persistant {

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
      std::visit([&](const auto& value) {
	Overload{}(paramList[index++], value);  
      }, variant.v);
    }
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
    std::visit([&](const auto& val) {
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
    }, storedValue);
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
  void fromUavcan(const uavcan_protocol_param_Value& uavcanValue, StoredValue& storedValue)
  {
    switch (uavcanValue.union_tag) {
    case UAVCAN_PROTOCOL_PARAM_VALUE_EMPTY:
      storedValue = NoValue{};
      break;
    case UAVCAN_PROTOCOL_PARAM_VALUE_INTEGER_VALUE:
      storedValue = uavcanValue.integer_value;
      break;
    case UAVCAN_PROTOCOL_PARAM_VALUE_REAL_VALUE:
      storedValue = uavcanValue.real_value;
      break;
    case UAVCAN_PROTOCOL_PARAM_VALUE_BOOLEAN_VALUE:
      storedValue = static_cast<bool>(uavcanValue.boolean_value);
      break;
    case UAVCAN_PROTOCOL_PARAM_VALUE_STRING_VALUE:
      if (std::holds_alternative<StoredString*>(storedValue)) {
        StoredString* strPtr = std::get<StoredString*>(storedValue);
	assert (strPtr && "strPtr should not be null*");
	strPtr->assign(reinterpret_cast<const char*>(uavcanValue.string_value.data), 
		       uavcanValue.string_value.len);
      } else {
        assert(false && "StoredValue does not hold a StoredString*");
        storedValue = NoValue{}; // Graceful fallback
      }
      break;
    default:
      assert(false && "Unknown UAVCAN value type");
      storedValue = NoValue{};
      break;
    }
  }

/**
 * @brief Converts a NumericValue (std::variant) into a UAVCAN NumericValue structure.
 * @param numericValue The NumericValue to convert.
 * @param uavcanValue The resulting UAVCAN structure.
 */
void toUavcan(const NumericValue& numericValue, uavcan_protocol_param_NumericValue& uavcanValue)
{
    std::visit([&](const auto& val) {
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
    }, numericValue);
}

void toUavcan(const Default& defaultValue, uavcan_protocol_param_Value& uavcanValue)
{    std::visit([&](const auto& val) {
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
    }, defaultValue);
}
/**
 * @brief Converts a UAVCAN NumericValue structure into a NumericValue (std::variant).
 * @param uavcanValue The UAVCAN structure to convert.
 * @param numericValue The resulting NumericValue.
 */
void fromUavcan(const uavcan_protocol_param_NumericValue& uavcanValue, NumericValue& numericValue)
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
        assert(false && "Unknown UAVCAN numeric value type");
        numericValue = NoValue{}; // Fallback to NoValue in case of an invalid type
        break;
    }
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
void getResponse(uavcan_protocol_param_GetSetRequest &req,
		 uavcan_protocol_param_GetSetResponse& resp)
{
  // request by name 
  if (req.name.len != 0) {
    memcpy(&resp.name, &req.name, sizeof(req.name));
    // it's by name let's find the corresponding index
    // don't know if string is null terminated, so make it
    req.name.data[std::min<uint16_t>(req.name.len, sizeof(req.name.data) -1U)] = 0;
    const auto index = Parameter::findIndex(req.name.data);
    if (index < 0) {
      resp.value.union_tag = 
      resp.default_value.union_tag = UAVCAN_PROTOCOL_PARAM_VALUE_EMPTY;
      resp.max_value.union_tag = 
      resp.min_value.union_tag = UAVCAN_PROTOCOL_PARAM_NUMERICVALUE_EMPTY;
      return;
    }
    // if index is found, indicate it in response
    req.index = index;
  } else {
    // request by index
    const auto& paramName =  std::next(frozenParameters.begin(), req.index)->first;
    resp.name.len = paramName.size();
    memcpy(resp.name.data, paramName.data(),
	   std::min<uint16_t>(sizeof(resp.name.data), resp.name.len + 1U));
  }
  // now we use index either found by name or directly given in message
  // is it a set_and_request or just request ?
  const auto& [stored, deflt] = Parameter::find(req.index);
  if (req.value.union_tag != UAVCAN_PROTOCOL_PARAM_VALUE_EMPTY) {
    fromUavcan(resp.value, stored);
    // TODO: write value in eeprom
  }
  // fill all fields of response
  toUavcan(stored, resp.value);
  toUavcan(deflt.v, resp.default_value);
  toUavcan(deflt.min, resp.min_value);
  toUavcan(deflt.max, resp.max_value);
}





std::array<StoredValue, params_list_len> Parameter::paramList;
}
