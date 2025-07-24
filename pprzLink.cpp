#include "ch.h"
#include <pprzLink.hpp>


/**
 * @brief Generates a complete PPRZLink message.
 * @param[out] msg The ETL vector to store the generated message.
 * @param ppHeader The PPRZLink header.
 * @param payload The message payload.
 */
void PprzEncoder::genPprzMsg(PprzMsg_t& msg, const PprzHeader &ppHeader,
			     const etl::span<const uint8_t> &payload)
{
  const uint8_t header[] = {0x99, static_cast<uint8_t>(payload.size() + 4U + sizeof(ppHeader))};
  msg.assign(std::begin(header), std::end(header));
  msg.insert(msg.end(), std::begin(ppHeader.raw), std::end(ppHeader.raw));
  msg.insert(msg.end(), payload.begin(), payload.end());
  const uint16_t chksum = fletcher16(etl::span<const uint8_t>(msg.begin()+1, msg.end()));
  const uint8_t tail[] = { static_cast<uint8_t>(chksum & 0xFF),
			   static_cast<uint8_t>((chksum >> 8) & 0xFF) };
  msg.insert(msg.end(), std::begin(tail), std::end(tail));
}


/**
 * @brief Generates a complete XBee API frame encapsulating a PPRZLink message.
 * @param[out] msg The ETL vector to store the generated message.
 * @param xbHeader The XBee API header.
 * @param ppHeader The PPRZLink header.
 * @param payload The message payload.
 *
 * @details This function constructs an XBee API frame with the following structure:
 * - **Start Delimiter:** 0x7E
 * - **Length:** 2 bytes (MSB, LSB) covering the frame from the type to the checksum.
 * - **Frame Data:**
 *   - **XBee Header:** (e.g., TX16 Request)
 *     - Frame Type (e.g., 0x01 for TX16)
 *     - Frame ID
 *     - Destination Address (16-bit)
 *     - Options
 *   - **PPRZLink Data:**
 *     - **PPRZ Header:**
 *       - Source
 *       - Destination
 *       - Class/Component ID
 *       - Message ID
 *     - **PPRZ Payload:**
 *       - Data from messages.xml
 * - **Checksum:** 1 byte (8-bit sum of frame data bytes, subtracted from 0xFF).
 */
void PprzEncoder::genXbeeMsg(XbeeMsg_t& msg, const XbeeHeader &xbHeader,
			     const PprzHeader &ppHeader,
			     const etl::span<const uint8_t> &payload)
{
  const uint16_t len = payload.size() + sizeof(XbeeHeader) + sizeof(PprzHeader);
  const uint8_t header[] = {0x7e, static_cast<uint8_t>(len >> 8),
			    static_cast<uint8_t>(len & 0xff)};
  msg.assign(std::begin(header), std::end(header));
  msg.insert(msg.end(), std::begin(xbHeader.raw), std::end(xbHeader.raw));
  msg.insert(msg.end(), std::begin(ppHeader.raw), std::end(ppHeader.raw));
  msg.insert(msg.end(), payload.begin(), payload.end());
  const uint8_t chksum = chksum8(etl::span<const uint8_t>(msg.begin()+3, msg.end()));
  msg.insert(msg.end(), chksum);
}
