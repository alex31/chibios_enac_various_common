#include <array>
#include <type_traits>
#include <functional>
#include <tuple>
#include <cassert>

#pragma once

/*
  Todo : 
  
  ° optimisation de la serialisation au choix par un memcopy (safe, lent) 
    ou une union avec pragma pack (unsafe, rapide) (ok, c'est une UB, mais dans la pratique, 
    ça fonctionne) 
  ° static_assert sur les tailles mémoire des objets passés à unserializeTuple
  ° type de registrer address dans un parametre template plutot que dans un type alias

 */


#define MKTP(...) std::forward_as_tuple(__VA_ARGS__)

template <typename> struct is_tuple_t: std::false_type {};
template <typename ...T> struct is_tuple_t<std::tuple<T...>>: std::true_type {};
template<typename Type>
constexpr bool is_tuple(const Type &x) {
  (void) x;
  if constexpr (is_tuple_t<Type>::value) 
        return true;
  else
        return false;
}

enum class OpRead{V};
enum class OpWrite{V};
using RegisterAddr7b = uint8_t;


template<class F, class...Ts, std::size_t...Is>
constexpr void for_each_in_tuple(const std::tuple<Ts...> & tuple, F func,
				 std::index_sequence<Is...>){
    using expander = int[];
    (void)expander { 0, ((void)func(std::get<Is>(tuple)), 0)... };
}
template<class F, class...Ts>
constexpr void for_each_in_tuple(const std::tuple<Ts...> & tuple, F func){
    for_each_in_tuple(tuple, func, std::make_index_sequence<sizeof...(Ts)>());
}

template<class... Ts>
constexpr auto size_of_tuple(std::tuple<Ts...>) {
  constexpr std::size_t ret = (0 + ... + sizeof(Ts));
  return std::integral_constant<std::size_t, ret>{};
}



template <typename T>
constexpr std::array<uint8_t, sizeof(T)> serializeIntegral(const T& t,
							   const bool littleendian=true)
{
  constexpr size_t s = sizeof(T);
  std::array<uint8_t, s> arr = {};

  for (size_t i=0; i< s; i++) {
    if (littleendian)
      arr[i] = (t >> (8*i)) & 0xff ;
    else
      arr[i] = (t >> (8*(s-1-i))) & 0xff ;
  }

  return arr;
}

template <typename T, typename S>
constexpr std::array<uint8_t, sizeof(T)+sizeof(RegisterAddr7b)>
serializeIntegralWithRegAdr([[maybe_unused]] const S sop,
			    const RegisterAddr7b regaddr,
			    const T& t,
			    const bool littleendian=true)
{
  constexpr size_t sr = sizeof(RegisterAddr7b);
  constexpr size_t st = sizeof(T);

  std::array<uint8_t, sr+st> arr = {};

  // calculate 8 bits address (from 7 bit one) where bit0 indicate R or W
  // this convention has been borrowed to I²C bus
  // ugly IIFE because lack of constexpr ternary conditional
  assert((regaddr & (~0b1111111)) == 0);
  const auto regaddr8b = [&] {
    if constexpr (std::is_same_v<S, OpRead>)
      return regaddr | 0x80;
    else
      return regaddr;
  } ();

  // serialize register address
  for (size_t i=0; i< sr; i++) {
    arr[i] = (regaddr8b >> (8*i)) & 0xff ;
  }

  // serialize data
  for (size_t i=0; i< st; i++) {
    if (littleendian)
      arr[sr+i] = (t >> (8*i)) & 0xff ;
    else
      arr[sr+i] = (t >> (8*(st-1-i))) & 0xff ;
  }
  
  return arr;
}

template <typename T, size_t N>
constexpr T unSerializeIntegral(const std::array<uint8_t, N> &arr,
				const size_t offset=0,
				const bool littleendian=true)
{
  constexpr size_t s = sizeof(T);
  T ret=0;

  for (size_t i=0; i< s; i++) {
    assert((i + offset) < N);
    if (littleendian)
      ret |= arr[i+offset] << (8*i);
    else
      ret |= arr[i+offset] << (8*(s-1-i));
  }

  return ret;
}

template <typename T>
constexpr auto serializeTuple(const T &tup) {
  static_assert(is_tuple(tup) == true, "error serializeTuple argument must be a tuple");
  constexpr size_t s=size_of_tuple(tup);
  std::array<uint8_t, s> arr = {};

  // with constexpr std::copy of c++20 this will be more clearly written
  size_t curDestIdx = 0;
  for_each_in_tuple(tup, [&arr, &curDestIdx](auto &&x) {
      auto serializedElemArray = serializeIntegral(x);
      for(size_t i=0; i< serializedElemArray.size(); i++)
	arr[curDestIdx++]=serializedElemArray[i];
    });
  return arr;
}

