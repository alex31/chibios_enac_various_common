#pragma once

#include <type_traits>
#include <variant>
#include <array>
#include <optional>
#include "frozen/unordered_map.h"
#include "frozen/string.h"
#include "frozen/set.h"
#include "tinyString.hpp" 
#include "customAllocator.hpp"

/*
  FAIT :


 TODO :


 ° review de tous les nom de type et variable : casing ? ptr indication ?


 ° fonction ou methode qui construit un StoredValue depuis le type fourni par DSDL
 ° fonction ou methode qui construit un message DSDL depuis un StoredValue
 ° fonction ou methode qui accede (r/w)  paramList en eeprom/flash
 
 ° gerer les scenarios (trouver meilleur nom) : le nom d'un param doit
 commencer par un prefixe de 5 lettres qui indique le scénario

 ° notion de constant string
   + on ne reserve pas de place dans la ram,
     un pointeur de char* pointe sur frozen::string en flash
   + comment on specifie une chaine constante dans nodeParameters.hpp -> un type particuler ?

 + fonction constexpr qui vérifie que les noms respectent la convention et
 que le scenario existe  
 
*/


namespace Persistant {
  // must be (4*N)-1 to avoid to spill ram in padding
  constexpr size_t tinyStrSize = 47;
  // Integer type choice : int64_t or int32_t
  // if int64_t : mirror DSDL message format, but each param use 16 bytes in ram
  // if int32_t : does NOT mirror DSDL message format, but each param use 8 bytes in ram
  using Integer =  int64_t;
  struct NoValue {};
  using Default = std::variant<NoValue, frozen::string, Integer, float>;
  using NumericValue = std::variant<NoValue, Integer, float>;
  struct ParamDefault {
    NumericValue min = (NoValue){};
    NumericValue max = (NoValue){};
    Default	 v = (NoValue){};
  };

  

  static constexpr std::pair<frozen::string, ParamDefault> params_list[]
    {
#include "nodeParameters.hpp"
    };
  
 
  constexpr ssize_t  params_list_len =
    sizeof(params_list) / sizeof(params_list[0]);


  // helper to calculate the amount or ram needed to store
  // string parameters. Custom allocator will
  // use a static area managed by TintString class
  static consteval size_t getTinyStringMemoryPoolSize() {
    struct Overload {
      constexpr size_t operator()(Persistant::NoValue) const {
	return 0;
      }
      constexpr size_t operator()(Integer) const {
	return 0;
      }
      constexpr size_t operator()(float) const {
	return 0;
      }
      constexpr size_t  operator()(const frozen::string &) const {
	return sizeof(TinyString<0, tinyStrSize>);
      }
    };
    
    size_t size = 0;
    for (size_t i=0; i < params_list_len; i++) {  
      std::visit([&](const auto& param) {
	size += Overload{}(param);  
      }, params_list[i].second.v);
    }
    
    return size;
  }

  
  
  using StoredString = TinyString<getTinyStringMemoryPoolSize(), tinyStrSize>;
  using StoredValue = std::variant<NoValue, StoredString*, Integer, float>;


  // Compile-time checks for an entry
  // ° min and max are else NoValue or the same type as v
  // ° neither min or max for string parameters
  // ° default value is in the range min..max it they are supplied
  consteval bool isValidDefault(const ParamDefault& param) {
    return std::visit([&](const auto& v) -> bool {
      using T = std::decay_t<decltype(v)>;

      if constexpr (std::is_same_v<T, Integer> || std::is_same_v<T, float>) {
	return std::visit([&](const auto& min_val, const auto& max_val) -> bool {
	  using MinT = std::decay_t<decltype(min_val)>;
	  using MaxT = std::decay_t<decltype(max_val)>;

	  if constexpr(std::is_same_v<MinT, T>) {
	    if (v < min_val)
	      return 0;
	  }
	  if constexpr(std::is_same_v<MaxT, T>) {
	    if (v > max_val)
	      return 0;
	  } 
	  return (std::is_same_v<MinT, NoValue> || std::is_same_v<MinT, T>) &&
	    (std::is_same_v<MaxT, NoValue> || std::is_same_v<MaxT, T>);
	}, param.min, param.max);
	  
      } else if constexpr (std::is_same_v<T, frozen::string>) {
	return std::visit([](const auto& min_val, const auto& max_val) -> bool {
	  using MinT = std::decay_t<decltype(min_val)>;
	  using MaxT = std::decay_t<decltype(max_val)>;

	  return (std::is_same_v<MinT, NoValue>) && (std::is_same_v<MaxT, NoValue>);
	}, param.min, param.max);
      }
      return true; // If v is not Integer or float, no check is needed
    }, param.v);
  }

