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

#include <string.h>
#include "ch.h"
#include "hal.h"
#include "printf.h"
#include "ff.h"  
#include "stdutil.h"
#include "globalVar.h"
#include "sdio.h"
#include "portage.h"

#ifndef GPIOC_SDIO_D0
#if defined STM32F4XX 
#define GPIOC_SDIO_D0  PC08_SDIO_D0
#define GPIOC_SDIO_D1  PC09_SDIO_D1
#define GPIOC_SDIO_D2  PC10_SDIO_D2
#define GPIOC_SDIO_D3  PC11_SDIO_D3
#define GPIOC_SDIO_CK  PC12_SDIO_CK
#define GPIOD_SDIO_CMD PD02_SDIO_CMD
#elif defined STM32F7XX 
#define	GPIOC_SDIO_D0  PC08_SDMMC1_D0
#define	GPIOC_SDIO_D1  PC09_SDMMC1_D1
#define	GPIOC_SDIO_D2  PC10_SDMMC1_D2
#define	GPIOC_SDIO_D3  PC11_SDMMC1_D3
#define GPIOC_SDIO_CK  PC12_SDMMC1_CK
#define GPIOD_SDIO_CMD PD02_SDMMC1_CMD
#endif
#endif


/*
#                 ______   _____     _____    ____          
#                /  ____| |  __ \   |_   _|  / __ \         
#                | (___   | |  | |    | |   | |  | |        
#                 \___ \  | |  | |    | |   | |  | |        
#                .____) | | |__| |   _| |_  | |__| |        
#                \_____/  |_____/   |_____|  \____/         
*/


/*
#                 _______   ______   ______   _______   ______         
#                |__   __| |  ____| /  ____| |__   __| /  ____|        
#                   | |    | |__    | (___      | |    | (___          
#                   | |    |  __|    \___ \     | |     \___ \         
#                   | |    | |____  .____) |    | |    .____) |        
#                   |_|    |______| \_____/     |_|    \_____/         
*/


/*
 * SDIO configuration.
 */

#define SDC_DATA_DESTRUCTIVE_TEST   TRUE

#define SDC_BURST_SIZE      8 /* how many sectors reads at once */
static uint8_t outbuf[MMCSD_BLOCK_SIZE * SDC_BURST_SIZE + 1];
static uint8_t  inbuf[MMCSD_BLOCK_SIZE * SDC_BURST_SIZE + 1];



/**
 * @brief   Parody of UNIX badblocks program.
 *
 * @param[in] start       first block to check
 * @param[in] end         last block to check
 * @param[in] blockatonce number of blocks to check at once
 * @param[in] pattern     check pattern
 *
 * @return              The operation status.
 * @retval SDC_SUCCESS  operation succeeded, the requested blocks have been
 *                      read.
 * @retval SDC_FAILED   operation failed, the state of the buffer is uncertain.
 */
bool badblocks(uint32_t start, uint32_t end, uint32_t blockatonce, uint8_t pattern){
  uint32_t position = 0;
  uint32_t i = 0;


#if (CH_KERNEL_MAJOR > 2)
  chDbgCheck (blockatonce <= SDC_BURST_SIZE);
#else
  chDbgCheck (blockatonce <= SDC_BURST_SIZE, "badblocks");
#endif

  /* fill control buffer */
  for (i=0; i < MMCSD_BLOCK_SIZE * blockatonce; i++)
    outbuf[i] = pattern;

  /* fill SD card with pattern. */
  position = start;
  while (position < end){
    if (sdcWrite(&SDCD1, position, outbuf, blockatonce))
      goto ERROR;
    position += blockatonce;
  }

  /* read and compare. */
  position = start;
  while (position < end){
    if (sdcRead(&SDCD1, position, inbuf, blockatonce))
      goto ERROR;
    if (memcmp(inbuf, outbuf, blockatonce * MMCSD_BLOCK_SIZE) != 0)
      goto ERROR;
    position += blockatonce;
  }
  return FALSE;

ERROR:
  return TRUE;
}

/**
 *
 */
void fillbuffer(uint8_t pattern, uint8_t *b){
  uint32_t i = 0;
  for (i=0; i < MMCSD_BLOCK_SIZE * SDC_BURST_SIZE; i++)
    b[i] = pattern;
}

