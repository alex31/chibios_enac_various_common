#ifndef __STD_UTIL_H__
#define __STD_UTIL_H__

#include "ch.h"
#include "hal.h"

#ifdef WSTRICT_CONVERSION
#pragma GCC diagnostic warning "-Wconversion"
#endif

/*===========================================================================*/
/* Generic MACRO */
/*===========================================================================*/
//#define TRACE_EEPROM 1
//#define TRACE_PWM 1
//#define TRACE_IOMODE 1

#if (CH_KERNEL_MAJOR == 2)
#define WORKING_AREA_ARRAY(s, n, al) stkalign_t s[al] [THD_WA_SIZE(n) \
						       / sizeof(stkalign_t)]
#else
#define THD_WORKING_AREA_ARRAY(s, n, al) stkalign_t s[al] [THD_WORKING_AREA_SIZE(n) \
						       / sizeof(stkalign_t)]
#define SYSTEM_CLOCK_MHZ (STM32_PLLN_VALUE/STM32_PLLP_VALUE)
#endif


#define ARRAY_LEN(a) (sizeof(a)/sizeof(a[0]))
#define MIN(x , y)  (((x) < (y)) ? (x) : (y))
#define MAX(x , y)  (((x) > (y)) ? (x) : (y))
#define ABS(val) ((val) < 0 ? -(val) : (val))
#define INRANGE(min,max,x) MAX(min,MIN(max,x))
#define  CLAMP_TO(l, h, v) (clampToVerify (__FILE__, __LINE__, l, h, v))

//#define DEBUG_MB 1

#if defined TRACE 
#include "printf.h"
#define DebugTrace(fmt, ...) chprintf (chp, fmt "\r\n", ## __VA_ARGS__ )
#else
#define DebugTrace(...) 
#endif // TRACE

// MODE LITTLE ENDIAN; have to be reversed for BIG ENDIAN
#define  POINT_TO_IP4(a,b,c,d)	 \
  ((u32_t)((d) & 0xff) << 24) |  \
  ((u32_t)((c) & 0xff) << 16) |  \
  ((u32_t)((b) & 0xff) << 8)  |  \
   (u32_t)((a) & 0xff)

#define  SWAP_ENDIAN32_BY_8(a,b,c,d)	 \
  ((uint32_t)((d) & 0xff) << 24) |  \
  ((uint32_t)((c) & 0xff) << 16) |  \
  ((uint32_t)((b) & 0xff) << 8)  |  \
   (uint32_t)((a) & 0xff)

#define  SWAP_ENDIAN16_BY_8(a,b)  \
  ((uint32_t)((b) & 0xff) << 8)  |  \
   (uint32_t)((a) & 0xff)
  
#define SWAP_ENDIAN32(x) ( (((x)>>24) & 0x000000FFL) \
			    | (((x)>>8)  & 0x0000FF00L) \
			    | (((x)<<8)  & 0x00FF0000L) \
			    | (((x)<<24) & 0xFF000000L) )

#define SWAP_ENDIAN16(x) ((int16_t) ((((x) & 0xff) << 8) | (((x) & 0xff00) >> 8)))

#define REINTERPRET_CAST(type, val) ({_Static_assert(sizeof(val) <= sizeof(type), \
						    "sizeof (type) is too small");  \
				       *((type *) (&val));}

#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)

  
/* #define RotL(x,shift) ((x << shift) | (x >> (sizeof(x) - shift))) */
/* #define RotR(x,shift) ((x >> shift) | (x << (sizeof(x) - shift))) */

/* #define RotLnb(x,shift,nbits) ((x << shift) | (x >> (nbits - shift))) */
/* #define RotRnb(x,shift,nbits) ((x >> shift) | (x << (nbits - shift))) */


// optimised rotation routine
// http://en.wikipedia.org/wiki/Circular_shift


static inline uint32_t RotL (const uint32_t x, const uint32_t shift) {
  return (x << shift) | (x >> (sizeof(x) - shift));
}
static inline uint32_t RotR(const uint32_t x,const uint32_t shift)  {
  return (x >> shift) | (x << (sizeof(x) - shift));
}
static inline uint32_t RotLnb(const uint32_t x, const uint32_t shift, const uint32_t nbits)  {
  return (x << shift) | (x >> (nbits - shift));
};
static inline uint32_t RotRnb(const uint32_t x, const uint32_t shift, const uint32_t nbits)  {
  return (x >> shift) | (x << (nbits - shift));
}

