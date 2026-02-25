#include <ch.h>
#include <hal.h>
#include <new>
#include <cstring>
#include "futabaSbusSio.hpp"
#include "stdutil.h"

#define SBUS_START_BYTE 0x0f
#define SBUS_END_BYTE   0x00
#define SBUS_FLAGS_BYTE 23U
#define SBUS_BUFFLEN 25U

namespace {
  /** @brief  DMA-backed Continuous SIO instance for SBUS. */
  using SbusSIO = SIO::Continuous<128, 8>;

#if defined(ENABLE_SBUS_FRAME_GENERATION) && ENABLE_SBUS_FRAME_GENERATION
#define SBUS_FRAME_GENERATION_ENABLED 1
#else
#define SBUS_FRAME_GENERATION_ENABLED 0
#endif

  /** @brief  Wrapper to hold SIO instance and parsing state. */
  struct SbusSioWrapper {
    SbusSIO *sio;
    uint8_t sync_buf[SBUS_BUFFLEN * 2];
    size_t sync_idx;
#if SBUS_FRAME_GENERATION_ENABLED
    uint8_t tx_buf[SBUS_BUFFLEN];
#endif

    SbusSioWrapper()
        : sio(nullptr), sync_idx(0) {
      std::memset(sync_buf, 0, sizeof(sync_buf));
#if SBUS_FRAME_GENERATION_ENABLED
      std::memset(tx_buf, 0, sizeof(tx_buf));
#endif
    }
    ~SbusSioWrapper() = delete;
  };

  static SIOConfig sbusSioConfig = {
    .baud = 100000,
    .presc = USART_PRESC1,
#ifdef USART_CR2_RXINV // UARTv2
    .cr1 = USART_CR1_PCE | USART_CR1_M0, // 8 bits + even parity => 9 bits mode
#else			// UARTv1
    .cr1 = USART_CR1_PCE | USART_CR1_M, // 8 bits + even parity => 9 bits mode
#endif
    .cr2 = USART_CR2_STOP2_BITS,
    .cr3 = 0
  };

  static void decodeSbusBuffer(const uint8_t *src, SBUSFrame *frm);
#if SBUS_FRAME_GENERATION_ENABLED
  static void encodeSbusBuffer(const SBUSFrame *frm, uint8_t *dest);
#endif

  static inline void invoqueError(const SBUSConfig *cfg, SBUSError err) {
    if (cfg && cfg->errorCb) {
      cfg->errorCb(err);
    }
  }

  /**
   * @brief SIO RX callback implementing sliding window SBUS parsing.
   */
  void sbusRxCb(const SIO::ByteSpan &slice, void *user) {
    auto *sbusp = static_cast<SBUSDriver *>(user);
    auto *wrapper = static_cast<SbusSioWrapper *>(sbusp->sio);
    const auto *cfg = sbusp->config;

    for (const uint8_t byte : slice) {
      if (wrapper->sync_idx < sizeof(wrapper->sync_buf)) {
        wrapper->sync_buf[wrapper->sync_idx++] = byte;
      } else {
        wrapper->sync_idx = 0; // Buffer safety reset
      }

      while (wrapper->sync_idx >= SBUS_BUFFLEN) {
        if (wrapper->sync_buf[0] == SBUS_START_BYTE) {
          if (wrapper->sync_buf[SBUS_BUFFLEN - 1] == SBUS_END_BYTE) {
            // Found a potentially valid frame
            if ((wrapper->sync_buf[SBUS_FLAGS_BYTE] >> SBUS_FRAME_LOST_BIT) & 0x1) {
              invoqueError(cfg, SBUS_LOST_FRAME);
            } else if ((wrapper->sync_buf[SBUS_FLAGS_BYTE] >> SBUS_FAILSAFE_BIT) & 0x1) {
              invoqueError(cfg, SBUS_FAILSAFE);
            } else if (cfg->frameCb) {
              SBUSFrame frame;
              decodeSbusBuffer(wrapper->sync_buf, &frame);
              cfg->frameCb(&frame);
            }
            // Consume frame
            std::memmove(wrapper->sync_buf, &wrapper->sync_buf[SBUS_BUFFLEN],
                         wrapper->sync_idx - SBUS_BUFFLEN);
            wrapper->sync_idx -= SBUS_BUFFLEN;
          } else {
            // Start byte matched but end byte failed. Shift and search again.
            invoqueError(cfg, SBUS_MALFORMED_FRAME);
            std::memmove(wrapper->sync_buf, &wrapper->sync_buf[1], wrapper->sync_idx - 1);
            wrapper->sync_idx -= 1;
          }
        } else {
          // Not at a start byte. Shift and search again.
          std::memmove(wrapper->sync_buf, &wrapper->sync_buf[1], wrapper->sync_idx - 1);
          wrapper->sync_idx -= 1;
        }
      }
    }
  }
} // namespace

