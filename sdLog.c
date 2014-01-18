#include <string.h>
#include <stdlib.h>
#include "sdLog.h"
#include "ch.h"
#include "hal.h"
#include "ff.h"
#include "printf.h"
#include "sdio.h"
#include "rtcAccess.h"

#define MIN(x , y)  (((x) < (y)) ? (x) : (y))
#define MAX(x , y)  (((x) > (y)) ? (x) : (y))


#ifndef  SDLOG_WRITE_BUFFER_SIZE
#error SDLOG_WRITE_BUFFER_SIZE should be defined in mcuconf.h
#endif

#ifndef SDLOG_MAX_MESSAGE_LEN 
#error  SDLOG_MAX_MESSAGE_LENshould be defined in mcuconf.h
#endif

#ifndef  SDLOG_QUEUE_SIZE
#error  SDLOG_QUEUE_SIZE should be defined in mcuconf.h
#endif


#ifndef SDLOG_QUEUE_BUCKETS 
#error  SDLOG_QUEUE_BUCKETS should be defined in mcuconf.h
#endif
 


#ifdef SDLOG_NEED_QUEUE
#include "varLengthMsgQ.h"
VARLEN_MSGQUEUE_DECL(static, TRUE, messagesQueue,  SDLOG_QUEUE_SIZE, SDLOG_QUEUE_BUCKETS, 
		     __attribute__ ((section(".ccmram"), aligned(8))));
static  uint8_t perfBuffer[SDLOG_WRITE_BUFFER_SIZE];

struct LogMessage {
  FIL *fileObject;
  char mess[0];
} ;
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
static msg_t thdSdLog(void *arg) ;
static Thread *sdLogThd = NULL;
#endif

static SdioError  getNextFileName(const char* prefix, const char* directoryName, 
				  char* nextFileName, const size_t nameLength);
static uint32_t uiGetIndexOfLogFile (const char* prefix, const char* fileName) ;

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

  FRESULT rc = f_mount(0, &fatfs);
  if (rc != FR_OK) {
    return SDLOG_FATFS_ERROR;
  }

  if (freeSpaceInKo != NULL) {
    f_getfree("/", &clusters, &fsp);
    *freeSpaceInKo = clusters * (uint32_t)fatfs.csize / 2;
  }

  return  SDLOG_OK ;
}


SdioError sdLogFinish (void)
{
  FRESULT rc = f_mount(0, NULL);
  if (rc != FR_OK) {
    return SDLOG_FATFS_ERROR;
  }

  // if we mount, unmount, don't disconnect sdio
  /* if (sdioDisconnect () == FALSE) */
  /*   return  SDLOG_NOCARD; */

  return  SDLOG_OK ;
}




SdioError sdLogOpenLog (FIL *fileObject, const char* directoryName, const char* prefix)
{
  FRESULT rc; /* fatfs result code */
  SdioError sde; /* sdio result code */
  //DIR dir; /* Directory object */
  //FILINFO fno; /* File information object */
  char fileName[32];

  sde = getNextFileName(prefix, directoryName, fileName, sizeof (fileName));
  if (sde != SDLOG_OK) {
    // sd card is not inserted, so logging task can be deleted
    return SDLOG_FATFS_ERROR;
  }


  rc = f_open(fileObject, fileName, FA_WRITE | FA_CREATE_ALWAYS);
  if (rc) {
    return SDLOG_FATFS_ERROR;
  }

  return SDLOG_OK;
}


SdioError sdLogCloseLog (FIL *fileObject)
{
  FRESULT rc; /* Result code */

  if (fileObject->fs == NULL)
    return SDLOG_FATFS_ERROR;

  rc = f_close(fileObject);
  if (rc) {
    return SDLOG_FATFS_ERROR;
  }

  return SDLOG_OK;
}


