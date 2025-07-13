#pragma once

#include <concepts>
#include <type_traits>
#include <cstdint>
#include <span>
#include <array>
#include <etl/vector.h>
#include "ch.h"


/**
 * @file pprzLink.hpp
 * @brief PPRZLink and XBee API message parsing module.
 *
 * @mainpage PPRZLink/XBee Serial Protocol Parser
 *
 * @section overview Overview
 * This module implements a robust and efficient parser for serial data streams following the
 * PPRZLink protocol, optionally encapsulated with XBee API frames.
 * It is designed for resource-constrained embedded systems and supports both byte-by-byte and
 * DMA-style buffered input.
 *
 * @section features Features
 * - Robust state machine for streaming serial protocol decoding.
 * - Support for PPRZLink and XBee API encapsulation policies (switchable via template).
 * - Efficient buffer management using ETL containers for embedded environments.
 * - Callback-based message delivery and error reporting.
 * - CRC-16 and CRC-8 validation (Fletcher algorithm).
 * - Flexible feed interface: accepts single bytes or spans of bytes (DMA/IRQ-friendly).
 * - Compile-time customization via C++ concepts and templates.
 *
 * @section usage Usage
 * Instantiate a @c PprzDecoder with appropriate callbacks and policy. Feed incoming data
 * either byte-by-byte or in chunks (std::span). The decoder will automatically synchronize,
 * extract messages, validate CRC, and invoke callbacks.
 *
 * @section design Design Notes
 * - The parser is implemented as a state machine with explicit error recovery.
 * - Designed for portability and low memory overhead.
 * - Uses Embedded Template Library (ETL) vectors for static allocation and deterministic behavior.
 * - All public and private members are documented with Doxygen.
 *
 * @section performance Performance
 * - Parse at a rate of 13Mb/s on stm32f7@216MHz compiled with -OFast 
 * - Parse at a rate of 5Mb/s on stm32g4@170MHz compiled with -OFast 
 *
 * @author alexandre.bustico@enac.fr
 * @date 06/2025
 */
  

enum class PprzPolicy {PPRZ, XBEE_API};

enum class PprzLinkDecoderState {WAIT_FOR_SYNC, WAIT_FOR_LEN1, WAIT_FOR_LEN2,
				 PROCESSING_PAYLOAD, WAIT_FOR_CHECKSUM_1, WAIT_FOR_CHECKSUM_2};

using PprzPayload_t = etl::vector<uint8_t, 251>; 
using PprzMsg_t = etl::vector<uint8_t, PprzPayload_t::MAX_SIZE + 4>; 
using XbeeMsg_t = etl::vector<uint8_t, PprzPayload_t::MAX_SIZE + 9>; 


template <auto F>
concept SendMsgConcept = requires(PprzPolicy p, const std::span<uint8_t> m) { { F(p, m) }; };
template <auto F>
concept TrapErrorConcept = requires(uint32_t valid, uint32_t invalid) { { F(valid, invalid) }; };


enum class XbeeRole : uint8_t {Tx = 0x01, Rx = 0x81};
union XbeeHeader {
  /*
    0 XBEE_TX16 (0x01) / XBEE_RX16 (0x81)
    1 FRAME_ID (0)     / SRC_ID_MSB
    2 DEST_ID_MSB      / SRC_ID_LSB
    3 DEST_ID_LSB      / XBEE_RSSI
    4 TX16_OPTIONS (0) / RX16_OPTIONS
  */
  
  // Redéclaration explicite des constructeurs/opérateurs par défaut
  XbeeHeader() = default;
  XbeeHeader(const XbeeHeader&) = default;
  XbeeHeader(XbeeHeader&&) = default;
  XbeeHeader& operator=(const XbeeHeader&) = default;
  XbeeHeader& operator=(XbeeHeader&&) = default;
  ~XbeeHeader() = default;

  std::strong_ordering operator<=>(const XbeeHeader& other) const {
    int cmp = memcmp(this, &other, sizeof(XbeeHeader));
    if (cmp < 0) return std::strong_ordering::less;
    if (cmp > 0) return std::strong_ordering::greater;
    return std::strong_ordering::equal;
  }
  
  bool operator!=(const XbeeHeader& other) const {
    return (*this <=> other) != 0;
  }
  bool operator==(const XbeeHeader& other) const {
    return (*this <=> other) == 0;
  }
  
