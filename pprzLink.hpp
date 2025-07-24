#pragma once

#include <concepts>
#include <type_traits>
#include <cstdint>
#include <span>
#include <array>
#include <etl/vector.h>
#include "etl/span.h"
#include "ch.h"
//#include "stdutil.h"


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
 * either byte-by-byte or in chunks (etl::span). The decoder will automatically synchronize,
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

/**
 * @enum PprzPolicy
 * @brief Defines the encapsulation policy for the decoder.
 */
enum class PprzPolicy {PPRZ, XBEE_API};

/**
 * @enum PprzLinkDecoderState
 * @brief Defines the states of the PPRZLink decoder state machine.
 */
enum class PprzLinkDecoderState {WAIT_FOR_SYNC, WAIT_FOR_LEN1, WAIT_FOR_LEN2,
				 PROCESSING_PAYLOAD, WAIT_FOR_CHECKSUM_1, WAIT_FOR_CHECKSUM_2};

/**
 * @typedef PprzPayload_t
 * @brief ETL vector for storing a PPRZ message payload.
 */
using PprzPayload_t = etl::vector<uint8_t, 251>;
/**
 * @typedef PprzMsg_t
 * @brief ETL vector for storing a complete PPRZ message.
 */
using PprzMsg_t = etl::vector<uint8_t, PprzPayload_t::MAX_SIZE + 4>;
/**
 * @typedef XbeeMsg_t
 * @brief ETL vector for storing a complete XBee message.
 */
using XbeeMsg_t = etl::vector<uint8_t, PprzPayload_t::MAX_SIZE + 9>;


/**
 * @concept SendMsgConcept
 * @brief Concept for a function that can send a message.
 */
template <auto F>
concept SendMsgConcept = requires(PprzPolicy p, const etl::span<uint8_t> m) { { F(p, m) }; };
/**
 * @concept TrapErrorConcept
 * @brief Concept for a function that can handle a trap error.
 */
template <auto F>
concept TrapErrorConcept = requires(uint32_t valid, uint32_t invalid) { { F(valid, invalid) }; };


/**
 * @enum XbeeRole
 * @brief Defines the role of an XBee module (Transmitter or Receiver).
 */
enum class XbeeRole : uint8_t {Tx = 0x01, Rx = 0x81};
/**
 * @union XbeeHeader
 * @brief Represents the header of an XBee API frame.
 * @details This union provides multiple views of the XBee header for both transmit (Tx) and receive (Rx) modes.
 *
 *          **Tx Frame Structure:**
 *          - `role`: `XbeeRole::Tx` (0x01)
 *          - `frameId`: Frame identifier
 *          - `destId_bigEndian`: 16-bit destination address (big-endian)
 *          - `options`: Transmission options
 *
 *          **Rx Frame Structure:**
 *          - `role`: `XbeeRole::Rx` (0x81)
 *          - `srcId_bigEndian`: 16-bit source address (big-endian)
 *          - `rssi`: Received Signal Strength Indicator
 *          - `options`: Reception options
 */
union XbeeHeader {
  // Explicit re-declaration of default constructors/operators
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

  /**
   * @brief Constructs an XbeeHeader by copying data from a span and advancing it.
   * @tparam T The type of data in the span (must be 1 byte and trivially copyable).
   * @param bytes The span to copy from. It will be advanced by the size of the header.
   */
  template<typename T>
  requires (sizeof(T) == 1 && std::is_trivially_copyable_v<T>)
  XbeeHeader(etl::span<const T>& bytes)
  {
    this->bitCopy(bytes);
  }

   /**
    * @brief Constructor for a transmit (Tx) header.
    * @param _frameId Frame identifier.
    * @param _dstId 16-bit destination address.
    * @param _options Transmission options.
    */
    constexpr XbeeHeader(uint8_t _frameId, uint16_t _dstId, uint8_t _options)
    : role(XbeeRole::Tx),
      frameId(_frameId),
      destId_bigEndian(__builtin_bswap16(_dstId)),
      options(_options)
    {}

    /**
     * @brief Constructor for a receive (Rx) header.
     * @param _srcId 16-bit source address.
     * @param _rssi Received Signal Strength Indicator.
     * @param _options Reception options.
     */
    constexpr XbeeHeader(uint16_t _srcId, uint8_t _rssi, uint8_t _options)
    : role(XbeeRole::Rx),
      srcId_bigEndian(__builtin_bswap16(_srcId)),
      rssi(_rssi),
      options(_options)
    {}

  // Suppress possibility of conversion
    template<typename T1, typename T2, typename T3>
    requires (!(std::is_same_v<T1, uint8_t> && std::is_same_v<T2, uint16_t> &&
		std::is_same_v<T3, uint8_t>) &&
              !(std::is_same_v<T1, uint16_t> && std::is_same_v<T2, uint8_t>
		&& std::is_same_v<T3, uint8_t>))
    XbeeHeader(T1, T2, T3) = delete;


