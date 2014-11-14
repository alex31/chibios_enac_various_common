#pragma once
#include "ch.h"
#include "hal.h"

#include "ff.h"
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

  /*
    This module is highly coupled with fatfs, and mcuconf.h
    several MACRO should be defined before use

    FATFS (ffconf.h) :
    ° _FS_SHARE : number of simultaneously open file
    ° _FS_REENTRANT : If you need to open / close file during log, this should be set to 1 at
                       the expense of more used cam and cpu.
                    If you open all files prior to log data on them, it should be left to 0  

   MCUCONF.H (or any other header included before sdLog.h
   ° SDLOG_ALL_BUFFERS_SIZE : (in bytes) performance ram buffer size shared between all opened log file
   ° SDLOG_MAX_MESSAGE_LEN  : (in bytes) maximum length of a message
   ° SDLOG_QUEUE_SIZE       : (in bytes) size of the message queue
   ° SDLOG_QUEUE_BUCKETS    : number of entries in queue


   use of the api :
   sdLogInit (initialize peripheral,  verify sdCard availibility
   sdLoglaunchThread : launnch worker thread
   sdLogOpenLog : open file
   sdLogWriteXXX
   sdLogCloseLog
   sdLogFinish


   and asynchronous emergency close (power outage detection by example) :
   sdLogCloseAllLogs
   sdLogFinish

   */


#if SDLOG_ALL_BUFFERS_SIZE == 0 || SDLOG_MAX_MESSAGE_LEN == 0 || \
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
  SDLOG_FDFULL,
  SDLOG_QUEUEFULL,
  SDLOG_NOTHREAD,
  SDLOG_INTERNAL_ERROR,
} SdioError;


typedef uint8_t FileDes;

/**
 * @brief	initialise sdLog
 * @details	init sdio peripheral, verify sdCard is inserted, check and mount filesystem,
 *		launch worker thread
 * @param[in]	binaryLog : if true, does not append end marker at end of file when closing
 * @param[out]	freeSpaceInKo : if pointer in nonnull, return free space on filesystem
 */
SdioError sdLogInit (const bool_t binaryLog, uint32_t* freeSpaceInKo);


/**
 * @brief	unmount filesystem
 * @details	unmount filesystem, free sdio peripheral
 * @param[in]	binaryLog : if true, does not append end marker at end of file when closing
 * @param[out]	freeSpaceInKo : if pointer in nonnull, return free space on filesystem
 */
SdioError sdLogFinish (void);


/**
 * @brief	open new log file
 * @details	always open new file with numeric index
 * @param[out]	fileObject : file descriptor : small integer between 0 and _FS_REENTRANT-1
 * @param[in]	directoryName : name of directory just under ROOT, created if nonexistant
 * @param[in]	fileName : the name will be appended with 3 digits number
 */
SdioError sdLogOpenLog (FileDes *fileObject, const char* directoryName, const char* fileName);


/**
 * @brief	flush ram buffer associated with file to sdCard
 * @param[in]	fileObject : file descriptor returned by sdLogOpenLog
 */
SdioError sdLogFlushLog (const FileDes fileObject);


/**
 * @brief	flush ram buffer then close file.
 * @param[in]	fileObject : file descriptor returned by sdLogOpenLog
 */
SdioError sdLogCloseLog (const FileDes fileObject);

/**
 * @brief	close all opened logs then stop worker thread
 * @param[in]	flush : if true : flush all ram buffers before closing (take more time)
 *			if false : close imediatly files without flushing buffers,
 *			           more chance to keep filesystem integrity in case of
 *				   emergency close after power outage is detected
 */
SdioError sdLogCloseAllLogs (bool flush);


#ifdef SDLOG_NEED_QUEUE


/**
 * @brief	log text 
 * @param[in]	fileObject : file descriptor returned by sdLogOpenLog
 * @param[in]	fmt : format and args in printf convention
 */
SdioError sdLogWriteLog (const FileDes fileObject, const char* fmt, ...);


/**
 * @brief	log binary data 
 * @param[in]	fileObject : file descriptor returned by sdLogOpenLog
 * @param[in]	buffer : memory pointer on buffer
 * @param[in]	len : number of bytes to write
 */
SdioError sdLogWriteRaw (const FileDes fileObject, const uint8_t* buffer, const size_t len);


/**
 * @brief	log one byte of binary data 
 * @param[in]	fileObject : file descriptor returned by sdLogOpenLog
 * @param[in]	value : byte to log
 */
SdioError sdLogWriteByte (const FileDes fileObject, const uint8_t value);
#endif


#ifdef __cplusplus
}
#endif