  template<typename T>
  requires (sizeof(T) == 1 && std::is_trivially_copyable_v<T>)
  XbeeHeader(std::span<const T> bytes)
  {
    this->bitCopy(bytes);
  }
  
  template<typename T>
  requires (sizeof(T) == 1 && std::is_trivially_copyable_v<T>)
  void bitCopy(std::span<const T> bytes)
  {
    chDbgAssert(bytes.size() >= sizeof(XbeeHeader), "Buffer too small for XbeeHeader");
    memcpy(this, bytes.data(), sizeof(XbeeHeader));
  }

  struct {
    XbeeRole  role; 
    union  {
      struct  {
	uint8_t  frameId;
	uint16_t destId_bigEndian;
      } __attribute((packed));
      
      struct  {
	uint16_t srcId_bigEndian;
	uint8_t  rssi;
      } __attribute((packed));
    } __attribute((packed));
    
    uint8_t  options;
  } __attribute((packed));

  uint8_t raw[5];
};
static_assert(sizeof(XbeeHeader) == 5);


/*
     C PPRZ_DATA
      0 SOURCE (~sender_ID)
      1 DESTINATION (can be a broadcast ID)
      2 CLASS/COMPONENT
        bits 0-3: 16 class ID
        bits 4-7: 16 component ID
      3 MSG_ID
      4 MSG_PAYLOAD
      . DATA (messages.xml)
*/
union PprzHeader {
  struct {
    uint8_t source;
    uint8_t destination;
    uint8_t classId:4; // low nibble
    uint8_t componentId:4; // high nibble
    uint8_t msgId;
  } __attribute__((packed));
   uint8_t raw[4];

  PprzHeader() = default;
  PprzHeader(const PprzHeader&) = default;
  PprzHeader(PprzHeader&&) = default;
  PprzHeader& operator=(const PprzHeader&) = default;
  PprzHeader& operator=(PprzHeader&&) = default;
  ~PprzHeader() = default;

  template<typename T>
  requires (sizeof(T) == 1 && std::is_trivially_copyable_v<T>)
  PprzHeader(std::span<const T> bytes)
  {
    this->bitCopy(bytes);
  }

  template<typename T>
  requires (sizeof(T) == 1 && std::is_trivially_copyable_v<T>)
  void bitCopy(std::span<const T> bytes)
  {
    chDbgAssert(bytes.size() >= sizeof(PprzHeader), "Buffer too small for PprzHeader");
    memcpy(this, bytes.data(), sizeof(PprzHeader));
  }

} __attribute__((packed));
static_assert(sizeof(PprzHeader) == 4);

constexpr auto default_sendmsg = [](PprzPolicy, const std::span<uint8_t>) {};
constexpr auto default_traperr = [](uint32_t, uint32_t) {};

constexpr unsigned long long operator""_ms(unsigned long long ms) {
    return ms;
}

namespace {
  /**
   * @brief Computes the CRC-16 of the provided buffer (Fletcher16).
   */
  uint16_t fletcher16(std::span<const uint8_t> data);
  
  /**
   * @brief Computes the CRC-8 of the provided buffer.
   */
  uint8_t chksum8(std::span<const uint8_t> data);
}

namespace PprzEncoder {
  /**
   * @brief Generates a PPRZLink message from the given payload.
   * @param payload Data to encapsulate
   * @return Complete PPRZLink message (header + payload + CRC)
   */
  PprzMsg_t genPprzMsg(const PprzPayload_t &payload);
  void genPprzMsg(PprzMsg_t& msg, const PprzHeader &ppHeader,
			 const std::span<const uint8_t> &payload);

  /**
   * @brief Generates an XBee API message from the given header and payload.
   * @param xbHeader XBee header (5 bytes)
   * @param payload  Payload to send
   * @return Complete XBee message (header + payload + CRC)
   */
  XbeeMsg_t genXbeeMsg(const XbeeHeader &xbHeader, const PprzPayload_t &payload);
  void genXbeeMsg(XbeeMsg_t& msg, const XbeeHeader &xbHeader,
		  const PprzHeader &ppHeader, const std::span<const uint8_t> &payload);
}

template<PprzPolicy P,
	 auto TIMEOUT_MS = 1000,
	 auto SENDMSG_f = default_sendmsg,
         auto TRAPERR_f = default_traperr>
requires SendMsgConcept<SENDMSG_f> && TrapErrorConcept<TRAPERR_f>
class PprzDecoder {
public:

