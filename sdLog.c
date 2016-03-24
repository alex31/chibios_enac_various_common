#include <string.h>
#include <stdlib.h>
#include "sdLog.h"
#include "ch.h"
#include "hal.h"
#include "ff.h"
#include "printf.h"
#include "sdio.h"
#include "rtcAccess.h"
#include <ctype.h>
#include "stdutil.h"
#include "globalVar.h"

#define MIN(x , y)  (((x) < (y)) ? (x) : (y))
#define MAX(x , y)  (((x) > (y)) ? (x) : (y))


#if _FATFS < 8000
#if _FS_SHARE == 0
#define SDLOG_NUM_BUFFER 1
#else
#define SDLOG_NUM_BUFFER _FS_SHARE
#endif

#else // _FATFS > 8000
#if _FS_LOCK == 0
#define SDLOG_NUM_BUFFER 2
#else
#define SDLOG_NUM_BUFFER (_FS_LOCK/2)
#endif
#endif

#ifndef  SDLOG_ALL_BUFFERS_SIZE
#error SDLOG_ALL_BUFFERS_SIZE should be defined in mcuconf.h
#endif

#define SDLOG_WRITE_BUFFER_SIZE (SDLOG_ALL_BUFFERS_SIZE/SDLOG_NUM_BUFFER)

#ifndef SDLOG_MAX_MESSAGE_LEN
#error  SDLOG_MAX_MESSAGE_LENshould be defined in mcuconf.h
#endif

#ifndef  SDLOG_QUEUE_SIZE
#error  SDLOG_QUEUE_SIZE should be defined in mcuconf.h
#endif


#ifndef SDLOG_QUEUE_BUCKETS
#error  SDLOG_QUEUE_BUCKETS should be defined in mcuconf.h
#endif

#if _FS_REENTRANT == 0
#warning "_FS_REENTRANT = 0 in ffconf.h DO NOT open close file during log"
#endif


#ifdef SDLOG_NEED_QUEUE
#include "varLengthMsgQ.h"
VARLEN_MSGQUEUE_DECL(static, TRUE, messagesQueue,  SDLOG_QUEUE_SIZE, SDLOG_QUEUE_BUCKETS,
		     __attribute__((section(".ccmram"),aligned(8))));

struct FilePoolUnit {
  FIL   fil;
  bool  inUse;
  bool  tagAtClose;
};

static  struct FilePoolUnit fileDes[SDLOG_NUM_BUFFER] =
  {[0 ... SDLOG_NUM_BUFFER-1] = {.fil = {0}, .inUse = false, .tagAtClose=false}};

typedef enum {
  FCNTL_WRITE = 0b00,
  FCNTL_FLUSH = 0b01,
  FCNTL_CLOSE = 0b10,
  FCNTL_EXIT =  0b11
} FileFcntl;


typedef struct {
  uint8_t fcntl:2;
  uint8_t fd:6;
} FileOp;

struct LogMessage {
  FileOp op;
  char mess[0];
};

#define LOG_MESSAGE_PREBUF_LEN (SDLOG_MAX_MESSAGE_LEN+sizeof(LogMessage))
#endif


//static LogMessage logMessages[QUEUE_LENGTH] __attribute__ ((section(".ccmram"), aligned(8))) ;
//static msg_t _bufferQueue[QUEUE_LENGTH];
//static MAILBOX_DECL(xQueue, _bufferQueue, QUEUE_LENGTH);
//static MEMORYPOOL_DECL(memPool, sizeof(LogMessage), NULL);

static FATFS fatfs; /* File system object */

#ifdef SDLOG_NEED_QUEUE
static size_t logMessageLen (const LogMessage *lm);
static size_t logRawLen (const size_t len);
static SdioError sdLoglaunchThread (void);
static SdioError sdLogStopThread (void);
static Thread *sdLogThd = NULL;
static SdioError  getNextFIL (FileDes *fd);

