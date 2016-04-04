#pragma once
#include "tlsf_heaps_conf.h"

#if defined(__cplusplus)
//extern "C" {
#endif

struct _tlsf_memory_heap_t;
typedef struct _tlsf_memory_heap_t tlsf_memory_heap_t;

#ifdef HEAP_CCM
extern tlsf_memory_heap_t HEAP_CCM;
#endif

#ifdef HEAP_SRAM
extern tlsf_memory_heap_t HEAP_SRAM;
#endif

#ifdef HEAP_EXTERN
extern tlsf_memory_heap_t HEAP_EXTERN;
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

/* get memory heap base addr*/
void* tlsf_get_heap_addr(const tlsf_memory_heap_t *heap);

/* Returns nonzero if any internal consistency check fails. */
int tlsf_check_r (tlsf_memory_heap_t *heap);

#if defined(__cplusplus)
};
#endif
