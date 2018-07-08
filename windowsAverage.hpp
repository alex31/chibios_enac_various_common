#pragma once

#include <algorithm>
#include <functional>
#include <numeric>
#include <array>
#include <type_traits>

template <typename T, size_t N>
class WindowAverage
{
public:
  WindowAverage (void);
  void push (const T i);
  T getSum (void)  const {return accum;};
  T getMean (void) const {return accum/N;};
  const T& operator[] (const size_t i);
  constexpr size_t size(void) {return N;};
protected:
  T accum;
  std::array<T, N> ring;
private:
    T index;
};

template <typename T, size_t N>
WindowAverage<T, N>::WindowAverage (void) :
  accum(0),
  index(0)
{
  static_assert(std::is_integral<T>::value, "Integral type required");
  ring.fill(0);
}


template <typename T, size_t N>
void WindowAverage<T, N>::push (const T i)
{
  accum += i;
  // elegant but ineffective, see https://godbolt.org/g/XbNCPa
  // index = (index+1) % N;
  
  if (++index == N)
    index = 0;
  accum -= ring[index];
  ring[index] = i;
}

template <typename T, size_t N>
const T& WindowAverage<T, N>::operator[] (const size_t i)
{
  return ring[(N+index+i)%N];
}



template <typename T, size_t N, size_t M>
class WindowMedianAverage : public WindowAverage<T, N>
{
public:
  T getMean (void) const;
};


template <typename T, size_t N, size_t M>
T WindowMedianAverage<T, N, M>::getMean (void) const
{
  static_assert(N > (2*M), "array need to be larger than median elimitated number of elements");
  std::array<T, N> toSort = WindowAverage<T, N>::ring; // make inplace sort on a copy
 
  std::sort(toSort.begin(), toSort.end());
  const T medianAccum = WindowAverage<T, N>::accum -
    std::accumulate(toSort.begin(), toSort.begin()+M, 0) -
    std::accumulate(toSort.end()-M, toSort.end(), 0);
  return medianAccum / (N-(2*M));
}
