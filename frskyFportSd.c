#include <ch.h>
#include <hal.h>
#include <string.h>
#include "frskyFportSd.h"
#include "stdutil.h"
#include "math.h"

// delay before answering to command frame, must no exceed 1ms, but must
// leave time for the frsky receiver to reconfigure RXTX pin in input mode
#define FPORT_ANSWER_DELAY_US 350U
#define WORKING_AREA_SIZE     1024U


static void receivingLoopThread (void *arg);
static void sendTelemetryAfter300Us(const void *buffer, size_t len,
				    void *optArg);
static void newControlMsg(const FportFsmContext *context);


static SerialConfig fportSerialConfig = {
  .speed = 115200,
  .cr1 = 0, // 8 bits, no parity, one stop bit
  .cr2 = USART_CR2_STOP1_BITS, 
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
void fportSdObjectInit(FPORTDriver *fportp)
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
void fportSdStart(FPORTDriver *fportp, const FPORTConfig *configp)
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

#ifdef USART_CR2_RXINV
  if (configp->driverShouldInvert == true)
    fportSerialConfig.cr2 |= (USART_CR2_RXINV | USART_CR2_TXINV);
#endif

  sdStart(configp->sd, &fportSerialConfig);
}


/*
#                        _              _ __          
#                       | |            | '_ \         
#                 ___   | |_     ___   | |_) |        
#                / __|  | __|   / _ \  | .__/         
#                \__ \  \ |_   | (_) | | |            
#                |___/   \__|   \___/  |_|            
*/
void fportSdStop(FPORTDriver *fportp)
{
  sdStop(fportp->config->sd);
  fportSdObjectInit(fportp);
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
void fportSdStartReceive(FPORTDriver *fportp)
{
  chDbgAssert(fportp->wth == NULL, "already in active state");
  fportp->wth = chThdCreateFromHeap (NULL, WORKING_AREA_SIZE, "fport_loop", 
				    NORMALPRIO, receivingLoopThread, fportp);
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
void fportSdStopReceive(FPORTDriver *fportp)
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
    //    DebugTrace("O=%u", optReadLen);
    const size_t readLen = sdReadTimeout(fportp->config->sd, buffer, optReadLen, TIME_MS2I(10));
    if (optReadLen != readLen) {
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
  uint8_t loopbackBuffer[24];
  // we are in thread context, can use sleep API
  chThdSleepMicroseconds(300);
  sdWrite(fportp->config->sd, buffer, len);
  sdRead(fportp->config->sd, loopbackBuffer, len); // loopback
}

static void newControlMsg(const FportFsmContext *context)
{
  FPORTDriver *fportp = (FPORTDriver *) context->config->optArg;
  if (fportp->config->ctrlMsgCb != nullptr)
    fportp->config->ctrlMsgCb(fportp);
}

#if FPORT_MAX_DATAIDS > 0
  void fportSdTelemetryUpData(FPORTDriver *fportp, const fportAppId appId, 
			    const FportTelemetryValue v)
  {
    fportTelemetryUpData(&fportp->context, appId, v);
  }
  
  void fportSdTelemetryUpDataMultiplexed(FPORTDriver *fportp, const fportAppId appId, 
				       const FportTelemetryValue *v,
				       const FportMultiplexData nbMplx)
  {
    fportTelemetryUpDataMultiplexed(&fportp->context, appId, v, nbMplx);
  }
#endif
