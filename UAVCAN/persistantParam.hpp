/**
 * @file persistantParam.hpp
 * @brief Persistent parameter storage and management for MCU applications.
 *
 * This module provides a parameter system that stores default values in ROM
 * using frozen::unordered_map and allows runtime modifications in RAM.
 *
 * Features:
 * - Compile-time validation of default parameters.
 * - Efficient memory allocation using a static memory pool.
 * - Optimized storage for strings using TinyString.
 * - Support for integer and floating-point values with constraints.
 * - Custom allocator for string storage in a static memory buffer.
 */

#pragma once


#include "dronecan_msgs.h"
#include <type_traits>
#include <etl/vector.h>
#include <span>
#include "persistantParamTypes.hpp"
#include "persistantParamCrc.hpp"
#include "ch.h"
#include "etl/span.h"
/*
  FAIT :


  TODO :


  ° /fix : fonctions de conversion UAV CAN : enforce minmax
 
  ° stoquer si getSet from UAVCan

  ° refaire un tour : style, nom des fonctions/methodes (restore ??)

  ° gestion CONST.PARAMETERS.CRC32 par get direct à haut niveau plutôt
  que dans persistantStorage.cpp ?

  ° gain en mémoire : remplacer le stockage d'un variant par le stockage du type
  reel, en ayant un calcul constexpr de la place prise dimensionant un memory pool.
  Pour chaque paramètre, on a une fonction constexpr qui donne son adresse dans le tableau.
  Si le nom est literal, on force la resolution consteval du calcul de l'adresse, sion on parcourt
  la map frozen.

*/

/*

Using gcc15.2 in C++26 mode :
  
in /home/alex/DEV/STM32/CHIBIOS/COMMON/various/UAVCAN directory, there is a module that store persistant
  parameters : persistantParam.hpp + persistantParam.cpp .
  The parameters name are constant described by a
  frozen::map of variant. We find the name, minmum, maximun and
  default value. Associated are values, that can be changed, and are
  in ram. The actual code is not optimal in size (target = small MCU)
  because the ram value is also stored as a variant, and take 16 bytes
  when most parameters are integer values that occupy 4 bytes on arm32.
  My idea is to store all
  ram value in a flat dedicated space. whole size can be calculated at
  compile time adding each size for the frozen map entries. Also,
  given a key in the map, we can compute the address in the ram store
  where is the data, at compile time. We can generate at compile a frozen map key -> ram address so that we can easyly find ram address at runtime given the key (which is parameter name) or if we want to save flash size, we also can compute this at runtime but we have to walk thru the entire map to sum the size and compute the address in the flat ram table.

  With this architecture, we should not need anymore getTinyStringMemoryPoolSize() since string will be associated with a flat space in this ram table.

  example

  ° baudrate 19200 // is an integer : 4 bytes : address in the ram store 0
  ° ROLE.gps true // is bool : 1 byte : address in the ram store 4
  ° node.name "MINICAN" :  TinyString : 47 bytes : address in the ram store 5
  ° pid.k : float : 4 bytes : address in the ram store 52
  ° total size 56 bytes


  Ideally, I would made internal changes to save RAM, but keeping the externam API so that
  all the modules that use the API continue to compile cleanly.

  What do you think of that idea ?
  do you understand my demand ?
  do you want I explain points and details ?
  do you think you are up to the task ?

  - Question : Rough count/types of parameters and whether any
   non–trivially-copyable types beyond TinyString/bool/int/float
   exist.

   - Answer : you will find modules that use theses API in COMMON/source, and the
  actual list of parameters is in COMMON/source/nodeParameters.hpp.

   - Question : Current API surface you need preserved (get/set by name?
   iterating? serialization?).

   - Answer :  I want to preserve all API surface, at least all the surface that is
  used in the modules. 

  -Question : Alignment requirements for the flat  buffer (e.g., 4-byte align for ints/floats).
   - Answer : No alignment required, armv7-m can manage
  unaligned access at the price of a slighty more cpu cycles, but
  access are scarce, and when value is needed in a scarce loop, it is
  cache in a local variable.

  
  -Question : Acceptable trade-off: more flash for a precomputed frozen offset map
    vs. small runtime cost to sum sizes. Which do you prefer?

  - Answer :  For the trade off, I don't know, it will
  depend on the size that the map will occupy. In fact most of the map
  size will be occupy by the key (frozen string), if there is a
  possibility that theses string are shared between the two frozen
  map, the index map will not cost a lot

 if you ask yoursel question about the variant visit method : I use g++15.2 in gnu++26 mode,
 so the visit method, which is a c++26 addiction, is available. Consider you can
  use all c++26 options that are available with gcc15.2


Questions :
  1. Precomputed frozen offset map vs. runtime prefix-sum walk?
  2. OK to ship a small proxy that mimics the variant surface (to avoid touching callers), or should I instead patch the couple of holds_alternative/visit uses?

Answers :
 1/when the retreive function is called for a constant key, we don't need holds_alternative/visit since the type is known at compile time. But when we call the API
  with a runtime key, the type is unknown and the variant API is the best, is is well known from programmers, son don't reinvent the wheel.

  2/ Give a try to precomputed frozen map : To save space, it could be a map "index of parameter" -> "ram offset" instead of "name" -> "ram offset". You first get the index from the big frozen map,
  then ram offset from the smaller offset map



  
 */

