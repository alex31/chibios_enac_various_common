#pragma once

#include <ch.h>
#include <hal.h>
#include <array>
#include <span>
#include <cstdint>
#include "eeprom_stm_m95p.h"
#include "stdutil.h"

/*
  TODO : integrer tout dans un firmware H7
  * callback du message getset
 */

namespace  Eeprom_M95 {
  template <typename T>
  concept SingleByteIntegralOrEnum = 
  (std::integral<T> && sizeof(T) == 1) || 
    (std::is_enum_v<T> && sizeof(std::underlying_type_t<T>) == 1);


  class Device {
  public:
    enum Status {
      WIP  = 1 << 0,
      WEL  = 1 << 1,
      BP0  = 1 << 2,
      BP1  = 1 << 3,
      BP2  = 1 << 4,
      TB   = 1 << 6,
      SRWD = 1 << 7,
    };
    
   enum Configuration {
      LID  = 1 << 0,
      DRV0 = 1 << 5,
      DRV1 = 1 << 6,
    };
    
   enum Safety {
      ECC3DS  = 1 << 0,
      ECC3D  = 1 << 1,
      ECC2C  = 1 << 2,
      ECC1C  = 1 << 3,
      PRF    = 1 << 4,
      ERF    = 1 << 5,
      PUF   = 1 << 6,
      PAMAF = 1 << 7,
    };

  enum Volatile {
      BUFLD  = 1 << 0,
      BUFEN  = 1 << 1
  };

  struct Identification {
    uint8_t manufacturer;
    uint8_t family;
    uint8_t density;
    uint8_t uidLength;
  };

    constexpr Device(SPIDriver& driver, uint16_t divider, ioline_t cs);
    constexpr Device(SPIDriver& driver, const SPIConfig &_cfg);
    constexpr bool isValid() const {return capacity != 0;}
    msg_t waitReady() {return buzyLoopWaitReady();}
    msg_t readStatus(Status& statusReg) const;
    msg_t readConfAndSafety(Configuration& conf, Safety& safety) const;
    msg_t softwareReset() const;
    msg_t clearSafety() const;
    msg_t checkSafety() const;
    msg_t readVolatile(Volatile& vol) const;
    msg_t writeVolatile(Volatile vol) const;
    msg_t writeStatusAndConf(Status statusReg, Configuration conf) const;
    msg_t read(int offset, std::span<uint8_t> readBuffer) const;
    msg_t eraseChip() const; // whole chip
    msg_t eraseBlock(int offset) const; // block : 64K bytes  
    msg_t eraseSector(int offset) const; // sector : 4K bytes
    msg_t erasePage(int offset) const; // page : 512 bytes
    msg_t erase(int offset, size_t size) const; // optimised for arbitrary size
    // max 512 bytes , page should have been previsously erased
    msg_t programPage(int offset, std::span<const uint8_t> tx) const; 
    // max 512 bytes , auto erase
    msg_t writePage(int offset, std::span<const uint8_t> tx) const; // max 512 bytes, auto erase
    Identification getIdentification() const {return identification;}

    msg_t program(int offset, std::span<const uint8_t> tx) const; 
    msg_t write(int offset, std::span<const uint8_t> tx) const; 
    M95P_instance_t c_obj() {
      return (M95P_instance_t) {.device = this};
    }

  private:
      enum  Command : std::uint8_t {
        WREN   = 0x06, // Write Enable, 1-byte command, no arguments
        WRDI   = 0x04, // Write Disable, 1-byte command, no arguments
        RDSR   = 0x05, // Read Status Register, 1-byte command, returns 1-byte status
        WRSR   = 0x01, // Write Status Register, 1-byte command, 1 or 2 data bytes

