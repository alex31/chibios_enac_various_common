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

enum class PprzPolicy {PPRZ, XBEE_API};

enum class PprzLinkDecoderState {WAIT_FOR_SYNC, WAIT_FOR_LEN1, WAIT_FOR_LEN2,
				 PROCESSING_PAYLOAD, WAIT_FOR_CHECKSUM_1, WAIT_FOR_CHECKSUM_2};

using Byte_t = uint8_t;
using PprzPayload_t = etl::vector<Byte_t, 251>; 
using PprzMsg_t = etl::vector<Byte_t, 255>; 
using XbeeMsg_t = etl::vector<Byte_t, 260>; 


template <auto F>
concept SendMsgConcept = requires(const PprzPayload_t &m) { { F(m) }; };

template<auto SENDMSG_f, PprzPolicy P = PprzPolicy::PPRZ>
requires SendMsgConcept<SENDMSG_f>
class PprzDecoder {
public: 
  void feed(Byte_t b);
  static PprzMsg_t genPprzMsg(const PprzPayload_t &payload);
private:
  PprzPayload_t payload = {};
  size_t	remaining = {};
  uint16_t      chksum = {};
  PprzLinkDecoderState state = PprzLinkDecoderState::WAIT_FOR_SYNC;
  static uint16_t fletcher16WithLen(std::span<const Byte_t> data);
  static uint8_t chksum8(std::span<const Byte_t> data);
};


template<auto SENDMSG_f, PprzPolicy P>
requires SendMsgConcept<SENDMSG_f>
PprzMsg_t PprzDecoder<SENDMSG_f, P>::genPprzMsg(const PprzPayload_t &payload)
{
  PprzMsg_t msg;
  const uint8_t header[] = {0x99, static_cast<uint8_t>(payload.size() + 4U)};
  msg.insert(msg.begin(), std::begin(header), std::end(header));
  msg.insert(msg.end(), payload.begin(), payload.end());
  const uint16_t chksum = fletcher16WithLen(std::span<const Byte_t>(payload.begin(), payload.end()));
  const uint8_t tail[] = { static_cast<uint8_t>(chksum & 0xFF),
			   static_cast<uint8_t>((chksum >> 8) & 0xFF) };
  msg.insert(msg.end(), std::begin(tail), std::end(tail));
  
  return msg;
}


template<auto SENDMSG_f, PprzPolicy P>
requires SendMsgConcept<SENDMSG_f>
void PprzDecoder<SENDMSG_f, P>::feed(Byte_t b)
{
  static constexpr Byte_t syncBeacon = P == PprzPolicy::PPRZ ? 0x99 : 0x7E;
  switch (state) {
  case PprzLinkDecoderState::WAIT_FOR_SYNC :
    DebugTrace ("WAIT_FOR_SYNC 0x%x", b);
    payload[0] = b;
    
    if (b  == syncBeacon) {
      state = PprzLinkDecoderState::WAIT_FOR_LEN1;
    } 
    break;
    
  case PprzLinkDecoderState::WAIT_FOR_LEN1 :
    DebugTrace ("WAIT_FOR_LEN1");
    // the length is the length of the entire message
    // payload length is len - (start, len, chks1, chks2)
    if constexpr (P == PprzPolicy::PPRZ) {
      remaining = b - 4U;
    } else {
      remaining = b << 8; // MSB First
    }
    payload.clear();
    DebugTrace ("LEN = %u", remaining);
    state = P == PprzPolicy::XBEE_API ? PprzLinkDecoderState::WAIT_FOR_LEN2 :
      PprzLinkDecoderState::PROCESSING_PAYLOAD;
    break;

  case PprzLinkDecoderState::WAIT_FOR_LEN2 :
    DebugTrace ("WAIT_FOR_LEN2");
    // the length is the length of the entire message
    // payload length is len - (start, len, chks1, chks2)
    if constexpr (P == PprzPolicy::XBEE_API) {
      remaining |= b; // LSB Last
    } else {
      chSysHalt("WAIT_FOR_LEN2 state incompatible with PPRZ format");
    }
    payload.clear();
    DebugTrace ("LEN = %u", remaining);
    state = PprzLinkDecoderState::PROCESSING_PAYLOAD;
    break;
    
  case PprzLinkDecoderState::PROCESSING_PAYLOAD :
    DebugTrace ("PROCESSING_PAYLOAD");
    payload.push_back(b);
    if (not (--remaining)) {
      state = PprzLinkDecoderState::WAIT_FOR_CHECKSUM_1;
    }
    break;
    
  case PprzLinkDecoderState::WAIT_FOR_CHECKSUM_1 :
    DebugTrace ("WAIT_FOR_CHECKSUM_1");
    chksum = b;
    state = P == PprzPolicy::XBEE_API ? PprzLinkDecoderState::WAIT_FOR_SYNC :
                                        PprzLinkDecoderState::WAIT_FOR_CHECKSUM_2;
    if constexpr (P == PprzPolicy::XBEE_API) {
      const uint8_t calculatedChksum = chksum8(std::span<const Byte_t>(payload.begin(),
									       payload.end()));
      if (calculatedChksum == chksum) {
	SENDMSG_f(payload);
      } else {
	DebugTrace ("CHKSUM ERROR : calculated 0x%x != in message 0x%x", calculatedChksum, 
		    chksum);
      }
      state = PprzLinkDecoderState::WAIT_FOR_SYNC;
    } else {
      state = PprzLinkDecoderState::WAIT_FOR_CHECKSUM_2;
    }
    break;

  case PprzLinkDecoderState::WAIT_FOR_CHECKSUM_2 :
    DebugTrace ("WAIT_FOR_CHECKSUM_2");
    if constexpr (P == PprzPolicy::XBEE_API) {
      chSysHalt("WAIT_FOR_CHECKSUM_2 state incompatible with PPRZ format");
    }
    chksum = chksum | (b << 8);
    const uint16_t calculatedChksum = fletcher16WithLen(std::span<const Byte_t>(payload.begin(), payload.end()));
    if (calculatedChksum == chksum) {
      SENDMSG_f(payload);
    } else {
      DebugTrace ("CHKSUM ERROR : calculated 0x%x != in message 0x%x", calculatedChksum, 
		  chksum);
    }
    state = PprzLinkDecoderState::WAIT_FOR_SYNC;
    break;
  }
}



template<auto SENDMSG_f, PprzPolicy P>
requires SendMsgConcept<SENDMSG_f>
uint16_t PprzDecoder<SENDMSG_f, P>::fletcher16WithLen (std::span<const Byte_t> data)
{
  uint16_t sum1 = 0xff, sum2 = 0xff;
  size_t bytes = data.size(); 
  auto ptr = data.data();
  
  // it's the len as in the pprz message : len of payload + header + trailer
  sum1 = (uint16_t)(sum1 + bytes + 4U);
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

template<auto SENDMSG_f, PprzPolicy P>
requires SendMsgConcept<SENDMSG_f>
uint8_t PprzDecoder<SENDMSG_f, P>::chksum8 (std::span<const Byte_t> data)
{
  uint8_t sum = 0;
  for (const auto b : data) {
    sum += b;
  }
  return sum;
}
