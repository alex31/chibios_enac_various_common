#pragma once

#include "ch.h"
#include "hal.h"
#include "math.h"

#ifdef WSTRICT_CONVERSION
#pragma GCC diagnostic warning "-Wconversion"
#endif

/*===========================================================================*/
/* Generic MACRO */
/*===========================================================================*/
//#define TRACE_EEPROM 1
//#define TRACE_PWM 1
//#define TRACE_IOMODE 1

#define THD_WORKING_AREA_ARRAY(s, n, al) stkalign_t s[al] [THD_WORKING_AREA_SIZE(n) \
						       / sizeof(stkalign_t)]
#define SYSTEM_CLOCK_MHZ (STM32_PLLN_VALUE/STM32_PLLP_VALUE)


#define ARRAY_LEN(a) (sizeof(a)/sizeof(a[0]))

// use gcc block evaluation to avoid multiple evaluation of a and b
#define MAX(a,b)		\
  ({ const typeof (a) _aa = (a);	\
    const typeof (b) _bb = (b);	\
    _aa > _bb ? _aa : _bb; })

#define MIN(a,b) \
  ({ const typeof (a) _a = (a);	\
    const typeof (b) _b = (b);	\
    _a < _b ? _a : _b; })



#define ABS(val) ((val) < 0 ? -(val) : (val))
#define INRANGE(min,max,x) (MAX(min,MIN(max,x)))
#define  CLAMP_TO(l, h, v) (clampToVerify (__FILE__, __LINE__, l, h, v))

//#define DEBUG_MB 1

#ifndef CONSOLE_DEV_USB
#define CONSOLE_DEV_USB FALSE
#endif

#if defined CONSOLE_DEV_SD || (CONSOLE_DEV_USB == TRUE)
extern BaseSequentialStream *chp;
#endif

#if (CONSOLE_DEV_USB == TRUE)
extern SerialUSBDriver SDU1;
#endif


#if defined TRACE && (defined CONSOLE_DEV_SD || (CONSOLE_DEV_USB == TRUE))
#include "printf.h"
#define DebugTrace(fmt, ...) if (chp) {chprintf (chp, fmt "\r\n", ## __VA_ARGS__ );}
#else
#define DebugTrace(...) 
#endif // TRACE


typedef enum {PWM_NORMAL=0, PWM_COMPLEMENTARY} PwmOutputSide;


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
  
#define SWAP_ENDIAN16(x) __builtin_bswap16(x)

#define SWAP_ENDIAN32(x) __builtin_bswap32(x)

#define SWAP_ENDIAN64(x) __builtin_bswap64(x)

#define REINTERPRET_CAST(type, val) ({_Static_assert(sizeof(val) <= sizeof(type), \
						    "sizeof (type) is too small");  \
				       *((type *) (&val));}


#if (CH_KERNEL_MAJOR < 7)
#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)
#endif
  

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

static inline __attribute__((always_inline))
uint32_t arm_ror_imm(const uint32_t v, const uint32_t sh) {
  uint32_t d;
  asm ("ROR %[Rd], %[Rm], %[Is]" : [Rd] "=r" (d) : [Rm] "r" (v), [Is] "i" (sh));
  return d;
}

static inline __attribute__((always_inline))
uint32_t arm_rol_imm(const uint32_t v, const uint32_t sh) {
  return arm_ror_imm(v, 32U-sh);
}
 
static inline __attribute__((always_inline))
uint32_t arm_ror(uint32_t v, uint32_t sh) {
  uint32_t d;
  asm ("ROR %[Rd], %[Rm], %[Rs]" : [Rd] "=r" (d) : [Rm] "r" (v), [Rs] "r" (sh));
  return d;
}

static inline __attribute__((always_inline))
uint32_t arm_rol(uint32_t v, const uint32_t sh) {
  return arm_ror(v, 32U-sh);
}


static inline rtcnt_t rtcntDiff (const rtcnt_t start, const  rtcnt_t stop) 
{
  return stop - start;
}

static inline rtcnt_t rtcntDiffNow (const rtcnt_t begin)
{
  return rtcntDiff(begin, chSysGetRealtimeCounterX());
}

// old api name, for compatibility only
__attribute__ ((deprecated))
static inline rtcnt_t halCounterDiffNow (const rtcnt_t begin) 
{
  return rtcntDiffNow(begin);
}

