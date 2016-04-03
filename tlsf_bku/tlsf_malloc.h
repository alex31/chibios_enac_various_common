#pragma once
#include "tlsf_heaps_conf.h"

#if defined(__cplusplus)
//extern "C" {
#endif

struct _tlsf_memory_heap_t;
typedef struct _tlsf_memory_heap_t tlsf_memory_heap_t;

#ifdef HEAP_BANK1_NAME
extern tlsf_memory_heap_t HEAP_BANK1_NAME;
#endif

#ifdef HEAP_BANK2_NAME
extern tlsf_memory_heap_t HEAP_BANK2_NAME;
#endif

#ifdef HEAP_BANK3_NAME
extern tlsf_memory_heap_t HEAP_BANK3_NAME;
#endif



struct tlsf_stat_t {
  size_t mfree;
  size_t mused;
};



/* Create/destroy a memory pool. */
void tlsf_init_heaps(void);


/* malloc/memalign/realloc/free replacements. */
void* tlsf_malloc_r(tlsf_memory_heap_t *heap, size_t bytes);
void* tlsf_memalign_r(tlsf_memory_heap_t *heap, size_t align, size_t bytes);
void* tlsf_realloc_r(tlsf_memory_heap_t *heap, void* ptr, size_t size);
void  tlsf_free_r(tlsf_memory_heap_t *heap, void* ptr);


/* Debugging. */
void tlsf_stat_r (tlsf_memory_heap_t *heap, struct tlsf_stat_t *stat);

#if defined(__cplusplus)
};
#endif
