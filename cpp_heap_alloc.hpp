#pragma once

//#include "stdutil.h"

#ifdef __cplusplus
extern "C++" {

#if defined(__cpp_exceptions) 
inline void* operator new(size_t size)  { return malloc_m (size); }
#else
inline void* operator new(size_t size) noexcept { return malloc_m (size); }
#endif

inline void operator delete( void *p) noexcept {free_m (p);}
inline void operator delete[]( void *p) noexcept  { free_m (p); }
inline void operator delete( void *p, size_t s) noexcept { (void) s; free_m (p); }
inline void operator delete[]( void *p, size_t s) noexcept  {  (void) s; free_m (p); }

}

// to avoid accidental use of stdlib malloc and
extern "C" {
  inline void *malloc(size_t) {chSysHalt("use of malloc is forbidden"); return (void*) 0;}
}



#endif
