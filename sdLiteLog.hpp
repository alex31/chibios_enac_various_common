#pragma once

#include "ch.h"
#include "hal.h"
#include <cstring>
#include <stdlib.h>
#include <cstdio>
#include "ff.h"
#include "sdio.h"
#include "rtcAccess.h"
#include <ctype.h>
#include "stdutil.h"
#include <reent.h>
#include <cstddef>
#include <algorithm>
#include <etl/vector.h>

static inline  constexpr size_t numberMax = 9999;

enum class SdLiteStatus  {
	OK,
	NOCARD,
	OUT_OF_RAM,
	TOO_BIG,
	FATFS_ERROR,
	FATFS_NOENT,
	FSFULL,
	NOTHREAD,
	INTERNAL_ERROR,
	LOGNUM_ERROR,
	WAS_LAUNCHED,
	BORROW_ERROR,
	NOT_READY
};

enum class TerminateBehavior {
	WAIT,
	DONT_WAIT
};

// class MutexRAII
// {
// public:
//   MutexRAII(mutex_t *_mut) : mut(_mut) {chMtxLock(mut);};
//   ~MutexRAII() {chMtxUnlock(mut);};
// private:
//   mutex_t *mut;
// };
struct SdView {
  SdView(std::byte * const _start, const size_t _len) {
    start = _start;
    len = _len;
  }
  SdView(const SdView& f) {
    start = f.start;
    len = f.len;
  }
  SdView() {
    start = nullptr;
    len = 0;
  }
  
  const SdView& operator=(const SdView& f) {
    start = f.start;
    len = f.len;
    return f;
  }
  
  void set(std::byte * const _start, const size_t _len) {
    start = _start;
    len = _len;
  }
  
  std::tuple<std::byte *, size_t> get (void) const {
    std::byte * const s = start;
    const size_t l = len;
    return {s, l};
  }
  
private:
  std::byte *start;
    size_t len;
};


class SdLiteLogBase {
  struct SdChunk {
    SdChunk(FIL* const  _fil) : view(SdView()), fil(_fil) {};
      
    SdView getView(void) const {
      chMtxLock(&mut);
      SdView ret = view;
      chMtxUnlock(&mut);
      return ret;
    }
    FIL* getFil(void) const {
      chMtxLock(&mut);
      FIL* ret = fil;
      chMtxUnlock(&mut);
      return ret;
    }
    void setView(const SdView &f) {
      chMtxLock(&mut);
      view = f;
      chMtxUnlock(&mut);
    }
    
  private:
    SdView view;
    FIL* const  fil;
    mutable mutex_t mut = _MUTEX_DATA(mut);
  };

  static constexpr size_t NO_BORROW = 0U; 

  
public:
  SdLiteLogBase(void) {};
  ~SdLiteLogBase(void);
  SdLiteStatus openLog(const char* prefix, const char* directoryName);
  SdLiteStatus closeLog(void);
  static void terminate(const TerminateBehavior tb = TerminateBehavior::DONT_WAIT);

protected:
  static FATFS fatfs; 
  static struct _reent reent;
  static size_t nbBytesWritten;
  static thread_t *workerThdPtr;
  static uint32_t freeSpaceInKo;
  static mailbox_t mbChunk;
  static etl::vector<SdLiteLogBase *, 16> instances;



  SdLiteStatus status = SdLiteStatus::NOT_READY;
  FIL fil{};
  size_t borrowSize = NO_BORROW;
  static SdLiteStatus initOnce(uint32_t* freeSpaceInKo);
  static SdLiteStatus getFileName(const char* prefix, const char* directoryName,
				  char* nextFileName, const size_t nameLength,
				  const int indexOffset);
  static int32_t uiGetIndexOfLogFile(const char* prefix, const char* fileName);
  static SdLiteStatus closeAllLogs(void);

  static void workerThd(void* opt);
};





template <size_t N>
class SdLiteLog : public SdLiteLogBase
{
 public:
  SdLiteLog() :  chunk(&fil) {};
  ~SdLiteLog() {flushHalfBuffer();}
  SdLiteStatus writeFmt(const char* fmt, ...) // low perf, high lag, reentrant API
    __attribute__ ((format (printf, 2, 3)));
  template <typename T>
  std::tuple<SdLiteStatus, T&> borrow(void);
  template <typename T>
  std::tuple<SdLiteStatus, T*> borrow(const size_t l);
  SdLiteStatus giveBack(const size_t l);
 
