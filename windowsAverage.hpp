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
  const T& operator[] (const ssize_t i);
  static constexpr size_t size(void) {return N;};
protected:
  T accum;
  std::array<T, N> ring;
private:
  size_t index;
};

template <typename T, size_t N, typename L>
WindowAverage<T, N, L>::WindowAverage (void) :
  accum(T()),
  index(0)
{
  static_assert(std::is_integral<T>::value, "Integral type required");
  ring.fill(T());
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
const T& WindowAverage<T, N, L>::operator[] (const ssize_t i)
{
  return ring[(N+index+i+1)%N];
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
  T getMean () const {return getSum() / (N-(2*M));};
  T getSum () const;
};


template <typename T, size_t N, size_t M, typename L>
T WindowMedianAverage<T, N, M, L>::getSum () const
{
  static_assert(N > (2*M), "array need to be larger than median elimitated number of elements");
  L::lock();
  std::array<T, N> toSort = WindowAverage<T, N, L>::ring; // make inplace sort on a copy
  auto laccum =  WindowAverage<T, N, L>::accum ;
  L::unlock();
  
  // optim : no need to sort all the array
  //  std::sort(toSort.begin(), toSort.end());
  for (size_t m=0; m<M; m++) {
    std::nth_element(toSort.begin(), toSort.begin() + m, toSort.end());
    laccum -= *(toSort.begin() + m);
    std::nth_element(toSort.begin(), toSort.end() - 1 -m, toSort.end());
    laccum -= *(toSort.end() - 1 - m);
  }
  
  return laccum;
}


template <typename T, size_t N, typename L=Lock::None>
class ResizableWindowAverage
{
public:
  ResizableWindowAverage (void);
  void push (const T i);
  T getSum (void)  const {return accum;};
  T getMean (void) const {return getSum() / size();};
  //  size_t size(void) const {return std::distance(ring.begin(), dynEnd);};
  ssize_t size(void) const {return dynEnd - ring.begin();};
  void resize(const size_t s);
  const std::pair<const T, const T> getMinMax(void) const;
  const T& operator[] (const ssize_t i) const;
  static constexpr size_t maxsize(void) {return N;};
protected:
  T accum;
  std::array<T, N> ring;
private:
  typename std::array<T, N>::iterator current;
  typename std::array<T, N>::iterator dynEnd;
};

template <typename T, size_t N, typename L>
ResizableWindowAverage<T, N, L>::ResizableWindowAverage (void) :
  accum(T()),
  current(ring.begin()),
  dynEnd(ring.end())
{
  static_assert(std::is_integral<T>::value, "Integral type required");
  ring.fill(T());
}

template <typename T, size_t N, typename L>
void ResizableWindowAverage<T, N, L>::resize(const size_t s)
{
  L::lock();
  const ssize_t previousSize = size();
  const ssize_t newSize = std::min(s, N);
  if (newSize < previousSize) {
    std::array<T, N> nring = ring;
    for (ssize_t i=0; i<newSize; i++)
      nring[i]=(*this)[i-newSize];
    ring = nring;
    dynEnd = ring.begin() + newSize;
    current=dynEnd-1;
  } else if (newSize > previousSize) {
    const T mean = getMean();
    size_t diff = newSize - previousSize;
    dynEnd = ring.begin() + newSize;
    const auto oldCurrent = current;
    while (diff--) {
      push(mean);
    }
    current = oldCurrent;
  }
  accum = std::accumulate(ring.begin(), dynEnd, 0);
  L::unlock();
}


template <typename T, size_t N, typename L>
void ResizableWindowAverage<T, N, L>::push (const T i)
{
  using Signed_T = typename std::make_signed<T>::type;

  if (++current >= dynEnd)
    current = ring.begin();

  // calculate diff between in item and out item
  // to be able to update accum in one atomic 
  // instruction.
  // diff sould be of a signed version of the T type if T is unsigned
  const Signed_T diff = static_cast<Signed_T>(i) -
			static_cast<Signed_T>(*current);

  // atomic operation, avoid costly lock here
  // https://godbolt.org/g/ffxjRM
  accum += diff;
  // printf("DBG> replace [@%ld/%ld] %d by %d\n", current-ring.begin(),
  // 	 dynEnd - ring.begin(),
  // 	 *current, i);
  *current = i;
}

template <typename T, size_t N, typename L>
const T& ResizableWindowAverage<T, N, L>::operator[] (const ssize_t i) const
{
  const size_t index = current - ring.begin();
  return ring[(size() + index + i + 1) % size()];
}

template <typename T, size_t N, typename L>
const std::pair<const T, const T> ResizableWindowAverage<T, N, L>::getMinMax(void) const
{
  auto p = std::minmax_element(ring.begin(), dynEnd);
  return std::make_pair(*p.first, *p.second);
}


template <typename T, size_t N, typename L=Lock::None>
class ResizableWindowMedianAverage : public ResizableWindowAverage<T, N, L>
{
public:
  T     getSum (const bool fromIsr=false) const;
  T     getMean (void) const {return getSum() / (ResizableWindowAverage<T, N, L>::size()
						 - (2 * medianFilterSize));};
  T     getMeanFromIsr (void) const {return getSum(true) / (ResizableWindowAverage<T, N, L>::size()
						 - (2 * medianFilterSize));};
  void  setMedianFilterSize(const size_t mfs);
  size_t getMedianFilterSize(void) {return medianFilterSize;};
        ResizableWindowMedianAverage(const size_t mfs=0) {
	  setMedianFilterSize(mfs);
	};
private:
  size_t medianFilterSize;
};

template <typename T, size_t N, typename L>
void ResizableWindowMedianAverage<T, N, L>::setMedianFilterSize(const size_t mfs)
{
  if ((2 * mfs) < N)
    medianFilterSize = mfs;
}

template <typename T, size_t N, typename L>
T ResizableWindowMedianAverage<T, N, L>::getSum (const bool fromIsr) const
{
  if (not fromIsr)
    L::lock();
  std::array<T, N> toSort = ResizableWindowAverage<T, N, L>::ring; // make inplace sort on a copy
  auto laccum =  ResizableWindowAverage<T, N, L>::accum ;
  if (not fromIsr)
    L::unlock();
  
  const auto tosort_dynEnd = toSort.begin()+ResizableWindowAverage<T, N, L>::size();

  // optim : no need to sort all the array, just the median eliminated elements
  for (size_t m=0; m<medianFilterSize; m++) {
    std::nth_element(toSort.begin(), toSort.begin() + m, tosort_dynEnd);
    laccum -= *(toSort.begin() + m);
    std::nth_element(toSort.begin(), tosort_dynEnd - 1 - m, tosort_dynEnd);
    laccum -= *(tosort_dynEnd - 1 - m);
  }

  return laccum;
}

