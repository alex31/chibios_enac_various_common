#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <initializer_list>
#include <tuple>
#include <vector>
#include <limits>
#include <typeinfo>
#include <cmath>
#include "stdutil.h"


namespace tuple_foreach
{
  template<int... Is>
  struct seq { };

  template<int N, int... Is>
  struct gen_seq : gen_seq<N - 1, N - 1, Is...> { };

  template<int... Is>
  struct gen_seq<0, Is...> : seq<Is...> { };

  template<typename T, typename F, int... Is>
  void for_each(T&& t, F f, seq<Is...>)
  {
    auto l __attribute__ ((unused)) = { (f(std::get<Is>(t)), 0)... };
  }
}

template<typename... Ts, typename F>
void for_each_in_tuple(std::tuple<Ts...> const& t, F f)
{
  tuple_foreach::for_each(t, f, tuple_foreach::gen_seq<sizeof...(Ts)>());
}


namespace flatten_byte_array {
  typedef unsigned long ulong;
  // g++-4.9  -Wall -Wextra -std=gnu++11  -fdiagnostics-color=auto tplt7_bis.cpp -o ess


  template<uint8_t... Is> struct seq{};
  template<uint8_t N, uint8_t... Is>
  struct gen_seq : gen_seq<N-1, N-1, Is...>{};
  template<uint8_t... Is>
  struct gen_seq<0, Is...> : seq<Is...>{};

  template<ulong N1, uint8_t... I1, ulong N2, uint8_t... I2>
  // Expansion pack
  constexpr std::array<uint8_t, N1+N2> concat(const std::array<uint8_t, N1>& a1, const std::array<uint8_t, N2>& a2, seq<I1...>, seq<I2...>){
    return { a1[I1]..., a2[I2]... };
  }

  template<ulong N1, ulong N2>
  // Initializer for the recursion
  constexpr std::array<uint8_t, N1+N2> concat(const std::array<uint8_t, N1>& a1, const std::array<uint8_t, N2>& a2){
    return concat(a1, a2, gen_seq<N1>{}, gen_seq<N2>{});
  }

  template<ulong N1, uint8_t... I1, ulong N2, uint8_t... I2, ulong N3, uint8_t... I3>
  // Expansion pack
  constexpr std::array<uint8_t, N1+N2+N3> concat(const std::array<uint8_t, N1>& a1, 
						 const std::array<uint8_t, N2>& a2, 
						 const std::array<uint8_t, N3>& a3, 
						 seq<I1...>, seq<I2...>, seq<I3...>){
    return { a1[I1]..., a2[I2]..., a3[I2]...};
  }

  template<ulong N1, ulong N2, ulong N3>
  // Initializer for the recursion
  constexpr std::array<uint8_t, N1+N2+N3> concat(const std::array<uint8_t, N1>& a1, 
						 const std::array<uint8_t, N2>& a2,
						 const std::array<uint8_t, N3>& a3){
    return concat(a1, a2, a3, gen_seq<N1>{}, gen_seq<N2>{}, gen_seq<N3>{});
  }
}


template <typename AT>
int findIndexOfOld(const AT& arr, const typename AT::value_type value)
{
  for (size_t idx=0; idx<arr.size(); idx++) {
    if (arr[idx] == value)
      return idx;
  }
  return -1;
}

template <typename T>
int findIndexOf(const std::vector<T> &vec, const T value)
{
  for (size_t idx=0; idx<vec.size(); idx++) {
    if (vec[idx] == value)
      return idx;
  }
  return -1;
}

template <typename AT>
int findIndexOfNearestOld(const AT& arr, const typename AT::value_type value)
{
  AT absDiff (arr.size());
  using  T = typename AT::value_type;
  T min = std::numeric_limits<T>::max();

  for (size_t idx=0; idx<absDiff.size(); idx++) {
    absDiff[idx] = std::abs (value - arr[idx]);
    if (absDiff[idx] < min)  
      min = absDiff[idx];
  }
  return findIndexOf (absDiff, min);
}



template <typename T>
int findIndexOfNearest(const std::vector<T> &vec, const T value)
{
  std::vector<T>  absDiff (vec.size());
  T min = std::numeric_limits<T>::max();
  int indexMin = -1;

  for (size_t idx=0; idx<vec.size(); idx++) {
    absDiff[idx] = std::abs (value - vec[idx]);
    //    DebugTrace ("absDiff[%d]= abs (%.2f - %.2f) = %.2f", idx, value, vec[idx], absDiff[idx]);
    if (absDiff[idx] < min) { 
      min = absDiff[idx];
      indexMin = idx;
      //      DebugTrace ("min = %.2f indexMin=%d", min, indexMin);
    }
  }

  //  DebugTrace ("return indexMin=%d", indexMin);
  return indexMin;
}

