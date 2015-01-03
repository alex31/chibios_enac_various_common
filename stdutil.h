#ifndef __STD_UTIL_H__
#define __STD_UTIL_H__

#include "ch.h"
#include "hal.h"

/*===========================================================================*/
/* Generic MACRO */
/*===========================================================================*/
//#define TRACE_EEPROM 1
//#define TRACE_PWM 1
//#define TRACE_IOMODE 1
#define ARRAY_LEN(a) (sizeof(a)/sizeof(a[0]))
#define WORKING_AREA_ARRAY(s, n, al) stkalign_t s[al] [THD_WA_SIZE(n) \
						       / sizeof(stkalign_t)]
#define MIN(x , y)  (((x) < (y)) ? (x) : (y))
#define MAX(x , y)  (((x) > (y)) ? (x) : (y))
#define INRANGE(min,max,x) MAX(min,MIN(max,x))
#define  CLAMP_TO(l, h, v) (clampToVerify (__FILE__, __LINE__, l, h, v))

//#define DEBUG_MB 1

#if defined TRACE 
#include "printf.h"
#define DebugTrace(...) {{chprintf (chp, __VA_ARGS__); chprintf (chp, "\r\n");}}
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

#define RotL(x,shift) ((x << shift) | (x >> (sizeof(x) - shift)))
#define RotR(x,shift) ((x >> shift) | (x << (sizeof(x) - shift)))

#define RotLnb(x,shift,nbits) ((x << shift) | (x >> (nbits - shift)))
#define RotRnb(x,shift,nbits) ((x >> shift) | (x << (nbits - shift)))

#undef assert
#define assert(__e) ((__e) ? (void)0 : my_assert_func (__FILE__, __LINE__, #__e))


/*===========================================================================*/
/* Generic FUNCTIONS */
/*===========================================================================*/





#ifdef __cplusplus
extern "C" {
#endif

 typedef struct  {
   GPIO_TypeDef	*gpio;
   uint32_t	pin;
} GpioPin;
 

#if CH_USE_HEAP || CH_HEAP_USE_TLSF
  size_t initHeap (void);
  size_t getHeapFree (void);
  void *malloc_m (size_t size);
  void free_m(void *p);
#endif


  float atof_m(const char *s);
  void systemReset (void);
  void systemDeepSleep (void);
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
  static inline bool_t isInRangef (float x, float min, float max) {
    return (x >= min) && (x <= max);
  }

#ifdef __cplusplus
}
#endif



#endif // __STD_UTIL_H__
