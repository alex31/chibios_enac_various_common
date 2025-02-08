#include "persistantParam.hpp"

namespace Persistant {

 struct Overload {
    void operator()(Persistant::NoValue n) const {
      new Parameter(n);
    }
    void operator()(int64_t i) const {
      new Parameter(i);
    }
    void operator()(float f) const {
      new Parameter(f);
    }
    void operator()(const frozen::string& s) const {
      StoredString ets(s.data());
      new Parameter(ets);
    }
  };

  
  void ParameterBase::populateDefaults()
  {
     for (const auto& [_, variant] : frozenParameters) {  
       std::visit([&](const auto& value) {
	 Overload{}(value);  
       }, variant.v);
     }
  }

  etl::vector<ParameterBase *, params_list_len> ParameterBase::paramList;
}





namespace {
}

