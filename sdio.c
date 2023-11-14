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
#include "sdio.h"


#ifndef FFCONF_DEF
#define  FFCONF_DEF _FATFS
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
static uint8_t outbuf[MMCSD_BLOCK_SIZE * SDC_BURST_SIZE + 4] __attribute__ ((section(SDMMC_SECTION), aligned(32)));
static uint8_t inbuf[MMCSD_BLOCK_SIZE * SDC_BURST_SIZE + 4]__attribute__ ((section(SDMMC_SECTION), aligned(32)));

static void fatfsTest(bool format);
static size_t findFirstDiff(const uint8_t *p1, const uint8_t* p2, size_t len);


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

void fillbuffers(uint8_t pattern){
  memset(inbuf, pattern, sizeof(inbuf));
  memset(outbuf, pattern, sizeof(inbuf));
}


/**
 *
 */
void cmd_sdiotestOld(BaseSequentialStream *lchp, int argc,const char * const argv[]) {
  (void)argc;
  (void)argv;
  uint32_t i = 0;
  bool format = FALSE;
  static const char *mode[] = {"SDV11", "SDV20", "MMC", NULL};
  bool err = false;
  
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
    chprintf(chp, "OK\r\n\r\nCard Info\r\n");
    chprintf(chp, "CSD      : %08lX %8lX %08lX %08lX \r\n",
	     SDCD1.csd[3], SDCD1.csd[2], SDCD1.csd[1], SDCD1.csd[0]);
    chprintf(chp, "CID      : %08lX %8lX %08lX %08lX \r\n",
	     SDCD1.cid[3], SDCD1.cid[2], SDCD1.cid[1], SDCD1.cid[0]);
    chprintf(chp, "Mode     : %s\r\n", mode[SDCD1.cardmode & 3U]);
    chprintf(chp, "Capacity : %ld MB\r\n", SDCD1.capacity / 2048);
    
    chprintf(lchp, "Single aligned read...");
    chThdSleepMilliseconds(100);
    if (sdcRead(&SDCD1, 0, inbuf, 1))
      goto error;
    chprintf(lchp, " OK\r\n");
    chThdSleepMilliseconds(100);
    
    
#if   STM32_SDC_SDMMC_UNALIGNED_SUPPORT
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
#endif
  
  chprintf(lchp, "Multiple aligned reads...");
  chThdSleepMilliseconds(100);
  fillbuffers(42);
  /* fill reference buffer from SD card */
  if ((err = sdcRead(&SDCD1, 0, inbuf, MMCSD_BLOCK_SIZE)))
    goto error;
  for (i=0; i<1000; i++){
    if (! (i % 100)) {
      chprintf(lchp, "%lu ... ", i);
    }
     chprintf(lchp, "\r\n\r\n");
		      
    if ((err = sdcRead(&SDCD1, 0, outbuf, MMCSD_BLOCK_SIZE)))
      goto error;
    if (memcmp(inbuf, outbuf, MMCSD_BLOCK_SIZE) != 0) {
      err = 129;
      for (size_t j=0; j<MMCSD_BLOCK_SIZE; j++) {
	if ( inbuf[j] != outbuf[j])
	  chprintf(lchp, "[%u] = {%u, %u} ", j, inbuf[j], outbuf[j]);
      }
      chprintf(lchp, "\r\n\r\n");
      goto error;
    }
  }
  chprintf(lchp, " OK\r\n");
  chThdSleepMilliseconds(100);
  
 #if   STM32_SDC_SDMMC_UNALIGNED_SUPPORT 
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
#endif
  
#if SDC_DATA_DESTRUCTIVE_TEST 
  if (format) {
    
    chprintf(lchp, "Single aligned write...");
    chThdSleepMilliseconds(100);
    memset(inbuf, 0xAA, sizeof(inbuf));
    if (sdcWrite(&SDCD1, 0, inbuf, 1))
      goto error;
    memset(outbuf, 0x0, sizeof(outbuf));
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
  
  
 
  error:
  chprintf(lchp, "SDC error [%d] occurs\r\n", err);
  sdioDisconnect();
  }
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

bool sdioIsCardResponding(void)
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
    DebugTrace("No sd card");
    return FALSE;
  }

  if (SDCD1.state == BLK_READY) {
    return TRUE;
  }

   sdcStart(&SDCD1, &sdc1bitCfg);
  uint32_t count = 10;
  while ((sdcConnect(&SDCD1) != OSAL_SUCCESS) && count--) {
    chThdSleepMilliseconds(100);
    DebugTrace ("sdcConnect still not connected");
  }

  //  DebugTrace ("SDC START");
  
  return count != 0;
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