  /**
   * @brief Feeds the parser with a buffer of bytes (from stream DMA/IRQ).
   * @param data Buffer of bytes to parse
   */
  void feed(std::span<const uint8_t> data);


  /**
   * @brief return number of valid and invalid parsed frames
   */
  std::pair<uint32_t, uint32_t> getStat() {return {valid, invalid};}

  
private:
  /**
   * @brief Buffer for accumulating the payload of the current message.
   */
  PprzPayload_t payload = {};

  /**
   * @brief Number of bytes remaining to read for the current payload.
   */
  size_t remaining = 0;

  /**
   * @brief CRC accumulator.
   */
  uint16_t chksum = 0;

  /**
   * @brief Counter of valid messages received.
   */
  uint32_t valid = 0;

  /**
   * @brief Counter of invalid/dropped messages (CRC error, framing, overflow, etc).
   */
  uint32_t invalid = 0;

  /**
   * @brief Current state of the parser state machine.
   */
  PprzLinkDecoderState state = PprzLinkDecoderState::WAIT_FOR_SYNC;

  /**
   * @brief Timestamp of the last received data.
   */
  systime_t last_activity = 0;
  
  /**
   * @brief Resets the parser to the initial state.
   */
  void reset() {
    state = PprzLinkDecoderState::WAIT_FOR_SYNC;
    payload.clear();
  }

  /**
   * @brief Checks for a timeout since the last received byte.
   * If a timeout occurs, it calls the error callback and resets the parser state.
   */
  void check_timeout() {
    if (state != PprzLinkDecoderState::WAIT_FOR_SYNC &&
        chVTTimeElapsedSinceX(last_activity) >= TIME_MS2I(TIMEOUT_MS)) {
      TRAPERR_f(valid, ++invalid);
      reset();
    }
    last_activity = chVTGetSystemTimeX();
  }
};



/**
 * @brief PPRZLink/XBee API frame decoder.
 *
 * This parser handles the segmentation and validation of PPRZLink messages (optionally with XBee API framing).
 *
 * @tparam P         Encapsulation policy (PPRZ, XBEE_API, etc.)
 * @tparam SENDMSG_f Callback called when a valid message is received.
 * @tparam TRAPERR_f Callback called on a parsing error.
 *
 * @details
 * The class implements a robust state machine to process a serial stream (byte-by-byte or by buffer).
 *
 * \par State machine transitions:
 * @dot
 * digraph G {
 *   WAIT_FOR_SYNC    -> WAIT_FOR_LEN        [label="SYNC received"]
 *   WAIT_FOR_LEN     -> PROCESSING_PAYLOAD  [label="LEN received"]
 *   PROCESSING_PAYLOAD -> WAIT_FOR_CHECKSUM_1 [label="Payload complete"]
 *   WAIT_FOR_CHECKSUM_1 -> WAIT_FOR_CHECKSUM_2 [label="CRC LSB received"]
 *   WAIT_FOR_CHECKSUM_2 -> WAIT_FOR_SYNC     [label="CRC OK"]
 *   WAIT_FOR_CHECKSUM_2 -> WAIT_FOR_SYNC     [label="CRC ERROR"]
 *   PROCESSING_PAYLOAD -> WAIT_FOR_SYNC      [label="Payload overflow/error"]
 * }
 * @enddot
 *
 * Any parsing error (invalid size, CRC mismatch, overflow) triggers a reset to WAIT_FOR_SYNC.
 */
