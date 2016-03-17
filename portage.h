#include "hal.h"

#if (CH_KERNEL_MAJOR > 2)

typedef event_source_t 		EventSource;
typedef thread_t 		Thread;
typedef rtcnt_t			halrtcnt_t;
typedef mutex_t 		Mutex;
typedef event_listener_t 	EventListener;
typedef mailbox_t		Mailbox;
typedef binary_semaphore_t	BinarySemaphore;

#define RDY_OK MSG_OK
#define RDY_RESET MSG_RESET
#define RDY_TIMEOUT MSG_TIMEOUT
#define CH_USE_HEAP CH_CFG_USE_HEAP
#define CH_USE_DYNAMIC CH_CFG_USE_DYNAMIC
#define THD_STATE_NAMES CH_STATE_NAMES
#define THD_WA_SIZE(s) THD_WORKING_AREA_SIZE(s)
#define WORKING_AREA THD_WORKING_AREA
#define CH_SUCCESS	OSAL_SUCCESS
#define CH_FAILED	OSAL_FAILED

#define I2CD_BUS_ERROR		I2C_BUS_ERROR
#define I2CD_ARBITRATION_LOST 	I2C_ARBITRATION_LOST
#define I2CD_ACK_FAILURE 	I2C_ACK_FAILURE
#define I2CD_OVERRUN 		I2C_OVERRUN
#define I2CD_PEC_ERROR 		I2C_PEC_ERROR
#define I2CD_TIMEOUT 		I2C_TIMEOUT
#define I2CD_SMB_ALERT 		I2C_SMB_ALERT

#define PAL_STM32_PUDR_PULLUP	PAL_STM32_PUPDR_PULLUP
#define PAL_STM32_PUDR_PULLDOWN	PAL_STM32_PUPDR_PULLDOWN
#define PAL_STM32_PUDR_FLOATING	PAL_STM32_PUPDR_FLOATING

#define chTimeNow chVTGetSystemTimeX
#define chThdTerminated chThdTerminatedX
#define chThdShouldTerminate chThdShouldTerminateX
#define chEvtInit chEvtObjectInit
#define chMtxInit chMtxObjectInit
#define chBSemInit chBSemObjectInit
#define chCoreStatus chCoreGetStatusX
#define chRegGetThreadName chRegGetThreadNameX

#define halGetCounterValue() chSysGetRealtimeCounterX()
#define halGetCounterFrequency() STM32_SYSCLK	
#define halPolledDelay chSysPolledDelayX
#define halIsCounterWithin(a,b) chSysIsCounterWithinX(chSysGetRealtimeCounterX(),a,b)

#define chSysLockFromIsr() chSysLockFromISR()
#define chSysUnlockFromIsr() chSysUnlockFromISR()

#define S2RTT(sec) (sec * STM32_SYSCLK)
#define CH_FREQUENCY CH_CFG_ST_FREQUENCY
#endif
