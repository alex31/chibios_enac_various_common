#include "sdLiteLog.hpp"

namespace {
  constexpr size_t mbChunkSize = 16U;
  msg_t mbChunkBuffer[mbChunkSize];
}

MAILBOX_DECL(SdLiteLogBase::mbChunk, mbChunkBuffer, mbChunkSize);


//
//
IN_DMA_SECTION(FATFS    SdLiteLogBase::fatfs);
size_t   SdLiteLogBase::nbBytesWritten = 0U;
thread_t *SdLiteLogBase::workerThdPtr = nullptr;
uint32_t SdLiteLogBase::freeSpaceInKo=0;


SdLiteLogBase::SdLiteLogBase(uint32_t syncPeriodSeconds)
{
  syncPeriod = TIME_S2I(syncPeriodSeconds);
  syncTs = chVTGetSystemTimeX();
}

SdLiteLogBase::~SdLiteLogBase(void)
{
   closeLog();
   instances.erase(std::remove(instances.begin(), instances.end(), this), instances.end());
}

void SdLiteLogBase::terminate(const TerminateBehavior tb)
{
  if (workerThdPtr != nullptr) {
    chThdTerminate(workerThdPtr);
    if (tb == TerminateBehavior::WAIT)
      chThdWait(workerThdPtr);
    workerThdPtr = nullptr;
  }
}

void SdLiteLogBase::workerThd([[maybe_unused]] void* opt) {
  msg_t msg{};
  UINT bw;
  while (!chThdShouldTerminateX()) {
    if (chMBFetchTimeout(&SdLiteLogBase::mbChunk, &msg,
			 TIME_MS2I(100)) == MSG_OK) {
      const SdChunk& sdChunk = *(reinterpret_cast<SdChunk *>(msg));
      const auto [s, l] = sdChunk.getView().get();
      FIL * const fil = sdChunk.getFil();
      f_write(fil, s, l, &bw);
      sdChunk.signalSem();
      SdLiteLogBase::nbBytesWritten += bw;
      if (sdChunk.needSync()) {
	f_sync(fil);
      }
      if (bw != l) {
	DebugTrace("f_write length error %u != %u", bw, l);
      } 
    }
  }
  closeAllLogs();
  chThdExit(0);
}

SdLiteStatus SdLiteLogBase::openLog(const char* prefix, const char* directoryName)
{
  char fileName[32];
  if ((status = initOnce(&freeSpaceInKo));
      (status != SdLiteStatus::OK) && (status != SdLiteStatus::WAS_LAUNCHED))
    return status;
  
  status = getFileName(prefix, directoryName, fileName, sizeof(fileName), +1);
  if (status != SdLiteStatus::OK) {
    return status;
  }

  const FRESULT rc = f_open(&fil, fileName, FA_WRITE | FA_CREATE_ALWAYS);
  if (rc) {
    status = SdLiteStatus::FATFS_ERROR;
  } else {
    status = SdLiteStatus::OK;
    instances.push_back(this);
  }
  return status;
}

SdLiteStatus SdLiteLogBase::closeLog(void)
{
  DebugTrace("closeLog");
  const FRESULT rc = f_close(&fil);
  if (rc) 
    status = SdLiteStatus::FATFS_ERROR;
  else
    status = SdLiteStatus::OK;
  
  DebugTrace("close FIL %p", &fil);
  return status;
}

SdLiteStatus SdLiteLogBase::closeAllLogs(void)
{
  SdLiteStatus status = SdLiteStatus::OK;
  for (auto &obj : instances) {
    if (const SdLiteStatus nstatus = obj->closeLog(); nstatus != SdLiteStatus::OK)
      status = nstatus;
  }
  
  return status;
}

void SdLiteLogBase::flushAllLogs(void)
{
  for (auto &obj : instances)
    obj->flushHalfBuffer();
}