#ifdef SDLOG_NEED_QUEUE
SdioError sdLogWriteLog (FIL *fileObject, const char* fmt, ...)
{
  if (fileObject->fs == NULL)
    return SDLOG_FATFS_ERROR;

  va_list ap;
  va_start(ap, fmt);

  LogMessage *lm = alloca(LOG_MESSAGE_PREBUF_LEN);

  lm->fileObject = fileObject;
  chvsnprintf (lm->mess, SDLOG_MAX_MESSAGE_LEN-1,  fmt, ap);
  lm->mess[SDLOG_MAX_MESSAGE_LEN-1]=0;
  va_end(ap);

  if (varLenMsgQueuePush (&messagesQueue, lm, logMessageLen(lm), VarLenMsgQueue_REGULAR) < 0) {
    return SDLOG_QUEUEFULL;
  }

  return SDLOG_OK;
}

SdioError sdLogWriteRaw (FIL *fileObject, const uint8_t * buffer, const size_t len)
{
  if (fileObject->fs == NULL)
    return SDLOG_FATFS_ERROR;

  LogMessage *lm = alloca(LOG_MESSAGE_PREBUF_LEN);

  lm->fileObject = fileObject;
  memcpy (lm->mess, buffer, len);

  if (varLenMsgQueuePush (&messagesQueue, lm, logRawLen(len), VarLenMsgQueue_REGULAR) < 0) {
    return SDLOG_QUEUEFULL;
  }

  return SDLOG_OK;
}

SdioError sdLogWriteByte (FIL *fileObject, const uint8_t value)
{
  if (fileObject->fs == NULL)
    return SDLOG_FATFS_ERROR;

  LogMessage *lm = alloca(sizeof(LogMessage)+1);

  lm->fileObject = fileObject;
  lm->mess[0] = value;

  if (varLenMsgQueuePush (&messagesQueue, lm, sizeof(LogMessage)+1, VarLenMsgQueue_REGULAR) < 0) {
    return SDLOG_QUEUEFULL;
  }

  return SDLOG_OK;
}




/* enregistrer les fichiers ouverts de manière à les fermer
   si necessaire
   */
static WORKING_AREA(waThdSdLog, 2048);
SdioError sdLoglaunchThread (const bool_t binaryLog)
{
  chThdSleepMilliseconds(100);

  sdLogThd = chThdCreateStatic(waThdSdLog, sizeof(waThdSdLog),
			       NORMALPRIO, thdSdLog, (void *) binaryLog);
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

  static LogMessage lm;


  lm.fileObject = NULL;
  if (varLenMsgQueuePush (&messagesQueue, &lm, sizeof(LogMessage), VarLenMsgQueue_OUT_OF_BAND) < 0) {
    retVal= SDLOG_QUEUEFULL;
  }

  chThdTerminate (sdLogThd);
  chThdWait (sdLogThd);
  sdLogThd = NULL;
  return retVal;
}
#endif

/*
#                 _____           _                    _
#                |  __ \         (_)                  | |
#                | |__) |  _ __   _   __   __   __ _  | |_     ___
#                |  ___/  | '__| | |  \ \ / /  / _` | | __|   / _ \
#                | |      | |    | |   \ V /  | (_| | \ |_   |  __/
#                |_|      |_|    |_|    \_/    \__,_|  \__|   \___|
*/



static SdioError getNextFileName(const char* prefix, const char* directoryName, 
				 char* nextFileName, const size_t nameLength)
{
  DIR dir; /* Directory object */
  FRESULT rc; /* Result code */
  FILINFO fno; /* File information object */
  uint32_t fileIndex ;
  uint32_t maxCurrentIndex = 0;
  char *fn;   /* This function is assuming non-Unicode cfg. */
#if _USE_LFN
  char lfn[_MAX_LFN + 1];
  fno.lfname = lfn;
  fno.lfsize = sizeof lfn;
#endif
  const size_t directoryNameLen = strlen (directoryName);
  char slashDirName[directoryNameLen+2];
  strcpy (slashDirName, "/");
  strcat (slashDirName, directoryName);

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
      //vPrintMsg ("%8lu  %s\n", fno.fsize, fn);
      fileIndex = uiGetIndexOfLogFile (prefix, fn);
      maxCurrentIndex = MAX (maxCurrentIndex, fileIndex);
    }
  }
  if (rc) {
    return SDLOG_FATFS_ERROR;
  }

  chsnprintf (nextFileName, nameLength, "%s\\%s%.03d.LOG", 
	      directoryName, prefix, maxCurrentIndex+1);
  return SDLOG_OK;
}




