#include "persistantParam.hpp"
#include <cassert>

namespace Persistant {

  // does not use heap, custom allocator will use statically
  // defined memory area
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

 
  std::array<ParameterBase *, params_list_len> ParameterBase::paramList;
  size_t ParameterBase::paramCurrentIndex = 0;
  SimpleMemoryPool<Persistant::getMemoryPoolSize()> ParameterBase::memPool;
}





namespace {
}