__attribute__ ((deprecated))
static inline rtcnt_t halCounterDiff (const rtcnt_t start, const  rtcnt_t stop) 
{
  return rtcntDiff(start, stop);
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

typedef struct {
  float v[3];
} Vec3f;

typedef struct {
  float v[4];
} Vec4f;
static inline bool vec3fIsEqual(const Vec3f *v1, const Vec3f *v2) {
  return  v1->v[0] == v2->v[0] &&  v1->v[1] == v2->v[1] &&  v1->v[2] == v2->v[2];
}

static inline Vec3f vec3fAdd(const Vec3f *v1, const Vec3f *v2) {
    Vec3f v;
    v.v[0] = v1->v[0] + v2->v[0];
    v.v[1] = v1->v[1] + v2->v[1];
    v.v[2] = v1->v[2] + v2->v[2];
    return v;
}

  static inline Vec3f vec3fSub(const Vec3f *v1, const Vec3f *v2) {
    Vec3f v;
    v.v[0] = v1->v[0] - v2->v[0];
    v.v[1] = v1->v[1] - v2->v[1];
    v.v[2] = v1->v[2] - v2->v[2];
    return v;
}

  static inline Vec3f vec3fMult(const Vec3f *v1, const float f) {
    Vec3f v;
    v.v[0] = v1->v[0] * f;
    v.v[1] = v1->v[1] * f;
    v.v[2] = v1->v[2] * f;
    return v;
}

  static inline Vec3f vec3fDiv(const Vec3f *v1, const float f) {
    Vec3f v;
    v.v[0] = v1->v[0] / f;
    v.v[1] = v1->v[1] / f;
    v.v[2] = v1->v[2] / f;
    return v;
  }
  
  static inline Vec3f vec3fAbs(const Vec3f *v1) {
    Vec3f v;
    v.v[0] = fabsf(v1->v[0]);
    v.v[1] = fabsf(v1->v[1]);
    v.v[2] = fabsf(v1->v[2]);
    return v;
  }


  
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
#define SDMMC_SECTION DMA_SECTION
#elif  defined STM32F7XX
#define STD_SECTION ".ram0" 
#define FAST_SECTION ".ram0" 
#define DMA_SECTION ".ram3"    
#define BCKP_SECTION ".ram5"
#define SDMMC_SECTION DMA_SECTION
#elif (defined (STM32L422xx) || defined (STM32L431xx) || defined (STM32L432xx) || \
       defined (STM32L433xx) || defined (STM32L442xx) || defined (STM32L443xx))
#define STD_SECTION ".ram0" 
#define FAST_SECTION ".ram0" 
#define DMA_SECTION ".ram0"    
#elif  (defined (STM32L471xx) || defined (STM32L475xx) || defined (STM32L476xx) || \
	defined (STM32L485xx) || defined (STM32L486xx))
#define STD_SECTION ".ram0" 
#define FAST_SECTION ".ram4" 
#define DMA_SECTION ".ram0"    
#elif  defined STM32F3XX
#define STD_SECTION ".ram0" 
#define FAST_SECTION ".ram4" 
#define DMA_SECTION ".ram0"    
#elif  defined STM32H7XX
#define STD_SECTION ".ram0" 
#define FAST_SECTION ".ram5" 
#define DMA_SECTION ".ram3"    
#define BDMA_SECTION ".ram4"    
#define AXIDMA_SECTION ".ram0nc"
#define SDMMC_SECTION AXIDMA_SECTION
#else
#error "section defined only for STM32F3, STM32F4, STM32F7, STM32L4 and STM32H7"
#endif

#if (defined __DCACHE_PRESENT) && (__DCACHE_PRESENT != 0)
#define CACHE_ALIGNED(var) var __attribute__ ((aligned(32)))
#else
#define CACHE_ALIGNED(var) var
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

#ifdef BDMA_SECTION
#define		IN_BDMA_SECTION_NOINIT(var) var __attribute__ ((section(BDMA_SECTION), aligned(8)))
#define 	IN_BDMA_SECTION_CLEAR(var) var __attribute__ ((section(BDMA_SECTION "_clear"), aligned(8)))
#define 	IN_BDMA_SECTION(var) var __attribute__ ((section(BDMA_SECTION "_init"), aligned(8)))
#endif

#ifdef SDMMC_SECTION
#define		IN_SDMMC_SECTION_NOINIT(var) var __attribute__ ((section(SDMMC_SECTION), aligned(8)))
#define 	IN_SDMMC_SECTION_CLEAR(var) var __attribute__ ((section(SDMMC_SECTION "_clear"), \
								 aligned(8)))