static inline uint32_t RotLnbOfst(const uint32_t x, const uint32_t shift, const uint32_t nbits,
				  const uint32_t ofst)  {
  const uint32_t ar = RotR(x, ofst);
  const uint32_t ar2 = (ar << shift) | (ar >> (nbits - shift));
  return RotL(ar2, ofst);
};

static inline uint32_t RotRnbOfst(const uint32_t x, const uint32_t shift, const uint32_t nbits,
				  const uint32_t ofst)  {
  const uint32_t ar = RotR(x, ofst);
  const uint32_t ar2 =  (ar >> shift) | (ar << (nbits - shift));
  return RotL(ar2, ofst);
}

#if (CH_KERNEL_MAJOR == 2)
static inline halrtcnt_t rtcntDiff (const halrtcnt_t start, const  halrtcnt_t stop) 
{
  if (stop > start) 
    return stop - start;
  else
    return start - stop; 
}


static inline bool_t isTimerExpiredAndRearm (halrtcnt_t *timer, const halrtcnt_t interval)
{
  if (halIsCounterWithin (*timer, *timer+interval)) {
    return false;
  } else {
    *timer =  halGetCounterValue();
    return true;
  }
}


static inline halrtcnt_t halCounterDiff (const halrtcnt_t begin, const halrtcnt_t end)
{
  return   (end > begin) ? end-begin : (UINT32_MAX-begin) + end;
}

static inline halrtcnt_t halCounterDiffNow (const halrtcnt_t begin)
{
  return halCounterDiff (begin, halGetCounterValue());
}
#else // (CH_KERNEL_MAJOR > 2)
static inline rtcnt_t rtcntDiff (const rtcnt_t start, const  rtcnt_t stop) 
{
  if (stop > start) 
    return stop - start;
  else
    return start - stop; 
}


static inline bool isTimerExpiredAndRearm (rtcnt_t *timer, const rtcnt_t interval)
{
  if (chSysIsCounterWithinX (chSysGetRealtimeCounterX(), *timer, *timer+interval)) {
    return false;
  } else {
    *timer =  chSysGetRealtimeCounterX();
    return true;
  }
}


static inline rtcnt_t halCounterDiff (const rtcnt_t begin, const rtcnt_t end)
{
  return   (end > begin) ? end-begin : (UINT32_MAX-begin) + end;
}

static inline rtcnt_t halCounterDiffNow (const rtcnt_t begin)
{
  return halCounterDiff (begin, chSysGetRealtimeCounterX());
}

#endif // (CH_KERNEL_MAJOR == 2)

// optimised counting bits routine from https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html#Other-Builtins
/*
 — Built-in Function: int __builtin_ffs (int x)

    Returns one plus the index of the least significant 1-bit of x, or if x is zero, returns zero. 

— Built-in Function: int __builtin_clz (unsigned int x)

    Returns the number of leading 0-bits in x, starting at the most significant bit position. If x is 0, the result is undefined. 

— Built-in Function: int __builtin_ctz (unsigned int x)

    Returns the number of trailing 0-bits in x, starting at the least significant bit position. If x is 0, the result is undefined. 

— Built-in Function: int __builtin_clrsb (int x)

    Returns the number of leading redundant sign bits in x, i.e. the number of bits following the most significant bit that are identical to it. There are no special cases for 0 or other values. 

— Built-in Function: int __builtin_popcount (unsigned int x)

    Returns the number of 1-bits in x. 

— Built-in Function: int __builtin_parity (unsigned int x)

    Returns the parity of x, i.e. the number of 1-bits in x modulo 2. 


 */


#undef assert
#define assert(__e) ((__e) ? (void)0 : my_assert_func (__FILE__, __LINE__, #__e))


/*===========================================================================*/
/* Generic FUNCTIONS */
/*===========================================================================*/