namespace Persistant {
  

  /**
   * @brief Compute the required memory for string parameters.
   * @return The total size needed for string storage.
   */
  static consteval size_t getTinyStringMemoryPoolSize() {
    struct Overload {
      constexpr size_t operator()(Persistant::NoValue) const { return 0; }
      constexpr size_t operator()(Integer) const { return 0; }
      constexpr size_t operator()(float) const { return 0; }
      constexpr size_t operator()(bool) const { return 0; }
      constexpr size_t operator()(const frozen::string &) const {
	return sizeof(TinyString<0, tinyStrSize>);
      }
    };

    size_t size = 0;
    for (size_t i = 0; i < params_list_len; i++) {
      params_list[i].second.v.visit([&](const auto &param) { size += Overload{}(param); });
    }

    return size;
  }

  using StoredString = TinyString<getTinyStringMemoryPoolSize(), tinyStrSize>;
  using StoredValue = std::variant<NoValue, Integer, float, bool, StoredString *>;
  using StoreData = std::pair<ssize_t, StoredValue>;
  using StoreSerializeBuffer = etl::vector<uint8_t, 256>;
  /**
   * @brief Compile-time validation of a parameter entry.
   * @param param The parameter definition to validate.
   * @note    min and max are else NoValue or the same type as v
   *		neither min or max for string parameters
   *		default value is in the range min..max it they are supplied
   * @return True if valid, false otherwise.
   */
  consteval bool isValidDefault(const ParamDefault &param) {
    return param.v.visit(
		      [&](const auto &v) -> bool {
			using T = std::decay_t<decltype(v)>;

			if constexpr (std::is_same_v<T, Integer> || std::is_same_v<T, float>) {
			  return std::visit(
					    [&](const auto &min_val, const auto &max_val) -> bool {
					      using MinT = std::decay_t<decltype(min_val)>;
					      using MaxT = std::decay_t<decltype(max_val)>;

					      if constexpr (std::is_same_v<MinT, T>) {
						if (v < min_val)
						  return 0;
					      }
					      if constexpr (std::is_same_v<MaxT, T>) {
						if (v > max_val)
						  return 0;
					      }
					      return (std::is_same_v<MinT, NoValue> ||
						      std::is_same_v<MinT, T>) &&
						(std::is_same_v<MaxT, NoValue> ||
						 std::is_same_v<MaxT, T>);
					    },
					    param.min, param.max);

			} else {
			  return std::visit(
					    [](const auto &min_val, const auto &max_val) -> bool {
					      using MinT = std::decay_t<decltype(min_val)>;
					      using MaxT = std::decay_t<decltype(max_val)>;

					      return (std::is_same_v<MinT, NoValue>) &&
						(std::is_same_v<MaxT, NoValue>);
					    },
					    param.min, param.max);
			}
		      });
  }

