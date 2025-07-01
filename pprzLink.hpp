#pragma once

#include <concepts>
#include <type_traits>
#include <cstdint>
#include <span>
#include <array>
#include <etl/vector.h>

/*
  on recupere en dma avec stop sur idle char, donc on recupère un buffer de taille variable
  pour chaque caractère, on appelle la methode feed de la classe pprzLinkDecoderState
  cette methode joue la machine à état de decodage, et appelle ume fonction quand elle a segmenté
  un message valide
 */


enum class pprzLinkDecoderState {WAIT_FOR_SYNC, WAIT_FOR_LEN,
				 PROCESSING_PAYLOAD, WAIT_FOR_CHECKSUM_1, WAIT_FOR_CHECKSUM_2};

using Byte_t = uint8_t;
using PprzPayload_t = etl::vector<Byte_t, 251>; 
using PprzMsg_t = etl::vector<Byte_t, 255>; 


template <auto F>
concept SendMsgConcept = requires(const PprzPayload_t &m) { { F(m) }; };

template<auto SENDMSG_f>
requires SendMsgConcept<SENDMSG_f>
class PprzDecoder {
public: 
  void feed(Byte_t b);
  static PprzMsg_t genMsg(const PprzPayload_t &payload);
private:
  PprzPayload_t payload = {};
  size_t	remaining = {};
  uint16_t      crc = {};
  pprzLinkDecoderState state = pprzLinkDecoderState::WAIT_FOR_SYNC;
  static uint16_t fletcher16WithLen(std::span<const Byte_t> data);
};


template<auto SENDMSG_f>
requires SendMsgConcept<SENDMSG_f>
PprzMsg_t PprzDecoder<SENDMSG_f>::genMsg(const PprzPayload_t &payload)
{
  PprzMsg_t msg;
  const uint8_t header[] = {0x99, static_cast<uint8_t>(payload.size() + 4U)};
  msg.insert(msg.begin(), std::begin(header), std::end(header));
  msg.insert(msg.end(), payload.begin(), payload.end());
  const uint16_t crc = fletcher16WithLen(std::span<const Byte_t>(payload.begin(), payload.end()));
  const uint8_t tail[] = { static_cast<uint8_t>(crc & 0xFF),
			   static_cast<uint8_t>((crc >> 8) & 0xFF) };
  msg.insert(msg.end(), std::begin(tail), std::end(tail));
  
  return msg;
}


  template<auto SENDMSG_f>
requires SendMsgConcept<SENDMSG_f>
void PprzDecoder<SENDMSG_f>::feed(Byte_t b)
{
  switch (state) {
    
  case pprzLinkDecoderState::WAIT_FOR_SYNC :
    DebugTrace ("WAIT_FOR_SYNC 0x%x", b);
    payload[0] = b;
    
    if (b  == 0x99) {
      state = pprzLinkDecoderState::WAIT_FOR_LEN;
    } 
    break;
    
  case pprzLinkDecoderState::WAIT_FOR_LEN :
    DebugTrace ("WAIT_FOR_LEN");
    // the length is the length of the entire message
    // payload length is len - (start, len, chks1, chks2)
    remaining = b - 4U; 
    payload.clear();
    DebugTrace ("LEN = %u", remaining);
    state = pprzLinkDecoderState::PROCESSING_PAYLOAD;
    break;
    
  case pprzLinkDecoderState::PROCESSING_PAYLOAD :
    DebugTrace ("PROCESSING_PAYLOAD");
    payload.push_back(b);
    if (not (--remaining)) {
      state = pprzLinkDecoderState::WAIT_FOR_CHECKSUM_1;
    }
    break;
    
  case pprzLinkDecoderState::WAIT_FOR_CHECKSUM_1 :
    DebugTrace ("WAIT_FOR_CHECKSUM_1");
    crc = b;
    state = pprzLinkDecoderState::WAIT_FOR_CHECKSUM_2;
    break;

  case pprzLinkDecoderState::WAIT_FOR_CHECKSUM_2 :
    DebugTrace ("WAIT_FOR_CHECKSUM_2");
    crc = crc | (b << 8);
    const uint16_t calculatedCrc = fletcher16WithLen(std::span<const Byte_t>(payload.begin(), payload.end()));
    if (calculatedCrc == crc) {
      SENDMSG_f(payload);
    } else {
      DebugTrace ("CRC ERROR : calculated 0x%x != in message 0x%x", calculatedCrc, 
		  crc);
    }
    state = pprzLinkDecoderState::WAIT_FOR_SYNC;
    break;
  }
}



template<auto SENDMSG_f>
requires SendMsgConcept<SENDMSG_f>
uint16_t PprzDecoder<SENDMSG_f>::fletcher16WithLen (std::span<const Byte_t> data)
{
  uint16_t sum1 = 0xff, sum2 = 0xff;
  size_t bytes = data.size();
  auto ptr = data.data();
  
  sum1 = (uint16_t)(sum1 + bytes);
  sum2 = (uint16_t)(sum2 + sum1);
  sum1 = (uint16_t)((sum1 & 0xff) + (sum1 >> 8));
  sum2 = (uint16_t)((sum2 & 0xff) + (sum2 >> 8));
  
  while (bytes) {
    size_t tlen = bytes > 20 ? 20 : bytes;
    bytes -= tlen;
    do {
      sum1 = (uint16_t)(sum1 + *ptr++);
      sum2 = (uint16_t)(sum2 + sum1);
    } while (--tlen);
    sum1 = (uint16_t)((sum1 & 0xff) + (sum1 >> 8));
    sum2 = (uint16_t)((sum2 & 0xff) + (sum2 >> 8));
  }
  // Second reduction step to reduce sums to 8 bits
  sum1 = (uint16_t)((sum1 & 0xff) + (sum1 >> 8));
  sum2 = (uint16_t)((sum2 & 0xff) + (sum2 >> 8));
  return (uint16_t)((sum2 % 0xff) << 8) | (sum1 % 0xff);
}
