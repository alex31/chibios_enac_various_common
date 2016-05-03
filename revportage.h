#pragma once
#include "hal.h"

#if (CH_KERNEL_MAJOR < 3)

typedef 	EventSource              event_source_t ;	  
typedef 	Thread			 thread_t 	 ; 
typedef 	halrtcnt_t		 rtcnt_t	;	  
typedef 	Mutex			 mutex_t 	 ; 
typedef 	EventListener		 event_listener_t ; 
typedef 	Mailbox			 mailbox_t	  ;
typedef 	BinarySemaphore		 binary_semaphore_t;

#define MSG_OK                          RDY_OK	       
#define MSG_RESET			RDY_RESET      
#define MSG_TIMEOUT			RDY_TIMEOUT    
#define CH_CFG_USE_HEAP			CH_USE_HEAP    
#define CH_CFG_USE_DYNAMIC		CH_USE_DYNAMIC 
#define CH_STATE_NAMES			THD_STATE_NAMES
#define THD_WORKING_AREA_SIZE(s)	THD_WA_SIZE(s) 
#define THD_WORKING_AREA		WORKING_AREA   
#define OSAL_SUCCESS			CH_SUCCESS     
#define OSAL_FAILED			CH_FAILED      

#define I2C_BUS_ERROR                   I2CD_BUS_ERROR	       
#define I2C_ARBITRATION_LOST		I2CD_ARBITRATION_LOST  
#define I2C_ACK_FAILURE			I2CD_ACK_FAILURE       
#define I2C_OVERRUN			I2CD_OVERRUN 	       
#define I2C_PEC_ERROR			I2CD_PEC_ERROR 	       
#define I2C_TIMEOUT			I2CD_TIMEOUT 	       
#define I2C_SMB_ALERT			I2CD_SMB_ALERT 	       

#define PAL_STM32_PUPDR_PULLUP     PAL_STM32_PUDR_PULLUP	
#define PAL_STM32_PUPDR_PULLDOWN   PAL_STM32_PUDR_PULLDOWN	
#define PAL_STM32_PUPDR_FLOATING   PAL_STM32_PUDR_FLOATING	

#define chVTGetSystemTimeX         chTimeNow	     
#define chThdTerminatedX	   chThdTerminated	     
#define chThdShouldTerminateX	   chThdShouldTerminate 
#define chEvtObjectInit		   chEvtInit 	     
#define chMtxObjectInit		   chMtxInit 	     
#define chBSemObjectInit	   chBSemInit 	     
#define chMBObjectInit		   chMBInit 	     
#define chCoreGetStatusX	   chCoreStatus 	     
#define chRegGetThreadNameX    	   chRegGetThreadName   

// #define halGetCounterValue()    	   chSysGetRealtimeCounterX()
// #define halGetCounterFrequency()    	   STM32_SYSCLK	
// #define halPolledDelay    	      	   chSysPolledDelayX
// #define halIsCounterWithin(a,b)    	   chSysIsCounterWithinX(chSysGetRealtimeCounterX(),a,b)

#define   chSysLockFromISR()   chSysLockFromIsr()   
#define   chSysUnlockFromISR() chSysUnlockFromIsr()

#define   chMtxUnlock(m) chMtxUnlock()

#undef    chSysHalt
#define   chSysHalt(m)   port_halt()
#define   f_closedir(d)  MSG_OK

// #define S2RTT(sec) S2RTC(STM32_SYSCLK, sec)
// #define MS2RTT(msec) MS2RTC(STM32_SYSCLK, msec)
// #define US2RTT(usec) US2RTC(STM32_SYSCLK, usec)

#define CH_CFG_ST_FREQUENCY CH_FREQUENCY 
#endif
