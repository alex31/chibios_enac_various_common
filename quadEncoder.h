#pragma once

#include "ch.h"
#include "hal.h"

#ifdef __cplusplus
extern "C" {
#endif

  typedef struct {
    uint32_t cnt;
    bool     hasChanged;
  } QEncoderCnt;

typedef struct {
    stm32_tim_t * const timer;
  } QEncoderConfig;


  typedef struct {
    const QEncoderConfig *config;
    uint32_t lastCnt;
    mutex_t mut;
  } QEncoderDriver;



void qencoderObjectInit(QEncoderDriver *qencoderp);
void qencoderStart(QEncoderDriver *qencoderp, const QEncoderConfig *configp);
void qencoderStop(QEncoderDriver *qencoderp);
QEncoderCnt qencoderGetCNT(QEncoderDriver * const qencoderp);
void qencoderSetCNT(QEncoderDriver *qencoderp, const uint32_t cnt);


#ifdef __cplusplus
}
#endif
