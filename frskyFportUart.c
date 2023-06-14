#include <ch.h>
#include <hal.h>
#include <string.h>
#include "frskyFportUart.h"
#include "stdutil.h"
#include "math.h"
#include "ttyConsole.h"

// delay before answering to command frame, must no exceed 1ms, but must
// leave time for the frsky receiver to reconfigure RXTX pin in input mode
#define FPORT_ANSWER_DELAY_US 350U
#define WORKING_AREA_SIZE     1024U


static void receivingLoopThread (void *arg);
static void sendTelemetryAfter300Us(const void *buffer, size_t len,
				    void *optArg);
static void newControlMsg(const Fport_ControlFrame *, void *optArg);


static UARTConfig fportUartConfig = {
  .speed = 115200,
  .cr1 = 0, // 8 bits, no parity, one stop bit
  .cr2 = USART_CR2_STOP1_BITS | USART_CR2_LINEN, 
  .cr3 = USART_CR3_HDSEL // half duplex
};

/*
#                         _         _                 _      _____           _    _            
#                        | |       (_)               | |    |_   _|         (_)  | |           
#                  ___   | |__     | |   ___    ___  | |_     | |    _ __    _   | |_          
#                 / _ \  | '_ \    | |  / _ \  / __| | __|    | |   | '_ \  | |  | __|         
#                | (_) | | |_) |  _/ | |  __/ | (__  \ |_    _| |_  | | | | | |  \ |_          
#                 \___/  |_.__/  |__/   \___|  \___|  \__|  |_____| |_| |_| |_|   \__|         
*/
void fportUartObjectInit(FPORTDriver *fportp)
{
  memset(fportp, 0, sizeof(FPORTDriver));
}


/*
#                        _                     _            
#                       | |                   | |           
#                 ___   | |_     __ _   _ __  | |_          
#                / __|  | __|   / _` | | '__| | __|         
#                \__ \  \ |_   | (_| | | |    \ |_          
#                |___/   \__|   \__,_| |_|     \__|         
*/
void fportUartStart(FPORTDriver *fportp, const FPORTConfig *configp)
{
  fportp->config = configp;
  fportp->errorMsk = FPORT_OK;
  static FportFsmContextConfig fsmContextConfig;

  fsmContextConfig = (FportFsmContextConfig) {
    .tlmSendCb = &sendTelemetryAfter300Us,
    .ctrlReceiveCb = newControlMsg,
    .optArg = fportp,
    .middlePoint = configp->middlePoint
  };
  
  fportContextInit(&fportp->context, &fsmContextConfig);

#ifndef USART_CR2_RXINV
  // USARTv1 without level inversion capability
  // signal must have been inverted by external device
  chDbgAssert(configp->driverShouldInvert == false,
	      "fonction not available on UARTv1 device (F4xx)");
#else
  if (configp->driverShouldInvert == true)
    fportUartConfig.cr2 |= (USART_CR2_RXINV | USART_CR2_TXINV);
#endif

  uartStart(configp->uart, &fportUartConfig);
}


/*
#                        _              _ __          
#                       | |            | '_ \         
#                 ___   | |_     ___   | |_) |        
#                / __|  | __|   / _ \  | .__/         
#                \__ \  \ |_   | (_) | | |            
#                |___/   \__|   \___/  |_|            
*/
void fportUartStop(FPORTDriver *fportp)
{
  uartStop(fportp->config->uart);
  fportUartObjectInit(fportp);
}


/*
#                        _                     _            
#                       | |                   | |           
#                 ___   | |_     __ _   _ __  | |_          
#                / __|  | __|   / _` | | '__| | __|         
#                \__ \  \ |_   | (_| | | |    \ |_          
#                |___/   \__|   \__,_| |_|     \__|         
#                 _____                         _                         
#                |  __ \                       (_)                        
#                | |__) |   ___    ___    ___   _   __   __   ___         
#                |  _  /   / _ \  / __|  / _ \ | |  \ \ / /  / _ \        
#                | | \ \  |  __/ | (__  |  __/ | |   \ V /  |  __/        
#                |_|  \_\  \___|  \___|  \___| |_|    \_/    \___|        
*/
void fportUartStartReceive(FPORTDriver *fportp)
{
  chDbgAssert(fportp->wth == NULL, "already in active state");
  fportp->wth = chThdCreateFromHeap (NULL, WORKING_AREA_SIZE, "fport_loop", 
				    NORMALPRIO + 2, receivingLoopThread, fportp);
}


