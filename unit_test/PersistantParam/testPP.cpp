#include "persistantParam.hpp"
#include "persistantStorage.hpp"
#include <cstdio>
#include <format>
#include <iostream>
#include <eeprom.hpp>


namespace {
   Persistant::EepromStoreHandle eepromHandle = {
     .writeFn = eeprom_write<256>,
     .readFn = eeprom_read<256>,
     .getLen = eeprom_getlen<256>
   };
}



int main(int argc, char **)
{
  Persistant::Storage storage(eepromHandle);
  
  assert(storage.storeAll() == true);

  // const ssize_t searchIndex = storage.binarySearch(0, "CONST.PARAMETERS.CRC32");
  // printf("searchIndex ('CONST.PARAMETERS.CRC32') = %ld\n", searchIndex);

  constexpr ssize_t index1 = Persistant::Parameter::findIndex("ratio");
  static_assert(index1 >= 0, "ratio not found");

  ssize_t index2 = Persistant::Parameter::findIndex("ration");
  std::cout << std::format("argc = {} i1 = {}, i2 = {}\n",
			   argc, index1, index2);

  
  struct Overload {
    void operator()(const char* name, Persistant::NoValue) const {
      std::cout << name << " has no Value !! ;";
    }
    void operator()(const char* name, Persistant::Integer i) const {
      std::cout << name << " is Integer = " << i << "; ";
    }
    void operator()(const char* name, float f) const {
      std::cout << name << " is Double = " << f << "; ";
    }
    void operator()(const char* name, bool b) const {
      std::cout << name << " is Bool = " << b << "; ";
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
   void operator()(const ssize_t i, const frozen::string& name, Persistant::NoValue) const {
     std::cout << "store " << i << ' ' << name.data() << " has no Value !! \n";
    }
    void operator()(const ssize_t i, const frozen::string& name, Persistant::Integer j) const {
      std::cout << "store " << i << ' ' << name.data() << " is Integer = " << j << std::endl;
    }
    void operator()(const ssize_t i, const frozen::string& name, bool b) const {
      std::cout << "store " << i << ' ' << name.data() << " is Bool = " << b << std::endl;
    }
    void operator()(const ssize_t i, const frozen::string& name, float f) const {
      std::cout << "store " << i << ' ' << name.data() << " is Double = " << f << std::endl;
    }
    void operator()(const ssize_t i, const frozen::string& name, Persistant::StoredString *s) const {
      std::cout << "store " << i << ' ' << name.data() << " is String = " << s->c_str() << std::endl;
    }
  };


 for (ssize_t i=0; i < Persistant::params_list_len; i++) {
   const frozen::string& paramName =  std::next(Persistant::frozenParameters.begin(), i)->first;
   std::visit([&](const auto& param_ptr) {
     OverloadDyn{}(i, paramName, param_ptr);  
   }, Persistant::Parameter::find(i).first);
 }
 std::cout << std::endl << std::endl;
 
 const char* ratio = "ratio";

 constexpr ssize_t powerIndex = Persistant::Parameter::findIndex("power");
 const auto& p = Persistant::Parameter::find(powerIndex);
 const auto& p2 = Persistant::Parameter::find(ratio);
 // const auto& p6 = Persistant::Parameter::find("title");

 Persistant::Integer dynval = Persistant::Parameter::get<Integer>(p);
 if (dynval >= 100)
   dynval = 1;
 Persistant::Parameter::set(p, dynval * 2);
 Persistant::Parameter::set(p2, 0.42f);
 storage.store(powerIndex);
 

 for (ssize_t i=0; i < Persistant::params_list_len; i++) {  
   const frozen::string& paramName =  std::next(Persistant::frozenParameters.begin(), i)->first;
   std::visit([&](const auto& param_ptr) {
     OverloadDyn{}(i, paramName, param_ptr);
   }, Persistant::Parameter::find(i).first);
 }
 std::cout << std::endl << std::endl;
 
 
 std::cout << std::format("noval = {}, float = {}; Persistant::Integer = {}\n"
			  "tinyString poolSize = {}; storedString = {}\n"
			  "storedValue = {}; frozenEntry = {}\n",
			  sizeof(Persistant::NoValue),
			  sizeof(float),
			  sizeof(Persistant::Integer),
			  Persistant::getTinyStringMemoryPoolSize(),
			  sizeof(Persistant::StoredString),
			  sizeof(Persistant::StoredValue),
			  sizeof(Persistant::Default));


 Persistant::StoreSerializeBuffer serialBuffer;
 Persistant::Parameter::serializeStoredValue(6, serialBuffer);
 Persistant::Parameter::set(p2, 100.0f);
 const bool success = Persistant::Parameter::deserializeStoredValue(6, serialBuffer);
 const auto& name = Persistant::Parameter::deserializeGetName(serialBuffer);
 printf("stored param name = \"%s\" status = %d\n",
	reinterpret_cast<const char*>(name.data()),
	success);

 for (ssize_t i=0; i < Persistant::params_list_len; i++) {  
   const frozen::string& paramName =  std::next(Persistant::frozenParameters.begin(), i)->first;
   std::visit([&](const auto& param_ptr) {
     OverloadDyn{}(i, paramName, param_ptr);
   }, Persistant::Parameter::find(i).first);
 }
 std::cout << std::endl << std::endl;

 assert(storage.restoreAll() == true);
 
  for (ssize_t i=0; i < Persistant::params_list_len; i++) {  
   const frozen::string& paramName =  std::next(Persistant::frozenParameters.begin(), i)->first;
   std::visit([&](const auto& param_ptr) {
     OverloadDyn{}(i, paramName, param_ptr);
   }, Persistant::Parameter::find(i).first);
 }
 std::cout << std::endl << std::endl;


 return 0;
}