void cmd_sdiotest(BaseSequentialStream *lchp, int ,const char * const *)
{
  static const char *mode[] = {"SDV11", "SDV20", "MMC", NULL};
  systime_t start, end;
  uint32_t n, startblk;
  fillbuffers(0x0);


  /* Card presence check.*/
  if (!blkIsInserted(&SDCD1)) {
    chprintf(lchp, "Card not inserted, aborting.\r\n");
    return;
  }

  /* Connection to the card.*/
  if (!sdioConnect()) {
     chprintf(lchp, "sdioConnect failed, aborting\r\n");
     return;
  }
    

  chprintf(lchp, "OK\r\n\r\nCard Info\r\n");
  chprintf(lchp, "CSD      : %08lx %8lx %08lx %08lx \r\n",
           SDCD1.csd[3], SDCD1.csd[2], SDCD1.csd[1], SDCD1.csd[0]);
  chprintf(lchp, "CID      : %08lx %8lx %08lx %08lx \r\n",
           SDCD1.cid[3], SDCD1.cid[2], SDCD1.cid[1], SDCD1.cid[0]);
  chprintf(lchp, "Mode     : %s\r\n", mode[SDCD1.cardmode & 3U]);
  chprintf(lchp, "Capacity : %ldMB\r\n", SDCD1.capacity / 2048);

  /* The test is performed in the middle of the flash area.*/
  startblk = (SDCD1.capacity / MMCSD_BLOCK_SIZE) / 2;


  /* Single block read performance, aligned.*/
  chprintf(lchp, "Single block aligned read performance:           ");
  start = chVTGetSystemTime();
  end = chTimeAddX(start, TIME_MS2I(1000));
  n = 0;
  if (blkRead(&SDCD1, startblk, outbuf, 1)) {
    chprintf(lchp, "failed\r\n");
    goto exittest;
  }
  do {
    if (blkRead(&SDCD1, startblk, inbuf, 1)) {
      chprintf(lchp, "failed\r\n");
      goto exittest;
    }
    if (memcmp(inbuf, outbuf, sizeof(inbuf)) != 0) {
      const uint32_t ferr = findFirstDiff(inbuf, outbuf, sizeof(inbuf));
      chprintf(lchp, "error cmp single block read @iter %lu @index %lu\r\n", n, ferr);
      chprintf(lchp, "I[%lu]=%u, O[%lu]=%u\r\n", ferr, inbuf[ferr], ferr, outbuf[ferr]);
      break;
    }
    n++;
  } while (chVTIsSystemTimeWithin(start, end));
  chprintf(lchp, "%ld blocks/S, %ld bytes/S\r\n", n, n * MMCSD_BLOCK_SIZE);
  
  /* Multiple sequential blocks read performance, aligned.*/
  chprintf(lchp, "16 sequential blocks aligned read performance:   ");
  start = chVTGetSystemTime();
  end = chTimeAddX(start, TIME_MS2I(1000));
  n = 0;
  if (blkRead(&SDCD1, startblk, outbuf, SDC_BURST_SIZE)) {
    chprintf(lchp, "failed\r\n");
    goto exittest;
  }

  do {
    if (blkRead(&SDCD1, startblk, inbuf, SDC_BURST_SIZE)) {
      chprintf(lchp, "failed\r\n");
      goto exittest;
    }
    if (memcmp(inbuf, outbuf, sizeof(inbuf)) != 0) {
      const uint32_t ferr = findFirstDiff(inbuf, outbuf, sizeof(inbuf));
      chprintf(lchp, "error cmp successive block read @iter %lu @index %lu\r\n", n, ferr);
      chprintf(lchp, "I[%lu]=%u, O[%lu]=%u\r\n", ferr, inbuf[ferr], ferr, outbuf[ferr]);
      break;
    }
    n += SDC_BURST_SIZE;
  } while (chVTIsSystemTimeWithin(start, end));
  chprintf(lchp, "%ld blocks/S, %ld bytes/S\r\n", n, n * MMCSD_BLOCK_SIZE);


#if STM32_SDC_SDMMC_UNALIGNED_SUPPORT
  /* Single block read performance, unaligned.*/
  chprintf(chp, "Single block unaligned read performance:         ");
  start = chVTGetSystemTime();
  end = chTimeAddX(start, TIME_MS2I(1000));
  n = 0;
  do {
    if (blkRead(&SDCD1, startblk, inbuf + 1, 1)) {
      chprintf(chp, "failed\r\n");
      goto exittest;
    }
    n++;
  } while (chVTIsSystemTimeWithin(start, end));
  chprintf(chp, "%ld blocks/S, %ld bytes/S\r\n", n, n * MMCSD_BLOCK_SIZE);
  
  /* Multiple sequential blocks read performance, unaligned.*/
  chprintf(chp, "16 sequential blocks unaligned read performance: ");
  start = chVTGetSystemTime();
  end = chTimeAddX(start, TIME_MS2I(1000));
  n = 0;
  do {
    if (blkRead(&SDCD1, startblk, inbuf + 1, SDC_BURST_SIZE)) {
      chprintf(chp, "failed\r\n");
        goto exittest;
    }
    n += SDC_BURST_SIZE;
  } while (chVTIsSystemTimeWithin(start, end));
  chprintf(chp, "%ld blocks/S, %ld bytes/S\r\n", n, n * MMCSD_BLOCK_SIZE);
#endif /* STM32_SDC_SDMMC_UNALIGNED_SUPPORT */
  




  
  fatfsTest(false);

 exittest:
  sdioDisconnect();
}

