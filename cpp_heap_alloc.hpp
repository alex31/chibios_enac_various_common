#pragma once

//#include "stdutil.h"

#ifdef __cplusplus
extern "C++" {
// inline void* operator new( size_t size) noexcept {
//   void *p =  malloc_m (size);
//   DebugTrace("new %p", p);
//   return p;
// }
inline void* operator new( size_t size) noexcept {return  malloc_m (size);}
//inline void* operator new[]( size_t size) noexcept { return malloc_m (size); }
inline void operator delete( void *p) noexcept {
  //  DebugTrace("delete %p", p);
  //  chThdSleepMilliseconds(50);
  free_m (p);
}
inline void operator delete[]( void *p) noexcept  { free_m (p); }
inline void operator delete( void *p, size_t s) noexcept { (void) s; free_m (p); }
inline void operator delete[]( void *p, size_t s) noexcept  {  (void) s; free_m (p); }
}

// to avoid accidental use of stdlib malloc and
extern "C" {
  inline void *malloc(size_t) {chSysHalt("use of malloc is forbidden"); return (void*) 0;}
}
  
#endif
