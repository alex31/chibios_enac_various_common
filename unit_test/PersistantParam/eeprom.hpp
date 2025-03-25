#pragma once
#include <cstdint>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <cstring>
#include <cstdint>
#include <cassert>
#include <filesystem>

#include <filesystem>
#include <iostream>


namespace {
  constexpr const char* EEPROM_FILE = "eeprom_data.bin";
  constexpr size_t EEPROM_SIZE = 2048 * 1024;  // Simulated EEPROM total size
  constexpr size_t HEADER_SIZE = sizeof(uint32_t);  // 4-byte size metadata
}

/**
 * @brief Writes a record to EEPROM with a size header.
 * @param recordIndex Index where the record should be stored.
 * @param data Pointer to the data to write.
 * @param size Size of the data.
 */



 template<size_t RECORD_MAX_SIZE=256>
   bool eeprom_erase() {
   return std::filesystem::remove(EEPROM_FILE);
 }
 
 template<size_t RECORD_MAX_SIZE=256>
   bool eeprom_write(size_t recordIndex, const void *data, size_t size) {
  if (size > RECORD_MAX_SIZE)
    return false;
  recordIndex *= RECORD_MAX_SIZE;
  if (recordIndex + HEADER_SIZE + size > EEPROM_SIZE) {
    std::cerr << "EEPROM overflow! Write aborted.\n";
    return false;
  }

  std::fstream eeprom(EEPROM_FILE, std::ios::in | std::ios::out | std::ios::binary);
  if (!eeprom) {
    std::ofstream newFile(EEPROM_FILE, std::ios::binary | std::ios::trunc);
    newFile.seekp(EEPROM_SIZE - 1);
    newFile.write("", 1);  // Extend file
    newFile.close();
    eeprom.open(EEPROM_FILE, std::ios::in | std::ios::out | std::ios::binary);
  }

  // Write size header
  eeprom.seekp(recordIndex);
  uint32_t dataSize = static_cast<uint32_t>(size);
  eeprom.write(reinterpret_cast<char*>(&dataSize), HEADER_SIZE);

  // Write actual data
  eeprom.write(reinterpret_cast<const char*>(data), size);
  eeprom.close();
  return true;
}

/**
 * @brief Reads a record from EEPROM using its size header.
 * @param recordIndex Index where the record is stored.
 * @param data Pointer to buffer for reading data.
 * @param size In: buffer size, Out: actual size read.
 */
template<size_t RECORD_MAX_SIZE=256>
bool eeprom_read(size_t recordIndex, void *data, size_t& size) {
  recordIndex *= RECORD_MAX_SIZE;
  if (recordIndex + HEADER_SIZE > EEPROM_SIZE) {
    std::cerr << "Invalid record index.\n";
    size = 0;
    return false;
  }

  std::ifstream eeprom(EEPROM_FILE, std::ios::binary);
  if (!eeprom) {
    std::cerr << "EEPROM file not found!\n";
    size = 0;
    return false;
  }

  // Read size header
  eeprom.seekg(recordIndex);
  uint32_t dataSize = 0;
  eeprom.read(reinterpret_cast<char*>(&dataSize), HEADER_SIZE);

  // Validate data size
  if (dataSize > size || recordIndex + HEADER_SIZE + dataSize > EEPROM_SIZE) {
    std::cerr << "Buffer too small or corrupted data.\n";
    size = 0;
    return false;
  }

  // Read actual data
  eeprom.read(reinterpret_cast<char*>(data), dataSize);
  size = dataSize;  // Update actual read size
  eeprom.close();
  return true;
}

/**
 * @brief Finds the highest index where data is stored in EEPROM.
 * @return The highest record index where valid data is found. Returns -1 if empty.
 */
template<size_t RECORD_MAX_SIZE = 256>
size_t eeprom_getlen() {
    std::ifstream eeprom(EEPROM_FILE, std::ios::binary);
    if (!eeprom) {
        std::cerr << "EEPROM file not found!\n";
        return 0;
    }

    size_t highestIndex = 0;
    size_t recordIndex = 0;
    uint32_t dataSize = 0;

    while (recordIndex + HEADER_SIZE <= EEPROM_SIZE) {
        eeprom.seekg(recordIndex);
        eeprom.read(reinterpret_cast<char*>(&dataSize), HEADER_SIZE);

        if (dataSize > 0 && (recordIndex + HEADER_SIZE + dataSize) <= EEPROM_SIZE) {
            highestIndex = recordIndex / RECORD_MAX_SIZE;  // Update last valid index
        } else {
            break;  // Stop searching if we hit an empty record
        }

        recordIndex += RECORD_MAX_SIZE;  // Move to the next record
    }

    return highestIndex + 1;
}
