#pragma once

#include "ch.h"
#include "hal.h"
#include "ff.h"
#include "sdio.h"
#include "rtcAccess.h"
#include "stdutil.h"
#include <cstring>
#include <stdlib.h>
#include <cstdio>
#include <ctype.h>
#include <reent.h>
#include <cstddef>
#include <stdarg.h>
#include <algorithm>
#include <etl/vector.h>

/*
  TODO:
 */
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
      SdView ret = view;
      return ret;
    }
    FIL* getFil(void) const {
      FIL* ret = fil;
      return ret;
    }
    void setView(const SdView &f) {
      view = f;
    }
    void waitSem(void) const {
      chBSemWait(&sem);
    }
    void signalSem(void) const {
      chBSemSignal(&sem);
    }
    void setSync(const bool sync) {shouldSync=sync;}
    bool needSync(void) const {return shouldSync;} 

  private:
    SdView view;
    FIL* const  fil;
    bool shouldSync=false;
    mutable binary_semaphore_t sem =
      _BSEMAPHORE_DATA(sem, false);
  };

  static constexpr size_t NO_BORROW = 0U; 

  
public:
  SdLiteLogBase(time_secs_t syncPeriodSeconds);

  virtual ~SdLiteLogBase(void);
  SdLiteStatus openLog(const char* prefix, const char* directoryName,
		       const size_t minimalIndex=0U);
  SdLiteStatus closeLog(void);
  static SdLiteStatus initOnce(uint32_t* freeSpaceInKo);
  static void terminate(const TerminateBehavior tb = TerminateBehavior::DONT_WAIT);
  static void flushAllLogs(void);
  static SdLiteStatus closeAllLogs(void);
  static SdLiteStatus getFileNameIndex(const char* prefix, const char* directoryName,
				       size_t *index);

protected:
  static FATFS fatfs; 
  static size_t nbBytesWritten;
  static thread_t *workerThdPtr;
  static uint32_t freeSpaceInKo;
  static mailbox_t mbChunk;
  static etl::vector<SdLiteLogBase *, 16> instances;



  struct _reent reent = _REENT_INIT(reent);
  SdLiteStatus status = SdLiteStatus::NOT_READY;
  FIL fil{};
  size_t borrowSize = NO_BORROW;
  sysinterval_t syncPeriod;
  systime_t     syncTs;
  static SdLiteStatus getFileName(const char* prefix, const char* directoryName,
				  char* nextFileName, const size_t nameLength,
				  const int indexOffset, const size_t minimalIndex=0U);
  static int32_t uiGetIndexOfLogFile(const char* prefix, const char* fileName);
  virtual void flushHalfBuffer(void) = 0;

  static void workerThd(void* opt);
};



template <size_t N>
class SdLiteLog : public SdLiteLogBase
{
 public:
  SdLiteLog(time_secs_t syncPeriodSeconds) : SdLiteLogBase(syncPeriodSeconds),
					     chunk0(&fil), chunk1(&fil) {};
  ~SdLiteLog() {flushHalfBuffer();}
  SdLiteStatus writeFmt(int borrowLen, const char* fmt, ...) 
    __attribute__ ((format (printf, 3, 4)));
  SdLiteStatus vwriteFmt(int borrowLen, const char* fmt, va_list *ap);
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

  void flushHalfBuffer(void) override;
  size_t getSize(void);
  size_t getRemainingSize(void);

  std::byte buffer[N];
  std::byte *halfPtr = buffer;
  std::byte *writePtr = buffer;
  std::byte * const secondHalf = buffer + N/2;
  std::byte * const secondEnd = buffer + N;
 
  size_t    currLen=0U;
  SdChunk chunk0, chunk1;
};


template <size_t N>
SdLiteStatus SdLiteLog<N>::writeFmt(int borrowLen,
				    const char* fmt, ...) 
{
  va_list ap;

  va_start(ap, fmt);
  const auto s = vwriteFmt(borrowLen, fmt, &ap);
  va_end(ap);
  
  return s;
}

template <size_t N>
SdLiteStatus SdLiteLog<N>::vwriteFmt(int borrowLen,
				    const char* fmt, va_list *ap) 
{
  if (borrowLen == 0)
    borrowLen =  N/2;
  
  const bool logMode = (syncPeriod == 0);
  
  auto [s, b] = borrow<char>(borrowLen);
  if (s == SdLiteStatus::OK) {
    int nbbytes = std::min(borrowLen-2,
			   _vsnprintf_r(&reent, b, borrowLen-2,
					fmt, *ap));
    // DebugTrace("b=%p len=%d nbbytes=%d", b, borrowLen, nbbytes);
    if (nbbytes > 0) {
      b[nbbytes-1] = '\r';
      b[nbbytes++] = '\n';
      giveBack(nbbytes);
      if (logMode)
	flushHalfBuffer();
    } else {
      DebugTrace("writeFmt _vsnprintf_r error : "
		 "fmt=%s b=%p len=%d nbbytes=%d", fmt, b, borrowLen, nbbytes);
    }
  } else {
    DebugTrace("writeFmt borrow error");
  }
  
  return s;
}



template <size_t N>
template <typename T>
std::tuple<SdLiteStatus, T&> SdLiteLog<N>::borrow(void)
{
  constexpr size_t l = sizeof(T);
  T* tptr = nullptr;

  SdChunk &chunk = (halfPtr == buffer) ? chunk0 : chunk1;
  chunk.waitSem();
  chunk.signalSem();
 
  // give back last borrowed
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

  SdChunk &chunk = (halfPtr == buffer) ? chunk0 : chunk1;
  chunk.waitSem();
  chunk.signalSem();

  // give back last borrowed
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
  SdChunk &chunk = (halfPtr == buffer) ? chunk0 : chunk1;
  chunk.waitSem();
  chunk.setView(SdView(halfPtr, getSize()));
  if (chTimeIsInRangeX(chVTGetSystemTimeX(), syncTs, syncTs+syncPeriod)) {
    chunk.setSync(false);
  } else {
    syncTs = chVTGetSystemTimeX();
    chunk.setSync(true);
  }
  //  DebugTrace("flushHalfBuffer w=%p h=%p [%p %p] len=%u",
  //	      writePtr, halfPtr, buffer, secondHalf, getSize());
  chMBPostTimeout(&mbChunk, (msg_t) &chunk, TIME_INFINITE);
  writePtr = halfPtr = (halfPtr == buffer) ? secondHalf : buffer;
}