uint32_t uiGetIndexOfLogFile (const char* prefix, const char* fileName)
{
  const size_t len = strlen(prefix);

  // if filename does not began with prefix, return 0
  if (strncmp (prefix, fileName, len))
    return 0;

  // we point on the first char after prefix
  const char* suffix = &(fileName[len]);


  return (uint32_t) atoi (suffix);
}


#ifdef SDLOG_NEED_QUEUE
static msg_t thdSdLog(void *arg)
{
  UINT bw;
  uint32_t curBufFill=0;
  FIL *foSaved = NULL;
  const bool_t appendCloseLogMsg = (bool_t) arg;

  /* LogMessage *lm = alloca(LOG_MESSAGE_PREBUF_LEN) */;

  chRegSetThreadName("thdSdLog");
  while (!chThdShouldTerminate()) {
    //     if ((retLen = varLenMsgQueuePop (&messagesQueue, lm, LOG_MESSAGE_PREBUF_LEN)) > 0) {
    const ChunkBufferRO cbro = varLenMsgQueuePopChunk (&messagesQueue);
    const int32_t retLen = cbro.blen;
    if (retLen > 0) {
      const LogMessage *lm = (LogMessage *) cbro.bptr;
      if (lm->fileObject == NULL) {
        if (foSaved) {
          if (curBufFill) {
            f_write(foSaved, perfBuffer, curBufFill, &bw);
          }
	  if (appendCloseLogMsg) {
	    f_write(foSaved, "\r\nEND_OF_LOG\r\n", 14, &bw);
	  }
          f_sync (foSaved);
        }
        chThdExit(SDLOG_OK);
        continue; /* To exit from thread when asked : chThdTerminate
                     then send special message with fileObject NULL to unblock thread
                     */
      }
      foSaved = lm->fileObject;
      // put end of string at end of message
      //lm->mess[retLen-sizeof(LogMessage)]=0;
      const uint32_t messLen = retLen-sizeof(LogMessage);
      if (messLen < (SDLOG_WRITE_BUFFER_SIZE-curBufFill)) {
        // the buffer can accept this message
        memcpy (&(perfBuffer[curBufFill]), lm->mess, messLen);
        curBufFill += messLen;
      } else {
        // fill the buffer
        const uint32_t stayLen = SDLOG_WRITE_BUFFER_SIZE-curBufFill;
        memcpy (&(perfBuffer[curBufFill]), lm->mess, stayLen);
        FRESULT rc = f_write(lm->fileObject, perfBuffer, SDLOG_WRITE_BUFFER_SIZE, &bw);
        f_sync (lm->fileObject);
        if (rc) {
          return SDLOG_FATFS_ERROR;
        } else if (bw != SDLOG_WRITE_BUFFER_SIZE) {
          return SDLOG_FSFULL;
        }

        memcpy (perfBuffer, &(lm->mess[stayLen]), messLen-stayLen);
        curBufFill=messLen-stayLen;
      }
      varLenMsgQueueFreeChunk (&messagesQueue, &cbro);
    } else {
      return SDLOG_INTERNAL_ERROR;
    }
  }

  return SDLOG_OK;
  }

static size_t logMessageLen (const LogMessage *lm)
{
  return sizeof(LogMessage) + strnlen (lm->mess, SDLOG_MAX_MESSAGE_LEN);
}

static size_t logRawLen (const size_t len)
{
  return sizeof(LogMessage) + len;
}

#endif
