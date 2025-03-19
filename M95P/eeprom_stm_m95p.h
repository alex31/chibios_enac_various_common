#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "hal.h"

  typedef struct {
    void *device;
  } M95P_instance_t;

  typedef enum  {
    M95P_WIP  = 1 << 0,
    M95P_WEL  = 1 << 1,
    M95P_BP0  = 1 << 2,
    M95P_BP1  = 1 << 3,
    M95P_BP2  = 1 << 4,
    M95P_TB   = 1 << 6,
    M95P_SRWD = 1 << 7,
  }  M95P_status_t;

  typedef struct {
    uint8_t manufacturer;
    uint8_t family;
    uint8_t density;
    uint8_t uidLength;
  } MP95_Identification;

  M95P_instance_t M95P_new(SPIDriver *spid, const SPIConfig *config);
  msg_t M95P_read(const M95P_instance_t self,
		  uint32_t offset, uint32_t n, uint8_t *buffer);
  msg_t M95P_readStatus(const M95P_instance_t self, M95P_status_t* status);
  msg_t M95P_readIdentification(const M95P_instance_t self,
				MP95_Identification *ident);
  msg_t M95P_softwareReset(const M95P_instance_t self);
  msg_t M95P_program(const M95P_instance_t self,
		     uint32_t offset, uint32_t n, const uint8_t *buffer);
  msg_t M95P_write(const M95P_instance_t self,
		     uint32_t offset, uint32_t n, const uint8_t *buffer);
  msg_t M95P_erase(const M95P_instance_t self,
		   uint32_t offset, uint32_t n);
  msg_t M95P_erase_chip(const M95P_instance_t self);
  msg_t M95P_erase_block(const M95P_instance_t self, uint32_t offset);
  msg_t M95P_erase_sector(const M95P_instance_t self, uint32_t offset);
  msg_t M95P_erase_page(const M95P_instance_t self, uint32_t offset);
  
#ifdef __cplusplus
}
#endif