  static size_t getFreeSpace(void) {return freeSpaceInKo;}
  static size_t getTotalWritten(void) {return nbBytesWritten;}
private:
  template<typename T>
  static constexpr T* nextAlignedAddr(T* const addr, const size_t align)
  {
    return reinterpret_cast<T *> ((reinterpret_cast<size_t>(addr) + align - 1U)
				  & ~(align -1U));
  }

  void flushHalfBuffer();
  size_t getSize(void);
  size_t getRemainingSize(void);

  std::byte buffer[N];
  std::byte *halfPtr = buffer;
  std::byte *writePtr = buffer;
  std::byte * const secondHalf = buffer + N/2;
  std::byte * const secondEnd = buffer + N;
  size_t    currLen=0U;
  SdChunk chunk;
  mutable mutex_t mutFmt = _MUTEX_DATA(mutFmt);
};


template <size_t N>
SdLiteStatus SdLiteLog<N>::writeFmt(const char* fmt, ...) 
{
  using Buf_t = std::array<char, (N/2)>;
  
  va_list ap;
  va_start(ap, fmt);
  
  chMtxLock(&mutFmt); // no queue, just a lock to make API reentrant,
		      // enough for low bandwidth log
  auto [s, b] = borrow<Buf_t>();
  if (s == SdLiteStatus::OK) {
    int nbbytes = std::min(static_cast<int>((N/2)-2),
			   _vsnprintf_r(&reent, b.data(), (N/2)-2,  fmt, ap));
    if (nbbytes > 0) {
      b[nbbytes++] = '\r';
      b[nbbytes++] = '\n';
      giveBack(nbbytes);
      flushHalfBuffer();
      f_sync(&fil);
    }
  } else {
    DebugTrace("writeFmt borrow error");
  }
  
  va_end(ap);
  chMtxUnlock(&mutFmt);
  return s;
}



template <size_t N>
template <typename T>
std::tuple<SdLiteStatus, T&> SdLiteLog<N>::borrow(void)
{
  constexpr size_t l = sizeof(T);
  T* tptr = nullptr;

  if (borrowSize != NO_BORROW) 
    writePtr += borrowSize;
  
  if (l > N/2) {
    borrowSize = NO_BORROW;
    return  {SdLiteStatus::TOO_BIG, *tptr};
  } else if (l > getRemainingSize()) {
    flushHalfBuffer();
  }

  borrowSize = l;
  tptr = reinterpret_cast<T *>(writePtr);
  return {SdLiteStatus::OK, *tptr};
}

template <size_t N>
template <typename T>
std::tuple<SdLiteStatus, T*> SdLiteLog<N>::borrow(const size_t l)
{
  T* tptr = nullptr;
  if (borrowSize != NO_BORROW) 
    writePtr += borrowSize;
  
  if (l > N/2) {
    borrowSize = NO_BORROW;
    return  {SdLiteStatus::TOO_BIG, tptr};
  } else if (l > getRemainingSize()) {
    flushHalfBuffer();
  }

  borrowSize = l;
  tptr = reinterpret_cast<T *>(writePtr);
  return {SdLiteStatus::OK, tptr};
}

template <size_t N>
SdLiteStatus SdLiteLog<N>::giveBack(const size_t l)
{
  if (borrowSize == NO_BORROW) {
    return SdLiteStatus::BORROW_ERROR;
  } 
  writePtr += l;
  borrowSize = NO_BORROW; 
  return SdLiteStatus::OK;
}

template <size_t N>
size_t  SdLiteLog<N>::getRemainingSize(void)
{
  return halfPtr == buffer ?
   secondHalf - writePtr  :
   secondEnd  - writePtr  ;
}

template <size_t N>
size_t  SdLiteLog<N>::getSize(void)
{
  return halfPtr == buffer ?
    writePtr - buffer:
    writePtr - secondHalf ;
}

template <size_t N>
void SdLiteLog<N>::flushHalfBuffer()
{
  chunk.setView(SdView(halfPtr, getSize()));
  //  DebugTrace("flushHalfBuffer w=%p h=%p [%p %p] len=%u",
  //	     writePtr, halfPtr, buffer, secondHalf, getSize());
  chMBPostTimeout(&mbChunk, (msg_t) &chunk, TIME_INFINITE);
  writePtr = halfPtr = (halfPtr == buffer) ? secondHalf : buffer;
}




