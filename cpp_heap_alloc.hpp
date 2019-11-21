#pragma once

#ifdef __cplusplus
inline void* operator new( size_t size) noexcept { return malloc_m (size); }
inline void* operator new[]( size_t size) noexcept { return malloc_m (size); }
inline void operator delete( void *p) noexcept { free_m (p); }
inline void operator delete[]( void *p) noexcept  { free_m (p); }
inline void operator delete( void *p, size_t s) noexcept { (void) s; free_m (p); }
inline void operator delete[]( void *p, size_t s) noexcept  {  (void) s; free_m (p); }
#endif