extern "C" {

void sbusObjectInit(SBUSDriver *sbusp) {
  sbusp->config = nullptr;
  sbusp->sio = nullptr;
  sbusp->wth = nullptr;
}

void sbusStart(SBUSDriver *sbusp, const SBUSConfig *configp) {
  sbusp->config = configp;
  sbusSioConfig.cr2 = USART_CR2_STOP2_BITS;
#ifndef USART_CR2_RXINV
  chDbgAssert(configp->externallyInverted == true,
              "signal must have been inverted by external device on UARTv1 device");
#else
  if (configp->externallyInverted == false) {
    sbusSioConfig.cr2 |= (USART_CR2_RXINV | USART_CR2_TXINV);
  }
#endif

  if (sbusp->sio == nullptr) {
    auto *wrapper = new SbusSioWrapper();
    if (!wrapper) {
      invoqueError(configp, SBUS_MALLOC_ERROR);
      return;
    }
    const SIO::ContinuousConfig cfg = {
      .driver = *configp->siop,
      .rx_dma_cfg = configp->rx_dma_cfg,
#if SBUS_FRAME_GENERATION_ENABLED
      .tx_dma_cfg = configp->tx_dma_cfg,
#else
      .tx_dma_cfg = {},
#endif
      .sio_config = sbusSioConfig,
      .rx_half_cb = sbusRxCb,
      .rx_user = sbusp,
      .rx_thread_name = "sbus rx",
      .rx_thread_prio = NORMALPRIO + 1, // High priority for RC input
      .rx_thread_wa_size = configp->threadWASize
    };
    wrapper->sio = new SbusSIO(cfg);
    if (wrapper->sio == nullptr) {
      free_m(wrapper);
      invoqueError(configp, SBUS_MALLOC_ERROR);
      return;
    }
    sbusp->sio = wrapper;
  }

  auto *wrapper = static_cast<SbusSioWrapper *>(sbusp->sio);
  wrapper->sio->setConfig(sbusSioConfig);
  (void)wrapper->sio->start();
  wrapper->sio->stopRx(); // Keep DMA idle until StartReceive is called
}

void sbusStop(SBUSDriver *sbusp) {
  if (sbusp->sio != nullptr) {
    auto *wrapper = static_cast<SbusSioWrapper *>(sbusp->sio);
    wrapper->sio->stop();
    // Destructor is protected and resources should never be cleared.
  }
  sbusp->wth = nullptr;
}

void sbusStartReceive(SBUSDriver *sbusp) {
  if (sbusp->sio != nullptr) {
    auto *wrapper = static_cast<SbusSioWrapper *>(sbusp->sio);
    wrapper->sio->startRx();
    // In Continuous mode, the thread is managed by the SIO wrapper.
    // We set wth to a non-null value just to satisfy existing logic checks.
    sbusp->wth = reinterpret_cast<thread_t *>(0xDEADBEEF);
  }
}

void sbusStopReceive(SBUSDriver *sbusp) {
  if (sbusp->sio != nullptr) {
    auto *wrapper = static_cast<SbusSioWrapper *>(sbusp->sio);
    wrapper->sio->stopRx();
    sbusp->wth = nullptr;
  }
}

#if SBUS_FRAME_GENERATION_ENABLED
void sbusSend(SBUSDriver *sbusp, const SBUSFrame *frame) {
  if (sbusp->sio != nullptr) {
    auto *wrapper = static_cast<SbusSioWrapper *>(sbusp->sio);
    encodeSbusBuffer(frame, wrapper->tx_buf);
    (void)wrapper->sio->writeTimeout(SIO::ByteSpan(wrapper->tx_buf, SBUS_BUFFLEN),
                                     TIME_INFINITE);
  }
}
#endif

} // extern "C"

