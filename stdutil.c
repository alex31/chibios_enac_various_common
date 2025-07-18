#include <ctype.h>  
#include <math.h>   
#include <string.h>
#include "ch.h"
#include "hal.h"
#include "printf.h"
#include "stdutil.h"

#if CH_HEAP_USE_TLSF
#include "tlsf_malloc.h"
#endif

#if HAL_USE_SERIAL_USB == TRUE
SerialUSBDriver SDU1;
#endif
#if defined CONSOLE_DEV_SD
BaseSequentialStream *chp = (BaseSequentialStream *) &CONSOLE_DEV_SD;
#elif (CONSOLE_DEV_USB == TRUE)
BaseSequentialStream *chp = (BaseSequentialStream *) &SDU1;
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



#if CH_CFG_USE_HEAP || CH_HEAP_USE_TLSF


#if (! defined CH_HEAP_USE_TLSF) || (CH_HEAP_USE_TLSF == 0)
#ifndef CH_HEAP_SIZE
#error CH_HEAP_SIZE should be defined if  CH_CFG_USE_HEAP or CH_HEAP_USE_TLSF are defined
#endif

static uint8_t ccmHeapBuffer[CH_HEAP_SIZE]  __attribute__ ((section(STD_SECTION), aligned(8))) ;
memory_heap_t ccmHeap;
#endif

#endif // #if CH_CFG_USE_HEAP || CH_HEAP_USE_TLSF


#if CH_CFG_USE_HEAP
size_t initHeap (void)
{
#if CH_HEAP_USE_TLSF
  struct  tlsf_stat_t stat;
  tlsf_init_heaps();
  tlsf_stat_r (&HEAP_DEFAULT, &stat);
  return stat.mfree;
#else
  size_t size;
  chHeapObjectInit(&ccmHeap, (void *) ccmHeapBuffer, sizeof (ccmHeapBuffer));
  chHeapStatus(&ccmHeap, &size, NULL);
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
  chHeapStatus(&ccmHeap, &size, NULL);
  return size;
#endif
}


void *malloc_m (size_t size)
{
#if CH_HEAP_USE_TLSF
  void * const ret = tlsf_malloc_r(&HEAP_DEFAULT, size);
#else
  void * const ret = chHeapAlloc (&ccmHeap, size);
#endif
  /* DebugTrace("alloc size=%d => %p from %p", size, ret, chThdGetSelfX()); */
  /* chThdSleepMilliseconds(10); */
  return ret;
}

void free_m(void *p)
{
  /* DebugTrace("free %p from %p", p, chThdGetSelfX()); */
  /* chThdSleepMilliseconds(10); */
#if CH_HEAP_USE_TLSF
  tlsf_free_r(&HEAP_DEFAULT, p);
#else
  chHeapFree (p);
#endif
}

#endif // CH_CFG_USE_HEAP

void systemReset(void)
{
  chSysLock();
  __disable_irq();
  
  NVIC_SystemReset();
}

/* to lower consumption until reset */
void systemDeepSleep (void)
#if defined(STM32F4XX) || defined(STM32F3XX)
#define __CR CR
#define __PWR_CR_PDDS PWR_CR_PDDS
#define __PWR_CR_LPDS PWR_CR_LPDS
#define __PWR_CR_CSBF PWR_CR_CSBF
#elif defined(STM32F7XX)
#define __CR CR1
#define __PWR_CR_PDDS PWR_CR1_PDDS
#define __PWR_CR_LPDS PWR_CR1_LPDS
#define __PWR_CR_CSBF PWR_CR1_CSBF
#endif
{
  chSysLock();

#if defined(STM32F4XX) || defined(STM32F7XX) || defined(STM32F3XX)
  /* clear PDDS and LPDS bits */
  PWR->__CR &= ~(__PWR_CR_PDDS | __PWR_CR_LPDS);
  /* set LPDS and clear  */
  PWR->__CR |= (__PWR_CR_LPDS | __PWR_CR_CSBF);
#elif defined(STM32L4XX) 
  PWR->CR1 =  (PWR->CR1 & (~PWR_CR1_LPMS)) | PWR_CR1_LPMS_SHUTDOWN;
#elif defined(STM32H7XX)
  PWR->CPUCR |= PWR_CPUCR_PDDS_D1 | PWR_CPUCR_PDDS_D2 | PWR_CPUCR_PDDS_D3;
#elif defined(STM32G4XX)
  PWR->CR1 |= PWR_CR1_LPMS_SHUTDOWN;
  PWR->SR1 &= PWR_SR1_WUF;
#else
#error neither F3XX, F4XX, F7XX, H7XX, L4XX : should be implemented
#endif

  /* Setup the deepsleep mask */
  SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
  
  __disable_irq();
  
  __SEV();
  __WFE();
  __WFE();
  
  __enable_irq();
  
  /* clear the deepsleep mask */
  SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
  chSysUnlock();
}