#if (CH_KERNEL_MAJOR > 2)
static void thdSdLog(void *arg) ;
#else
static msg_t thdSdLog(void *arg) ;
#endif

#endif




static int32_t uiGetIndexOfLogFile (const char* prefix, const char* fileName) ;

SdioError sdLogInit (uint32_t* freeSpaceInKo)
{
  DWORD clusters=0;
  FATFS *fsp=NULL;

#ifdef SDLOG_NEED_QUEUE
  varLenMsgDynamicInit (&messagesQueue);
#endif

  if  (!sdc_lld_is_card_inserted (NULL))
    return  SDLOG_NOCARD;


  sdioConnect ();
  chThdSleepMilliseconds (10);
  sdioDisconnect ();

  if (sdioConnect () == FALSE)
    return  SDLOG_NOCARD;

#if _FATFS < 8000
  FRESULT rc = f_mount(0, &fatfs);
#else
  FRESULT rc = f_mount(&fatfs, "", 0);
#endif
  
  if (rc != FR_OK) {
    return SDLOG_FATFS_ERROR;
  }

  if (freeSpaceInKo != NULL) {
    f_getfree("/", &clusters, &fsp);
    *freeSpaceInKo = clusters * (uint32_t)fatfs.csize / 2;
  }

#ifdef SDLOG_NEED_QUEUE
  for (uint8_t i=0; i<SDLOG_NUM_BUFFER; i++) {
    fileDes[i].inUse = fileDes[i].tagAtClose = false;
  }

  return sdLoglaunchThread ();
#else
  return SDLOG_OK;
#endif

}


SdioError sdLogFinish (void)
{
#if _FATFS < 8000
  FRESULT rc = f_mount(0, NULL);
#else
  FRESULT rc = f_mount(NULL, "", 0);
#endif
  if (rc != FR_OK) {
    return SDLOG_FATFS_ERROR;
  }

  // if we mount, unmount, don't disconnect sdio
  /* if (sdioDisconnect () == FALSE) */
  /*   return  SDLOG_NOCARD; */

  return  SDLOG_OK ;
}



#ifdef SDLOG_NEED_QUEUE
SdioError sdLogOpenLog (FileDes *fd, const char* directoryName, const char* prefix,
			bool appendTagAtClose)
{
  FRESULT rc; /* fatfs result code */
  SdioError sde; /* sdio result code */
  //DIR dir; /* Directory object */
  //FILINFO fno; /* File information object */
  char fileName[32];

  sde = getNextFIL (fd);
  if (sde != SDLOG_OK) {
    return sde;
  }

  sde = getFileName(prefix, directoryName, fileName, sizeof (fileName), +1);
  if (sde != SDLOG_OK) {
    // sd card is not inserted, so logging task can be deleted
    return SDLOG_FATFS_ERROR;
  }


  rc = f_open(&fileDes[*fd].fil, fileName, FA_WRITE | FA_CREATE_ALWAYS);
  if (rc) {
    fileDes[*fd].inUse = false;
    return SDLOG_FATFS_ERROR;
  } else {
    fileDes[*fd].tagAtClose = appendTagAtClose;
  }

  return SDLOG_OK;
}


SdioError sdLogCloseAllLogs (bool flush)
{
  FRESULT rc = 0; /* Result code */



  //    do not flush what is in ram, close as soon as possible
  if (flush == false) {
    // stop worker thread then close file
    sdLogStopThread ();
    for (FileDes fd=0; fd<SDLOG_NUM_BUFFER; fd++) {
      if (fileDes[fd].inUse) {
	FIL *fileObject = &fileDes[fd].fil;

	FRESULT trc = f_close(fileObject);
	fileDes[fd].inUse = false;
	if (!rc)
	  rc = trc;
      }
    }

    if (rc) {
      return SDLOG_FATFS_ERROR;
    }

    // flush ram buffer then close
  } else { // flush == true
    if (sdLogThd == NULL) {
      // something goes wrong, log thread is no more working
      return SDLOG_NOTHREAD;
    }
    
    // queue flush + close order, then stop worker thread
    for (FileDes fd=0; fd<SDLOG_NUM_BUFFER; fd++) {
      if (fileDes[fd].inUse) {
	sdLogCloseLog (fd);
      }
    }

    LogMessage lm;
    lm.op.fcntl = FCNTL_EXIT;

    if (varLenMsgQueuePush (&messagesQueue, &lm, sizeof(lm), VarLenMsgQueue_REGULAR) < 0) {
      return SDLOG_QUEUEFULL;
    } else {
      chThdWait (sdLogThd);
      sdLogThd = NULL;
    }

  }
  return SDLOG_OK;
}