/**
 *
 */
void fillbuffers(uint8_t pattern){
  fillbuffer(pattern, inbuf);
  fillbuffer(pattern, outbuf);
}


/**
 *
 */
void cmd_sdiotest(BaseSequentialStream *lchp, int argc,const char * const argv[]) {
  (void)argc;
  (void)argv;
  uint32_t i = 0;
  FRESULT err = 0;
  bool format = FALSE;

  if (argc == 1) {
    format = TRUE;
    chprintf(lchp, "tests io R/W et Formatage \r\n");
  } else {
    chprintf(lchp, "pas de formatage \r\n");
  }

  chprintf(lchp, "Trying to connect SDIO... ");
  chThdSleepMilliseconds(100);

  if (!sdioConnect ()) {
     chprintf(lchp, "   FAIL\r\n ");
    return;
  }

  if (TRUE) {
  chprintf(lchp, "OK\r\n");
  chprintf(lchp, "*** Card CSD content is: ");
  chprintf(lchp, "%X %X %X %X \r\n", (&SDCD1)->csd[3], (&SDCD1)->csd[2],
	   (&SDCD1)->csd[1], (&SDCD1)->csd[0]);
  

  chprintf(lchp, "capacity = %d sectors of 512 bytes = %d Mo\r\n", 
	   SDCD1.capacity, SDCD1.capacity / 2048);

  chprintf(lchp, "Single aligned read...");
  chThdSleepMilliseconds(100);
  if (sdcRead(&SDCD1, 0, inbuf, 1))
    goto error;
  chprintf(lchp, " OK\r\n");
  chThdSleepMilliseconds(100);
  
  
  chprintf(lchp, "Single unaligned read...");
  chThdSleepMilliseconds(100);
  if (sdcRead(&SDCD1, 0, inbuf + 1, 1))
    goto error;
  if (sdcRead(&SDCD1, 0, inbuf + 2, 1))
    goto error;
  if (sdcRead(&SDCD1, 0, inbuf + 3, 1))
    goto error;
  chprintf(lchp, " OK\r\n");
  chThdSleepMilliseconds(100);
  
  
  chprintf(lchp, "Multiple aligned reads...");
  chThdSleepMilliseconds(100);
  fillbuffers(0x55);
  /* fill reference buffer from SD card */
  if (sdcRead(&SDCD1, 0, inbuf, SDC_BURST_SIZE))
    goto error;
  for (i=0; i<1000; i++){
    if (sdcRead(&SDCD1, 0, outbuf, SDC_BURST_SIZE))
      goto error;
    if (memcmp(inbuf, outbuf, SDC_BURST_SIZE * MMCSD_BLOCK_SIZE) != 0)
      goto error;
  }
  chprintf(lchp, " OK\r\n");
  chThdSleepMilliseconds(100);
  
  
  chprintf(lchp, "Multiple unaligned reads...");
  chThdSleepMilliseconds(100);
  fillbuffers(0x55);
  /* fill reference buffer from SD card */
  if (sdcRead(&SDCD1, 0, inbuf + 1, SDC_BURST_SIZE))
    goto error;
  for (i=0; i<1000; i++){
    if (sdcRead(&SDCD1, 0, outbuf + 1, SDC_BURST_SIZE))
      goto error;
    if (memcmp(inbuf, outbuf, SDC_BURST_SIZE * MMCSD_BLOCK_SIZE) != 0)
      goto error;
  }
  chprintf(lchp, " OK\r\n");
  chThdSleepMilliseconds(100);
  
#if SDC_DATA_DESTRUCTIVE_TEST 
  if (format) {
    
    chprintf(lchp, "Single aligned write...");
    chThdSleepMilliseconds(100);
    fillbuffer(0xAA, inbuf);
    if (sdcWrite(&SDCD1, 0, inbuf, 1))
      goto error;
    fillbuffer(0, outbuf);
    if (sdcRead(&SDCD1, 0, outbuf, 1))
      goto error;
    if (memcmp(inbuf, outbuf, MMCSD_BLOCK_SIZE) != 0)
      goto error;
    chprintf(lchp, " OK\r\n");
    
    chprintf(lchp, "Running badblocks at 0x10000 offset...");
    chThdSleepMilliseconds(100);
    if(badblocks(0x10000, 0x11000, SDC_BURST_SIZE, 0xAA))
      goto error;
    chprintf(lchp, " OK\r\n");
  } else {
    
  }
#endif /* !SDC_DATA_DESTRUCTIVE_TEST */
  
  
  /**
   * Now perform some FS tests.
   */
  
  DWORD clusters=0;
  FATFS *fsp=NULL;
  FIL FileObject;
  uint32_t bytes_written=0;
  uint32_t bytes_read=0;
  FILINFO fno ;
#if _USE_LFN
    char lfn[_MAX_LFN + 1];
    fno.lfname = lfn;
    fno.lfsize = sizeof lfn;
#endif

  const uint8_t teststring[] = {"This is test file\r\n"} ;
  /* FS object.*/
  static FATFS SDC_FS;
  
  
   
  
#if SDC_DATA_DESTRUCTIVE_TEST 
  if (format) {
    chprintf(lchp, "Formatting... ");
    chThdSleepMilliseconds(100);
    
    chprintf(lchp, "Register working area for filesystem... ");
    chThdSleepMilliseconds(100);
#if _FATFS < 8000
   err = f_mount(0, &SDC_FS);
#else
   err = f_mount(&SDC_FS, "", 0);
#endif

    if (err != FR_OK){
      goto error;
    }
    else{
      chprintf(lchp, "OK\r\n");
    }
  }
    
  if (format) {
    chprintf(lchp, "f_mkfs starting ... ");
    chThdSleepMilliseconds(100);
    err = f_mkfs (0,0,0);
    if (err != FR_OK){
      goto error;
    }  else {
      chprintf(lchp, "OK\r\n");
    }
  }
#endif /* SDC_DATA_DESTRUCTIVE_TEST */
    
    
  chprintf(lchp, "get free space on filesystem... ");
  chThdSleepMilliseconds(100);
  err =  f_getfree(NULL, &clusters, &fsp);
 
  if (err != FR_OK)
    goto error;
  
  chprintf(lchp, "OK\r\n");
  chprintf(lchp,
	   "FS: %lu free clusters, %lu sectors per cluster, %lu bytes free\r\n",
	   clusters, (uint32_t)SDC_FS.csize,
	   clusters * (uint32_t)SDC_FS.csize * (uint32_t)MMCSD_BLOCK_SIZE);
  
  
  chprintf(lchp, "Create file \"chtest.txt\"... ");
  chThdSleepMilliseconds(100);
  err = f_open(&FileObject, "chtest.txt", FA_WRITE | FA_OPEN_ALWAYS);
  if (err != FR_OK) {
    goto error;
  }
  chprintf(lchp, "OK\r\n");
  chprintf(lchp, "Write some data in it... ");
  chThdSleepMilliseconds(100);
  err = f_write(&FileObject, teststring, sizeof(teststring), (void *)&bytes_written);
  if (err != FR_OK) {
    goto error;
  }
  else
    chprintf(lchp, "OK\r\n");
  
  chprintf(lchp, "Close file \"chtest.txt\"... ");
  err = f_close(&FileObject);
  if (err != FR_OK) {
    goto error;
  }
  else
    chprintf(lchp, "OK\r\n");
  

  chprintf(lchp, "Check file size \"chtest.txt\"... ");
  chThdSleepMilliseconds(10);
  err = f_stat("chtest.txt", &fno);
  chThdSleepMilliseconds(100);
  if (err != FR_OK) {
    goto error;
  } else {
    if (fno.fsize == sizeof(teststring))
      chprintf(lchp, "OK\r\n");
    else
      goto error;
  }
  
  
  chprintf(lchp, "Check file content \"chtest.txt\"... ");
  err = f_open(&FileObject, "chtest.txt", FA_READ | FA_OPEN_EXISTING);
  chThdSleepMilliseconds(100);
  if (err != FR_OK) {
    goto error;
  }
  uint8_t buf[sizeof(teststring)];
  err = f_read(&FileObject, buf, sizeof(teststring), (void *)&bytes_read);
  if (err != FR_OK) {
    goto error;
  } else {
    if (memcmp(teststring, buf, sizeof(teststring)) != 0){
      goto error;
    } else {
       chprintf(lchp, "OK\r\n");
    }
  }
  
  {
    FILINFO lfno;
    DIR dir;
    char *fn;   /* This function is assuming non-Unicode cfg. */
#if _USE_LFN
    char llfn[_MAX_LFN + 1];
    lfno.lfname = llfn;
    lfno.lfsize = sizeof llfn;
#endif
    const char *path = "";
    FRESULT res =0;

    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK) {
      for (;;) {
	res = f_readdir(&dir, &lfno);                   /* Read a directory item */
	if (res != FR_OK || lfno.fname[0] == 0) break;  /* Break on error or end of dir */
	if (lfno.fname[0] == '.') continue;             /* Ignore dot entry */
#if _USE_LFN
	fn = lfno.lfname;
#else
	fn = lfno.fname;
#endif
	                                     /* It is a file. */
	chprintf(lchp, "readdir %s/%s\r\n", path, fn);
      }
    }
  }
  
  chprintf(lchp, "Umount filesystem... ");
#if _FATFS < 8000
   err = f_mount(0, NULL);
#else
   err = f_mount(NULL, "", 0);
#endif

  chprintf(lchp, "OK\r\n");
  
  chprintf(lchp, "Disconnecting from SDIO...");
  chThdSleepMilliseconds(100);
  if (!sdioDisconnect())
    goto error;
  chprintf(lchp, " OK\r\n");
  chprintf(lchp, "------------------------------------------------------\r\n");
  chprintf(lchp, "All tests passed successfully.\r\n");
  chThdSleepMilliseconds(100);
  

  sdioDisconnect();
  return;
  }
 error:
  chprintf(lchp, "SDC error [%d] occurs\r\n", err);
  sdioDisconnect();
}


