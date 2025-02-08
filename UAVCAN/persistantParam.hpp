#pragma once

#include <variant>
#include <array>
#include <optional>
#include "frozen/unordered_map.h"
#include "frozen/string.h"
#include "frozen/set.h"
#include "etl/string.h"
#include "etl/vector.h"

/*
  FAIT :
  On a une table de pointeurs sur des Parameters dans le même ordre que
  la table frozen de Default.

  Un appel à ParameterBase::start recopie les defaut dans la table en ram

  TODO :
  ° ajout dans classe ParameterBase d'une reference au variant Default
    pour verifier dans le setter que l'on respecte min max si c'est une valeur numérique

  ° ajout setter au niveau de la classe dérivée et qui prend un
    variant en param : si le variant est pas du bon type : kaboum

  ° retrouver un param par son index et son nom

  ° fonction ou methode qui construit un StoredValue depuis le type fourni par DSDL
  ° fonction ou methode qui construit un message DSDL depuis un StoredValue
  ° fonction ou methode qui accede (r/w)  paramList en eeprom/flash

  TODO optimisation espace pris en RAM :
  ° ne pas utiliser new, mais des tables pré allouée par taille
    (en fonction de float, int64, Empty, etl::string)
    + la taille de ces 4 tables peut être retournée par une fonction constexpr
    + plutôt que de faire un new on alloue dans ces tables
    + plutôt que d'avoir une table statique de pointeur, on aurait une table
      statique d'index (sur 16 bits) : index de table sur 2 bits, index dans la table sur 14 bits
      * creation une structure avec des bitfield

  après faut gerer les scenarios : le nom d'un param soit commencer par un prefixe de 5 lettres
  qui indique le scénario

  en fonction des scenario activés (paramètre bitfield sur 64 bits
  stocké en flash) on n'indexe en ram que les paramètres utilisés par les scenarios actifs
 */

namespace {
 
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
  using FrozenDict = frozen::unordered_map<frozen::string, ParamDefault, 50>;

  
  static constexpr std::pair<frozen::string, ParamDefault> params_list[]
    {
#include "nodeParameters.hpp"
    };

  constexpr size_t  params_list_len =
    sizeof(params_list) / sizeof(params_list[0]);
  
  constexpr auto frozenParameters = frozen::make_unordered_map(params_list);

  class ParameterBase {
  public:
    virtual const StoredValue get() const = 0;
    virtual void  set(const StoredValue&) = 0;
    constexpr static ssize_t getIndex(const frozen::string key);
    static void populateDefaults();
  protected:
    static etl::vector<ParameterBase *, params_list_len> paramList;
  };
  
  template <typename T>
  class Parameter : public ParameterBase {
  public:
    Parameter();
    Parameter(const T& v);
    const StoredValue get() const override {return val;};
    void  set(const StoredValue&) override;
  private:
    T val;
  };


  constexpr ssize_t ParameterBase::getIndex(const frozen::string key)
  {
    const auto it = frozenParameters.find(key);
    if (it == frozenParameters.end()) {
      return -1;  // Key not found
    }
    return std::distance(frozenParameters.begin(), it);
  }
  
  
  template <typename T>
  Parameter<T>::Parameter() 
  {
    if (paramList.full()) {
      // chSysHalt("etl::vector paramList is full");
    }
    paramList.push_back(this);
  }

  template <typename T>
  Parameter<T>::Parameter(const T& v) : Parameter()
  {
    val = v;
  }

  template <typename T>
  void Parameter<T>::set(const StoredValue& variant)
  {
    val = std::get<T>(variant);
  }
  
}