template <typename T, typename S>
constexpr auto serializeTupleWithRegAdr([[maybe_unused]] const S sop,
					 const RegisterAddr7b regaddr,
					 const T& tup,
					 const bool littleendian=true)
{
  static_assert(is_tuple(tup) == true, "error serializeTuple argument must be a tuple");
  constexpr size_t sr = sizeof(RegisterAddr7b);
  constexpr size_t st=size_of_tuple(tup);
  std::array<uint8_t, sr+st> arr = {};

  // calculate 8 bits address (from 7 bit one) where bit0 indicate R or W
  // this convention has been borrowed to I²C bus
  // ugly IIFE because lack of constexpr ternary conditional
  assert((regaddr & (~0b1111111)) == 0);
  const auto regaddr8b = [&] {
    if constexpr (std::is_same_v<S, OpRead>)
      return regaddr | 0x80;
    else
      return regaddr;
  } ();
  
  size_t curDestIdx;
  // serialize register address
  for (curDestIdx=0; curDestIdx< sr; curDestIdx++) {
    arr[curDestIdx] = (regaddr8b >> (8*curDestIdx)) & 0xff ;
  }
  
  // serialize data
  // with constexpr std::copy of c++20 this will be more clearly written
  for_each_in_tuple(tup, [&arr, &curDestIdx](auto &&x) {
      auto serializedElemArray = serializeIntegral(x);
      for(size_t i=0; i< serializedElemArray.size(); i++)
	arr[curDestIdx++]=serializedElemArray[i];
    });
  return arr;
}

template <typename T, size_t S, typename Ot>
void unserializeTuple(const std::array<uint8_t, S> &arr, T tup, Ot regaddr) {
  static_assert(is_tuple(tup) == true, "error: unserializeTuple argument must be a tuple");
  static_assert((S-sizeof(Ot)) == sizeofTup(tup), "error: array and list of tupple size should be equal");

  // with constexpr std::copy of c++20 this will be more clearly written
  size_t curDestIdx = sizeof(Ot);
  for_each_in_tuple(tup, [&arr, &curDestIdx](auto &&x) {
      x = unSerializeIntegral<std::remove_reference_t<decltype(x)>>(arr, curDestIdx, true);
      curDestIdx += sizeof(x);
     });
 }


template<typename Tb>
constexpr size_t sizeofAny(const Tb& regVals)
{
   if constexpr (is_tuple(regVals)) {
       return  size_of_tuple(regVals);
     } else {
     return  sizeof(regVals);
   }
}


class registerBasedPeripheral
{
private:
  template<size_t SN>
  virtual bool send(const std::array<SN> &sarr) = 0;
  
  template<size_t RN>
  virtual bool receive(std::array<RN> &rarr) = 0;

  template<size_t SN, size_t RN>
  virtual bool exchange(const std::array<SN> &sarr, std::array<RN> &rarr) = 0;

  virtual void acquireBus(void) = 0;

  virtual void releaseBus(void) = 0;

public:

  template<typename Tb>
  bool sendAny(Driver &d, 
	       const RegisterAddr7b regaddr, const Tb& regVals)
  {
    if constexpr (is_tuple(regVals)) {
      const auto arr = serializeTupleWithRegAdr(OpWrite::V,
						regaddr,
						regVals);
      Send(&d, arr.size(), arr.data());
    } else { // Not a tupple
    const auto arr = serializeIntegralWithRegAdr(OpWrite::V,
						 regaddr,
						 regVals);
    Send(&d, arr.size(), arr.data());
  }
  return true;
}


template<typename Tb>
bool receiveAny(Driver &d, 
	     const RegisterAddr7b regaddr, Tb&& regVals)
{
  constexpr size_t st = sizeofAny(regVals);
  constexpr size_t as = st + sizeof(regaddr);
  const std::array<uint8_t, as> arr_s = {regaddr};
  std::array<uint8_t, as> arr_r;
  static_assert(sizeof(regaddr) == 1,
		"if sizeof(regaddr) != 1, regaddr serialisation and copy is mandatory");
  Exchange(&d, as, arr_s.data(), arr_r.data());
  for (size_t i=0; i<arr_r.size(); i++) {
    DebugTrace("AR_S[%d] = 0x%x", i, arr_s[i]);
    DebugTrace("AR_R[%d] = 0x%x", i, arr_r[i]);
  }
  if constexpr (is_tuple(regVals)) {
      unserializeTuple(arr_r, regVals, sizeof(regaddr));
    } else { // Not a tupple
    regVals = unSerializeIntegral<std::remove_reference_t<decltype(regVals)>> (arr_r, sizeof(regaddr));
  }

  return true;
}
    
template<typename Tb, typename S, typename ...Lasts>
bool transmitInternal(Driver &d,
	      [[maybe_unused]] const S sop,
	      const RegisterAddr7b regaddr, Tb&& buff,
	      Lasts&& ...lasts)
{
  bool ret=true;

  Select(&d);
  if constexpr (std::is_same_v<S, OpRead>) {
	ret &= receiveAny(d, regaddr|0x80, buff);
    } else if constexpr (std::is_same_v<S, OpWrite>) {
      ret &= sendAny(d, regaddr, buff);
    }
  Unselect(&d);

    
  if constexpr (sizeof...(lasts) > 0) 
    ret &= transmitInternal(d, lasts...);
  
  return ret;
}

}


  
template<typename Tb, typename S, typename ...Lasts>
bool transmit(Driver &d,
	      const S sop,
	      const RegisterAddr7b regaddr, Tb&& buff,
	      Lasts&& ...lasts)
{
  bool ret=true;
#if _USE_MUTUAL_EXCLUSION
  AcquireBus(&d);
#endif

    static_assert(std::is_same_v<S, OpRead> ||
		  std::is_same_v<S, OpWrite>,
		  "Type must be OpRead or OpWrite");

    ret = transmitInternal(d, sop, regaddr, buff, lasts...);
	    
#if _USE_MUTUAL_EXCLUSION
  ReleaseBus(&d);
#endif
  return ret;
}

