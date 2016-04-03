#include "ch.h"
#include "tlsf_malloc.h"
#include "tlsf_heaps_conf.h"
#include "tlsf_bku.h"
#include "portage.h"

struct _tlsf_memory_heap_t
{
  tlsf_t tlsf;
  mutex_t *mtx;
};


#ifdef HEAP_BANK1_NAME
#define HEAP_BANK1_BUFFER HEAP_BANK1_NAME ## _buffer
#define HEAP_BANK1_MTX    HEAP_BANK1_NAME ## _mtx 
static uint8_t HEAP_BANK1_BUFFER[HEAP_BANK1_SIZE] __attribute__ ((section(HEAP_BANK1_SECTION), aligned(8))) ;
static MUTEX_DECL(HEAP_BANK1_MTX);
tlsf_memory_heap_t HEAP_BANK1_NAME;
#endif

#ifdef HEAP_BANK2_NAME
#define HEAP_BANK2_BUFFER HEAP_BANK2_NAME ## _buffer
#define HEAP_BANK2_MTX    HEAP_BANK2_NAME ## _mtx 
static uint8_t HEAP_BANK2_BUFFER[HEAP_BANK2_SIZE] __attribute__ ((section(HEAP_BANK2_SECTION), aligned(8))) ;
static MUTEX_DECL(HEAP_BANK2_MTX);
tlsf_memory_heap_t HEAP_BANK2_NAME;
#endif

#ifdef HEAP_BANK3_NAME
#define HEAP_BANK3_BUFFER HEAP_BANK3_NAME ## _buffer
#define HEAP_BANK3_MTX    HEAP_BANK3_NAME ## _mtx 
static uint8_t HEAP_BANK3_BUFFER[HEAP_BANK3_SIZE] __attribute__ ((section(HEAP_BANK3_SECTION), aligned(8))) ;
static MUTEX_DECL(HEAP_BANK3_MTX);
tlsf_memory_heap_t HEAP_BANK3_NAME;
#endif

static void stat_tlsf_walker (void* ptr, size_t size, int used, void* user);

void tlsf_init_heaps(void)
{
#ifdef HEAP_BANK1_NAME
  HEAP_BANK1_NAME.mtx = &HEAP_BANK1_MTX;
  HEAP_BANK1_NAME.tlsf = tlsf_create_with_pool(HEAP_BANK1_BUFFER, HEAP_BANK1_SIZE);
#endif
#ifdef HEAP_BANK2_NAME
  HEAP_BANK2_NAME.mtx = &HEAP_BANK2_MTX;
  HEAP_BANK2_NAME.tlsf = tlsf_create_with_pool(HEAP_BANK2_BUFFER, HEAP_BANK2_SIZE);
#endif
#ifdef HEAP_BANK3_NAME
  HEAP_BANK3_NAME.mtx = &HEAP_BANK3_MTX;
  HEAP_BANK3_NAME_NAME.tlsf = tlsf_create_with_pool(HEAP_BANK3_BUFFER, HEAP_BANK3_SIZE);
#endif
}

void* tlsf_malloc_r(tlsf_memory_heap_t *heap, size_t bytes)
{
  chMtxLock (heap->mtx);
  void *ret = tlsf_malloc (heap->tlsf, bytes);
  chMtxUnlock (heap->mtx);
  return ret;
}

void* tlsf_memalign_r(tlsf_memory_heap_t *heap, size_t align, size_t bytes)
{
  chMtxLock (heap->mtx);
  void *ret = tlsf_memalign (heap->tlsf, align, bytes);
  chMtxUnlock (heap->mtx);
  return ret;
}

void* tlsf_realloc_r(tlsf_memory_heap_t *heap, void* ptr, size_t bytes)
{
  chMtxLock (heap->mtx);
  void *ret = tlsf_realloc (heap->tlsf, ptr, bytes);
  chMtxUnlock (heap->mtx);
  return ret;
}

void  tlsf_free_r(tlsf_memory_heap_t *heap, void* ptr)
{
  chMtxLock (heap->mtx);
  tlsf_free (heap->tlsf, ptr);
  chMtxUnlock (heap->mtx);
}


void tlsf_stat_r (tlsf_memory_heap_t *heap, struct tlsf_stat_t *stat)
{
  stat->mused= stat->mfree = 0;
  chMtxLock (heap->mtx);
  tlsf_walk_pool (tlsf_get_pool(heap->tlsf),  &stat_tlsf_walker, stat);
  chMtxUnlock (heap->mtx);
}

static void stat_tlsf_walker (void* ptr, size_t size, int used, void* user)
{
  (void) ptr;
  struct tlsf_stat_t * tstat = (struct tlsf_stat_t *) user;
  if (used)
    tstat->mused  += size;
  else
    tstat->mfree += size;
}
