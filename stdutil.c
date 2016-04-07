#include <ctype.h>  
#include <math.h>   
#include "ch.h"
#include "printf.h"
#include "globalVar.h"
#include "stdutil.h"
#if CH_HEAP_USE_TLSF
#include "tlsf_malloc.h"
#endif


float atof_m(const char *s)   
{   
  float v = 0.0f,   
    scale = 0.1f;   
  char  mneg = ' ',   
    eneg = ' ';   
  int   e = 0;   
              
  while (isspace((int) *s))   
    s++;   
       
  if (*s == '-')   
    mneg = *s++;   
  else   
    if (*s == '+')   
      s++;   
         
  while (isdigit((int) *s))   
    v = 10.0f * v + *s++ - '0';   
         
  if (*s == '.')    
    s++;   
          
  while(isdigit((int) *s))    
    {   
      v += (float)((*s++ - '0')) * scale;   
      scale /= 10.0f;   
    }   
         
  if (toupper((int) *s) == 'E')    
    {   
      s++;   
      if (*s == '-')   
	eneg = *s++;   
      else    
	if (*s == '+')   
	  s++;   
      while (isdigit((int) *s))   
	e = 10 * e + *s++ - '0';   
      if (eneg == '-')   
	v = v / powi(10,e);   
      else   
	v = v * powi(10,e);   
    }   
         
  if (mneg == '-')   
    v = -v;   
          
  return v;   
}   
       
float powi(int x, int y)   
// Determines x-raised-to-the-power-of-y and returns the   
// result as a float   
{   
  int d;   
  float p = 1;   
        
  for (d = 0; d<y; d++)   
    p *= (float)(x);   
         
  return p;   
}   

#if (CH_KERNEL_MAJOR == 2)

#if CH_USE_HEAP || CH_HEAP_USE_TLSF

#ifndef CH_HEAP_SIZE
#error CH_HEAP_SIZE should be defined if  CH_USE_HEAP or CH_HEAP_USE_TLSF are defined
#endif

#if defined STM32F4XX
#define NODMA_SECTION ".ram4"
#define DMA_SECTION ".ram0"
#elif  defined STM32F7XX
#define NODMA_SECTION ".ram0"
#define DMA_SECTION ".ram3"
#else
#error "section defined only for STM32F4 and STM32F7"
#endif


#if (! defined CH_HEAP_USE_TLSF) || (CH_HEAP_USE_TLSF == 0)
static uint8_t ccmHeapBuffer[CH_HEAP_SIZE] __attribute__ ((section(NODMA_SECTION), aligned(8))) ;
MemoryHeap ccmHeap;
#endif

#endif // if CH_USE_HEAP || CH_HEAP_USE_TLSF

#else // (CH_KERNEL_MAJOR > 2)

#if CH_CFG_USE_HEAP || CH_HEAP_USE_TLSF

#ifndef CH_HEAP_SIZE
#error CH_HEAP_SIZE should be defined if  CH_CFG_USE_HEAP or CH_HEAP_USE_TLSF are defined
#endif

#if (! defined CH_HEAP_USE_TLSF) || (CH_HEAP_USE_TLSF == 0)
static uint8_t ccmHeapBuffer[CH_HEAP_SIZE] __attribute__ ((section(NODMA_SECTION), aligned(8))) ;
memory_heap_t ccmHeap;
#endif

#endif // #if CH_CFG_USE_HEAP || CH_HEAP_USE_TLSF
#endif // if (CH_KERNEL_MAJOR == 2)



size_t initHeap (void)
{
#if CH_HEAP_USE_TLSF
  struct  tlsf_stat_t stat;
  tlsf_init_heaps();
  tlsf_stat_r (&HEAP_DEFAULT, &stat);
  return stat.mfree;
#else
  size_t size;
#if (CH_KERNEL_MAJOR == 2)
  chHeapInit(&ccmHeap, (void *) ccmHeapBuffer, sizeof (ccmHeapBuffer));
  chHeapStatus(&ccmHeap, &size);
#else
  chHeapObjectInit(&ccmHeap, (void *) ccmHeapBuffer, sizeof (ccmHeapBuffer));
  chHeapStatus(&ccmHeap, &size);
#endif
  return size;
#endif
}

size_t getHeapFree (void)
{
#if CH_HEAP_USE_TLSF
  struct  tlsf_stat_t stat;
  tlsf_stat_r (&HEAP_DEFAULT, &stat);
  return stat.mfree;
#else
  size_t size;
  chHeapStatus(&ccmHeap, &size);
  return size;
#endif
}


void *malloc_m (size_t size)
{
#if CH_HEAP_USE_TLSF
  //  DebugTrace ("tlsf alloc size=%d", size);
  return tlsf_malloc_r(&HEAP_DEFAULT, size);
#else
  return chHeapAlloc (&ccmHeap, size);
#endif
}

void free_m(void *p)
{
#if CH_HEAP_USE_TLSF
  //  DebugTrace ("tlsf free 0x%x", p);
  tlsf_free_r(&HEAP_DEFAULT, p);
#else
  chHeapFree (p);
#endif
}



