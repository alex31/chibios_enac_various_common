#include "persistantParam.hpp"
#include <cstdio>
#include <format>
#include <iostream>

int main(int argc, char **)
{
  Persistant::Parameter::populateDefaults();
  constexpr ssize_t index1 = Persistant::Parameter::findIndex("ratio");
  static_assert(index1 >= 0, "index1 not found");

  ssize_t index2 = Persistant::Parameter::findIndex("ration");
  std::cout << std::format("argc = {} i1 = {}, i2 = {}\n",
			   argc,index1, index2);

  
  struct Overload {
    void operator()(const char* name, Persistant::NoValue) const {
      std::cout << name << " has no Value !! ;";
    }
    void operator()(const char* name, int64_t i) const {
      std::cout << name << " is Integer = " << i << "; ";
    }
    void operator()(const char* name, float f) const {
      std::cout << name << " is Double = " << f << "; ";
    }
    void operator()(const char* name, const frozen::string& s) const {
      std::cout << name << " is String = " << s.data() << "; ";
    }
  };
  
  for (const auto& [key, value] : Persistant::frozenParameters) {  
    std::cout << std::endl << key.data() << " :: ";
    auto& [min, max, v] = value;
    
    std::visit([&](const auto& value) {
        Overload{}("min", value);  
    }, min);
  std::visit([&](const auto& value) {
        Overload{}("max", value);  
    }, max);
  std::visit([&](const auto& value) {
        Overload{}("default value", value);  
    }, v);
  }
std::cout << std::endl;
 
 struct OverloadDyn {
    void operator()(Persistant::NoValue) const {
      std::cout << "store" << " has no Value !! \n";
    }
    void operator()(int64_t i) const {
      std::cout << "store" << " is Integer = " << i << std::endl;
    }
    void operator()(float f) const {
      std::cout << "store" << " is Double = " << f << std::endl;
    }
    void operator()(Persistant::StoredString *s) const {
      std::cout << "store" << " is String = " << s->c_str() << std::endl;
    }
  };


 for (ssize_t i=0; i < Persistant::params_list_len; i++) {  
     std::visit([&](const auto& param_ptr) {
       OverloadDyn{}(param_ptr);  
     }, Persistant::Parameter::find(i).first);
   }

 const char* ratio = "ratio";
 
 const auto& p = Persistant::Parameter::find("power");
 const auto& p2 = Persistant::Parameter::find(ratio);
 Persistant::Parameter::set(p, 4200L);
 int64_t dynval = 2000;
 Persistant::Parameter::set(p, dynval);
 Persistant::Parameter::set(p2, 0.42f);

 for (ssize_t i=0; i < Persistant::params_list_len; i++) {  
   std::visit([&](const auto& param_ptr) {
     OverloadDyn{}(param_ptr);  
   }, Persistant::Parameter::find(i).first);
 }
 
 std::cout << std::format("noval = {}, float = {}; int64_t = {}\n"
			  "tinyString poolSize = {}; storedString = {}\n"
			  "frozenEntry = {}\n",
			  sizeof(Persistant::NoValue),
			  sizeof(float),
			  sizeof(int64_t),
			  Persistant::getTinyStringMemoryPoolSize(),
			  sizeof(Persistant::StoredString),
			  sizeof(Persistant::Default));
 
 return 0;
}