/*
#                                    _          
#                                   | |         
#                  ___   _ __     __| |         
#                 / _ \ | '_ \   / _` |         
#                |  __/ | | | | | (_| |         
#                 \___| |_| |_|  \__,_|         
#         _______   ______   ______   _______   ______
#        |__   __| |  ____| /  ____| |__   __| /  ____|
#           | |    | |__    | (___      | |    | (___
#           | |    |  __|    \___ \     | |     \___ \
#           | |    | |____  .____) |    | |    .____) |
#           |_|    |______| \_____/     |_|    \_____/
*/




/*
#                  ___    _____    _____         
#                 / _ \  |  __ \  |_   _|        
#                | |_| | | |__) |   | |          
#                |  _  | |  ___/    | |          
#                | | | | | |       _| |_         
#                |_| |_| |_|      |_____|        
*/



bool sdioConnect (void) 
{
  if (!sdc_lld_is_card_inserted (NULL)) {
    DebugTrace("No sd card");
    return FALSE;
  }

  if (SDCD1.state == BLK_READY) {
    DebugTrace ("allready connected");
    return TRUE;
  }

  /*
   * Initializes the SDIO drivers.
   */
  const uint32_t mode = PAL_MODE_ALTERNATE(12) | PAL_STM32_OTYPE_PUSHPULL |
    PAL_STM32_OSPEED_HIGHEST | PAL_STM32_PUDR_FLOATING | PAL_STM32_MODE_ALTERNATE;

  palSetPadMode (GPIOC, GPIOC_SDIO_D0, mode | PAL_STM32_PUDR_PULLUP);
  palSetPadMode (GPIOC, GPIOC_SDIO_D1, mode | PAL_STM32_PUDR_PULLUP);
  palSetPadMode (GPIOC, GPIOC_SDIO_D2, mode | PAL_STM32_PUDR_PULLUP);
  palSetPadMode (GPIOC, GPIOC_SDIO_D3, mode | PAL_STM32_PUDR_PULLUP);
  palSetPadMode (GPIOC, GPIOC_SDIO_CK, mode);
  palSetPadMode (GPIOD, GPIOD_SDIO_CMD, mode | PAL_STM32_PUDR_PULLUP);
  // palSetPadMode (GPIOD, GPIOD_SDIO_CMD, mode);

  chThdSleepMilliseconds(100);


  sdcStart(&SDCD1, NULL);   
  while (sdcConnect(&SDCD1) != CH_SUCCESS) {
    chThdSleepMilliseconds(100);
    DebugTrace ("sdcConnect still not connected");
  }

  //  DebugTrace ("SDC START");
  
  return TRUE;
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
