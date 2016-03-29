#define TLSF_USE_LOCKS 1
#include <ch.h>
#define TLSF_DESTROY_LOCK(_unused_)     do{}while(0)
#define TLSF_ACQUIRE_LOCK(l)		chMtxLock (l)

#if (CH_KERNEL_MAJOR == 2)
#define TLSF_MLOCK_T			Mutex
#define TLSF_CREATE_LOCK(l)     	chMtxInit (l)
#define TLSF_RELEASE_LOCK(_unused_)    	chMtxUnlock ()
#else // CH_KERNEL_MAJOR >= 3
#define TLSF_MLOCK_T			mutex_t
#define TLSF_CREATE_LOCK(l)     	chMtxObjectInit (l)
#define TLSF_RELEASE_LOCK(l)    	chMtxUnlock (l)
#endif

#define BLOCK_ALIGN (sizeof(void *))

#define MAX_FLI		(16) // limited to 2**MAX_FLI bytes : 64ko buffer
#define MAX_LOG2_SLI	(5) // 5 take 1.4ko overhead but limit fragmentation to 3%
			    // 4 take 700 bytes overhead but fragmentation raise to 6%