  /**
   * @brief Compile-time validation of the entire parameter list.
   * @param paramsPairList The list of parameter definitions.
   * @return The index of the first invalid entry, or -1 if all are valid.
   */
  consteval int validateDefaultsList(const auto &paramsPairList) {
    int index = 0;
    for (const auto &entry : paramsPairList) {
      if (!isValidDefault(entry.second))
	return index;
      index++;
    }
    return -1;
  }

  /// Static assert at compile-time
  /// if a check fail, compiler error message will show the first
  /// offending entry, numbered from 0
  static_assert(validateDefaultsList(params_list) < 0,
		"❌ params_list contains invalid ParamDefault entries!");

  /// Frozen map for fast parameter lookup.
  inline constexpr const auto frozenParameters = frozen::make_map(params_list);

  class Parameter {
  public:
    Parameter() = delete;

    /**
     * @brief Find the index of a parameter by name.
     * @param key The parameter name.
     * @return The index of the parameter, or -1 if not found.
     */
    constexpr static ssize_t findIndex(const frozen::string& key);
    consteval static ssize_t cfindIndex(const frozen::string& key) {
      return findIndex(key);
    }

    constexpr static ssize_t findIndex(const uint8_t *str) {
      return findIndex(frozen::string(reinterpret_cast<const char *>(str)));
    }

    constexpr static ssize_t findIndex(const char *str) {
      return findIndex(frozen::string(str));
    }

    constexpr static const frozen::string& findName(const size_t index);
  
    /**
     * @brief Retrieve a parameter by index.
     * @param index The parameter index.
     * @return A reference to the stored value and its default settings.
     */
    constexpr static std::pair<StoredValue &, const ParamDefault &>
    find(const ssize_t index);

    /**
     * @brief Retrieve a parameter by name.
     * @param key The parameter name.
     * @return A reference to the stored value and its default settings.
     */
    constexpr static std::pair<StoredValue &, const ParamDefault &>
    find(const frozen::string key);

    consteval static std::pair<StoredValue &, const ParamDefault &>
    cfind(const frozen::string key);

    /**
     * @brief Retrieve a parameter by name using a C-style string.
     * @param key The parameter name.
     * @return A reference to the stored value and its default settings.
     */
    constexpr static std::pair<StoredValue &, const ParamDefault &>
    find(const char *key);

    /**
     * @brief Retrieve a parameter by name using a C-style string.
     * @param key The parameter name.
     * @return the value of the underlying variant selected by the type
     */
    template<typename T>
    static const T& get(const char *key);
    
   /**
     * @brief Retrieve a parameter by index
     * @param index The parameter index.
     * @return the value of the underlying variant selected by the type
     */
    template<typename T>
    static const T& get(const size_t index);

    /**
     * @brief Retrieve a  by index and tag
     * @param index The parameter index.
     * @return the value of the underlying variant selected by the type
     */
    template<size_t TAG>
    static const auto& get(const size_t index);

    /**
     * @brief Clamp an integer value within its defined min/max range.
     */
    constexpr static Integer clamp(const ParamDefault &deflt,
				   const Integer &value);

    /**
     * @brief Clamp a float value within its defined min/max range.
     */
    constexpr static float clamp(const ParamDefault &deflt, const float &value);

    /**
     * @brief Set a new integer value for a parameter, ensuring constraints are
     * met.
     */
    constexpr static bool
    set(const std::pair<StoredValue &, const ParamDefault &> &p,
	const Integer &value);

   /**
     * @brief Set a new bool value for a parameter
     */
    constexpr static bool
    set(const std::pair<StoredValue &, const ParamDefault &> &p,
	const bool &value);

    /**
     * @brief Set a new float value for a parameter, ensuring constraints are met.
     */
    constexpr static bool
    set(const std::pair<StoredValue &, const ParamDefault &> &p,
	const float &value);

   /**
     * @brief Set a new Stored String value for a parameter
     */
    constexpr static bool
    set(const std::pair<StoredValue &, const ParamDefault &> &p,
	const StoredString &value);

