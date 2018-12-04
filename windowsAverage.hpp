#pragma once

#include <algorithm>
#include <functional>
#include <numeric>
#include <array>
#include <type_traits>

namespace Lock {
  class None {
    public:
    static void lock(void) {};
    static void unlock(void) {};
  };
};


template <typename T, size_t N, typename L=Lock::None>
class WindowAverage
{
public:
  WindowAverage (void);
  void push (const T i);
  T getSum (void)  const {return accum;};
  T getMean (void) const {return accum/N;};
  const std::pair<const T, const T> getMinMax(void) const;
  const T& operator[] (const size_t i);
  static constexpr size_t size(void) {return N;};
protected:
  T accum;
  std::array<T, N> ring;
private:
    T index;
};

template <typename T, size_t N, typename L>
WindowAverage<T, N, L>::WindowAverage (void) :
  accum(0),
  index(0)
{
  static_assert(std::is_integral<T>::value, "Integral type required");
  ring.fill(0);
}


template <typename T, size_t N, typename L>
void WindowAverage<T, N, L>::push (const T i)
{
  using Signed_T = typename std::make_signed<T>::type;

  // elegant but ineffective, see https://godbolt.org/g/XbNCPa
  // index = (index+1) % N;
  if (++index == N)
    index = 0;

  // calculate diff between in item and out item
  // to be able to update accum in one atomic 
  // instruction.
  // diff sould be of a signed version of the T type if T is unsigned
  const Signed_T diff = static_cast<Signed_T>(i) -
                        static_cast<Signed_T>(ring[index]);

  // atomic operation, avoid costly lock here
  // https://godbolt.org/g/ffxjRM
  accum += diff;
  
  ring[index] = i;
}

template <typename T, size_t N, typename L>
const T& WindowAverage<T, N, L>::operator[] (const size_t i)
{
  return ring[(N+index+i)%N];
}

template <typename T, size_t N, typename L>
const std::pair<const T, const T> WindowAverage<T, N, L>::getMinMax(void) const
{
  auto p = std::minmax_element(ring.begin(), ring.end());
  return std::make_pair(*p.first, *p.second);
}



template <typename T, size_t N, size_t M, typename L=Lock::None>
class WindowMedianAverage : public WindowAverage<T, N, L>
{
public:
  T getMean () const;
};


template <typename T, size_t N, size_t M, typename L>
T WindowMedianAverage<T, N, M, L>::getMean () const
{
  static_assert(N > (2*M), "array need to be larger than median elimitated number of elements");
  L::lock();
  std::array<T, N> toSort = WindowAverage<T, N, L>::ring; // make inplace sort on a copy
  const auto laccum =  WindowAverage<T, N, L>::accum ;
  L::unlock();
  
  // optim : no need to sort all the array
  //  std::sort(toSort.begin(), toSort.end());
  for (size_t m=0; m<M; m++) {
    std::nth_element(toSort.begin(), toSort.begin()+m, toSort.end());
    std::nth_element(toSort.begin(), toSort.end()-m, toSort.end());
  }
  const T medianAccum = laccum -
    std::accumulate(toSort.begin(), toSort.begin()+M, 0) -
    std::accumulate(toSort.end()-M, toSort.end(), 0);
  return medianAccum / (N-(2*M));
}
