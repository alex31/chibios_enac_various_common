#include "hal.h"

#if (CH_KERNEL_MAJOR > 2)

typedef event_source_t EventSource;
typedef thread_t Thread;
typedef rtcnt_t	halrtcnt_t;
typedef mutex_t Mutex;

#define RDY_OK MSG_OK
#define CH_USE_HEAP CH_CFG_USE_HEAP
#define CH_USE_DYNAMIC CH_CFG_USE_DYNAMIC
#define THD_STATE_NAMES CH_STATE_NAMES
#define THD_WA_SIZE(s) THD_WORKING_AREA_SIZE(s)

#define chTimeNow chVTGetSystemTimeX
#define chThdTerminated chThdTerminatedX
#define chThdShouldTerminate chThdShouldTerminateX
#define chEvtInit chEvtObjectInit
#define chMtxInit chMtxObjectInit
#define chCoreStatus chCoreGetStatusX
#define chRegGetThreadName chRegGetThreadNameX

#define halGetCounterValue() chSysGetRealtimeCounterX()
#define halGetCounterFrequency()	STM32_HCLK
#define halPolledDelay chSysPolledDelayX

#define chSysLockFromIsr() chSysLockFromISR()
#define chSysUnlockFromIsr() chSysUnlockFromISR()


#endif