/*
#                        _              _ __          
#                       | |            | '_ \         
#                 ___   | |_     ___   | |_) |        
#                / __|  | __|   / _ \  | .__/         
#                \__ \  \ |_   | (_) | | |            
#                |___/   \__|   \___/  |_|            
#                 _____                         _                         
#                |  __ \                       (_)                        
#                | |__) |   ___    ___    ___   _   __   __   ___         
#                |  _  /   / _ \  / __|  / _ \ | |  \ \ / /  / _ \        
#                | | \ \  |  __/ | (__  |  __/ | |   \ V /  |  __/        
#                |_|  \_\  \___|  \___|  \___| |_|    \_/    \___|        
*/
void fportUartStopReceive(FPORTDriver *fportp)
{
  if (fportp->wth != NULL) {
    chThdTerminate(fportp->wth);
    chThdWait(fportp->wth);
    fportp->wth = NULL;
  }
}


/*
#                 _                    _ __          
#                | |                  | '_ \         
#                | |    ___     ___   | |_) |        
#                | |   / _ \   / _ \  | .__/         
#                | |  | (_) | | (_) | | |            
#                |_|   \___/   \___/  |_|            
*/
static void receivingLoopThread (void *arg)
{
  FPORTDriver *fportp = (FPORTDriver *) arg;
  FportFsmContext *context = &fportp->context;
  uint8_t buffer[64];
  
  
  while(!chThdShouldTerminateX()) {
    const size_t optReadLen = fportGetNextReadLen(context);
    palWriteLine(LINE_LED_DBG1,  (optReadLen != 41) ? PAL_HIGH : PAL_LOW);
    size_t readLen = optReadLen;
    const msg_t status = uartReceiveTimeout(fportp->config->uart, &readLen, buffer, TIME_MS2I(10));
    //    uncomment following line to debug desync situation : listPush must be implemented in ttyConsole.c
    //    listPush(optReadLen, readLen);
    if ((optReadLen != readLen) || (status != MSG_OK)) {
      fportp->errorMsk |= FPORT_TIMEOUT;
    }
    fportp->errorMsk |= fportFeedFsm(context, buffer, readLen);
    if ((fportp->errorMsk != FPORT_OK) && (fportp->config->errCb != nullptr)){
      fportp->config->errCb(fportp);
    }
  }
}



static void sendTelemetryAfter300Us(const void *buffer, size_t len,
				    void *optArg)
{
  FPORTDriver *fportp = (FPORTDriver *) optArg;
  // uint8_t loopbackBuffer[24];
  // we are in thread context, can use sleep API
  chThdSleepMicroseconds(350);
  uartSendTimeout(fportp->config->uart, &len, buffer, TIME_INFINITE);
  chThdSleepMicroseconds(350);
}

static void newControlMsg(const Fport_ControlFrame *, void *optArg)
{
  FPORTDriver *fportp = (FPORTDriver *) optArg;
  if (fportp->config->ctrlMsgCb != nullptr)
    fportp->config->ctrlMsgCb(fportp);
}

#if FPORT_MAX_DATAIDS > 0
  void fportUartTelemetryUpData(FPORTDriver *fportp, const fportAppId appId, 
			    const FportTelemetryValue v)
  {
    fportTelemetryUpData(&fportp->context, appId, v);
  }
  
  void fportUartTelemetryUpDataMultiplexed(FPORTDriver *fportp, const fportAppId appId, 
				       const FportTelemetryValue *v,
				       const FportMultiplexData nbMplx)
  {
    fportTelemetryUpDataMultiplexed(&fportp->context, appId, v, nbMplx);
  }
#endif
