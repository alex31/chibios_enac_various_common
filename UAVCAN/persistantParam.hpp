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


 ° test de la validité de frozen : v est compris entre min et max ?
 
 ° quand on affecte un StoredValue : verifier si c'est pertinent min/max
 
 ° tester que find fonctionne en constexpr et aussi au runtime 

 ° review de tous les nom de type et variable : casing ? ptr indication ?


  TODO optimisation espace pris en RAM :
  
 
  ° fonction ou methode qui construit un StoredValue depuis le type fourni par DSDL
  ° fonction ou methode qui construit un message DSDL depuis un StoredValue
  ° fonction ou methode qui accede (r/w)  paramList en eeprom/flash

  ° gerer les scenarios (trouver meilleur nom) : le nom d'un param doit
    commencer par un prefixe de 5 lettres qui indique le scénario
   + fonction constexpr qui vérifie que les noms respectent la convention et
     que le scenario existe  

  */

namespace {
  // Helper : Template to get index of type T in std::variant<...>
  template <typename T, typename Variant>
  struct variant_index;

  template <typename T, typename... Types>
  struct variant_index<T, std::variant<Types...>> {
    static constexpr std::size_t value = [] {
      std::size_t index = 0;
      bool found = ((std::is_same_v<T, Types> ? true : (++index, false)) || ...);
      return found ? index : static_cast<std::size_t>(-1);
    }();
  };
  
  // Helper variable template
  template <typename T, typename Variant>
  inline constexpr std::size_t variant_index_v = variant_index<T, Variant>::value;
}


namespace Persistant {
  constexpr size_t tinyStrSize = 47;
  struct NoValue {};
  using Default = std::variant<NoValue, frozen::string, int64_t, float>;
  using NumericValue = std::variant<NoValue, int64_t, float>;
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


  static consteval size_t getTinyStringMemoryPoolSize() {
    struct Overload {
      constexpr size_t operator()(Persistant::NoValue) const {
	return 0;
      }
      constexpr size_t operator()(int64_t) const {
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
  using StoredValue = std::variant<NoValue, StoredString*, int64_t, float>;



    // Compile-time check for an entry
  consteval bool isValidDefault(const ParamDefault& param) {
    return std::visit([&](const auto& v) -> bool {
      using T = std::decay_t<decltype(v)>;

      if constexpr (std::is_same_v<T, int64_t> || std::is_same_v<T, float>) {
	return std::visit([](const auto& min_val, const auto& max_val) -> bool {
	  using MinT = std::decay_t<decltype(min_val)>;
	  using MaxT = std::decay_t<decltype(max_val)>;

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
      return true; // If v is not int64_t or float, no check is needed
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
  static_assert(validateDefaultsList(params_list) < 0,
   		"❌ params_list contains invalid ParamDefault entries!");
  


  constexpr auto frozenParameters = frozen::make_unordered_map(params_list);

  class  Parameter {
  public:
    Parameter() = delete;
    constexpr static ssize_t findIndex(const frozen::string key);
    static void populateDefaults();
    constexpr static StoredValue& find(const ssize_t index);
    constexpr static StoredValue& find(const frozen::string key);
    constexpr static StoredValue clamp(const ssize_t index, const StoredValue& v);
    constexpr static StoredValue clamp(const frozen::string key, const StoredValue& v);
   template<typename T>
    constexpr static T clamp(const ParamDefault& deflt, const T& value);

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

  constexpr StoredValue& Parameter::find(const ssize_t index)
  {
    assert((index >= 0) && (index < params_list_len));
    return paramList[index];
  }
  
  constexpr StoredValue& Parameter::find(const frozen::string key)
  {
    const auto index = findIndex(key);
    return find(index);
  }
  
  constexpr StoredValue Parameter::clamp(const frozen::string key,
					 const StoredValue& variant)
  {
    const auto index = findIndex(key);
    return clamp(index, variant);
  }
  
  constexpr StoredValue Parameter::clamp(const ssize_t index,
					 const StoredValue& variant)
  {
    assert((index >= 0) && (index < params_list_len));
    StoredValue ret;
    const auto& deflt = params_list[index].second;
    if (std::holds_alternative<int64_t>(variant)) {
      if (std::holds_alternative<int64_t>(deflt.min))
	ret =  std::max(std::get<int64_t>(variant), std::get<int64_t>(deflt.min));
      if (std::holds_alternative<int64_t>(deflt.max))
	ret =  std::min(std::get<int64_t>(variant), std::get<int64_t>(deflt.max));
    }
    if (std::holds_alternative<float>(variant)) {
      if (std::holds_alternative<float>(deflt.min))
	ret =  std::max(std::get<float>(variant), std::get<float>(deflt.min));
      if (std::holds_alternative<float>(deflt.max))
	ret =  std::min(std::get<float>(variant), std::get<float>(deflt.max));
    }
    
    return ret;
  }

  template<typename T>
  constexpr T Parameter::clamp(const ParamDefault& deflt,
					 const T& value)
  {
    T ret;

    if constexpr (std::is_same_v<int64_t, T>) {
      if (std::holds_alternative<int64_t>(deflt.min))
	ret =  std::max(value, std::get<int64_t>(deflt.min));
      if (std::holds_alternative<int64_t>(deflt.max))
	ret =  std::min(value, std::get<int64_t>(deflt.max));
    } else if constexpr (std::is_same_v<float, T>) {
      if (std::holds_alternative<float>(deflt.min))
	ret =  std::max(value, std::get<float>(deflt.min));
      if (std::holds_alternative<float>(deflt.max))
	ret =  std::min(value, std::get<float>(deflt.max));
    }
    
    return ret;
  }
  
}