SdioError sdLogWriteLog (const FileDes fd, const char* fmt, ...)
{
  if ((fd >= SDLOG_NUM_BUFFER) || (fileDes[fd].inUse == false))
    return SDLOG_FATFS_ERROR;

  va_list ap;
  va_start(ap, fmt);

  LogMessage *lm = alloca (LOG_MESSAGE_PREBUF_LEN);

  lm->op.fcntl = FCNTL_WRITE;
  lm->op.fd = fd & 0x1f;

  chvsnprintf (lm->mess, SDLOG_MAX_MESSAGE_LEN-1,  fmt, ap);
  lm->mess[SDLOG_MAX_MESSAGE_LEN-1]=0;
  va_end(ap);

  if (varLenMsgQueuePush (&messagesQueue, lm, logMessageLen(lm), VarLenMsgQueue_REGULAR) < 0) {
    return SDLOG_QUEUEFULL;
  }

  return SDLOG_OK;
}

SdioError sdLogFlushLog (const FileDes fd)
{
  if ((fd >= SDLOG_NUM_BUFFER) || (fileDes[fd].inUse == false))
    return SDLOG_FATFS_ERROR;

  LogMessage lm;

  lm.op.fcntl = FCNTL_FLUSH;
  lm.op.fd = fd & 0x1f;

  if (varLenMsgQueuePush (&messagesQueue, &lm, sizeof(lm), VarLenMsgQueue_REGULAR) < 0) {
    return SDLOG_QUEUEFULL;
  }

  return SDLOG_OK;
}

SdioError sdLogCloseLog (const FileDes fd)
{
  if ((fd >= SDLOG_NUM_BUFFER) || (fileDes[fd].inUse == false))
    return SDLOG_FATFS_ERROR;

  LogMessage lm;

  lm.op.fcntl = FCNTL_CLOSE;
  lm.op.fd = fd & 0x1f;

  if (varLenMsgQueuePush (&messagesQueue, &lm, sizeof(lm), VarLenMsgQueue_REGULAR) < 0) {
    return SDLOG_QUEUEFULL;
  }

  return SDLOG_OK;
}





SdioError sdLogWriteRaw (const FileDes fd, const uint8_t * buffer, const size_t len)
{
  if ((fd >= SDLOG_NUM_BUFFER) || (fileDes[fd].inUse == false))
    return SDLOG_FATFS_ERROR;

  LogMessage *lm = alloca(LOG_MESSAGE_PREBUF_LEN);

  lm->op.fcntl = FCNTL_WRITE;
  lm->op.fd = fd & 0x1f;
  memcpy (lm->mess, buffer, len);

  if (varLenMsgQueuePush (&messagesQueue, lm, logRawLen(len), VarLenMsgQueue_REGULAR) < 0) {
    return SDLOG_QUEUEFULL;
  }

  return SDLOG_OK;
}


SdioError sdLogWriteByte (const FileDes fd, const uint8_t value)
{
  if ((fd >= SDLOG_NUM_BUFFER) || (fileDes[fd].inUse == false))
    return SDLOG_FATFS_ERROR;

  LogMessage *lm = alloca(sizeof(LogMessage)+1);

  lm->op.fcntl = FCNTL_WRITE;
  lm->op.fd = fd & 0x1f;
  lm->mess[0] = value;

  if (varLenMsgQueuePush (&messagesQueue, lm, sizeof(LogMessage)+1, VarLenMsgQueue_REGULAR) < 0) {
    return SDLOG_QUEUEFULL;
  }

  return SDLOG_OK;
}




