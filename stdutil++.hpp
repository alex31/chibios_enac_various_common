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


//#include "globalVar.h"
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

