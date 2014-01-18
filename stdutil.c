#include <ctype.h>  
#include <math.h>   
#include "ch.h"
#include "printf.h"
#include "globalVar.h"


#define assert(__e) ((__e) ? (void)0 : my_assert_func (__FILE__, __LINE__, \
                                                       __ASSERT_FUNC, #__e))



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

uint32_t revbit (uint32_t value)
{
  uint32_t result=0;
  
  asm volatile ("rbit %0, %1" : "=r" (result) : "r" (value) );
  return(result);
}

void my_assert_func (const char* file, const int line, 
		     const char *cond)
{
  chprintf (chp, "assert failed : file %s: line %d : %s\r\n",
	    file, line, cond);
}

int32_t abs32(int32_t x)
{
  return (x > 0) ? x : -x;
}

float lerpf (const float x, const float y, const float w) 
{
  return x + (w * (y-x));
}

uint32_t lerpu32 (const uint32_t x, const uint32_t y, const float w)
{
  if (y>x) {
    return x + (uint32_t) (w * (y-x));
  } else {
    return x - (uint32_t) (w * (x-y));
  }
}
