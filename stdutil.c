#include <ctype.h>  
#include <math.h>   
#include "ch.h"
#include "printf.h"
#include "globalVar.h"
#include "stdutil.h"



static float powi(int x, int y);   
       
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
      v += (*s++ - '0') * scale;   
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
       
static float powi(int x, int y)   
// Determines x-raised-to-the-power-of-y and returns the   
// result as a float   
{   
  int d;   
  float p = 1;   
        
  for (d = 0; d<y; d++)   
    p *= x;   
         
  return p;   
}   


void *malloc_m (size_t size)
{
  return chHeapAlloc (&ccmHeap, size);
}

void free_m(void *p)
{
  chHeapFree (p);
}



void systemReset(void)
{
  *((unsigned long *)0x0E000ED0C) = 0x05FA0004;
  while(1);
}

/* to lower consumption until reset */
void systemDeepSleep (void)
{
  chSysLock();
  SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
  PWR->CR |= (PWR_CR_PDDS | PWR_CR_LPDS | PWR_CR_CSBF | PWR_CR_CWUF);
  __WFE();
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
    return x + (uint32_t) (w * (y-x));
  } else {
    return x - (uint32_t) (w * (x-y));
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
#if defined (DEBUG) && (HAL_USE_SERIAL_USB || defined CONSOLE_DEV_SD)
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
