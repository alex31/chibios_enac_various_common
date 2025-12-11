#include <ch.h>
#include <hal.h>
#include "futabaSbusUart.h"
#include "stdutil.h"

#define SBUS_START_BYTE 0x0f
#define SBUS_END_BYTE   0x00
#define SBUS_FLAGS_BYTE 23U
#define SBUS_BUFFLEN 25U
#define IN_SYNC_TIMEOUT TIME_MS2I(15)
#define OUT_SYNC_TIMEOUT (IN_SYNC_TIMEOUT / 2U)


static UARTConfig sbusUartConfig =  {
  .timeout_cb = nullptr,
  .timeout = 20,
  .speed = 100000,
#ifdef USART_CR2_RXINV // UARTv2
  .cr1 = USART_CR1_PCE | USART_CR1_M0, // 8 bits + even parity => 9 bits mode
#else			// UARTv1
  .cr1 = USART_CR1_PCE | USART_CR1_M, // 8 bits + even parity => 9 bits mode
#endif
  .cr2 = USART_CR2_STOP2_BITS,
  .cr3 = 0
};


static void receivingLoopThread(void *arg);
static void decodeSbusBuffer (const uint8_t *src, SBUSFrame  *frm);
static void encodeSbusBuffer (const SBUSFrame  *frm, uint8_t *dest);

void sbusObjectInit(SBUSDriver *sbusp)
{
  sbusp->config = NULL;
  sbusp->wth = NULL;
}

void sbusStart(SBUSDriver *sbusp, const SBUSConfig *configp)
{
  sbusp->config = configp;
#ifndef USART_CR2_RXINV
  // USARTv1 without level inversion capability
  // signal must have been inverted by external device
  chDbgAssert(configp->externallyInverted == true,
	      "signal must have been inverted by external device on UARTv1 device");
#else
  if (configp->externallyInverted == false)
    sbusUartConfig.cr2 |= (USART_CR2_RXINV | USART_CR2_TXINV);
#endif

  uartStart(configp->uartd, &sbusUartConfig);
}

void sbusStop(SBUSDriver *sbusp)
{
  uartStop(sbusp->config->uartd);
  sbusObjectInit(sbusp);
}

void sbusStartReceive(SBUSDriver *sbusp)
{
  const SBUSConfig *cfg = sbusp->config;
  chDbgAssert(sbusp->wth == NULL, "already in active state");
  sbusp->wth = chThdCreateFromHeap (NULL, cfg->threadWASize, "sbus_receive", 
				    NORMALPRIO, receivingLoopThread, sbusp);
}

void sbusStopReceive(SBUSDriver *sbusp)
{
  if (sbusp->wth != NULL) {
    chThdTerminate(sbusp->wth);
    chThdWait(sbusp->wth);
    sbusp->wth = NULL;
  }
}

static inline void invoqueError(const SBUSConfig *cfg, SBUSError err) {
  if (cfg->errorCb) {
    cfg->errorCb(err);
  }
}

static void receivingLoopThread (void *arg)
{
  SBUSDriver	   *sbusp = (SBUSDriver *) arg;
  const SBUSConfig *cfg = sbusp->config;
  uint8_t          *sbusBuffer = malloc_dma(SBUS_BUFFLEN);
  SBUSFrame	   frame;


  if (sbusBuffer == nullptr) {
     cfg->errorCb(SBUS_MALLOC_ERROR);
     chThdSleep(TIME_INFINITE);
  }
  
  while (!chThdShouldTerminateX()) {
    size_t size = SBUS_BUFFLEN;
    uartReceiveTimeout(cfg->uartd, &size, sbusBuffer, TIME_INFINITE);

    if (size !=  SBUS_BUFFLEN) {
      invoqueError(cfg, SBUS_TIMOUT);
      goto outOfSync;
    }
    
    if (sbusBuffer[0] != SBUS_START_BYTE) {
      invoqueError(cfg, SBUS_MALFORMED_FRAME);
      goto outOfSync;
    }
    
    if (sbusBuffer[SBUS_BUFFLEN - 1] != SBUS_END_BYTE) {
      invoqueError(cfg, SBUS_MALFORMED_FRAME);
      goto outOfSync;
    }
    
    if ((sbusBuffer[SBUS_FLAGS_BYTE] >> SBUS_FRAME_LOST_BIT) & 0x1) {
      invoqueError(cfg, SBUS_LOST_FRAME);
      continue;
    }
    
    if ((sbusBuffer[SBUS_FLAGS_BYTE] >> SBUS_FAILSAFE_BIT) & 0x1) {
      invoqueError(cfg, SBUS_FAILSAFE);
      continue;
    }
    
    if (cfg->frameCb) {
      decodeSbusBuffer(sbusBuffer+1, &frame);
      cfg->frameCb(&frame);
    }

    continue;
    
  outOfSync :
    chThdSleepMilliseconds(1);
  }
  
  chThdExit(0);
}