  // Compile-time check for an entire array
  consteval int validateDefaultsList(const auto& paramsPairList) {
    int index = 0;
    for (const auto& entry : paramsPairList) {
      if (!isValidDefault(entry.second)) return index;
      index++;
    }
    return -1;
  }

  // Static assert at compile-time
  // if a check fail, compiler error message will show the first
  // offending entry, numbered from 0
  static_assert(validateDefaultsList(params_list) < 0,
   		"❌ params_list contains invalid ParamDefault entries!");
  


  constexpr auto frozenParameters = frozen::make_unordered_map(params_list);

  class  Parameter {
  public:
    Parameter() = delete;
    constexpr static ssize_t findIndex(const frozen::string key);
    static void populateDefaults();
    constexpr static std::pair<StoredValue&, const ParamDefault&>
                     find(const ssize_t index);
    constexpr static std::pair<StoredValue&, const ParamDefault&>
		     find(const frozen::string key);
    constexpr static std::pair<StoredValue&, const ParamDefault&>
		     find(const char* key);
    constexpr static Integer clamp(const ParamDefault& deflt, const Integer& value);
    constexpr static float clamp(const ParamDefault& deflt, const float& value);
    constexpr static void set(const std::pair<StoredValue&, const ParamDefault&> &p,
			      const Integer& value);
    constexpr static void set(const std::pair<StoredValue&, const ParamDefault&> &p,
			      const float& value);

  private:
    static std::array<StoredValue, params_list_len> paramList;
  };
  


 
  constexpr ssize_t Parameter::findIndex(const frozen::string key)
  {
    const auto it = frozenParameters.find(key);
    if (it == frozenParameters.end()) {
      return -1;  // Key not found
    }
    return std::distance(frozenParameters.begin(), it);
  }

  constexpr std::pair<StoredValue&, const ParamDefault&>
  Parameter::find(const ssize_t index)
  {
    assert((index >= 0) && (index < params_list_len));
    return {paramList[index], params_list[index].second};
  }
  
  constexpr std::pair<StoredValue&, const ParamDefault&>
  Parameter::find(const frozen::string key)
  {
    const auto index = findIndex(key);
    return find(index);
  }

  constexpr std::pair<StoredValue&, const ParamDefault&>
  Parameter::find(const char* key)
  {
    const auto index = findIndex(frozen::string(key));
    return find(index);
  }
  
 
  constexpr Integer Parameter::clamp(const ParamDefault& deflt,
				     const Integer& value)
  {
    Integer ret = value;

    if (std::holds_alternative<Integer>(deflt.min))
      ret =  std::max(value, std::get<Integer>(deflt.min));
    if (std::holds_alternative<Integer>(deflt.max))
      ret =  std::min(value, std::get<Integer>(deflt.max));
    return ret;
  }

 constexpr float Parameter::clamp(const ParamDefault& deflt,
				  const float& value)
 {
    float ret = value;

    if (std::holds_alternative<float>(deflt.min))
     ret =  std::max(value, std::get<float>(deflt.min));
   if (std::holds_alternative<float>(deflt.max))
     ret =  std::min(value, std::get<float>(deflt.max));
   return ret;
 }

  constexpr void Parameter::set(const std::pair<StoredValue&, const ParamDefault&> &p,
			     const Integer& value)
  {
    const auto& [store, deflt] = p;
    if (not std::holds_alternative<Integer>(store)) {
      assert(0 && "cannot change StoredValue alternative");
    }
    store = clamp(deflt, value);
  }

  constexpr void Parameter::set(const std::pair<StoredValue&, const ParamDefault&> &p,
			     const float& value)
  {
    const auto& [store, deflt] = p;
    if (not std::holds_alternative<float>(store)) {
      assert(0 && "cannot change StoredValue alternative");
    }
    store = clamp(deflt, value);
  }
 

}