    /**
     * @brief Set a new value from string, doing best effort to convert
     * string to the holded type
     */
    constexpr static bool
    set(const std::pair<StoredValue &, const ParamDefault &> &p,
	const char* value);
    /**
     * @brief Set a new value from StoredValue
     */
    constexpr static bool
    set(const std::pair<StoredValue &, const ParamDefault &> &p,
	const StoredValue &value);

    template <typename T>
    constexpr static T
    get(const std::pair<StoredValue &, const ParamDefault &> &p);

    static void serializeStoredValue(size_t index, StoreSerializeBuffer& buffer);
    static bool deserializeStoredValue(StoredValue& value, const StoreSerializeBuffer& buffer);
    static bool deserializeStoredValue(size_t index, const StoreSerializeBuffer& buffer);
    static etl::span<const uint8_t> deserializeGetName(const StoreSerializeBuffer& buffer);

    /**  
     * @brief Populate default values into the parameter list.
     */
    static void populateDefaults();
    static void enforceMinMax(size_t index);
    static void enforceMinMax();
    
  private:
    static std::array<StoredValue, params_list_len>
    storedParamsList; ///< Runtime storage of parameter values
  };

  
  constexpr ssize_t Parameter::findIndex(const frozen::string &key) {
    const auto it = frozenParameters.find(key);
    if (it == frozenParameters.end()) {
      return -1; // Key not found
    }
    return std::distance(frozenParameters.begin(), it);
  }
  
  constexpr  const frozen::string& Parameter::findName(const size_t index)  {
    return std::next(frozenParameters.begin(), index < params_list_len ? index : 0)->first;
  }

  constexpr std::pair<StoredValue &, const ParamDefault &>
  Parameter::find(const ssize_t index) {
    static constexpr ParamDefault empty = {};
    if ((index >= 0) and (index < params_list_len)) {
      return {storedParamsList[index], std::next(frozenParameters.begin(), index)->second};
    } else {
      return {storedParamsList[0], empty};
    }
  }

  constexpr std::pair<StoredValue &, const ParamDefault &>
  Parameter::find(const frozen::string key) {
    const auto index = findIndex(key);
    return find(index);
  }

  consteval std::pair<StoredValue &, const ParamDefault &>
  Parameter::cfind(const frozen::string key) {
    const auto index = cfindIndex(key);
    return find(index);
  }

  constexpr std::pair<StoredValue &, const ParamDefault &>
  Parameter::find(const char *key) {
    const auto index = findIndex(frozen::string(key));
    return find(index);
  }

  template<typename T>
  const T& Parameter::get(const char *key)
  {
    const auto& [stored, _] = find(key);
    chDbgAssert(std::holds_alternative<T>(stored),
		"invalid tag for variant");
    return std::get<T>(stored);
  }

  template<typename T>
  const T& Parameter::get(const size_t index)
  {
    const auto& [stored, _] = find(index);
    chDbgAssert(std::holds_alternative<T>(stored),
		"invalid tag for variant");
    return std::get<T>(stored);
  }
  
  template<size_t TAG>
  const auto& Parameter::get(const size_t index)
  {
    const auto& [stored, _] = find(index);
    return std::get<TAG>(stored);
  }

  constexpr Integer Parameter::clamp(const ParamDefault &deflt,
				     const Integer &value) {
    Integer ret = value;

    if (std::holds_alternative<Integer>(deflt.min))
      ret = std::max(ret, std::get<Integer>(deflt.min));
    if (std::holds_alternative<Integer>(deflt.max))
      ret = std::min(ret, std::get<Integer>(deflt.max));
    return ret;
  }

  constexpr float Parameter::clamp(const ParamDefault &deflt,
				   const float &value) {
    float ret = value;

    if (std::holds_alternative<float>(deflt.min))
      ret = std::max(ret, std::get<float>(deflt.min));
    if (std::holds_alternative<float>(deflt.max))
      ret = std::min(ret, std::get<float>(deflt.max));
    return ret;
  }

