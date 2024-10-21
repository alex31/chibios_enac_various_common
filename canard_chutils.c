/**
 * @file    canard_chutils.c
 * @brief   Various utilities for libcanard use with ChibiOS.
 * @{
 */

#include <ch.h>
#include <hal.h>
#include <string.h>

#include "canard.h"

typedef struct {
  uint8_t id[16];
} UniqId_t;

/*===========================================================================*/
/* Module local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   DLC -> Data length converter.
 */

static uint16_t dlcToDataLength(uint16_t dlc) {
  static const uint16_t lookup[] = {0,1,2,3,4,5,6,7,8,12,16,20,24,32,48,64};
  
  chDbgAssert(dlc <= 15, "invalid DLC value");
  //  DebugTrace("dlcToDataLength(%u) = %u", dlc, lookup[dlc]);
  return lookup[dlc];
}

/**
 * @brief   DLC <- Data length converter.
 */

static uint8_t dataLengthToDlc(uint16_t data_length) {
  static const uint8_t lookup[] = {0, 1, 2, 3, 4, 5, 6, 7, 8,
                                    9, 9, 9, 9, 10,10,10,10,
                                    11,11,11,11,12,12,12,12,
                                    13,13,13,13,13,13,13,13,
                                    14,14,14,14,14,14,14,14,
                                    14,14,14,14,14,14,14,14,
                                    15,15,15,15,15,15,15,15,
                                    15,15,15,15,15,15,15,15};
  chDbgAssert(data_length <= 64, "invalid data length value");
  //  DebugTrace("dataLengthToDlc(%u) = %u", data_length, lookup[data_length]);
  return lookup[data_length];
}

/*===========================================================================*/
/* Module exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   ChibiOS CANRxFrame to Canard CanardCANFrame Type converter.
 */
CanardCANFrame chibiRx2canard(const CANRxFrame frame){
  CanardCANFrame out_frame;


  out_frame.id = frame.ext.EID | frame.common.XTD << 31U |
    frame.common.RTR << 30U | frame.common.ESI << 29U;
#if CANARD_ENABLE_CANFD
  out_frame.canfd = frame.FDF | frame.BRS;
#endif
  out_frame.iface_id = 0;
  out_frame.data_len = dlcToDataLength(frame.DLC);
  memcpy(out_frame.data, frame.data8, out_frame.data_len);
  return out_frame;
}

/**
 * @brief   Canard CanardCANFrame to ChibiOS CANTxFrame Type converter.
 */
CANTxFrame canard2chibiTx(const CanardCANFrame* framep){
  CANTxFrame out_frame;
  out_frame.common.XTD = 1;
  out_frame.common.RTR = 0;
  out_frame.ext.EID = framep->id & 0x1FFFFFFF;
#if CANARD_ENABLE_CANFD
  out_frame.FDF = framep->canfd;         /* FDCAN frame format. */
  out_frame.BPS = framep->canfd;
#else
  out_frame.FDF = false;
  out_frame.BPS = false;
#endif
  out_frame.DLC = dataLengthToDlc(framep->data_len);
  memcpy(out_frame.data8, framep->data, framep->data_len);
  return out_frame;
}

/**
 * @brief   Unique ID Fetcher.
 * @note    Retrieves the first two bytes of the MCU's unique id.
 */
void getUniqueID(UniqId_t *uid)
{
  memcpy(uid->id, (const uint8_t *)UID_BASE, sizeof(uid->id));
} 
/** @} */