        READ   = 0x03, // Read Data (Single Output), 1-byte command + 3-byte address, returns data
        FREAD  = 0x0B, // Fast Read (Single Output), 1-byte command + 3-byte address + 1 dummy byte, returns data
        FDREAD = 0x3B, // Fast Read (Dual Output), 1-byte command + 3-byte address + 1 dummy byte, returns data
        FQREAD = 0x6B, // Fast Read (Quad Output), 1-byte command + 3-byte address + 1 dummy byte, returns data

        PGWR   = 0x02, // Page Write (Auto Erase + Program), 1-byte command + 3-byte address + up to 512 bytes of data
        PGPR   = 0x0A, // Page Program (Without Auto Erase), 1-byte command + 3-byte address + up to 512 bytes of data
        PGER   = 0xDB, // Page Erase (512 bytes), 1-byte command + 3-byte address, no data
        SCER   = 0x20, // Sector Erase (4 KB), 1-byte command + 3-byte address, no data
        BKER   = 0xD8, // Block Erase (64 KB), 1-byte command + 3-byte address, no data
        CHER   = 0xC7, // Chip Erase, 1-byte command, no arguments

        RDID   = 0x83, // Read Identification (EEPROM), 1-byte command + 3-byte address, returns up to 1024 bytes
        FRDID  = 0x8B, // Fast Read Identification (EEPROM), 1-byte command + 3-byte address + 1 dummy byte, returns data
        WRID   = 0x82, // Write Identification Page, 1-byte command + 3-byte address + up to 512 bytes of data

        DPD    = 0xB9, // Deep Power-Down Enter, 1-byte command, no arguments
        RDPD   = 0xAB, // Deep Power-Down Release, 1-byte command, no arguments

        JEDID  = 0x9F, // JEDEC Identification, 1-byte command, returns 3 bytes (manufacturer, family, density)

        RDCR   = 0x15, // Read Configuration & Safety Register, 1-byte command, returns 2 bytes
        RDVR   = 0x85, // Read Volatile Register, 1-byte command, returns 1 byte
        WRVR   = 0x81, // Write Volatile Register, 1-byte command + 1 byte of data

        CLRSF  = 0x50, // Clear Safety Sticky Flags, 1-byte command, no arguments
        RDSFDP = 0x5A, // Read SFDP Register, 1-byte command + 3-byte address + 1 dummy byte, returns up to 512 bytes

        RSTEN  = 0x66, // Enable Reset, 1-byte command, no arguments
        SWRST  = 0x99  // Software Reset, 1-byte command, no arguments
    };


    static constexpr std::array<uint8_t, 4> makeCmd(Command cmd, uint32_t addr);
    static constexpr int getDividerExponent(uint16_t divider);
    static constexpr bool isValidBuffer(std::span<const uint8_t> v);

    // with 100Mhz clock source : 
    // 50Mhz spi is SPI_CFG1_MBR_DIV2
    // with external wires, 3.12Mhz is div32
    static constexpr SPIConfig cfgTemplate {
      .circular         = false,
	.slave          = false,
	.data_cb        = nullptr,
	.error_cb       = [](hal_spi_driver*) {chSysHalt("spi cb error");},
	//  .end_cb           = nullptr,
	.ssline         = 0,
	.cfg1           = SPI_CFG1_MBR_DIV32 | SPI_CFG1_DSIZE_VALUE(7),
	.cfg2           = 0 // SPI_CFG2_CPOL=0 SPI_CFG2_CPHA=0
	};

    static constexpr size_t BLOCK_SIZE = 64 * 1024;
    static constexpr size_t SECTOR_SIZE = 4 * 1024;
    static constexpr size_t PAGE_SIZE = 512;
    static constexpr int nullOffset = -1;
    static constexpr std::array<uint8_t, 0> nullBufferArray = {};
    static constexpr std::span<const uint8_t> nullBuffer = {};

    static const std::array<uint8_t, 2> readStatusCmd; 
    static std::array<uint8_t, 4> spicmd;
    static std::array<uint8_t, 2> dmaRamBuffer2b;
    static std::array<uint8_t, 1> dmaRamBuffer1b;


