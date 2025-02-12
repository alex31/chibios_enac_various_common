#include "persistantParam.hpp"
#include <cassert>

namespace Persistant {

  // does not use heap, custom allocator will use statically
  // defined memory area
 struct Overload {
   void operator()(const ParamDefault&, StoredValue& sv, NoValue n) const {
     sv = n;
   }
   void operator()(const ParamDefault& deflt, StoredValue& sv, int64_t i) const {
     sv = Parameter::clamp(deflt, i);
   }
   void operator()(const ParamDefault& deflt, StoredValue& sv, float f) const {
     sv = Parameter::clamp(deflt, f);
    }
    void operator()(const ParamDefault&, StoredValue& sv, const frozen::string& s) const {
      sv = new StoredString (s.data());
    }
  };

  
  void Parameter::populateDefaults()
  {
    size_t index = 0;
    for (const auto& [_, variant] : frozenParameters) {  
      std::visit([&](const auto& value) {
	Overload{}(variant, paramList[index++], value);  
      }, variant.v);
    }
  }

 
  std::array<StoredValue, params_list_len> Parameter::paramList;
}





namespace {
}