void systemReset(void)
{
  *((unsigned long *)0x0E000ED0C) = 0x05FA0004;
  while(1);
}

/* to lower consumption until reset */
void systemDeepSleep (void)
{
#if defined STM32F746xx || defined STM32F756xx
  /* clear PDDS and LPDS bits */
  PWR->CR1 &= ~(PWR_CR1_PDDS | PWR_CR1_LPDS);
  
  /* set LPDS and clear  */
  PWR->CR1 |= (PWR_CR1_LPDS | PWR_CR1_CSBF);
  
  /* Setup the deepsleep mask */
  SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
  
  __disable_irq();
  
  __SEV();
  __WFE();
  __WFE();
  
  __enable_irq();
  
  /* clear the deepsleep mask */
  SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
#else
  /* clear PDDS and LPDS bits */
  PWR->CR &= ~(PWR_CR_PDDS | PWR_CR_LPDS);
  
  /* set LPDS and clear  */
  PWR->CR |= (PWR_CR_LPDS | PWR_CR_CSBF | PWR_CR_CWUF);
  
  /* Setup the deepsleep mask */
  SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
  
  __disable_irq();
  
  __SEV();
  __WFE();
  __WFE();
  
  __enable_irq();
  
  /* clear the deepsleep mask */
  SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
#endif
}


uint32_t revbit (uint32_t value)
{
  uint32_t result=0;
  
  asm volatile ("rbit %0, %1" : "=r" (result) : "r" (value) );
  return(result);
}

void my_assert_func (const char* file, const int line, 
		     const char *cond)
{
#if HAL_USE_SERIAL_USB || defined CONSOLE_DEV_SD
  chprintf (chp, "assert failed : file %s: line %d : %s\r\n",
    file, line, cond);
#else
  (void) file;
  (void) line;
  (void) cond;
#endif
}

float lerpf (const float x, const float y, const float w) 
{
  return x + (w * (y-x));
}

float unlerpf (const float x, const float y, const float x_y) 
{
  return (x_y - x) / (y - x);
}

uint32_t lerpu32 (const uint32_t x, const uint32_t y, const float w)
{
  if (y>x) {
    return x + (uint32_t) (w * (float)(y-x));
  } else {
    return x - (uint32_t) (w * (float)(x-y));
  }
}

uint32_t lerpu32Fraction (const uint32_t x, const uint32_t y, const uint32_t numerator, 
			    const uint32_t denumerator) 
{
  if (y>x) {
    return x + (uint32_t) ((numerator * (y-x))/denumerator);
  } else {
    return x - (uint32_t) ((numerator * (x-y)/denumerator));
  }
}

float clampToVerify (const char* file, const int line, float l, float h, float v)
{
#if ((defined (DEBUG) && DEBUG) && (HAL_USE_SERIAL_USB || defined CONSOLE_DEV_SD))
  if ((v<l) || (v>h)) {
    chprintf (chp, "clampToVerify failed : file %s: line %d : %f not in [%f .. %f]\r\n",
	      file, line, v, l, h);
  }
#else
  (void) file;
  (void) line;
#endif
  return  (MAX(MIN(v,h),l));
}

float clampTo (float l, float h, float v)
{
  return  (MAX(MIN(v,h),l));
}



uint16_t fletcher16 (uint8_t const *data, size_t bytes)
{
  uint16_t sum1 = 0xff, sum2 = 0xff;
  
  while (bytes) {
    size_t tlen = bytes > 20 ? 20 : bytes;
    bytes -= tlen;
    do {
      sum1 = (uint16_t) (sum1 + *data++);
      sum2 =  (uint16_t) (sum2 + sum1);
    } while (--tlen);
    sum1 = (uint16_t) ((sum1 & 0xff) + (sum1 >> 8));
    sum2 = (uint16_t) ((sum2 & 0xff) + (sum2 >> 8));
  }
  /* Second reduction step to reduce sums to 8 bits */
  sum1 = (uint16_t) ((sum1 & 0xff) + (sum1 >> 8));
  sum2 = (uint16_t) ((sum2 & 0xff) + (sum2 >> 8));
  return (uint16_t) ((sum2 % 0xff) << 8) | (sum1 % 0xff);
}




// obviously not reentrant
#define FMT_BUF_SIZE (sizeof(uintmax_t) * 8)
char *binary_fmt(uintmax_t x)
{
  static char buf[FMT_BUF_SIZE];
  char *s = buf + FMT_BUF_SIZE;
  *--s = 0;
  if (!x) *--s = '0';
  for(; x; x/=2) *--s = (char) ('0' + x%2);
  return s;
}
#undef FMT_BUF_SIZE // don't pullute namespace


/* libc stub */
int _getpid(void) {return 1;}
/* libc stub */
void _exit(int i) {(void)i; while(1);}
/* libc stub */
#include <errno.h>
#undef errno
extern int errno;
int _kill(int pid, int sig) {
  (void)pid;
  (void)sig;
  errno = EINVAL;
  return -1;
}
void *__dso_handle = 0;