#ifdef __cplusplus
extern "C" {
#endif

 typedef struct  {
   ioportid_t  gpio;
   uint32_t	pin;
} GpioPin;


/*
 F4
 ram4: 64ko ccm, fast, no dma	 
 ram0: 128Ko     std               


F7
 ram0: std, fast, nodma
 ram3: dma

 256 ko 



 */


#if defined STM32F4XX
#define STD_SECTION ".ram0" 
#define FAST_SECTION ".ram4" 
#define DMA_SECTION ".ram0"    
#define BCKP_SECTION ".ram5"    
#elif  defined STM32F7XX
#define STD_SECTION ".ram0" 
#define FAST_SECTION ".ram0" 
#define DMA_SECTION ".ram3"    
#define BCKP_SECTION ".ram5"
#elif (defined (STM32L431xx) || defined (STM32L432xx) || defined (STM32L433xx) || defined (STM32L442xx) || defined (STM32L443xx))
#define STD_SECTION ".ram0" 
#define FAST_SECTION ".ram0" 
#define DMA_SECTION ".ram0"    
#elif  (defined (STM32L471xx) || defined (STM32L475xx) || defined (STM32L476xx) || defined (STM32L485xx) || defined (STM32L486xx))
#define STD_SECTION ".ram0" 
#define FAST_SECTION ".ram4" 
#define DMA_SECTION ".ram0"    
#elif  defined STM32F3XX
#define STD_SECTION ".ram0" 
#define FAST_SECTION ".ram4" 
#define DMA_SECTION ".ram0"    
#else
#error "section defined only for STM32F3, STM32F4, STM32L4 and STM32F7"
#endif


#define IN_STD_SECTION_NOINIT(var) var __attribute__ ((section(STD_SECTION), aligned(8)))
#define IN_STD_SECTION_CLEAR(var) var __attribute__ ((section(STD_SECTION "_clear"), aligned(8)))
#define IN_STD_SECTION(var) var __attribute__ ((section(STD_SECTION "_init"), aligned(8)))
  
#define IN_FAST_SECTION_NOINIT(var) var __attribute__ ((section(FAST_SECTION), aligned(8)))
#define IN_FAST_SECTION_CLEAR(var) var __attribute__ ((section(FAST_SECTION "_clear"), aligned(8)))
#define IN_FAST_SECTION(var) var __attribute__ ((section(FAST_SECTION "_init"), aligned(8)))

#define IN_DMA_SECTION_NOINIT(var) var __attribute__ ((section(DMA_SECTION), aligned(8)))
#define IN_DMA_SECTION_CLEAR(var) var __attribute__ ((section(DMA_SECTION "_clear"), aligned(8)))
#define IN_DMA_SECTION(var) var __attribute__ ((section(DMA_SECTION "_init"), aligned(8)))

#define IN_BCKP_SECTION_NOINIT(var) var __attribute__ ((section(BCKP_SECTION), aligned(8)))
#define IN_BCKP_SECTION_CLEAR(var) var __attribute__ ((section(BCKP_SECTION "_clear"), aligned(8)))
#define IN_BCKP_SECTION(var) var __attribute__ ((section(BCKP_SECTION "_init"), aligned(8)))
 
  
#if (CH_KERNEL_MAJOR == 2)
#if CH_USE_HEAP || CH_HEAP_USE_TLSF
  size_t initHeap (void);
  size_t getHeapFree (void);
  void *malloc_m (size_t size);
  void free_m(void *p);
#endif
#else
#if CH_CFG_USE_HEAP || CH_HEAP_USE_TLSF
  size_t initHeap (void);
  size_t getHeapFree (void);
  void *malloc_m (size_t size);
  void free_m(void *p);
#endif
#endif

#define lerp(x,y,w)				\
  ((y>x) ?					\
   (x + (typeof(x)) (w * (typeof(w))(y-x))) :	\
   (x - (typeof(x)) (w * (typeof(w))(x-y))))

#define isInRange(x, min, max) \
  ((x >= min) && (x <= max))

  float atof_m(const char *s);
  void systemReset (void);
  void systemDeepSleep (void);
  void systemDeepSleepFromISR (void);
  uint32_t revbit(uint32_t data);
  void my_assert_func (const char* file, const int line, 
		       const char *cond);
  float lerpf (const float x, const float y, const float w) ;
  float unlerpf (const float x, const float y, const float x_y) ;
  uint32_t lerpu32 (const uint32_t x, const uint32_t y, const float w) ;
  uint32_t lerpu32Fraction (const uint32_t x, const uint32_t y, const uint32_t numerator, 
			    const uint32_t denumerator) ;
  float clampToVerify (const char* file, const int line, float l, float h, float v);
  float clampTo (float l, float h, float v);
  static inline bool isInRangef (float x, float min, float max) {
    return (x >= min) && (x <= max);
  }

char *binary_fmt(uintmax_t x, const int fill);
  uint16_t fletcher16 (uint8_t const *data, size_t bytes);
  float powi(int x, int y) ;
  const char* getGpioName (const ioportid_t p);
#if (CH_KERNEL_MAJOR > 2)
  int32_t get_stack_free (const thread_t *tp);
#endif

#if HAL_USE_PWM 
 pwmcnt_t  pwmChangeFrequency (PWMDriver *pwmd, const uint32_t freq);
 void	   pwmMaskChannelOutput(PWMDriver *pwmd, const  pwmchannel_t channel,
				const bool masked);
#endif
#ifdef __cplusplus
}
#endif



#endif // __STD_UTIL_H__
