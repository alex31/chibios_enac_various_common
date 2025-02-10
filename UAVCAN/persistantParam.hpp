#pragma once

#include <type_traits>
#include <variant>
#include <array>
#include <optional>
#include "frozen/unordered_map.h"
#include "frozen/string.h"
#include "frozen/set.h"
#include "etl/string.h"


/*
  FAIT :
   ° On a une table de pointeurs sur des Parameters dans le même ordre que
     la table frozen de Default.

   ° Un appel à ParameterBase::start recopie les defaut dans la table en ram

   ° ajout dans classe ParameterBase d'une reference au variant Default
     pour verifier dans le setter que l'on respecte min max si c'est une valeur numérique

   ° ajout setter au niveau de la classe dérivée et qui prend un
     variant en param
     
   ° retrouver un param par son index et son nom

   ° verifier à la compilation la validité de la table frozen :
    + si le type est numérique -> min et max sont empty ou du même type
    + si le type est string -> min est max sont de type empty

   ° transformer les methodes find en methodes constexpr


 TODO :

 ° review de tous les nom de type et variable : casing ? ptr indication ?


  TODO optimisation espace pris en RAM :
  ° ne pas utiliser new, mais des tables pré allouée par taille
    (en fonction de float, int64, Empty, etl::string)
    + la taille de ces 4 tables peut être retournée par une fonction constexpr
    + plutôt que de faire un new on alloue dans ces tables
    + plutôt que d'avoir une table statique de pointeur, on aurait une table
      statique d'index (sur 16 bits) : index de table sur 2 bits, index dans la table sur 14 bits
      * creation une structure avec des bitfield

  ° fonction ou methode qui construit un StoredValue depuis le type fourni par DSDL
  ° fonction ou methode qui construit un message DSDL depuis un StoredValue
  ° fonction ou methode qui accede (r/w)  paramList en eeprom/flash

  ° gerer les scenarios (trouver meilleur nom) : le nom d'un param doit
    commencer par un prefixe de 5 lettres qui indique le scénario
   + fonction constexpr qui vérifie que les noms respectent la convention et
     que le scenario existe  

  en fonction des scenario activés (paramètre bitfield sur 64 bits
  stocké en flash) on n'indexe en ram que les paramètres utilisés par les scenarios actifs
 */

namespace {
  // Template to get index of type T in std::variant<...>
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
  struct NoValue {};
  using StoredString = etl::string<48>;
  using Default = std::variant<NoValue, frozen::string, int64_t, float>;
  using StoredValue = std::variant<NoValue, StoredString, int64_t, float>;
  using NumericValue = std::variant<NoValue, int64_t, float>;
  struct ParamDefault {
    NumericValue min = (NoValue){};
    NumericValue max = (NoValue){};
    Default	 v = (NoValue){};
  };



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


  
  
  static constexpr std::pair<frozen::string, ParamDefault> params_list[]
    {
#include "nodeParameters.hpp"
    };

  // Static assert at compile-time
  static_assert(validateDefaultsList(params_list) < 0,
   		"❌ params_list contains invalid ParamDefault entries!");
  
  
  constexpr ssize_t  params_list_len =
    sizeof(params_list) / sizeof(params_list[0]);
  
  constexpr auto frozenParameters = frozen::make_unordered_map(params_list);

  class ParameterBase {
  public:
    virtual const StoredValue get() const = 0;
    virtual void  set(const StoredValue&) = 0;
    constexpr static ssize_t findIndex(const frozen::string key);
    static void populateDefaults();
    constexpr static ParameterBase** find(const ssize_t index);
    constexpr static ParameterBase** find(const frozen::string key);
  protected:
    ParameterBase(const ParamDefault& _deflt) : deflt(_deflt) {};
    static std::array<ParameterBase *, params_list_len> paramList;
    static size_t paramCurrentIndex;
    const ParamDefault& deflt;
  };
  
  template <typename T>
  class Parameter : public ParameterBase {
  public:
    Parameter(const ParamDefault& _deflt);
    Parameter(const ParamDefault& _deflt, const T& v);
    const StoredValue get() const override {return val;};
    void  set(const StoredValue&) override;
  private:
    T val;
  };


  constexpr ssize_t ParameterBase::findIndex(const frozen::string key)
  {
    const auto it = frozenParameters.find(key);
    if (it == frozenParameters.end()) {
      return -1;  // Key not found
    }
    return std::distance(frozenParameters.begin(), it);
  }

  constexpr ParameterBase** ParameterBase::find(const ssize_t index)
  {
    assert((index >= 0) && (index < params_list_len));
    return &paramList[index];
  }
  
  constexpr ParameterBase** ParameterBase::find(const frozen::string key)
  {
    const auto index = findIndex(key);
    return find(index);
  }
  
  template <typename T>
  Parameter<T>::Parameter(const ParamDefault& _deflt) : ParameterBase(_deflt)
  {
    if (paramCurrentIndex == params_list_len) {
      // chSysHalt("etl::vector paramList is full");
    }
    paramList[paramCurrentIndex++] = this;
  }

  template <typename T>
  Parameter<T>::Parameter(const ParamDefault& _deflt, const T& v) : Parameter(_deflt)
  {
    val = v;
  }

  template <typename T>
  void Parameter<T>::set(const StoredValue& variant)
  {
    if (std::holds_alternative<T>(variant)) {
      val = std::get<T>(variant);
    } else {
      assert (false && "Parameter<T>::set variant mismatch");
    }
    if constexpr (std::is_same_v<T, int64_t>) {
      if (deflt.min.index() == variant_index_v<int64_t, NumericValue>) {
	val = std::max(val, std::get<int64_t>(deflt.min));
      }
      if (deflt.max.index() == variant_index_v<int64_t, NumericValue>) {
	val = std::min(val, std::get<int64_t>(deflt.max));
      }
    } else if constexpr (std::is_same_v<T, float>) {
      if (deflt.min.index() == variant_index_v<float, NumericValue>) {
	val = std::max(val, std::get<float>(deflt.min));
      }
      if (deflt.max.index() == variant_index_v<float, NumericValue>) {
	val = std::min(val, std::get<float>(deflt.max));
      }
    }
  }
  
}