/* enregistrer les fichiers ouverts de manière à les fermer
   si necessaire
   */
static WORKING_AREA(waThdSdLog, 1024);
SdioError sdLoglaunchThread ()
{
  chThdSleepMilliseconds(100);

  sdLogThd = chThdCreateStatic(waThdSdLog, sizeof(waThdSdLog),
			       NORMALPRIO+1, thdSdLog, NULL);
  if (sdLogThd == NULL)
    return SDLOG_INTERNAL_ERROR;
  else
    return SDLOG_OK;
}

SdioError sdLogStopThread (void)
{
  SdioError retVal=SDLOG_OK;

  if (sdLogThd == NULL)
    return SDLOG_NOTHREAD;

  LogMessage lm;

  // ask for closing (after flushing) all opened files
  for (uint8_t i=0; i<SDLOG_NUM_BUFFER; i++) {
    if (fileDes[i].inUse) {
      lm.op.fcntl = FCNTL_CLOSE;
      lm.op.fd = i & 0x1f;
      if (varLenMsgQueuePush (&messagesQueue, &lm, sizeof(LogMessage), VarLenMsgQueue_OUT_OF_BAND) < 0) {
	retVal= SDLOG_QUEUEFULL;
      }
    }
  }

  lm.op.fcntl = FCNTL_EXIT;
  if (varLenMsgQueuePush (&messagesQueue, &lm, sizeof(LogMessage), VarLenMsgQueue_OUT_OF_BAND) < 0) {
    retVal= SDLOG_QUEUEFULL;
  }

  chThdTerminate (sdLogThd);
  chThdWait (sdLogThd);
  sdLogThd = NULL;
  return retVal;
}
#endif


SdioError getFileName(const char* prefix, const char* directoryName,
			     char* nextFileName, const size_t nameLength, const int indexOffset)
{
  DIR dir; /* Directory object */
  FRESULT rc; /* Result code */
  FILINFO fno; /* File information object */
  int32_t fileIndex ;
  int32_t maxCurrentIndex = 0;
  char *fn;   /* This function is assuming non-Unicode cfg. */
#if _USE_LFN
  char lfn[_MAX_LFN + 1];
  fno.lfname = lfn;
  fno.lfsize = sizeof lfn;
#endif
  const size_t directoryNameLen = MIN(strlen (directoryName), 128);
  const size_t slashDirNameLen = directoryNameLen+2;
  char slashDirName[slashDirNameLen];
  strlcpy (slashDirName, "/", slashDirNameLen);
  strlcat (slashDirName, directoryName, slashDirNameLen);

  rc = f_opendir(&dir, directoryName);
  if (rc != FR_OK) {
    rc = f_mkdir(slashDirName);
    if (rc != FR_OK) {
      return SDLOG_FATFS_ERROR;
    }
    rc = f_opendir(&dir, directoryName);
    if (rc != FR_OK) {
      return SDLOG_FATFS_ERROR;
    }
  }

  for (;;) {
    rc = f_readdir(&dir, &fno); /* Read a directory item */
    if (rc != FR_OK || fno.fname[0] ==  0) break; /* Error or end of dir */
#if _USE_LFN
    fn = *fno.lfname ? fno.lfname : fno.fname;
#else
    fn = fno.fname;
#endif
    if (fn[0] == '.') continue;

    if (!(fno.fattrib & AM_DIR)) {
      //      DebugTrace ("fno.fsize=%d  fn=%s\n", fno.fsize, fn);
      fileIndex = uiGetIndexOfLogFile (prefix, fn);
      maxCurrentIndex = MAX (maxCurrentIndex, fileIndex);
    }
  }
  if (rc) {
    return SDLOG_FATFS_ERROR;
  }

  rc = f_closedir (&dir);
  if (rc) {
    return SDLOG_FATFS_ERROR;
  }
  
  if (maxCurrentIndex < NUMBERMAX) {
    chsnprintf (nextFileName, nameLength, NUMBERFMF,
		directoryName, prefix, maxCurrentIndex+indexOffset);
    return SDLOG_OK;
  } else {
    chsnprintf (nextFileName, nameLength, "%s\\%s%.ERR",
		directoryName, prefix);
    return SDLOG_LOGNUM_ERROR;
  }
}