/* to lower consumption until reset */
void systemDeepSleepFromISR (void)
#if defined(STM32F4XX) || defined(STM32F3XX)
#define __CR CR
#define __PWR_CR_PDDS PWR_CR_PDDS
#define __PWR_CR_LPDS PWR_CR_LPDS
#define __PWR_CR_CSBF PWR_CR_CSBF
#elif defined(STM32F7XX)
#define __CR CR1
#define __PWR_CR_PDDS PWR_CR1_PDDS
#define __PWR_CR_LPDS PWR_CR1_LPDS
#define __PWR_CR_CSBF PWR_CR1_CSBF
#endif
{
  chSysLockFromISR();

#if defined(STM32F4XX) || defined(STM32F7XX) || defined(STM32F3XX)
  /* clear PDDS and LPDS bits */
  PWR->__CR &= ~(__PWR_CR_PDDS | __PWR_CR_LPDS);
  /* set LPDS and clear  */
  PWR->__CR |= (__PWR_CR_LPDS | __PWR_CR_CSBF);
#elif defined(STM32L4XX)
  PWR->CR1 =  (PWR->CR1 & (~PWR_CR1_LPMS)) | PWR_CR1_LPMS_SHUTDOWN;
#elif defined(STM32H7XX)
  PWR->CPUCR |= PWR_CPUCR_PDDS_D1 | PWR_CPUCR_PDDS_D2 | PWR_CPUCR_PDDS_D3;
#elif defined(STM32G4XX)
  PWR->CR1 |= PWR_CR1_LPMS_SHUTDOWN;
  PWR->SR1 &= PWR_SR1_WUF;
#else
#error neither F3XX, F4XX, F7XX, H7XX, L4XX : should be implemented
#endif

  /* Setup the deepsleep mask */
  SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
  
  __disable_irq();
  
  __SEV();
  __WFE();
  __WFE();
  
  __enable_irq();
  
  /* clear the deepsleep mask */
  SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
  chSysUnlockFromISR();
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

#if HAL_USE_PWM
__attribute__((const))
size_t   getTimerWidth(const PWMDriver *pwmp)
{
  (void) pwmp;

  return(0
#if STM32_PWM_USE_TIM2
          || (pwmp == &PWMD2)
#endif
#if STM32_PWM_USE_TIM5
          || (pwmp == &PWMD5)
#endif
         ) ? 4 : 2;
}


pwmcnt_t pwmChangeFrequencyI (PWMDriver *pwmd, const uint32_t freq)
{
  const pwmcnt_t newPeriod = pwmd->config->frequency / freq;
  if (newPeriod &&
      ((getTimerWidth(pwmd) == 4) ||  newPeriod <= UINT16_MAX)) {
    pwmChangePeriodI(pwmd, newPeriod);
    return newPeriod;
  } else {
    return -1;
  }
}


pwmcnt_t pwmChangeFrequency (PWMDriver *pwmd, const uint32_t freq)
{
  chSysLock();
  const pwmcnt_t ret = pwmChangeFrequencyI(pwmd, freq);
  chSysUnlock();
  return ret;
}

#if defined (STM32F7XX) || defined (STM32H7XX)
static  inline void  peri_set_bit (volatile uint32_t *addr, const uint32_t pos,
				   const bool level)
{
  if (level)
    *addr |= (1U<<pos);
  else
    *addr &= ~(1U<<pos);
};

void	pwmEnableChannelOutput(PWMDriver *pwmd, const  pwmchannel_t channel,
			       const bool enabled)
{
  
  switch (channel) {
  case 0 : peri_set_bit (&(pwmd->tim->CCMR1), 5, enabled); break;
  case 1 : peri_set_bit (&(pwmd->tim->CCMR1), 13, enabled); break;
  case 2 : peri_set_bit (&(pwmd->tim->CCMR2), 5, enabled); break;
  case 3 : peri_set_bit (&(pwmd->tim->CCMR2), 13, enabled); break;
  }
  
}

void	pwmMaskChannelSide(PWMDriver *pwmd, const  pwmchannel_t channel,
			   const PwmOutputSide side, const bool masked)
{
  // CCxE and CCxNE
  if (channel < 4) {
    const uint32_t bitShift = (side == PWM_NORMAL) ? channel * 4 : (channel * 4) + 2;
    peri_set_bit (&(pwmd->tim->CCER), bitShift, !masked);
  }
}
#else // not F7
#include "bitband.h"
void	pwmEnableChannelOutput(PWMDriver *pwmd, const  pwmchannel_t channel,
			     const bool enabled)
{
  switch (channel) {
  case 0 : bb_peri_set_bit (&(pwmd->tim->CCMR1), 5, enabled); break;
  case 1 : bb_peri_set_bit (&(pwmd->tim->CCMR1), 13, enabled); break;
  case 2 : bb_peri_set_bit (&(pwmd->tim->CCMR2), 5, enabled); break;
  case 3 : bb_peri_set_bit (&(pwmd->tim->CCMR2), 13, enabled); break;
  }
}

void	pwmMaskChannelSide(PWMDriver *pwmd, const  pwmchannel_t channel,
			   const PwmOutputSide side, const bool masked)
{
  // CCxE and CCxNE
  if (channel < 4) {
    const uint32_t bitShift = (side == PWM_NORMAL) ? channel * 4 : (channel * 4) + 2;
    bb_peri_set_bit (&(pwmd->tim->CCER), bitShift, !masked);
  }
}


#endif // ifndef STM32F7XX
#endif // HAL_USE_PWM


#define _GPIOTEST(P) if (P == p) return #P
const char* getGpioName (const ioportid_t p)
{
  _GPIOTEST(GPIOA);
  _GPIOTEST(GPIOB);
  _GPIOTEST(GPIOC);
#if (!defined STM32L422xx) && (!defined STM32L432xx)
  _GPIOTEST(GPIOD);
  _GPIOTEST(GPIOE);
#if (!defined STM32F411xE)
  _GPIOTEST(GPIOF);
  _GPIOTEST(GPIOG);
#endif
#endif
#if (!defined STM32G4XX)
  _GPIOTEST(GPIOH);
#endif

  return "Unknown GPIO";
}
#undef _GPIOTEST


// obviously not reentrant
#define FMT_BUF_SIZE (sizeof(uintmax_t) * 16)
char *binary_fmt(uintmax_t x, int fill)
{
  static char buf[FMT_BUF_SIZE];
  char *s = buf + FMT_BUF_SIZE;
  *--s = 0;
  if (!x) *--s = '0';
  for(; x; x/=2) *--s = (char) ('0' + x%2);
  if (fill == 0) {
    return s;
  } else {
    char * const bg = buf + FMT_BUF_SIZE - fill -1;
    if (s != bg)
      memset (bg, '0', s -bg);
    return bg;
  }
}
#undef FMT_BUF_SIZE // don't pollute namespace

#if defined(CH_DBG_SYSTEM_STATE_CHECK) &&  CH_DBG_SYSTEM_STATE_CHECK
int32_t get_stack_free (const thread_t *tp)
{
  int32_t index = 0;
  extern const uint8_t __ram0_end__;
  
#if (CH_KERNEL_MAJOR == 3)
  unsigned long long *stkAdr =  (unsigned long long *) ((uint8_t *) tp->p_stklimit);
#elif (CH_KERNEL_MAJOR >= 4)
  unsigned long long *stkAdr =  (unsigned long long *) tp->wabase;
#endif
  
  while ((stkAdr[index] == 0x5555555555555555) && ( ((uint8_t *) &(stkAdr[index])) < &__ram0_end__))
    index++;
  
  const int32_t freeBytes =  index * (int32_t) sizeof(long long);
  return freeBytes;
}
#endif

__attribute__((used))
void _fini(void) {chSysHalt("_fini"); while(true) { asm volatile ("nop");};}

#if ((CH_KERNEL_MAJOR * 10 + CH_KERNEL_MINOR) < 61)

/* libc stub */
int _getpid(void) {return 1;}
/* libc stub */
void _exit(int i) {(void) i; chSysHalt("_exit"); while(true);}
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
#endif
void *__dso_handle = 0;




// legacy
const uint8_t *UniqProcessorId = (uint8_t *) UID_BASE;
const StmUuid *stmUuid =  (StmUuid *) UID_BASE;