template <typename T>
int findIndexOfImmediateSup(const std::vector<T> &vec, const T value)
{
  for (size_t idx=0; idx<vec.size(); idx++) {
    if (vec[idx] > value)
      return idx;
  }

  //  DebugTrace ("return indexMin=%d", indexMin);
  return vec.size();
}

template <typename T>
int findIndexOfImmediateInf(const std::vector<T> &vec, const T value)
{
  for (size_t idx=vec.size()-1; idx>0; idx--) {
    if (vec[idx] < value)
      return idx;
  }
  
  //  DebugTrace ("return indexMin=%d", indexMin);
  return 0;
}

class SyslockRAII_fromIsr
{
  SyslockRAII_fromIsr(const SyslockRAII_fromIsr &) = delete;
  SyslockRAII_fromIsr& operator=(const SyslockRAII_fromIsr &) = delete;
public:
  SyslockRAII_fromIsr() {chSysLockFromISR();};
  ~SyslockRAII_fromIsr() {chSysUnlockFromISR();};
};


class SyslockRAII
{
  SyslockRAII(const SyslockRAII &) = delete;
  SyslockRAII& operator=(const SyslockRAII &) = delete;
public:
  SyslockRAII() {chSysLock();};
  ~SyslockRAII() {chSysUnlock();};
};

class MutexGuard
{
public:
  MutexGuard(const MutexGuard&) = delete;
  MutexGuard& operator=(const MutexGuard &) = delete;
  MutexGuard(MutexGuard &&) = delete;
  MutexGuard & operator=(MutexGuard &&) = delete;
  
  MutexGuard(mutex_t &_mut) : mut(_mut) {
    chMtxLock(&mut);
  };
  ~MutexGuard() {chMtxUnlock(&mut);};
private:
  mutex_t &mut;
};


// Type your code here, or load an example.
#include <cstdio>
#include <cstdint>
#include <array>
#include <bit>
#include <numbers> 

enum class Endianness {BIG, LITTLE};

template<typename T, std::size_t LL, std::size_t RL>
constexpr std::array<T, LL+RL> join(std::array<T, LL> rhs, std::array<T, RL> lhs)
{
	std::array<T, LL+RL> ar;
    
    auto current = std::copy(lhs.begin(), lhs.end(), ar.begin());
    std::copy(rhs.begin(), rhs.end(), current);
    
    return ar;
}

template <typename T>
constexpr auto bufferize(Endianness endns, T head) {
   //printf("bufferise type of size %u\n", sizeof(T));
  
  if (endns == Endianness::BIG) {
      if constexpr (sizeof(T) == 2) {
          head = std::bit_cast<T>(__builtin_bswap16(std::bit_cast<uint16_t>(head)));
      } else if constexpr (sizeof(T) == 4) {
          head = std::bit_cast<T>(__builtin_bswap32(std::bit_cast<uint32_t>(head)));
      } else if constexpr (sizeof(T) == 8) {
          head = std::bit_cast<T>(__builtin_bswap64(std::bit_cast<uint64_t>(head)));
      } 
  }
  return std::bit_cast<std::array<std::byte, sizeof(T)>>(head);  
}

template <typename HEAD, typename... TAILs>
constexpr auto bufferize(Endianness endns, HEAD head, TAILs... tail) {
    return join(bufferize(endns, tail...), bufferize(endns, head)); 
}


// int main() { 
//     constexpr auto array = bufferize(Endianness::LITTLE_ENDIAN,
//                            static_cast<float>(std::numbers::pi),
//                            static_cast<uint16_t>(0xdead), 
//                            static_cast<std::uint8_t>(0xba));
  
//     for (const auto& elem : array) 
//         printf("array_1>> elem = 0x%x\n", elem);


//     int a = 10;
//     short b = 0xdead;
//     float f = std::numbers::pi;
//     auto array2 = bufferize(Endianness::BIG_ENDIAN, f, a, b);
//     for (const auto& elem : array2) 
//        printf("array_2>> elem = 0x%x\n", elem);
// }



template < typename ... Types >
struct SizeOf;

template < typename TFirst >
struct SizeOf < TFirst >
{
    static const auto Value = (sizeof(TFirst));
};

template < typename TFirst, typename ... TRemaining >
struct SizeOf < TFirst, TRemaining ... >
{
    static const auto Value = (sizeof(TFirst) + SizeOf<TRemaining...>::Value);
};

//Used as const int size = SizeOf<int, char, double>::Value; // 4 + 1 + 8 = 13


