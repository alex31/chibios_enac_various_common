#ifdef __cplusplus
void* operator new( size_t size) _GLIBCXX_USE_NOEXCEPT { return malloc_m (size); }
void* operator new[]( size_t size) _GLIBCXX_USE_NOEXCEPT { return malloc_m (size); }
void operator delete( void *p) _GLIBCXX_USE_NOEXCEPT { free_m (p); }
void operator delete[]( void *p) _GLIBCXX_USE_NOEXCEPT  { free_m (p); }
#endif
