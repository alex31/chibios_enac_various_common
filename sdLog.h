#pragma once
#include "ch.h"
#include "hal.h"

#include "ff.h"
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif


#if SDLOG_WRITE_BUFFER_SIZE == 0 || SDLOG_MAX_MESSAGE_LEN == 0 || \
    SDLOG_QUEUE_SIZE  == 0 || SDLOG_QUEUE_BUCKETS  == 0
#undef SDLOG_NEED_QUEUE
#else
#define SDLOG_NEED_QUEUE
#endif


#ifdef SDLOG_NEED_QUEUE
typedef struct LogMessage LogMessage;
#endif

typedef enum {
  SDLOG_OK,
  SDLOG_NOCARD,
  SDLOG_FATFS_ERROR,
  SDLOG_FSFULL,
  SDLOG_QUEUEFULL,
  SDLOG_NOTHREAD,
  SDLOG_INTERNAL_ERROR
} SdioError;

SdioError sdLogInit (uint32_t* freeSpaceInKo);
SdioError sdLogFinish (void);
SdioError sdLogOpenLog (FIL *fileObject, const char* directoryName, const char* fileName);
SdioError sdLogCloseLog (FIL *fileObject);

SdioError sdLogWriteLogDirect (FIL *fileObject, const char* fmt, ...);
SdioError sdLogWriteRawDirect (FIL *fileObject, const uint8_t* buffer, const size_t len);
SdioError sdLogWriteByteDirect (FIL *fileObject, const uint8_t value);

#ifdef SDLOG_NEED_QUEUE
SdioError sdLoglaunchThread (const bool_t binaryLog);
SdioError sdLogWriteLog (FIL *fileObject, const char* fmt, ...);
SdioError sdLogWriteRaw (FIL *fileObject, const uint8_t* buffer, const size_t len);
SdioError sdLogWriteByte (FIL *fileObject, const uint8_t value);
SdioError sdLogStopThread (void);
#endif


#ifdef __cplusplus
}
#endif