  constexpr bool
  Parameter::set(const std::pair<StoredValue &, const ParamDefault &> &p,
		 const Integer &value) {
    const auto &[store, deflt] = p;
    if (not std::holds_alternative<Integer>(store)) {
      return false;
    }
    store = clamp(deflt, value);
    return true;
  }

  constexpr bool
  Parameter::set(const std::pair<StoredValue &, const ParamDefault &> &p,
		 const bool &value) {
    const auto &[store, deflt] = p;
    if (not std::holds_alternative<bool>(store)) {
      return false;
    }
    store = value;
    return true;
  }

  constexpr bool
  Parameter::set(const std::pair<StoredValue &, const ParamDefault &> &p,
		 const float &value) {
    const auto &[store, deflt] = p;
    if (not std::holds_alternative<float>(store)) {
      return false;
    }
    store = clamp(deflt, value);
    return true;
  }

  constexpr bool
  Parameter::set(const std::pair<StoredValue &, const ParamDefault &> &p,
		 const StoredString &value) {
    const auto &[store, deflt] = p;
    if (not std::holds_alternative<StoredString *>(store)) {
      return false;
    }
    *(std::get<StoredString *>(store)) = value;
    return true;
  }

  constexpr bool
  Parameter::set(const std::pair<StoredValue &, const ParamDefault &> &p,
		 const char *value) {
    const auto &[store, deflt] = p;
    if (std::holds_alternative<Integer>(store)) {
      store =  clamp(deflt, atoll(value));
    } else if (std::holds_alternative<bool>(store)) {
      store = (*value == '0') or (tolower(*value) == 'f') ? false : true;
    } else if (std::holds_alternative<float>(store)) {
      store = clamp(deflt, static_cast<float>(atof(value)));
    } else if (std::holds_alternative<StoredString *>(store)) {
      *(std::get<StoredString *>(store)) = value;
    } else {
      return false;
    }
    return true;
  }

  constexpr  bool
  Parameter::set(const std::pair<StoredValue &, const ParamDefault &> &p,
		 const StoredValue &value)
  {
    auto &[store, deflt] = p;
    if (store.index() == value.index()) {
      if (std::holds_alternative<Integer>(store)) {
	store =  clamp(deflt, std::get<Integer>(value));
      } else if (std::holds_alternative<float>(store)) {
	store =  clamp(deflt, std::get<float>(value));
      } else {
	store = value;
      }
      return true;
    } else {
      return false;
    }
  }

  
  template <typename T>
  constexpr T
  Parameter::get(const std::pair<StoredValue &, const ParamDefault &> &p) {
    const auto &[store, deflt] = p;
    return std::get<T>(store);
  }

  void toUavcan(const StoredValue& storedValue, uavcan_protocol_param_Value& uavcanValue);
  void toUavcan(const FrozenDefault& defaultValue, uavcan_protocol_param_Value& uavcanValue);
  bool fromUavcan(const uavcan_protocol_param_Value& uavcanValue, StoredValue& storedValue);
  void toUavcan(const NumericValue& numericValue, uavcan_protocol_param_NumericValue& uavcanValue);
  bool fromUavcan(const uavcan_protocol_param_NumericValue& uavcanValue, NumericValue& numericValue);

  StoreData getSetResponse(const uavcan_protocol_param_GetSetRequest &req,
			   uavcan_protocol_param_GetSetResponse& resp);
  
} // namespace Persistant

#define PARAM_TYPECGET(type, name)				     \
  ({								     \
     constexpr ssize_t idx = Persistant::Parameter::findIndex(name); \
     static_assert(idx >= 0, name " not found");		     \
     Persistant::Parameter::get<type>(idx);			     \
  })

#define PARAM_CGET(name)					     \
  ({								     \
     constexpr ssize_t idx = Persistant::Parameter::findIndex(name); \
     static_assert(idx >= 0, name " not found");		     \
     constexpr auto& variant =					     \
       std::next(frozenParameters.begin(), idx)->second.v;	     \
     constexpr int tag = variant.index();			     \
     Persistant::Parameter::get<tag>(idx);			     \
  })

