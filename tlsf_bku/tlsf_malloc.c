#include "ch.h"
#include "tlsf_malloc.h"
#include "tlsf_heaps_conf.h"
#include "tlsf.h"
#include "portage.h"

struct _tlsf_memory_heap_t
{
  tlsf_t tlsf;
  mutex_t *mtx;
};


#ifdef HEAP_CCM
#define HEAP_CCM_BUFFER HEAP_CCM ## _buffer
#define HEAP_CCM_MTX    HEAP_CCM ## _mtx 
static uint8_t HEAP_CCM_BUFFER[HEAP_CCM_SIZE] __attribute__ ((section(HEAP_CCM_SECTION), aligned(8))) ;
static MUTEX_DECL(HEAP_CCM_MTX);
tlsf_memory_heap_t HEAP_CCM;
#endif

#ifdef HEAP_SRAM
#define HEAP_SRAM_BUFFER HEAP_SRAM ## _buffer
#define HEAP_SRAM_MTX    HEAP_SRAM ## _mtx 
static uint8_t HEAP_SRAM_BUFFER[HEAP_SRAM_SIZE] __attribute__ ((section(HEAP_SRAM_SECTION), aligned(8))) ;
static MUTEX_DECL(HEAP_SRAM_MTX);
tlsf_memory_heap_t HEAP_SRAM;
#endif

#ifdef HEAP_EXTERN
#define HEAP_EXTERN_BUFFER HEAP_EXTERN ## _buffer
#define HEAP_EXTERN_MTX    HEAP_EXTERN ## _mtx 
static uint8_t HEAP_EXTERN_BUFFER[HEAP_EXTERN_SIZE] __attribute__ ((section(HEAP_EXTERN_SECTION), aligned(8))) ;
static MUTEX_DECL(HEAP_EXTERN_MTX);
tlsf_memory_heap_t HEAP_EXTERN;
#endif

static void stat_tlsf_walker (void* ptr, size_t size, int used, void* user);

static void error_cb (const char* msg) {
  chSysHalt(msg);
}

void tlsf_init_heaps(void)
{
#ifdef HEAP_CCM
  HEAP_CCM.mtx = &HEAP_CCM_MTX;
  HEAP_CCM.tlsf = tlsf_create_with_pool(HEAP_CCM_BUFFER, HEAP_CCM_SIZE, &error_cb);
#endif
#ifdef HEAP_SRAM
  HEAP_SRAM.mtx = &HEAP_SRAM_MTX;
  HEAP_SRAM.tlsf = tlsf_create_with_pool(HEAP_SRAM_BUFFER, HEAP_SRAM_SIZE, &error_cb);
#endif
#ifdef HEAP_EXTERN
  HEAP_EXTERN.mtx = &HEAP_EXTERN_MTX;
  HEAP_EXTERN.tlsf = tlsf_create_with_pool(HEAP_EXTERN_BUFFER, HEAP_EXTERN_SIZE, &error_cb);
#endif
}

void* tlsf_get_heap_addr(const tlsf_memory_heap_t *heap)
{
  return heap->tlsf;
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


/* Returns nonzero if any internal consistency check fails. */
int tlsf_check_r (tlsf_memory_heap_t *heap)
{
  int ret=0;
  chMtxLock (heap->mtx);
  ret = tlsf_check(heap->tlsf);
  if (ret == 0) 
    ret = tlsf_check_pool (tlsf_get_pool(heap->tlsf));
  
  chMtxUnlock (heap->mtx);
  return ret;
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