SdioError removeEmptyLogs(const char* directoryName, const char* prefix, const size_t sizeConsideredEmpty)
{
  DIR dir; /* Directory object */
  FRESULT rc; /* Result code */
  FILINFO fno; /* File information object */
  char *fn;   /* This function is assuming non-Unicode cfg. */
#if _USE_LFN
  char lfn[_MAX_LFN + 1];
  fno.lfname = lfn;
  fno.lfsize = sizeof lfn;
#endif

  rc = f_opendir(&dir, directoryName);
  if (rc != FR_OK) {
    return SDLOG_FATFS_NOENT;
  }

  for (;;) {
    rc = f_readdir(&dir, &fno); /* Read a directory item */
    if (rc != FR_OK || fno.fname[0] ==  0) break; /* Error or end of dir */
#if _USE_LFN
    fn = *fno.lfname ? fno.lfname : fno.fname;
#else
    fn = fno.fname;
#endif
    if (fn[0] == '.') continue;

    if (!(fno.fattrib & AM_DIR)) {
      //      DebugTrace ("fno.fsize=%d  fn=%s\n", fno.fsize, fn);
      if ((strncmp (fn, prefix, strlen(prefix)) == 0) && (fno.fsize <= sizeConsideredEmpty)) {
	char absPathName[128];
	strlcpy (absPathName, directoryName, sizeof(absPathName));
	strlcat (absPathName, "/", sizeof(absPathName));
	strlcat (absPathName, fn, sizeof(absPathName));
	rc = f_unlink (absPathName);
	if (rc) 
	  break;
      }
    }
  }
  
  if (rc) {
    return SDLOG_FATFS_ERROR;
  }
  
  rc = f_closedir (&dir);
  if (rc) {
    return SDLOG_FATFS_ERROR;
  }

  return SDLOG_OK;
}

/*
#                 _____           _                    _
#                |  __ \         (_)                  | |
#                | |__) |  _ __   _   __   __   __ _  | |_     ___
#                |  ___/  | '__| | |  \ \ / /  / _` | | __|   / _ \
#                | |      | |    | |   \ V /  | (_| | \ |_   |  __/
#                |_|      |_|    |_|    \_/    \__,_|  \__|   \___|
*/







int32_t uiGetIndexOfLogFile (const char* prefix, const char* fileName)
{
  const size_t len = strlen(prefix);

  // if filename does not began with prefix, return 0
  if (strncmp (prefix, fileName, len) != 0)
    return 0;

  // we point on the first char after prefix
  const char* suffix = &(fileName[len]);

  // we test that suffix is valid (at least begin with digit)
    if (!isdigit ((int) suffix[0])) {
      DebugTrace ("DBG> suffix = %s", suffix);
      return 0;
    }

  return (int32_t) atoi (suffix);
}


