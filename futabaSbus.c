#include <ch.h>
#include <hal.h>
#include "futabaSbus.h"

#define SBUS_START_BYTE 0x0f
#define SBUS_END_BYTE   0x00
#define SBUS_FLAGS_BYTE 22U
#define SBUS_FRAME_LOST_BIT 2U
#define SBUS_FAILSAFE_BIT 3U
#define SBUS_BUFFLEN 24U


static SerialConfig sbusSerialConfig =
  {
   .speed = 100000,
#ifdef USART_CR2_RXINV // UARTv2
   .cr1 = USART_CR1_PCE | USART_CR1_M_0, // 8 bits + even parity => 9 bits mode
#else			// UARTv1
   .cr1 = USART_CR1_PCE | USART_CR1_M, // 8 bits + even parity => 9 bits mode
#endif
   .cr2 = USART_CR2_STOP2_BITS,
   .cr3 = 0
  };


static void receivingLoopThread(void *arg);
static void decodeSbusBuffer (const uint8_t *src, SBUSFrame  *frm);

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
  chDbgAssert(configp->invertedLevel == true,
	      "signal must have been inverted by external device on UARTv1 device");
#else
  if (configp->externallyInverted == false)
    sbusSerialConfig.cr2 |= USART_CR2_RXINV;
#endif

  sdStart(configp->sd, &sbusSerialConfig);
}

void sbusStop(SBUSDriver *sbusp)
{
  sdStop(sbusp->config->sd);
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
  SBUSDriver *sbusp = (SBUSDriver *) arg;
  const SBUSConfig *cfg = sbusp->config;
  uint8_t  sbusBuffer[SBUS_BUFFLEN];
  SBUSFrame frame;
  int rbyte;

  while (!chThdShouldTerminateX()) {
    do {
      rbyte = sdGetTimeout(cfg->sd, TIME_MS2I(100));
      if (rbyte <= 0) {
	invoqueError(cfg, SBUS_TIMOUT);
      }
    } while (rbyte != SBUS_START_BYTE);

    if (sdReadTimeout(cfg->sd, sbusBuffer, sizeof(sbusBuffer), TIME_MS2I(100))
	!= sizeof(sbusBuffer)) {
      invoqueError(cfg, SBUS_TIMOUT);
      continue;
    }

    if (sbusBuffer[SBUS_BUFFLEN-1] != SBUS_END_BYTE) {
      invoqueError(cfg, SBUS_MALFORMED_FRAME);
      continue;
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
      decodeSbusBuffer (sbusBuffer, &frame);
      cfg->frameCb(&frame);
    }
  }

  chThdExit(0);
}




static void decodeSbusBuffer (const uint8_t *src, SBUSFrame  *frm)
{
  int16_t *dst = frm->channel;
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
  frm->flags = src[22];
}