template<PprzPolicy P, auto TIMEOUT_MS, auto SENDMSG_f, auto TRAPERR_f>
requires SendMsgConcept<SENDMSG_f> && TrapErrorConcept<TRAPERR_f>
void PprzDecoder<P, TIMEOUT_MS, SENDMSG_f, TRAPERR_f>::feed(std::span<const uint8_t> data)
{
  static constexpr uint8_t syncBeacon = P == PprzPolicy::PPRZ ? 0x99 : 0x7E;

  // if this bunch of bytes arrive long after previous one, it's not the end of a
  // previous message, but the beginning of a new one, so we reset the state machine not to miss
  // the start of message.
  check_timeout();
  
  while (!data.empty()) {
    switch (state) {
    case PprzLinkDecoderState::WAIT_FOR_SYNC :
      // DebugTrace ("WAIT_FOR_SYNC 0x%x", b);
      if (data.front() == syncBeacon) {
	state = PprzLinkDecoderState::WAIT_FOR_LEN1;
      }
      data = data.subspan(1);
      break;
      
    case PprzLinkDecoderState::WAIT_FOR_LEN1 : {
      // DebugTrace ("WAIT_FOR_LEN1");
      // the length is the length of the entire message
      // payload length is len - (start, len, chks1, chks2)
      const auto b = data.front();
      data = data.subspan(1);
      if constexpr (P == PprzPolicy::PPRZ) {
	remaining = b - 4U;
	payload.push_back(b); // in this case, first byte is len since crc is
	//                       calculated on len and data
      } else {
	remaining = b << 8; // MSB First
      }
      // DebugTrace ("LEN = %u", remaining);
      state = P == PprzPolicy::XBEE_API ? PprzLinkDecoderState::WAIT_FOR_LEN2 :
	PprzLinkDecoderState::PROCESSING_PAYLOAD;
      break;
    }
      
    case PprzLinkDecoderState::WAIT_FOR_LEN2 : {
      // DebugTrace ("WAIT_FOR_LEN2");
      // the length is the length of the entire message
      // payload length is len - (start, len, chks1, chks2)
      const auto b = data.front();
      data = data.subspan(1);
      if constexpr (P == PprzPolicy::XBEE_API) {
	remaining |= b; // LSB Last
      } else {
	TRAPERR_f(valid, ++invalid);
	reset();
      }
      // DebugTrace ("LEN = %u", remaining);
      state = PprzLinkDecoderState::PROCESSING_PAYLOAD;
      break;
    }
      
    case PprzLinkDecoderState::PROCESSING_PAYLOAD : {
      // copy largest possible chunk in an optimised way instead byte by byte
      // DebugTrace ("PROCESSING_PAYLOAD %u left", remaining);
      size_t to_copy = std::min(remaining, data.size());
      if (to_copy <= payload.available()) {
	payload.insert(payload.end(), data.begin(), data.begin() + to_copy);
	remaining -= to_copy;
	data = data.subspan(to_copy);
	if (remaining == 0) {
	  state = PprzLinkDecoderState::WAIT_FOR_CHECKSUM_1;
	}
      } else { // LEN Error
	TRAPERR_f(valid, ++invalid);
	reset();
      }
      break;
    }
      
      
    case PprzLinkDecoderState::WAIT_FOR_CHECKSUM_1 : {
      // DebugTrace ("WAIT_FOR_CHECKSUM_1");
      const auto b = data.front();
      data = data.subspan(1);
      chksum = b;

      if constexpr (P == PprzPolicy::XBEE_API) {
	const uint8_t calculatedChksum = chksum8(payload);
	if (calculatedChksum == chksum) {
	  ++valid;
	  SENDMSG_f(P, payload);
	} else {
	  TRAPERR_f(valid, ++invalid);
	}
	reset();
      } else {
	state = PprzLinkDecoderState::WAIT_FOR_CHECKSUM_2;
      }
      break;
    }

    case PprzLinkDecoderState::WAIT_FOR_CHECKSUM_2 : {
      // DebugTrace ("WAIT_FOR_CHECKSUM_2");
      const auto b = data.front();
      data = data.subspan(1);
      if constexpr (P == PprzPolicy::XBEE_API) {
	TRAPERR_f(valid, ++invalid);
	reset();
      }
      chksum = chksum | (b << 8);
      const uint16_t calculatedChksum = fletcher16(payload);
      if (calculatedChksum == chksum) {
	++valid;
	SENDMSG_f(P, std::span(payload.begin() + 1, payload.end()));
      } else {
	TRAPERR_f(valid, ++invalid);
      }
      reset();
      break;
    }
    }
  }
}

/*
  simplified algo : overflow if len > 5802
  which won't affect us, hope we'll use hardware crc32 when we'll have messages this long
*/
namespace {
  uint16_t fletcher16(std::span<const uint8_t> data)
  {
    uint32_t c0 = 0, c1 = 0;
    for (const uint8_t b : data) { 
      c0 += b;
      c1 += c0;
    }
    c0 %= 0xff;
    c1 %= 0xff;
    return (c1 << 8) | c0;
  }
  

  uint8_t chksum8(std::span<const uint8_t> data)
  {
    uint8_t sum = 0;
    for (const auto b : data) {
      sum += b;
    }
    return 0xFF - sum;
  }
}
