#include "eeprom_stm_m95p.hpp"
#include "stdutil++.hpp"
#include <cstring>

namespace  {
  constexpr bool crossesPageBoundary(uint32_t offset, std::size_t length) {
    constexpr uint32_t PAGE_SIZE = 512;
    
    // Offset within the current page (faster than modulo)
    uint32_t pageOffset = offset & (PAGE_SIZE - 1);
    
    // Check if writing crosses the boundary
    return (pageOffset + length) > PAGE_SIZE;
  }

  bool isValidDmaRam(const void *addr) {
    if ((uint32_t)addr < 0x20000000)
      return true;
    if ((uint32_t)addr >= 0x30040000)
      return true;
    return false;
  }

 }
#define EXEC_AND_TEST(code) if (auto status = (code); status != MSG_OK) return status;

  namespace  Eeprom_M95 {
     
 
    template <SingleByteIntegralOrEnum T, SingleByteIntegralOrEnum R>
    msg_t Device::spiTransaction(Command command, int32_t offset,
				 const std::span<T> tx, std::span<R> rx) const
    {
      msg_t status = MSG_OK;
      chDbgAssert(not(isValidBuffer(tx) && isValidBuffer(rx)),
		  "supply tx OR rx buffer, not both");
      spiBusPrologue();
      
      ScopeGuard guard([this] {
	spiBusEpilogue();
      });
      
      if (offset == nullOffset) {
	spicmd[0] = command;
	status = spiSend(&spid, sizeof(spicmd[0]), &spicmd[0]);
      } else {
	spicmd = makeCmd(command, offset);
	status = spiSend(&spid, spicmd.size(), spicmd.data());
      }
      
      if (tx.size() > 0) {
	chDbgAssert(isValidDmaRam(tx.data()), "not DMA memory");
	//	DebugTrace("send %u byte @ 0x%lx", tx.size(), (uint32_t)tx.data());
        status = spiSend(&spid, tx.size(), tx.data());
      } else if (rx.size() > 0) {
	//	DebugTrace("rec %u byte @ 0x%lx", rx.size(), (uint32_t)rx.data());
	chDbgAssert(isValidDmaRam(rx.data()), "not DMA memory");
        status = spiReceive(&spid, rx.size(), rx.data());
      }
      
      return status;
    }

    msg_t Device::getStatus(Status &eepromStatus) const
    {
      spiBusPrologue();
      msg_t status = spiExchange(&spid, readStatusCmd.size(), readStatusCmd.data(), 
				 dmaRamBuffer2b.data());
      eepromStatus = static_cast<Status>(dmaRamBuffer2b[1]);
      spiBusEpilogue();
      return status;
    }
    
    msg_t Device::buzyLoopWaitReady(systime_t timout) const
    {
      Status eepromStatus;
      systime_t beginTs = chVTGetSystemTimeX();
      do {
	if (msg_t status = getStatus(eepromStatus); status != MSG_OK) {
	  return status;
	}
	chThdSleep(1);
      } while ((eepromStatus & Status::WIP) &&
	       (chTimeDiffX(beginTs, chVTGetSystemTimeX()) < timout));
      //      chDbgAssert((eepromStatus & Status::WIP) == 0, "buzy loop timeout");
      return eepromStatus & Status::WIP ? MSG_TIMEOUT : MSG_OK;
    }

    msg_t Device::writeEnable() const
    {
      msg_t status;
      if ((status = buzyLoopWaitReady()) == MSG_OK) {
	status = spiTransaction(Command::WREN);
      }

      return status;
    }

    void  Device::spiBusPrologue() const
    {
      if (manage_bus) {
	spiAcquireBus(&spid);
	spiStart(&spid, &cfg);
      }
      spiSelect(&spid);
    }
    
    void  Device::spiBusEpilogue() const
    {
      spiUnselect(&spid);
      if (manage_bus) {
	spiStop(&spid);
	spiReleaseBus(&spid);
      }
    }
    
    msg_t Device::readStatus(Status& statusReg) const
    {
      msg_t status = spiTransaction(Command::RDSR, nullOffset, nullBufferArray, dmaRamBuffer1b);
      statusReg = static_cast<Status>(dmaRamBuffer1b[0]);
      return status;
    }
    
    msg_t Device::readConfAndSafety(Configuration& conf, Safety& safety) const
    {
      msg_t status = spiTransaction(Command::RDCR, nullOffset, nullBufferArray, dmaRamBuffer2b);
      conf = static_cast<Configuration>(dmaRamBuffer2b[0]);
      safety = static_cast<Safety>(dmaRamBuffer2b[1]);
      return status;
    }
    
    msg_t Device::clearSafety() const
    {
      return spiTransaction(Command::CLRSF);
    }

    msg_t Device::softwareReset() const
    {
      spiTransaction(Command::RSTEN);
      const msg_t status = spiTransaction(Command::SWRST);
      chThdSleepMilliseconds(30);
      return status;
    }

    msg_t Device::checkSafety() const
    {
#if CH_DBG_ENABLE_ASSERTS
      Configuration conf;
      Safety safety;
      buzyLoopWaitReady();
      const msg_t status = readConfAndSafety(conf, safety);
      chDbgAssert(safety == 0, "eeprom hardware error");
      return status;
#else
      return MSG_OK;
#endif
    }
    
    msg_t Device::readVolatile(Volatile& vol) const
    {
      msg_t status = spiTransaction(Command::RDVR, nullOffset, nullBufferArray, dmaRamBuffer1b);
      vol = static_cast<Volatile>(dmaRamBuffer1b[0]);
      return status;
    }

    msg_t Device::writeVolatile(Volatile vol) const
    {
      dmaRamBuffer1b[0] = vol;
      EXEC_AND_TEST(writeEnable());
      msg_t status = spiTransaction(Command::WRVR, nullOffset, dmaRamBuffer1b);
      return status;
    }
    
    msg_t Device::writeStatusAndConf(Status statusReg, Configuration conf) const
    {
      dmaRamBuffer2b = {statusReg, conf};
      EXEC_AND_TEST(writeEnable());
      return spiTransaction(Command::WRSR, nullOffset, dmaRamBuffer2b);
    }

    msg_t Device::read(int offset, std::span<uint8_t> readBuffer) const
    {
      buzyLoopWaitReady();
      return spiTransaction(Command::READ, offset, nullBuffer, readBuffer);
    }

    msg_t Device::readIdentification()
    {
      constexpr uint32_t mega = 1024 * 1024;
      memset(&spicmd, 0, spicmd.size());
      auto status = spiTransaction(Command::RDID, 0, nullBufferArray, spicmd);
      if (status == MSG_OK) {
	memcpy(&identification, &spicmd, sizeof(Identification));
	capacity = 1U << identification.density;
	const size_t capacityM = capacity / mega;
	if ((identification.manufacturer != 0x20) || (identification.family != 0)
	    || (capacityM < 1 ) || (capacityM > 4)) {
	  memset(&identification, 0, sizeof(Identification));
	  capacity = 0;
	}
      }
      return status;
    }

    
    msg_t Device::eraseChip() const
    {
      EXEC_AND_TEST(writeEnable());
      const msg_t status =  spiTransaction(Command::CHER);
      checkSafety();
      return status;
    }

    msg_t Device::eraseBlock(int offset) const
    {
      EXEC_AND_TEST(writeEnable());
      const msg_t status =  spiTransaction(Command::BKER, offset);
      checkSafety();
      return status;
    }

    msg_t Device::eraseSector(int offset) const
    {
      EXEC_AND_TEST(writeEnable());
      const msg_t status =  spiTransaction(Command::SCER, offset);
      checkSafety();
      return status;
   }

    msg_t Device::erasePage(int offset) const
    {
      EXEC_AND_TEST(writeEnable());
      const msg_t status =  spiTransaction(Command::PGER, offset);
      checkSafety();
      return status;
   }
    
    // optimised for arbitrary size and alignments
    msg_t Device::erase(int offset, size_t size) const
    {
      chDbgAssert((offset + size) <= capacity, "Device::erase overflow capacity");
      while (size != 0) {
        if ((offset % BLOCK_SIZE == 0) && size >= BLOCK_SIZE) {
	  EXEC_AND_TEST(eraseBlock(offset));
	  offset += BLOCK_SIZE;
	  size -= BLOCK_SIZE;
        } else if ((offset % SECTOR_SIZE == 0) && size >= SECTOR_SIZE) {
	  EXEC_AND_TEST(eraseSector(offset));
	  offset += SECTOR_SIZE;
	  size -= SECTOR_SIZE;
        } else if ((offset % PAGE_SIZE == 0) && size >= PAGE_SIZE) {
	  EXEC_AND_TEST(erasePage(offset));
	  offset += PAGE_SIZE;
	  size -= PAGE_SIZE;
        } else {
#if CH_DBG_ENABLE_ASSERTS
	  chSysHalt("Device::erase size or/and alignment violation");
#endif
	  return MSG_RESET; // Instead of chSysHalt, return error
        }
      }
      return MSG_OK;
    }


    msg_t Device::programPage(int offset, std::span<const uint8_t> tx) const
    {
      EXEC_AND_TEST(writeEnable());
      chDbgAssert(not crossesPageBoundary(offset, tx.size()),
	"cannot cross 512 bytes boundary page");
      const msg_t status = spiTransaction(Command::PGPR, offset, tx);
      checkSafety();
      return status;
    }

    msg_t Device::writePage(int offset, std::span<const uint8_t> tx) const
    {
      EXEC_AND_TEST(writeEnable());
      chDbgAssert(not crossesPageBoundary(offset, tx.size()),
	"cannot cross 512 bytes boundary page");
      const msg_t status = spiTransaction(Command::PGWR, offset, tx);
      checkSafety();
      return status;
   }

 msg_t Device::program(int offset, std::span<const uint8_t> tx) const
 {
   chDbgAssert((offset + tx.size()) <= capacity, "Device::program overflow capacity");
   while (!tx.empty()) {
     // Compute the maximum writable size within the page
     const int pageRemaining = PAGE_SIZE - (offset % PAGE_SIZE);
     const int writeSize = std::min<int>(tx.size(), pageRemaining);

     // Ensure that we're not crossing a page boundary
     chDbgAssert(not crossesPageBoundary(offset, writeSize),
		 "writePage: Attempting to cross page boundary");

     // Program the current page segment
     EXEC_AND_TEST(programPage(offset, tx.first(writeSize)));
     
     // Move to the next chunk
     offset += writeSize;
     tx = tx.subspan(writeSize);
   }

    return MSG_OK;
}

 msg_t Device::write(int offset, std::span<const uint8_t> tx) const
 {
   chDbgAssert((offset + tx.size()) <= capacity, "Device::program overflow capacity");
   while (!tx.empty()) {
     // Compute the maximum writable size within the page
     const int pageRemaining = PAGE_SIZE - (offset % PAGE_SIZE);
     const int writeSize = std::min<int>(tx.size(), pageRemaining);

     // Ensure that we're not crossing a page boundary
     chDbgAssert(not crossesPageBoundary(offset, writeSize),
		 "writePage: Attempting to cross page boundary");

     // Write the current page segment
     EXEC_AND_TEST(writePage(offset, tx.first(writeSize)));
     
     // Move to the next chunk
     offset += writeSize;
     tx = tx.subspan(writeSize);
   }

    return MSG_OK;
}


    const std::array<uint8_t, 2> Device::readStatusCmd = {Command::RDSR, 0};
    __attribute__ ((section(DMA_SECTION)))
    std::array<uint8_t, 4> Device::spicmd = {};
    __attribute__ ((section(DMA_SECTION)))
    std::array<uint8_t, 2> Device::dmaRamBuffer2b = {};
     __attribute__ ((section(DMA_SECTION)))
    std::array<uint8_t, 1> Device::dmaRamBuffer1b = {};
    
    
  } // namespace