  /**
   * @brief Copies data from a span into the header and advances the span.
   * @tparam T The type of data in the span (must be 1 byte and trivially copyable).
   * @param bytes The span to copy from. It will be advanced by the size of the header.
   */
  template<typename T>
  requires (sizeof(T) == 1 && std::is_trivially_copyable_v<T>)
  void bitCopy(etl::span<const T>& bytes)
  {
    chDbgAssert(bytes.size() >= sizeof(XbeeHeader), "Buffer too small for XbeeHeader");
    memcpy(this, bytes.data(), sizeof(XbeeHeader));
    bytes = bytes.subspan(sizeof(XbeeHeader));
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


/**
 * @union PprzHeader
 * @brief Represents the header of a PPRZLink message.
 * @details This union defines the structure of a PPRZLink message header, including source,
 *          destination, class ID, component ID, and message ID.
 *
 *          **PPRZ Data Structure:**
 *          - `source`: Sender ID
 *          - `destination`: Destination ID (can be a broadcast ID)
 *          - `classId`: Class ID (4 bits)
 *          - `componentId`: Component ID (4 bits)
 *          - `msgId`: Message ID
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

  /**
   * @brief Constructs a PprzHeader with specified values.
   */
  constexpr PprzHeader(uint8_t src, uint8_t dst, uint8_t classid, uint8_t compid, uint8_t mid) {
    source = src; destination = dst; classId = classid; componentId = compid; msgId = mid;
  }
  PprzHeader() = default;
  PprzHeader(const PprzHeader&) = default;
  PprzHeader(PprzHeader&&) = default;
  PprzHeader& operator=(const PprzHeader&) = default;
  PprzHeader& operator=(PprzHeader&&) = default;
  ~PprzHeader() = default;

  /**
   * @brief Constructs a PprzHeader by copying data from a span and advancing it.
   * @tparam T The type of data in the span (must be 1 byte and trivially copyable).
   * @param bytes The span to copy from. It will be advanced by the size of the header.
   */
  template<typename T>
  requires (sizeof(T) == 1 && std::is_trivially_copyable_v<T>)
  PprzHeader(etl::span<const T>& bytes)
  {
    this->bitCopy(bytes);
  }

  /**
   * @brief Copies data from a span into the header and advances the span.
   * @tparam T The type of data in the span (must be 1 byte and trivially copyable).
   * @param bytes The span to copy from. It will be advanced by the size of the header.
   */
  template<typename T>
  requires (sizeof(T) == 1 && std::is_trivially_copyable_v<T>)
  void bitCopy(etl::span<const T>& bytes)
  {
    chDbgAssert(bytes.size() >= sizeof(PprzHeader), "Buffer too small for PprzHeader");
    memcpy(this, bytes.data(), sizeof(PprzHeader));
    bytes = bytes.subspan(sizeof(PprzHeader));
  }

} __attribute__((packed));
static_assert(sizeof(PprzHeader) == 4);

/**
 * @brief Default send message function (does nothing).
 */
constexpr auto default_sendmsg = [](PprzPolicy, const etl::span<uint8_t>) {};
/**
 * @brief Default trap error function (does nothing).
 */
constexpr auto default_traperr = [](uint32_t, uint32_t) {};

/**
 * @brief User-defined literal for milliseconds.
 */
constexpr unsigned long long operator""_ms(unsigned long long ms) {
    return ms;
}

namespace {
  /**
   * @brief Computes the CRC-16 of the provided buffer (Fletcher16).
   * @param data The data to compute the CRC on.
   * @return The calculated CRC-16.
   */
  uint16_t fletcher16(etl::span<const uint8_t> data);

  /**
   * @brief Computes the CRC-8 of the provided buffer.
   * @param data The data to compute the CRC on.
   * @return The calculated CRC-8.
   */
  uint8_t chksum8(etl::span<const uint8_t> data);
}

/**
 * @namespace PprzEncoder
 * @brief Contains functions for encoding PPRZLink and XBee messages.
 */
namespace PprzEncoder {
  /**
   * @brief Generates a PPRZLink message from the given header and payload.
   * @param[out] msg The generated PPRZLink message.
   * @param ppHeader The PPRZLink header.
   * @param payload The payload to encapsulate.
   */
  void genPprzMsg(PprzMsg_t& msg, const PprzHeader &ppHeader,
			 const etl::span<const uint8_t> &payload);