// not reentrant
void sbusSend(SBUSDriver *sbusp, const SBUSFrame *frame)
{
  static uint8_t IN_DMA_SECTION_NOINIT(sbusBuffer[SBUS_BUFFLEN]);
  size_t size = SBUS_BUFFLEN;
  encodeSbusBuffer(frame, sbusBuffer);
  uartSendTimeout(sbusp->config->uartd, &size, sbusBuffer, TIME_INFINITE);
}


static void decodeSbusBuffer (const uint8_t *src, SBUSFrame  *frm)
{
  uint16_t *dst = frm->channel;
  // decode sbus data
  dst[0]  = ((src[0]    ) | (src[1]<<8))                  & 0x07FF;
  dst[1]  = ((src[1]>>3 ) | (src[2]<<5))                  & 0x07FF;
  dst[2]  = ((src[2]>>6 ) | (src[3]<<2)  | (src[4]<<10))  & 0x07FF;
  dst[3]  = ((src[4]>>1 ) | (src[5]<<7))                  & 0x07FF;
  dst[4]  = ((src[5]>>4 ) | (src[6]<<4))                  & 0x07FF;
  dst[5]  = ((src[6]>>7 ) | (src[7]<<1 ) | (src[8]<<9))   & 0x07FF;
  dst[6]  = ((src[8]>>2 ) | (src[9]<<6))                  & 0x07FF;
  dst[7]  = ((src[9]>>5)  | (src[10]<<3))                 & 0x07FF;
  dst[8]  = ((src[11]   ) | (src[12]<<8))                 & 0x07FF;
  dst[9]  = ((src[12]>>3) | (src[13]<<5))                 & 0x07FF;
  dst[10] = ((src[13]>>6) | (src[14]<<2) | (src[15]<<10)) & 0x07FF;
  dst[11] = ((src[15]>>1) | (src[16]<<7))                 & 0x07FF;
  dst[12] = ((src[16]>>4) | (src[17]<<4))                 & 0x07FF;
  dst[13] = ((src[17]>>7) | (src[18]<<1) | (src[19]<<9))  & 0x07FF;
  dst[14] = ((src[19]>>2) | (src[20]<<6))                 & 0x07FF;
  dst[15] = ((src[20]>>5) | (src[21]<<3))                 & 0x07FF;

  frm->flags = src[SBUS_FLAGS_BYTE];
}


static void encodeSbusBuffer (const SBUSFrame  *_frm, uint8_t *dest)
{
  const uint16_t * const chan = _frm->channel;
 
  dest[0] = SBUS_START_BYTE;
  dest[1] =   (uint8_t) ((chan[0]   & 0x07FF));
  dest[2] =   (uint8_t) ((chan[0]   & 0x07FF) >> 8  | (chan[1]  & 0x07FF) << 3);
  dest[3] =   (uint8_t) ((chan[1]   & 0x07FF) >> 5  | (chan[2]  & 0x07FF) << 6);
  dest[4] =   (uint8_t) ((chan[2]   & 0x07FF) >> 2);
  dest[5] =   (uint8_t) ((chan[2]   & 0x07FF) >> 10 | (chan[3]  & 0x07FF) << 1);
  dest[6] =   (uint8_t) ((chan[3]   & 0x07FF) >> 7  | (chan[4]  & 0x07FF) << 4);
  dest[7] =   (uint8_t) ((chan[4]   & 0x07FF) >> 4  | (chan[5]  & 0x07FF) << 7);
  dest[8] =   (uint8_t) ((chan[5]   & 0x07FF) >> 1);
  dest[9] =   (uint8_t) ((chan[5]   & 0x07FF) >> 9  | (chan[6]  & 0x07FF) << 2);
  dest[10] =  (uint8_t) ((chan[6]   & 0x07FF) >> 6  | (chan[7]  & 0x07FF) << 5);
  dest[11] =  (uint8_t) ((chan[7]   & 0x07FF) >> 3);
  dest[12] =  (uint8_t) ((chan[8]   & 0x07FF));
  dest[13] =  (uint8_t) ((chan[8]   & 0x07FF) >> 8  | (chan[9]  & 0x07FF) << 3);
  dest[14] =  (uint8_t) ((chan[9]   & 0x07FF) >> 5  | (chan[10] & 0x07FF) << 6);
  dest[15] =  (uint8_t) ((chan[10]  & 0x07FF) >> 2);
  dest[16] =  (uint8_t) ((chan[10]  & 0x07FF) >> 10 | (chan[11] & 0x07FF) << 1);
  dest[17] =  (uint8_t) ((chan[11]  & 0x07FF) >> 7  | (chan[12] & 0x07FF) << 4);
  dest[18] =  (uint8_t) ((chan[12]  & 0x07FF) >> 4  | (chan[13] & 0x07FF) << 7);
  dest[19] =  (uint8_t) ((chan[13]  & 0x07FF) >> 1);
  dest[20] =  (uint8_t) ((chan[13]  & 0x07FF) >> 9  | (chan[14] & 0x07FF) << 2);
  dest[21] =  (uint8_t) ((chan[14]  & 0x07FF) >> 6  | (chan[15] & 0x07FF) << 5);
  dest[22] =  (uint8_t) ((chan[15]  & 0x07FF) >> 3);
  dest[23] = _frm->flags;
  dest[24] = SBUS_END_BYTE;
}
