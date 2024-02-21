/*
  ChibiOS/RT - Copyright (C) 2006,2007,2008,2009,2010,
  2011,2012 Giovanni Di Sirio.

  This file is part of ChibiOS/RT.

  ChibiOS/RT is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  ChibiOS/RT is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

  ---

  A special exception to the GPL can be applied should you wish to distribute
  a combined work that includes ChibiOS/RT, without being obliged to provide
  the source code for any proprietary components. See the file exception.txt
  for full details of how and when the exception can be applied.
*/

#include "ch.h"
#include "hal.h"
#include "sdio.h"



/*
  #                 ______   _____     _____    ____          
  #                /  ____| |  __ \   |_   _|  / __ \         
  #                | (___   | |  | |    | |   | |  | |        
  #                 \___ \  | |  | |    | |   | |  | |        
  #                .____) | | |__| |   _| |_  | |__| |        
  #                \_____/  |_____/   |_____|  \____/         
*/



static const SDCConfig sdc1bitCfg = {
  SDC_MODE_4BIT,
  0x0
};


/*
  #                  ___    _____    _____         
  #                 / _ \  |  __ \  |_   _|        
  #                | |_| | | |__) |   | |          
  #                |  _  | |  ___/    | |          
  #                | | | | | |       _| |_         
  #                |_| |_| |_|      |_____|        
*/

bool sdioIsCardResponding (void)
{
  if (SDCD1.state == BLK_READY) 
    return true;
  
  sdcStart(&SDCD1, &sdc1bitCfg);
  bool ret = sdcConnect(&SDCD1) == OSAL_SUCCESS;

  sdcDisconnect(&SDCD1);
  sdcStop(&SDCD1);
  return ret;
}

bool sdioIsConnected (void) 
{
  return (SDCD1.state == BLK_READY);
}


bool sdioConnect (void) 
{

  if (!sdc_lld_is_card_inserted (NULL)) {
    return FALSE;
  }

  if (SDCD1.state == BLK_READY) {
    return TRUE;
  }

  sdcStart(&SDCD1, &sdc1bitCfg);
  uint32_t count = 20;
  while ((sdcConnect(&SDCD1) != OSAL_SUCCESS) && --count) {
    chThdSleepMilliseconds(100);
  }

  return SDCD1.state == BLK_READY;
}


bool sdioDisconnect (void)
{
  if (SDCD1.state == BLK_STOP) 
    return TRUE;
  if (sdcDisconnect(&SDCD1)) {
    return FALSE;
  }
  sdcStop (&SDCD1);
  return TRUE;
}

bool isCardInserted  (void)
{
  return sdc_lld_is_card_inserted (NULL);
}