SdLiteStatus  SdLiteLogBase::getFileName(const char* prefix,
					const char* directoryName,
					char* nextFileName,
					const size_t nameLength,
					const int indexOffset)
{
   DIR dir; /* Directory object */
   FRESULT rc; /* Result code */
   FILINFO fno; /* File information object */
   size_t fileIndex = 0 ;
   size_t maxCurrentIndex = 0;

   const size_t directoryNameLen = std::min(strlen(directoryName), 126U);
   const size_t slashDirNameLen = directoryNameLen+2;
   char slashDirName[slashDirNameLen];
   strlcpy(slashDirName, "/", slashDirNameLen);
   strlcat(slashDirName, directoryName, slashDirNameLen);
   
   rc = f_opendir(&dir, directoryName);
   if (rc != FR_OK) {
     rc = f_mkdir(slashDirName);
     if (rc != FR_OK) {
       return SdLiteStatus::FATFS_ERROR;
     }
     rc = f_opendir(&dir, directoryName);
     if (rc != FR_OK) {
       return SdLiteStatus::FATFS_ERROR;
     }
   }
   
   for (;;) {
     rc = f_readdir(&dir, &fno); /* Read a directory item */
     if (rc != FR_OK || fno.fname[0] ==  0) break; /* Error or end of dir */
     
     
     if (fno.fname[0] == '.') continue;
     
     if (!(fno.fattrib & AM_DIR)) {
       //      DebugTrace ("fno.fsize=%d  fn=%s\n", fno.fsize, fn);
       fileIndex = uiGetIndexOfLogFile(prefix, fno.fname);
       maxCurrentIndex = std::max(maxCurrentIndex, fileIndex);
     }
   }
   if (rc) {
     return SdLiteStatus::FATFS_ERROR;
   }
   
   rc = f_closedir (&dir);
   if (rc) {
     return SdLiteStatus::FATFS_ERROR;
   }
   
   if (maxCurrentIndex < numberMax) {
     snprintf(nextFileName, nameLength, "%s\\%s%.04d.LOG",
	      directoryName, prefix, maxCurrentIndex+indexOffset);
     return SdLiteStatus::OK;
   } else {
     snprintf(nextFileName, nameLength, "%s\\%s.ERR",
		 directoryName, prefix);
     return SdLiteStatus::LOGNUM_ERROR;
   }
}




SdLiteStatus SdLiteLogBase::initOnce(uint32_t* freeSpaceInKo)
{
   DWORD clusters=0;
   FATFS *fsp=nullptr;

   // if init is already done, return 
   if (workerThdPtr != nullptr) {
     return  SdLiteStatus::WAS_LAUNCHED;
   }
  if  (!sdc_lld_is_card_inserted (nullptr))
    return  SdLiteStatus::NOCARD;

  if (not sdioIsConnected()) {
    if (sdioConnect() == FALSE)
      return  SdLiteStatus::NOCARD;

    FRESULT rc = f_mount(&fatfs, "/", 1);
  
    if (rc != FR_OK) {
      return SdLiteStatus::FATFS_ERROR;
    }
  }
  
  if (freeSpaceInKo != nullptr) {
    f_getfree("/", &clusters, &fsp);
    *freeSpaceInKo = clusters * (uint32_t)fatfs.csize / 2;
  }

  workerThdPtr =  chThdCreateFromHeap(nullptr, 2560U, "sdLiteLogThd",
				      NORMALPRIO+2,
				      &workerThd, nullptr);
  return workerThdPtr ? SdLiteStatus::OK : SdLiteStatus::OUT_OF_RAM;
}


int32_t SdLiteLogBase::uiGetIndexOfLogFile(const char* prefix,
					  const char* fileName)
{
  const size_t len = strlen(prefix);

  // if filename does not began with prefix, return 0
  if (strncmp(prefix, fileName, len) != 0)
    return 0;

  // we point on the first char after prefix
  const char* suffix = &(fileName[len]);

  // we test that suffix is valid (at least begin with digit)
    if (!isdigit((int) suffix[0])) {
      //      DebugTrace ("DBG> suffix = %s", suffix);
      return 0;
    }

  return (int32_t) atoi (suffix);
}

etl::vector<SdLiteLogBase *, 16> SdLiteLogBase::instances;
