#pragma once

#include <ch.h>
#include <hal.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SBUS_NUM_CHANNEL 16U

  typedef enum  {SBUS_LOST_FRAME, SBUS_FAILSAFE, SBUS_MALFORMED_FRAME,
		 SBUS_TIMOUT} SBUSError;
  
  typedef struct {
    int16_t channel[SBUS_NUM_CHANNEL];
    uint8_t flags;
  } SBUSFrame;
  
  typedef void (SBUSErrorCb) (SBUSError err) ;
  typedef void (SBUSFrameCb) (SBUSFrame *frame) ;

  typedef struct {
    SerialDriver *sd;
    SBUSErrorCb  *errorCb;
    SBUSFrameCb  *frameCb;
    size_t	 threadWASize;
    bool	 externallyInverted;
  } SBUSConfig;
  

  
  typedef struct {
    const SBUSConfig *config;
    thread_t *wth;
  } SBUSDriver;

  void sbusObjectInit(SBUSDriver *sbusp);
  void sbusStart(SBUSDriver *sbusp, const SBUSConfig *configp);
  void sbusStop(SBUSDriver *sbusp);
  void sbusStartReceive(SBUSDriver *sbusp);
  void sbusStopReceive(SBUSDriver *sbusp);


#ifdef __cplusplus
 }
#endif
