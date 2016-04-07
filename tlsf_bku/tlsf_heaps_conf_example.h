#if defined STM32F4XX
#define NODMA_SECTION ".ram4"
#define DMA_SECTION ".ram0"
#elif  defined STM32F7XX
#define NODMA_SECTION ".ram0"
#define DMA_SECTION ".ram3"
#else
#error "section defined only for STM32F4 and STM32F7"
#endif

#define HEAP_CCM		ccmHeap
#define HEAP_CCM_SIZE		16384
#define HEAP_CCM_SECTION	NODMA_SECTION

#define HEAP_DEFAULT		HEAP_CCM