#define 	IN_SDMMC_SECTION(var) var __attribute__ ((section(BDMA_SECTION "_init"), aligned(8)))
#endif
  
#if CH_CFG_USE_HEAP || CH_HEAP_USE_TLSF
  size_t initHeap (void);
  size_t getHeapFree (void);
  void *malloc_m (size_t size);
  void free_m(void *p);
#endif


// c++20 define std::lerp
#if (! defined(__cplusplus)) || __cplusplus <= 201703L
#define lerp(x,y,w)				\
  (((y)>(x)) ?					\
   ((x) + (typeof((x))) ((w) * (typeof((w)))((y)-(x)))) :	\
   ((x) - (typeof((x))) ((w) * (typeof((w)))((x)-(y)))))
#endif

#define unlerp(x,y,x_y)				\
  ((x_y - (x)) / ((y) - (x)))


#define homothetie map(x, in_min, in_max, out_min, out_max) \
((x - (typeof(x)) in_min) * ((typeof(x)) out_max - (typeof(x)) out_min) / \
 ((typeof(x)) in_max - (typeof(x)) in_min) + (typeof(x)) out_min)


#define isInRange(x, min, max) \
  ((x >= min) && (x <= max))

  float atof_m(const char *s);
  void systemReset (void);
  void systemDeepSleep (void);
  void systemDeepSleepFromISR (void);

#define revbit(X) _Generic((X),			\
			   uint32_t:  revbit32, \
			   uint16_t:  revbit16, \
			   uint8_t:   revbit8,	\
			   int32_t:  revbit32, \
			   int16_t:  revbit16, \
			   int8_t:   revbit8,	\
			   default:   revbit32	\
			   )(X)
  
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
int32_t get_stack_free (const thread_t *tp);


#if HAL_USE_PWM 
__attribute__((const))
size_t   getTimerWidth(const PWMDriver *pwmp);
pwmcnt_t  pwmChangeFrequency (PWMDriver *pwmd, const uint32_t freq);
pwmcnt_t  pwmChangeFrequencyI (PWMDriver *pwmd, const uint32_t freq);
void	  pwmEnableChannelOutput(PWMDriver *pwmd, const  pwmchannel_t channel,
				 const bool enabled);
void	   pwmMaskChannelSide(PWMDriver *pwmd, const  pwmchannel_t channel,
			      const PwmOutputSide side, const bool masked);
#endif

static inline  uint32_t revbit32 (uint32_t value)
{
  uint32_t result=0;
  
  asm volatile ("rbit %0, %1" : "=r" (result) : "r" (value) );
  return(result);
}

static inline  uint16_t revbit16 (uint16_t value)
{
  uint32_t result=0;
  
  asm volatile ("rbit %0, %1" : "=r" (result) : "r" (value) );
  return((uint16_t) (result>>16U));
}

static inline  uint8_t revbit8 (uint8_t value)
{
  uint32_t result=0;
  
  asm volatile ("rbit %0, %1" : "=r" (result) : "r" (value) );
  return((uint8_t) (result>>24U));
}

// STM UUID


// Legacy
#define UniqProcessorIdLen 12
extern const uint8_t *UniqProcessorId ;

// new api
typedef union {
  uint8_t  bytes[UniqProcessorIdLen];
  uint32_t words[UniqProcessorIdLen/sizeof(uint32_t)];
} StmUuid;

extern const StmUuid *stmUuid;



#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

extern "C++" {

#if CH_CFG_USE_HEAP
#include "cpp_heap_alloc.hpp"
#endif
  
static inline bool operator!=(const Vec3f& a, const Vec3f& b)
{
  return !vec3fIsEqual(&a, &b);
};

static inline Vec3f operator+(const Vec3f& a, const Vec3f& b)
{
  return vec3fAdd(&a, &b);
}
  
static inline Vec3f operator+=(Vec3f& a, const Vec3f& b)
{
  return (a = vec3fAdd(&a, &b));
}
  
static inline Vec3f operator-(const Vec3f& a, const Vec3f& b)
{
  return vec3fSub(&a, &b);
}

static inline Vec3f operator*(const Vec3f& a, const float f)
{
  return vec3fMult(&a, f);
}

static inline Vec3f operator/(const Vec3f& a, const float f)
{
  return vec3fDiv(&a, f);
}

}
  
#endif