#ifdef M95P_RAM_EMULATION
static  M95P_instance_t device = {};
static constexpr   MP95_Identification identification= {
  .manufacturer = 0x20,
  .family = 0,
  .density = 0x11,
  .uidLength = 0
};
static std::array<uint8_t, 1 << identification.density> eepromSim = {};

extern "C" {
  M95P_instance_t M95P_new(SPIDriver *, const SPIConfig *)
  {
    memset(eepromSim.data(), 0xff, eepromSim.size());
    return device;
  }
  
  msg_t M95P_read(M95P_instance_t, uint32_t offset, uint32_t n, uint8_t *buffer)
  {
    chDbgAssert((offset + n) <= eepromSim.size(), "read out of bound");
    memcpy(buffer, &eepromSim[offset], n);
    return MSG_OK;
  }

  msg_t M95P_readStatus(const M95P_instance_t, M95P_status_t* status)
  {
    *status = (M95P_status_t) 0;
    return MSG_OK;
  }

  msg_t M95P_readIdentification(const M95P_instance_t,
		  MP95_Identification *ident)
  {
    *ident = identification;
    return MSG_OK;
  }

  msg_t M95P_softwareReset(const M95P_instance_t)
  {
     return MSG_OK;
  }

  msg_t M95P_program(M95P_instance_t, uint32_t offset, uint32_t n, const uint8_t *buffer)
  {
    chDbgAssert((offset + n) <= eepromSim.size(), "program out of bound");
    for (size_t i=0; i<n; i++) {
      eepromSim[offset+i] &= buffer[i];
    }
    return MSG_OK;
  }
  
  msg_t M95P_write(M95P_instance_t, uint32_t offset, uint32_t n, const uint8_t *buffer)
  {
    chDbgAssert((offset + n) <= eepromSim.size(), "program out of bound");
    memcpy(&eepromSim[offset], buffer, n);
    return MSG_OK;
  }
  
  msg_t M95P_erase(M95P_instance_t, uint32_t offset, uint32_t n)
  {
    chDbgAssert((offset + n) <= eepromSim.size(), "erase out of bound");
    memset(&eepromSim[offset], 0xff, n);
    return MSG_OK;
  }
  
  msg_t M95P_erase_chip(M95P_instance_t)
  {
    return M95P_erase(device, 0, 1 << identification.density);
    return MSG_OK;
  }

  msg_t M95P_erase_block(M95P_instance_t, uint32_t offset)
  {
    return M95P_erase(device, offset, 1<<16);
    return MSG_OK;
  }

  msg_t M95P_erase_sector(M95P_instance_t, uint32_t offset)
  {
    return M95P_erase(device, offset, 4096);
    return MSG_OK;
  }

  msg_t M95P_erase_page(M95P_instance_t, uint32_t offset)
  {
    return M95P_erase(device, offset, 512);
    return MSG_OK;
  }

} // extern "C"
#else
extern "C" {
  M95P_instance_t M95P_new(SPIDriver *spid, const SPIConfig *config)
  {
    return (M95P_instance_t) {.device = new Eeprom_M95::Device(*spid, *config)};
  }
  
  msg_t M95P_read(M95P_instance_t self, uint32_t offset, uint32_t n, uint8_t *buffer)
  {
    const Eeprom_M95::Device *device = static_cast<Eeprom_M95::Device *>(self.device);
    std::span spanBuffer(buffer, n);
    return device->read(offset, spanBuffer);
  }

  msg_t M95P_readStatus(const M95P_instance_t self, M95P_status_t* status)
  {
    Eeprom_M95::Device::Status st;
    const Eeprom_M95::Device *device = static_cast<Eeprom_M95::Device *>(self.device);
    msg_t busStatus = device->readStatus(st);
    *status = std::bit_cast<M95P_status_t>(st);
    return busStatus;
  }

  msg_t M95P_readIdentification(const M95P_instance_t self,
		  MP95_Identification *ident)
  {
    const Eeprom_M95::Device *device = static_cast<Eeprom_M95::Device *>(self.device);
    Eeprom_M95::Device::Identification _ident = device->getIdentification();
    memcpy(ident, &_ident, sizeof(MP95_Identification));
    return MSG_OK;
  }
  
  msg_t M95P_softwareReset(const M95P_instance_t self)
  {
    const Eeprom_M95::Device *device = static_cast<Eeprom_M95::Device *>(self.device);
    return device->softwareReset();
  }

 msg_t M95P_program(M95P_instance_t self, uint32_t offset, uint32_t n, const uint8_t *buffer)
  {
    const Eeprom_M95::Device *device = static_cast<Eeprom_M95::Device *>(self.device);
    std::span spanBuffer(buffer, n);
    return device->program(offset, spanBuffer);
  }
  
 msg_t M95P_write(M95P_instance_t self, uint32_t offset, uint32_t n, const uint8_t *buffer)
  {
    const Eeprom_M95::Device *device = static_cast<Eeprom_M95::Device *>(self.device);
    std::span spanBuffer(buffer, n);
    return device->write(offset, spanBuffer);
  }
  
  msg_t M95P_erase(M95P_instance_t self, uint32_t offset, uint32_t n)
  {
    const Eeprom_M95::Device *device = static_cast<Eeprom_M95::Device *>(self.device);
    return device->erase(offset, n);
  }
  
  msg_t M95P_erase_chip(M95P_instance_t self)
  {
    const Eeprom_M95::Device *device = static_cast<Eeprom_M95::Device *>(self.device);
    return device->eraseChip();
  }

  msg_t M95P_erase_block(M95P_instance_t self, uint32_t offset)
  {
    const Eeprom_M95::Device *device = static_cast<Eeprom_M95::Device *>(self.device);
    return device->eraseBlock(offset);
  }

  msg_t M95P_erase_sector(M95P_instance_t self, uint32_t offset)
  {
    const Eeprom_M95::Device *device = static_cast<Eeprom_M95::Device *>(self.device);
    return device->eraseSector(offset);
  }

  msg_t M95P_erase_page(M95P_instance_t self, uint32_t offset)
  {
    const Eeprom_M95::Device *device = static_cast<Eeprom_M95::Device *>(self.device);
    return device->erasePage(offset);
  }
} // extern "C"
#endif