namespace {

static void decodeSbusBuffer(const uint8_t *src, SBUSFrame *frm) {
  uint16_t *dst = frm->channel;
  // src points to the full 25-byte buffer starting with 0x0F
  // Data starts at src[1]
  dst[0]  = ((src[1]    ) | (src[2]<<8))                  & 0x07FF;
  dst[1]  = ((src[2]>>3 ) | (src[3]<<5))                  & 0x07FF;
  dst[2]  = ((src[3]>>6 ) | (src[4]<<2)  | (src[5]<<10))  & 0x07FF;
  dst[3]  = ((src[5]>>1 ) | (src[6]<<7))                  & 0x07FF;
  dst[4]  = ((src[6]>>4 ) | (src[7]<<4))                  & 0x07FF;
  dst[5]  = ((src[7]>>7 ) | (src[8]<<1 ) | (src[9]<<9))   & 0x07FF;
  dst[6]  = ((src[9]>>2 ) | (src[10]<<6))                 & 0x07FF;
  dst[7]  = ((src[10]>>5) | (src[11]<<3))                 & 0x07FF;
  dst[8]  = ((src[12]   ) | (src[13]<<8))                 & 0x07FF;
  dst[9]  = ((src[13]>>3) | (src[14]<<5))                 & 0x07FF;
  dst[10] = ((src[14]>>6) | (src[15]<<2) | (src[16]<<10)) & 0x07FF;
  dst[11] = ((src[16]>>1) | (src[17]<<7))                 & 0x07FF;
  dst[12] = ((src[17]>>4) | (src[18]<<4))                 & 0x07FF;
  dst[13] = ((src[18]>>7) | (src[19]<<1) | (src[20]<<9))  & 0x07FF;
  dst[14] = ((src[20]>>2) | (src[21]<<6))                 & 0x07FF;
  dst[15] = ((src[21]>>5) | (src[22]<<3))                 & 0x07FF;

  frm->flags = src[SBUS_FLAGS_BYTE];
}

#if SBUS_FRAME_GENERATION_ENABLED
static void encodeSbusBuffer(const SBUSFrame *_frm, uint8_t *dest) {
  const uint16_t *const chan = _frm->channel;

  dest[0] = SBUS_START_BYTE;
  dest[1] = (uint8_t)((chan[0] & 0x07FF));
  dest[2] = (uint8_t)((chan[0] & 0x07FF) >> 8 | (chan[1] & 0x07FF) << 3);
  dest[3] = (uint8_t)((chan[1] & 0x07FF) >> 5 | (chan[2] & 0x07FF) << 6);
  dest[4] = (uint8_t)((chan[2] & 0x07FF) >> 2);
  dest[5] = (uint8_t)((chan[2] & 0x07FF) >> 10 | (chan[3] & 0x07FF) << 1);
  dest[6] = (uint8_t)((chan[3] & 0x07FF) >> 7 | (chan[4] & 0x07FF) << 4);
  dest[7] = (uint8_t)((chan[4] & 0x07FF) >> 4 | (chan[5] & 0x07FF) << 7);
  dest[8] = (uint8_t)((chan[5] & 0x07FF) >> 1);
  dest[9] = (uint8_t)((chan[5] & 0x07FF) >> 9 | (chan[6] & 0x07FF) << 2);
  dest[10] = (uint8_t)((chan[6] & 0x07FF) >> 6 | (chan[7] & 0x07FF) << 5);
  dest[11] = (uint8_t)((chan[7] & 0x07FF) >> 3);
  dest[12] = (uint8_t)((chan[8] & 0x07FF));
  dest[13] = (uint8_t)((chan[8] & 0x07FF) >> 8 | (chan[9] & 0x07FF) << 3);
  dest[14] = (uint8_t)((chan[9] & 0x07FF) >> 5 | (chan[10] & 0x07FF) << 6);
  dest[15] = (uint8_t)((chan[10] & 0x07FF) >> 2);
  dest[16] = (uint8_t)((chan[10] & 0x07FF) >> 10 | (chan[11] & 0x07FF) << 1);
  dest[17] = (uint8_t)((chan[11] & 0x07FF) >> 7 | (chan[12] & 0x07FF) << 4);
  dest[18] = (uint8_t)((chan[12] & 0x07FF) >> 4 | (chan[13] & 0x07FF) << 7);
  dest[19] = (uint8_t)((chan[13] & 0x07FF) >> 1);
  dest[20] = (uint8_t)((chan[13] & 0x07FF) >> 9 | (chan[14] & 0x07FF) << 2);
  dest[21] = (uint8_t)((chan[14] & 0x07FF) >> 6 | (chan[15] & 0x07FF) << 5);
  dest[22] = (uint8_t)((chan[15] & 0x07FF) >> 3);
  dest[23] = _frm->flags;
  dest[24] = SBUS_END_BYTE;
}
#endif

} // namespace

#undef SBUS_FRAME_GENERATION_ENABLED