    template <SingleByteIntegralOrEnum T, SingleByteIntegralOrEnum R,
              std::size_t TxSize, std::size_t RxSize>
    msg_t spiTransaction(Command command, int32_t offset,
			 const std::array<T, TxSize> &tx, std::array<R, RxSize> &rx) const {
      return spiTransaction(command, offset,
			    std::span<const uint8_t>(tx), std::span<uint8_t>(rx));
    }
    
    template <SingleByteIntegralOrEnum T, std::size_t TxSize>
    msg_t spiTransaction(Command command, int32_t offset,
			 const std::array<T, TxSize> &tx) const {
      return spiTransaction(command, offset,
			    std::span<const uint8_t>(tx));
    }
    
    template <SingleByteIntegralOrEnum T = const uint8_t, SingleByteIntegralOrEnum R = uint8_t>
    msg_t spiTransaction(Command command, int32_t offset,
			 const std::span<T> tx, std::span<R> rx) const;
    
    template <SingleByteIntegralOrEnum T = const uint8_t>
    msg_t spiTransaction(Command command, int32_t offset,
			 const std::span<T> tx) const
    {
      static  std::array<uint8_t, 0> nullBufferArrayMutable = {};
      static  std::span<uint8_t> nullBufferMutable = nullBufferArrayMutable;
      return spiTransaction(command, offset, tx, nullBufferMutable);
    }
    
    msg_t spiTransaction(Command command, int32_t offset = nullOffset) const
    {
      return spiTransaction(command, offset, nullBuffer);
    }

    msg_t readIdentification(); 
    msg_t getStatus(Status &eepromStatus) const;
    size_t getCapacity() const {return capacity;} ;
    // maximum buzy time should be 100 for all chip erase
    msg_t buzyLoopWaitReady(systime_t timout = TIME_MS2I(100)) const;
    msg_t writeEnable() const;
    void  spiBusPrologue() const;
    void  spiBusEpilogue() const;
    
  SPIDriver& spid;
  SPIConfig  cfg;
  Identification identification = {};
  size_t   capacity = 0;
  bool       manage_bus;
  };


  constexpr int Device::getDividerExponent(uint16_t divider)
  {
    // Check if divider is a power of 2
    const bool valid = 
      (divider >= 2) && (divider <= 256) &&
      (divider & (divider - 1)) == 0;
       if not consteval {
	   chDbgAssert(valid,
		       "Error: Divider is not a power of 2 in the range [2 .. 256]");
	 } 
    
    // Compute log2 using std::countr_zero (fast hardware instruction)
       return valid ? std::countr_zero(divider) : -1;
  }

  constexpr bool Device::isValidBuffer(std::span<const uint8_t> v)
  {
    return not (v.data() == nullBuffer.data() && v.size() == nullBuffer.size());
  }

  constexpr std::array<uint8_t, 4> Device::makeCmd(Command cmd, uint32_t addr)
  {
    return {
      static_cast<uint8_t>(cmd),               // Ensure `cmd` is treated as uint8_t
      static_cast<uint8_t>((addr >> 16) & 0xFF), // Extract the upper byte
      static_cast<uint8_t>((addr >> 8) & 0xFF),  // Extract the middle byte
      static_cast<uint8_t>(addr & 0xFF)         // Extract the lowest byte
    };
  }
  
  constexpr Device::Device(SPIDriver& driver, uint16_t divider,
			   ioline_t cs) :
    spid(driver), cfg(cfgTemplate), manage_bus(true)
  {
    cfg.ssline = cs;
    int divexp = getDividerExponent(divider);
    cfg.cfg1 |= SPI_CFG1_MBR_VALUE(divexp-1);
    softwareReset();
    readIdentification();
  }

constexpr Device::Device(SPIDriver& driver, const SPIConfig &_cfg) :
  spid(driver), cfg(_cfg), manage_bus(false)
{
  softwareReset();
  readIdentification();
}

}
