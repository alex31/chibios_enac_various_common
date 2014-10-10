#pragma once

#ifdef __cplusplus
 inline void* operator new( size_t size) _GLIBCXX_USE_NOEXCEPT { return malloc_m (size); }
 inline void* operator new[]( size_t size) _GLIBCXX_USE_NOEXCEPT { return malloc_m (size); }
 inline void operator delete( void *p) _GLIBCXX_USE_NOEXCEPT { free_m (p); }
 inline void operator delete[]( void *p) _GLIBCXX_USE_NOEXCEPT  { free_m (p); }
#endif
