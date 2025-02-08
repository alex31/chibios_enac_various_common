#include "persistantParam.hpp"
#include <cassert>

namespace Persistant {

 struct Overload {
   void operator()(const ParamDefault& deflt, NoValue n) const {
      new Parameter(deflt, n);
    }
      void operator()(const ParamDefault& deflt, int64_t i) const {
      new Parameter(deflt, i);
    }
   void operator()(const ParamDefault& deflt, float f) const {
      new Parameter(deflt, f);
    }
    void operator()(const ParamDefault& deflt, const frozen::string& s) const {
      StoredString ets(s.data());
      new Parameter(deflt, ets);
    }
  };

  
  void ParameterBase::populateDefaults()
  {
     for (const auto& [_, variant] : frozenParameters) {  
       std::visit([&](const auto& value) {
	 Overload{}(variant, value);  
       }, variant.v);
     }
  }

  ParameterBase& ParameterBase::find(const ssize_t index)
  {
    assert((index >= 0) && (index < params_list_len));
    return *paramList[index];
  }

  ParameterBase& ParameterBase::find(const frozen::string key)
  {
    const auto index = findIndex(key);
    return find(index);
  }

  etl::vector<ParameterBase *, params_list_len> ParameterBase::paramList;
}





namespace {
}