#ifdef SDLOG_NEED_QUEUE
#if (CH_KERNEL_MAJOR > 2)
static void thdSdLog(void *arg) 
#else
static msg_t thdSdLog(void *arg) 
#endif
{
  (void) arg;
  struct PerfBuffer {
    uint8_t buffer[SDLOG_WRITE_BUFFER_SIZE];
    uint16_t size;
  } ;

  UINT bw;
  static struct PerfBuffer perfBuffers[SDLOG_NUM_BUFFER] =
    {[0 ... SDLOG_NUM_BUFFER-1] = {.buffer = {0}, .size = 0}};

  chRegSetThreadName("thdSdLog");
  while (!chThdShouldTerminate()) {
    ChunkBufferRO cbro;
    const int32_t retLen = ( int32_t) (varLenMsgQueuePopChunk (&messagesQueue, &cbro));
    if (retLen > 0) {
      const LogMessage *lm = (LogMessage *) cbro.bptr;
      FIL *fo =  &fileDes[lm->op.fd].fil;
      uint8_t * const perfBuffer = perfBuffers[lm->op.fd].buffer;

      switch (lm->op.fcntl) {

      case FCNTL_FLUSH:
      case FCNTL_CLOSE:
	{
	  const uint16_t curBufFill = perfBuffers[lm->op.fd].size;
	  if (fileDes[lm->op.fd].inUse) {
	    if (curBufFill) {
	      f_write(fo, perfBuffer, curBufFill, &bw);
	      perfBuffers[lm->op.fd].size = 0;
	    }
	    if (lm->op.fcntl ==  FCNTL_FLUSH) {
	      f_sync (fo);
	    } else { // close
	      if (fileDes[lm->op.fd].tagAtClose) {
		f_write(fo, "\r\nEND_OF_LOG\r\n", 14, &bw);
	      }
	      f_close (fo);
	    fileDes[lm->op.fd].inUse = false; // store that file is closed
	    }
	  }
	}
	break;

      case FCNTL_EXIT:
	chThdExit(SDLOG_OK);
	break; /* To exit from thread when asked : chThdTerminate
		  then send special message with FCNTL_EXIT   */


      case FCNTL_WRITE:
	{
	  const uint16_t curBufFill = perfBuffers[lm->op.fd].size;
	  if (fileDes[lm->op.fd].inUse) {
	    const int32_t messLen = retLen-(int32_t) (sizeof(LogMessage));
	    if (messLen < (SDLOG_WRITE_BUFFER_SIZE-curBufFill)) {
	      // the buffer can accept this message
	      memcpy (&(perfBuffer[curBufFill]), lm->mess, (size_t) (messLen));
	      perfBuffers[lm->op.fd].size = (uint16_t) ((perfBuffers[lm->op.fd].size)+messLen);
	    } else {
	      // fill the buffer
	      const int32_t stayLen = SDLOG_WRITE_BUFFER_SIZE-curBufFill;
	      memcpy (&(perfBuffer[curBufFill]), lm->mess, (size_t)(stayLen));
	      FRESULT rc = f_write(fo, perfBuffer, SDLOG_WRITE_BUFFER_SIZE, &bw);
	      f_sync (fo);
	      if (rc) {
		chThdExit (SDLOG_FATFS_ERROR);
	      } else if (bw != SDLOG_WRITE_BUFFER_SIZE) {
		chThdExit (SDLOG_FSFULL);
	      }
	      
	      memcpy (perfBuffer, &(lm->mess[stayLen]),  (uint32_t) (messLen-stayLen));
	      perfBuffers[lm->op.fd].size = (uint16_t) (messLen-stayLen); // curBufFill
	    }
	  }
	}
      }
      varLenMsgQueueFreeChunk (&messagesQueue, &cbro);
    } else {
      chThdExit(SDLOG_INTERNAL_ERROR);
    }
  }
#if (CH_KERNEL_MAJOR == 2)
  return SDLOG_OK;
#endif
}

static size_t logMessageLen (const LogMessage *lm)
{
  return sizeof(LogMessage) + strnlen (lm->mess, SDLOG_MAX_MESSAGE_LEN);
}

static size_t logRawLen (const size_t len)
{
  return sizeof(LogMessage) + len;
}

static SdioError  getNextFIL (FileDes *fd)
{
  // if there is a free slot in fileDes, use it
  // else, if all slots are buzy, maximum open files limit
  // is reach.
  for (FileDes i=0; i<SDLOG_NUM_BUFFER; i++) {
    if (fileDes[i].inUse ==  false) {
      *fd = i;
      fileDes[i].inUse = true;
      return SDLOG_OK;
    }
  }
  return SDLOG_FDFULL;
}

#endif