static size_t findFirstDiff(const uint8_t *p1, const uint8_t* p2, size_t len)
{
  size_t count = 0;
  while (count < len) {
    if (*p1++ != *p2++)
      break;
    else
      count++;
  }
  return count;
}


static void fatfsTest(bool format)
 {
   /**
    * Now perform some FS tests.
    */
   
   DWORD clusters=0;
   FIL FileObject;
   uint32_t bytes_written=0;
   uint32_t bytes_read=0;
   FILINFO fno ;
   BYTE work[FF_MAX_SS]; /* Work area (larger is better for processing time) */
   FRESULT err = 0;
   
   
   
   const uint8_t teststring[] = {"This is test file\r\n"} ;
   /* FS object.*/
   static FATFS SDC_FS __attribute__ ((section(SDMMC_SECTION), aligned(32)));
   FATFS *fsp=&SDC_FS;
   
   
   
#if SDC_DATA_DESTRUCTIVE_TEST 
   chprintf(chp, "Mounting... ");
   chThdSleepMilliseconds(100);
   
   chprintf(chp, "Register working area for filesystem... ");
   chThdSleepMilliseconds(100);
#if FFCONF_DEF < 8000
   err = f_mount(0, &SDC_FS);
#else
   err = f_mount(&SDC_FS, "", 0);
#endif
   
   if (err != FR_OK){
     goto error;
   }
   else{
     chprintf(chp, "OK\r\n");
   }
 
   
   if (format) {
     chprintf(chp, "f_mkfs starting ... ");
     chThdSleepMilliseconds(100);
     /* Create FAT volume */
#if FF_DEFINED == 87030
     err = f_mkfs("", FM_ANY, 0, work, sizeof work);
#else
     err = f_mkfs("", NULL, work, sizeof work);
#endif
     if (err != FR_OK){
       goto error;
     }  else {
       chprintf(chp, "OK\r\n");
     }
   }
#else /* SDC_DATA_DESTRUCTIVE_TEST */
   chprintf(chp, "Mounting... ");
   chThdSleepMilliseconds(100);
   err = f_mount(&SDC_FS, "/", 1);
   if (err != FR_OK){
     goto error;
   }
   else{
     chprintf(chp, "OK\r\n");
   }
   
#endif /* SDC_DATA_DESTRUCTIVE_TEST */
   
   
   chprintf(chp, "get free space on filesystem... ");
   chThdSleepMilliseconds(100);
   err =  f_getfree("/", &clusters, &fsp);
   
   if (err != FR_OK)
     goto error;
   
   chprintf(chp, "OK\r\n");
   chprintf(chp,
	    "FS: %lu free clusters, %lu sectors per cluster, %lu bytes free\r\n",
	    clusters, (uint32_t)SDC_FS.csize,
	    clusters * (uint32_t)SDC_FS.csize * (uint32_t)MMCSD_BLOCK_SIZE);
  
  
   chprintf(chp, "Create file \"chtest.txt\"... ");
   chThdSleepMilliseconds(100);
   err = f_open(&FileObject, "chtest.txt", FA_WRITE | FA_OPEN_ALWAYS);
   if (err != FR_OK) {
     goto error;
   }
   chprintf(chp, "OK\r\n");
   chprintf(chp, "Write some data in it... ");
   chThdSleepMilliseconds(100);
   err = f_write(&FileObject, teststring, sizeof(teststring), (void *)&bytes_written);
   if (err != FR_OK) {
     goto error;
   }
   else
     chprintf(chp, "OK\r\n");
  
   chprintf(chp, "Close file \"chtest.txt\"... ");
   err = f_close(&FileObject);
   if (err != FR_OK) {
     goto error;
   }
   else
     chprintf(chp, "OK\r\n");
  

   chprintf(chp, "Check file size \"chtest.txt\"... ");
   chThdSleepMilliseconds(10);
   err = f_stat("chtest.txt", &fno);
   chThdSleepMilliseconds(100);
   if (err != FR_OK) {
     goto error;
   } else {
     if (fno.fsize == sizeof(teststring))
       chprintf(chp, "OK\r\n");
     else
       goto error;
   }
  
  
   chprintf(chp, "Check file content \"chtest.txt\"... ");
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
       chprintf(chp, "OK\r\n");
     }
   }
  
   {
     FILINFO lfno;
     DIR dir;
     char *fn;   /* This function is assuming non-Unicode cfg. */

     const char *path = "";
     FRESULT res =0;

     res = f_opendir(&dir, path);                       /* Open the directory */
     if (res == FR_OK) {
       for (;;) {
	 res = f_readdir(&dir, &lfno);                   /* Read a directory item */
	 if (res != FR_OK || lfno.fname[0] == 0) break;  /* Break on error or end of dir */
	 if (lfno.fname[0] == '.') continue;             /* Ignore dot entry */
	 fn = lfno.fname;

	 /* It is a file. */
	 chprintf(chp, "readdir %s/%s\r\n", path, fn);
       }
     }
   }
  
   chprintf(chp, "Umount filesystem... ");
#if FFCONF_DEF < 8000
   err = f_mount(0, NULL);
#else
   err = f_mount(NULL, "", 0);
#endif

   chprintf(chp, "OK\r\n");
  
   chprintf(chp, "Disconnecting from SDIO...");
   chThdSleepMilliseconds(100);
   if (!sdioDisconnect())
     goto error;
   chprintf(chp, " OK\r\n");
   chprintf(chp, "------------------------------------------------------\r\n");
   chprintf(chp, "All tests passed successfully.\r\n");
   chThdSleepMilliseconds(100);
  

  error:
  chprintf(chp, "FATFS error [%d] occurs\r\n", err);
  sdioDisconnect();
 }