  /**
   * @brief Generates an XBee API message from the given headers and payload.
   * @param[out] msg The generated XBee message.
   * @param xbHeader The XBee header.
   * @param ppHeader The PPRZLink header.
   * @param payload  The payload to encapsulate.
   */
  void genXbeeMsg(XbeeMsg_t& msg, const XbeeHeader &xbHeader,
		  const PprzHeader &ppHeader, const etl::span<const uint8_t> &payload);
}

/**
 * @class PprzDecoder
 * @brief PPRZLink/XBee API frame decoder.
 *
 * This parser handles the segmentation and validation of PPRZLink messages (optionally with XBee API framing).
 *
 * @tparam P         Encapsulation policy (PPRZ, XBEE_API, etc.)
 * @tparam TIMEOUT_MS Timeout in milliseconds for receiving a message.
 * @tparam SENDMSG_f Callback called when a valid message is received.
 * @tparam TRAPERR_f Callback called on a parsing error.
 *
 * @details
 * The class implements a robust state machine to process a serial stream (byte-by-byte or by buffer).
 *
 * \par State machine transitions:
 * @dot
 * digraph G {
 *   WAIT_FOR_SYNC    -> WAIT_FOR_LEN1       [label="SYNC received"]
 *   WAIT_FOR_LEN1    -> WAIT_FOR_LEN2       [label="LEN MSB received (XBEE)" URL="@ref PprzLinkDecoderState::WAIT_FOR_LEN2"]
 *   WAIT_FOR_LEN1    -> PROCESSING_PAYLOAD  [label="LEN received (PPRZ)" URL="@ref PprzLinkDecoderState::PROCESSING_PAYLOAD"]
 *   WAIT_FOR_LEN2    -> PROCESSING_PAYLOAD  [label="LEN LSB received" URL="@ref PprzLinkDecoderState::PROCESSING_PAYLOAD"]
 *   PROCESSING_PAYLOAD -> WAIT_FOR_CHECKSUM_1 [label="Payload complete" URL="@ref PprzLinkDecoderState::WAIT_FOR_CHECKSUM_1"]
 *   WAIT_FOR_CHECKSUM_1 -> WAIT_FOR_CHECKSUM_2 [label="CRC LSB received (PPRZ)" URL="@ref PprzLinkDecoderState::WAIT_FOR_CHECKSUM_2"]
 *   WAIT_FOR_CHECKSUM_1 -> WAIT_FOR_SYNC     [label="CRC OK (XBEE)" URL="@ref PprzLinkDecoderState::WAIT_FOR_SYNC"]
 *   WAIT_FOR_CHECKSUM_2 -> WAIT_FOR_SYNC     [label="CRC OK/ERROR (PPRZ)" URL="@ref PprzLinkDecoderState::WAIT_FOR_SYNC"]
 *   PROCESSING_PAYLOAD -> WAIT_FOR_SYNC      [label="Payload overflow/error" URL="@ref PprzLinkDecoderState::WAIT_FOR_SYNC"]
 * }
 * @enddot
 *
 * Any parsing error (invalid size, CRC mismatch, overflow) triggers a reset to WAIT_FOR_SYNC.
 */
template<PprzPolicy P,
	 auto TIMEOUT_MS = 1000,
	 auto SENDMSG_f = default_sendmsg,
         auto TRAPERR_f = default_traperr>
requires SendMsgConcept<SENDMSG_f> && TrapErrorConcept<TRAPERR_f>
class PprzDecoder {
public:

  /**
   * @brief Feeds the parser with a buffer of bytes (from stream DMA/IRQ).
   * @param data Buffer of bytes to parse.
   */
  void feed(etl::span<const uint8_t> data);


  /**
   * @brief Returns the number of valid and invalid parsed frames.
   * @return A pair containing the number of valid and invalid frames.
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
 * @brief Feeds the decoder with a span of data.
 * @tparam P The PPRZ policy.
 * @tparam TIMEOUT_MS The timeout in milliseconds.
 * @tparam SENDMSG_f The send message callback.
 * @tparam TRAPERR_f The trap error callback.
 * @param data The data to feed.
 */
template<PprzPolicy P, auto TIMEOUT_MS, auto SENDMSG_f, auto TRAPERR_f>
requires SendMsgConcept<SENDMSG_f> && TrapErrorConcept<TRAPERR_f>
void PprzDecoder<P, TIMEOUT_MS, SENDMSG_f, TRAPERR_f>::feed(etl::span<const uint8_t> data)
{
  static constexpr uint8_t syncBeacon = P == PprzPolicy::PPRZ ? 0x99 : 0x7E;

  // if this bunch of bytes arrive long after previous one, it's not the end of a
  // previous message, but the beginning of a new one, so we reset the state machine not to miss
  // the start of message.
  check_timeout();

  while (!data.empty()) {
    switch (state) {
    case PprzLinkDecoderState::WAIT_FOR_SYNC :
      // DebugTrace ("WAIT_FOR_SYNC");
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
	// DebugTrace("PROCESSING_PAYLOAD paylaod too small need %u have %u",
	//		   to_copy, payload.available());
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
	  // DebugTrace("Checksum error");
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
	SENDMSG_f(P, etl::span(payload.begin() + 1, payload.end()));
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
  /**
   * @brief Computes the Fletcher-16 checksum.
   * @param data The data to checksum.
   * @return The Fletcher-16 checksum.
   * @note Simplified algorithm: overflows if len > 5802.
   */
  uint16_t fletcher16(etl::span<const uint8_t> data)
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


  /**
   * @brief Computes an 8-bit checksum.
   * @param data The data to checksum.
   * @return The 8-bit checksum.
   */
  uint8_t chksum8(etl::span<const uint8_t> data)
  {
    uint8_t sum = 0;
    for (const auto b : data) {
      sum += b;
    }
    return 0xFF - sum;
  }
}
