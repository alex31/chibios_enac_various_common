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

 
  std::array<StoredValue, params_list_len> Parameter::paramList;
}
