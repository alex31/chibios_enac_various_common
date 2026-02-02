/**
 * @file sioWrapper.hpp
 * @brief C++23 SIO wrapper interfaces.
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>

#include <ch.h>
#include <hal.h>
#include "hal_stm32_dma.h"
#include "hal_buffered_sio.h"
#include "fifoObject.hpp"

/** @brief SIO C++ wrapper namespace. */
namespace SIO {

/** @brief Configuration for SIO::Base. */
struct BaseConfig {
  SIODriver &driver;              ///< Low-level SIO driver.
  const SIOConfig *sio_config = nullptr; ///< Optional SIO configuration (copied if provided).
};

/** @brief Common SIO wrapper base class (non-copyable). */
class Base {
public:
  /** @brief Construct base with driver and optional config. */
  explicit Base(const BaseConfig &cfg);
  /** @brief Copy disabled. */
  Base(const Base &) = delete;
  /** @brief Copy disabled. */
  Base &operator=(const Base &) = delete;
  /** @brief Move disabled. */
  Base(Base &&) = delete;
  /** @brief Move disabled. */
  Base &operator=(Base &&) = delete;

  /** @brief Start the underlying driver. */
  virtual msg_t start() = 0;
  /** @brief Stop the underlying driver. */
  virtual void stop() = 0;
  /** @brief Update the stored configuration (caller must restart if needed). */
  void setConfig(const SIOConfig &cfg);

  /** @brief Acquire the SIO bus mutex (caller-managed mutual exclusion). */
  void acquireBus();
  /** @brief Release the SIO bus mutex. */
  void releaseBus();

protected:
  /** @brief Protected virtual destructor (no public delete). */
  virtual ~Base();

  /** @brief Access the wrapped driver (mutable). */
  SIODriver &driver();
  /** @brief Access the wrapped driver (const). */
  const SIODriver &driver() const;
  /** @brief Access the copied SIOConfig pointer (or nullptr). */
  const SIOConfig *config() const;

  /** @brief Install driver callback and argument. */
  void setCallback(siocb_t cb, void *arg = nullptr);

  /** @brief Enable event flags (thread context). */
  void writeEnableFlags(sioevents_t mask);
  /** @brief Enable event flags (I-class). */
  void writeEnableFlagsX(sioevents_t mask);
  /** @brief Set event flags (thread context). */
  void setEnableFlags(sioevents_t mask);
  /** @brief Set event flags (I-class). */
  void setEnableFlagsX(sioevents_t mask);
  /** @brief Clear event flags (thread context). */
  void clearEnableFlags(sioevents_t mask);
  /** @brief Clear event flags (I-class). */
  void clearEnableFlagsX(sioevents_t mask);

  /** @brief Get and clear error flags. */
  sioevents_t getAndClearErrors();
  /** @brief Get and clear pending events. */
  sioevents_t getAndClearEvents();
  /** @brief Get current event flags without clearing. */
  sioevents_t getEvents() const;

  /** @brief Start an asynchronous read. */
  size_t asyncRead(uint8_t *buffer, size_t n);
  /** @brief Start an asynchronous write. */
  size_t asyncWrite(const uint8_t *buffer, size_t n);

  /** @brief Wait for RX completion or timeout. */
  msg_t synchronizeRX(sysinterval_t timeout = TIME_INFINITE);
  /** @brief Wait for RX idle event or timeout. */
  msg_t synchronizeRXIdle(sysinterval_t timeout = TIME_INFINITE);
  /** @brief Wait for TX completion or timeout. */
  msg_t synchronizeTX(sysinterval_t timeout = TIME_INFINITE);
  /** @brief Wait for TX end event or timeout. */
  msg_t synchronizeTXEnd(sysinterval_t timeout = TIME_INFINITE);

  SIODriver *siop_;          ///< Wrapped driver pointer.
  SIOConfig config_storage_{}; ///< Local copy of SIOConfig when provided.
  const SIOConfig *config_;  ///< Pointer to config_storage_ or nullptr.
  mutex_t bus_mutex_;        ///< Bus mutex for caller-managed exclusion.
};


/** @brief Configuration for SIO::Buffered. */
struct BufferedConfig {
  SIODriver &driver;              ///< Low-level SIO driver.
  const SIOConfig *sio_config = nullptr; ///< Optional SIO configuration (copied if provided).
};

/** @brief Read-only byte view used for RX/TX spans. */
using ByteSpan = std::span<const uint8_t>;
/** @brief RX half-buffer callback (thread context). */
using RxHalfCallback = void (*)(const ByteSpan &slice, void *user);
/** @brief TX end callback (ISR context). */
using TxCallback = void (*)(void *user);

/** @brief User-facing reduced DMA configuration. */
struct DmaUserConfig {
  uint32_t stream; ///< DMA stream identifier.
#if STM32_DMA_SUPPORTS_DMAMUX
  uint32_t dmamux; ///< DMAMUX request identifier.
#else
#if STM32_DMA_SUPPORTS_CSELR
  uint8_t channel; ///< DMA channel selection.
#else
  uint8_t channel; ///< DMA channel selection.
#endif
#endif
  uint8_t irq_priority = 12; ///< IRQ priority.
  uint8_t dma_priority = 0;  ///< DMA priority.
  uint8_t psize = 1;         ///< Peripheral access size.
  uint8_t msize = 1;         ///< Memory access size.
  dmaerrorcallback_t error_cb = nullptr; ///< Optional DMA error callback.
#if STM32_DMA_USE_ASYNC_TIMOUT
  sysinterval_t timeout = TIME_INFINITE; ///< Optional DMA timeout.
#endif
#if __DCACHE_PRESENT
  bool activate_dcache_sync = false; ///< Enable D-cache sync on transfer.
#endif
#if STM32_DMA_ADVANCED
  uint8_t pburst = 0; ///< Peripheral burst size.
  uint8_t mburst = 0; ///< Memory burst size.
  uint8_t fifo = 0;   ///< FIFO threshold.
  bool periph_inc_size_4 = false; ///< Peripheral increment by 4 bytes.
  bool transfert_end_ctrl_by_periph = false; ///< Peripheral flow controller.
#endif
};

/**
 * @brief Buffered SIO wrapper.
 * @tparam N RX/TX buffer size.
 */
template <size_t N>
class Buffered final : public Base {
public:
  /** @brief Configuration type alias. */
  using Config = BufferedConfig;
  /** @brief Construct buffered wrapper with config. */
  explicit Buffered(const Config &cfg);

  /** @brief Start the buffered driver. */
  msg_t start() override;
  /** @brief Stop the buffered driver. */
  void stop() override;

  /** @brief Access buffered driver (mutable). */
  BufferedSIODriver &bufferedDriver();
  /** @brief Access buffered driver (const). */
  const BufferedSIODriver &bufferedDriver() const;

  /** @brief Put one byte from ISR context. */
  msg_t putI(uint8_t b);
  /** @brief Put one byte from thread context. */
  msg_t put(uint8_t b);
  /** @brief Put one byte with timeout. */
  msg_t putTimeout(uint8_t b, sysinterval_t timeout);

  /** @brief Get one byte from ISR context. */
  msg_t getI();
  /** @brief Get one byte from thread context. */
  msg_t get();
  /** @brief Get one byte with timeout. */
  msg_t getTimeout(sysinterval_t timeout);

  /** @brief Write from ISR context. */
  size_t writeI(const uint8_t *buffer, size_t n);
  /** @brief Write (blocking). */
  size_t write(const uint8_t *buffer, size_t n);
  /** @brief Write with timeout. */
  size_t writeTimeout(const uint8_t *buffer, size_t n, sysinterval_t timeout);
  /** @brief Start asynchronous write. */
  size_t writeAsync(const uint8_t *buffer, size_t n);

  /** @brief Read from ISR context. */
  size_t readI(uint8_t *buffer, size_t n);
  /** @brief Read (blocking). */
  size_t read(uint8_t *buffer, size_t n);
  /** @brief Read with timeout. */
  size_t readTimeout(uint8_t *buffer, size_t n, sysinterval_t timeout);
  /** @brief Start asynchronous read. */
  size_t readAsync(uint8_t *buffer, size_t n);

private:
  BufferedSIODriver bsiop_; ///< Buffered SIO driver instance.
  uint8_t sio_rxbuf_[N];    ///< RX ring buffer storage.
  uint8_t sio_txbuf_[N];    ///< TX ring buffer storage.

protected:
  /** @brief Protected destructor. */
  ~Buffered() override = default;
};

/** @brief Configuration for SIO::Datagram. */
struct DatagramConfig {
  SIODriver &driver;              ///< Low-level SIO driver.
  DmaUserConfig rx_dma_cfg;       ///< RX DMA settings.
  DmaUserConfig tx_dma_cfg;       ///< TX DMA settings.
  const SIOConfig *sio_config = nullptr; ///< Optional SIO configuration (copied if provided).
};

/**
 * @brief DMA oneshot SIO wrapper (UART-like semantics).
 */
class Datagram final : public Base {
public:
  /** @brief Configuration type alias. */
  using Config = DatagramConfig;
  /** @brief Construct datagram wrapper with config. */
  explicit Datagram(const Config &cfg);

  /** @brief Start SIO + DMA. */
  msg_t start() override;
  /** @brief Stop DMA and driver. */
  void stop() override;

  /** @brief Update the stored configuration (caller must restart if needed). */
  void setConfig(const SIOConfig &cfg);

  /** @brief Blocking TX write with timeout. Returns bytes written. */
  size_t writeTimeout(const uint8_t *buffer, size_t n, sysinterval_t timeout);
  /** @brief Blocking TX write with infinite timeout. Returns bytes written. */
  size_t write(const uint8_t *buffer, size_t n);
  /** @brief Blocking RX read with timeout. Returns bytes read. */
  size_t readTimeout(uint8_t *buffer, size_t n, sysinterval_t timeout);
  /** @brief Blocking RX read with infinite timeout. Returns bytes read. */
  size_t read(uint8_t *buffer, size_t n);

  /** @brief Check if TX DMA is active. */
  bool txBusy() const;
  /** @brief Check if RX DMA is active. */
  bool rxBusy() const;

private:
  static void initDmaCfg(DMAConfig &dst, const DmaUserConfig &src, bool is_rx);

  DMADriver rx_dma_;         ///< RX DMA driver instance.
  DMADriver tx_dma_;         ///< TX DMA driver instance.
  DMAConfig rx_cfg_storage_; ///< RX DMA config storage.
  DMAConfig tx_cfg_storage_; ///< TX DMA config storage.
  const DMAConfig *rx_cfg_;  ///< RX DMA config pointer.
  const DMAConfig *tx_cfg_;  ///< TX DMA config pointer.

protected:
  /** @brief Protected destructor. */
  ~Datagram() override = default;
};


/** @brief Configuration for SIO::Continuous. */
struct ContinuousConfig {
  SIODriver &driver;              ///< Low-level SIO driver.
  DmaUserConfig rx_dma_cfg;       ///< User RX DMA settings.
  DmaUserConfig tx_dma_cfg;       ///< User TX DMA settings.
  const SIOConfig *sio_config = nullptr; ///< Optional SIO configuration (copied if provided).
  RxHalfCallback rx_half_cb = nullptr; ///< RX half-buffer callback (thread context).
  void *rx_user = nullptr;        ///< User context for RX callback.
  const char *rx_thread_name = "sio-rx"; ///< RX worker thread name.
  tprio_t rx_thread_prio = NORMALPRIO;   ///< RX worker thread priority.
  size_t rx_thread_wa_size = THD_WORKING_AREA_SIZE(512); ///< RX worker stack size.
};

/**
 * @brief Continuous DMA SIO wrapper (RX circular + TX oneshot).
 * @tparam DBS DMA RX buffer size (must be even).
 * @tparam FD  FIFO depth for RX slices.
 */
template <size_t DBS, size_t FD>
class Continuous final : public Base {
public:
  /** @brief Full RX buffer size. */
  static constexpr size_t RxBufferSize = DBS;
  /** @brief Half-buffer size. */
  static constexpr size_t RxHalfSize = DBS / 2U;
  /** @brief RX FIFO depth. */
  static constexpr size_t FifoDepth = FD;

  /** @brief Byte span alias. */
  using Slice = ByteSpan;
  /** @brief RX callback type alias. */
  using RxHalfCallback = SIO::RxHalfCallback;
  /** @brief Construct continuous wrapper with config. */
  explicit Continuous(const ContinuousConfig &cfg);

  /** @brief Start SIO + DMA + worker thread. */
  msg_t start() override;
  /** @brief Stop DMA, driver, and worker thread. */
  void stop() override;

  /** @brief Set RX callback and context. */
  void setRxCallback(RxHalfCallback cb, void *user);

  /** @brief Start continuous RX DMA. */
  void startRx();
  /** @brief Stop continuous RX DMA. */
  void stopRx();

  /** @brief Start one-shot TX DMA (non-blocking). */
  bool writeI(const uint8_t *data, size_t len);
  /** @brief Set TX completion callback and context. */
  void setTxCallback(TxCallback cb, void *user);
  /** @brief Blocking TX write with timeout. */
  msg_t writeTimeout(const ByteSpan &slice, sysinterval_t timeout);
  /** @brief Blocking TX write with infinite timeout. */
  msg_t write(const ByteSpan &slice);
  /** @brief Check if TX DMA is active. */
  bool txBusy() const;

  /** @brief Access RX DMA buffer (mutable). */
  uint8_t *rxBuffer();
  /** @brief Access RX DMA buffer (const). */
  const uint8_t *rxBuffer() const;

private:
  static_assert((DBS % 2U) == 0U, "DBS must be even for half-buffer callbacks");
  static_assert((FD > 0U), "FD must be greater than zero");

  /** @brief DMAConfig plus back-pointer to owner. */
  struct DmaCfgWithOwner {
    DMAConfig cfg{};           ///< Full DMA configuration.
    Continuous *owner = nullptr; ///< Back-pointer for callbacks.
  };
  static_assert(offsetof(DmaCfgWithOwner, cfg) == 0, 
                "DMAConfig must be the first member of DmaCfgWithOwner for unsafe cast");

  /** @brief FIFO item storing one RX half-buffer. */
  struct RxItem {
    size_t len;                ///< Valid byte count.
    uint8_t data[RxHalfSize];  ///< RX data copy.
  };

  /** @brief RX DMA half-buffer callback (ISR). */
  static void dmaRxCb(DMADriver *dmap, void *buffer, const size_t n);
  /** @brief TX DMA completion callback (ISR). */
  static void dmaTxCb(DMADriver *dmap, void *buffer, const size_t n);
  /** @brief SIO idle event callback (ISR). */
  static void sioIdleCb(SIODriver *siop);
  /** @brief RX worker thread entry. */
  static void rxWorker(void *arg);

  DMADriver rx_dma_;                ///< RX DMA driver instance.
  DMADriver tx_dma_;                ///< TX DMA driver instance.
  const DMAConfig *rx_cfg_;         ///< RX DMA config pointer.
  const DMAConfig *tx_cfg_;         ///< TX DMA config pointer.
  DmaCfgWithOwner rx_cfg_storage_;  ///< RX DMA config storage.
  DmaCfgWithOwner tx_cfg_storage_;  ///< TX DMA config storage.
  RxHalfCallback rx_cb_;            ///< RX half-buffer callback.
  void *rx_user_;                   ///< RX callback context.
  TxCallback tx_cb_;                ///< TX end callback.
  void *tx_user_;                   ///< TX callback context.
  const char *rx_thread_name_;      ///< RX worker thread name.
  tprio_t rx_thread_prio_;          ///< RX worker thread priority.
  size_t rx_thread_wa_size_;        ///< RX worker stack size.
  thread_t *rx_thread_;             ///< RX worker thread handle.
  size_t rx_dropped_;               ///< Dropped RX slices count.
  ObjectFifo<RxItem, FD> rx_fifo_;  ///< RX FIFO storage.
  alignas(4) uint8_t sio_rxbuf_[DBS]; ///< RX DMA buffer.

protected:
  /** @brief Protected destructor. */
  ~Continuous() override = default;
};

template <size_t N>
inline Buffered<N>::Buffered(const Config &cfg)
    : Base(BaseConfig{cfg.driver, cfg.sio_config}) {
  if (config_ != nullptr) {
    config_storage_.cr3 &= static_cast<uint32_t>(~(USART_CR3_DMAR | USART_CR3_DMAT));
  }
  bsioObjectInit(&bsiop_, &cfg.driver,
                 sio_rxbuf_, sizeof(sio_rxbuf_),
                 sio_txbuf_, sizeof(sio_txbuf_));
}

template <size_t N>
inline msg_t Buffered<N>::start() {
  return bsioStart(&bsiop_, config_);
}

template <size_t N>
inline void Buffered<N>::stop() {
  bsioStop(&bsiop_);
}

template <size_t N>
inline BufferedSIODriver &Buffered<N>::bufferedDriver() {
  return bsiop_;
}

template <size_t N>
inline const BufferedSIODriver &Buffered<N>::bufferedDriver() const {
  return bsiop_;
}

template <size_t N>
inline msg_t Buffered<N>::putI(uint8_t b) {
  return bsioPutI(&bsiop_, b);
}

template <size_t N>
inline msg_t Buffered<N>::put(uint8_t b) {
  return bsioPut(&bsiop_, b);
}

template <size_t N>
inline msg_t Buffered<N>::putTimeout(uint8_t b, sysinterval_t timeout) {
  return bsioPutTimeout(&bsiop_, b, timeout);
}

template <size_t N>
inline msg_t Buffered<N>::getI() {
  return bsioGetI(&bsiop_);
}

template <size_t N>
inline msg_t Buffered<N>::get() {
  return bsioGet(&bsiop_);
}

template <size_t N>
inline msg_t Buffered<N>::getTimeout(sysinterval_t timeout) {
  return bsioGetTimeout(&bsiop_, timeout);
}

template <size_t N>
inline size_t Buffered<N>::writeI(const uint8_t *buffer, size_t n) {
  return bsioWriteI(&bsiop_, buffer, n);
}

template <size_t N>
inline size_t Buffered<N>::write(const uint8_t *buffer, size_t n) {
  return bsioWrite(&bsiop_, buffer, n);
}

template <size_t N>
inline size_t Buffered<N>::writeTimeout(const uint8_t *buffer, size_t n, sysinterval_t timeout) {
  return bsioWriteTimeout(&bsiop_, buffer, n, timeout);
}

template <size_t N>
inline size_t Buffered<N>::writeAsync(const uint8_t *buffer, size_t n) {
  return bsioAsynchronousWrite(&bsiop_, buffer, n);
}

template <size_t N>
inline size_t Buffered<N>::readI(uint8_t *buffer, size_t n) {
  return iqReadI(&bsiop_.iqueue, buffer, n);
}

template <size_t N>
inline size_t Buffered<N>::read(uint8_t *buffer, size_t n) {
  return bsioRead(&bsiop_, buffer, n);
}

template <size_t N>
inline size_t Buffered<N>::readTimeout(uint8_t *buffer, size_t n, sysinterval_t timeout) {
  return bsioReadTimeout(&bsiop_, buffer, n, timeout);
}

template <size_t N>
inline size_t Buffered<N>::readAsync(uint8_t *buffer, size_t n) {
  return bsioAsynchronousRead(&bsiop_, buffer, n);
}

inline Datagram::Datagram(const Config &cfg)
    : Base(BaseConfig{cfg.driver, cfg.sio_config}),
      rx_dma_{},
      tx_dma_{},
      rx_cfg_storage_{},
      tx_cfg_storage_{},
      rx_cfg_(nullptr),
      tx_cfg_(nullptr) {
  if (config_ != nullptr) {
    config_storage_.cr3 |= (USART_CR3_DMAR | USART_CR3_DMAT);
  }
  initDmaCfg(rx_cfg_storage_, cfg.rx_dma_cfg, true);
  rx_cfg_ = &rx_cfg_storage_;
  initDmaCfg(tx_cfg_storage_, cfg.tx_dma_cfg, false);
  tx_cfg_ = &tx_cfg_storage_;
}

inline msg_t Datagram::start() {
  msg_t msg = sioStart(siop_, config_);
  if (msg != HAL_RET_SUCCESS) {
    return msg;
  }

  dmaObjectInit(&rx_dma_);
  dmaObjectInit(&tx_dma_);
  if (!dmaStart(&rx_dma_, rx_cfg_) || !dmaStart(&tx_dma_, tx_cfg_)) {
    dmaStop(&tx_dma_);
    dmaStop(&rx_dma_);
    sioStop(siop_);
    return HAL_RET_HW_FAILURE;
  }
  return HAL_RET_SUCCESS;
}

inline void Datagram::stop() {
  dmaStop(&tx_dma_);
  dmaStop(&rx_dma_);
  sioStop(siop_);
}

inline void Datagram::setConfig(const SIOConfig &cfg) {
  config_storage_ = cfg;
  config_storage_.cr3 |= (USART_CR3_DMAR | USART_CR3_DMAT);
  config_ = &config_storage_;
}

inline size_t Datagram::writeTimeout(const uint8_t *buffer, size_t n, sysinterval_t timeout) {
  if ((buffer == nullptr) || (n == 0U)) {
    return 0U;
  }
#if STM32_DMA_USE_WAIT == TRUE
  const msg_t msg = dmaTransfertTimeout(&tx_dma_, &driver().usart->TDR,
                                        const_cast<uint8_t *>(buffer), n, timeout);
  if (msg == MSG_OK) {
    return n;
  }
  const size_t remaining = dmaGetTransactionCounter(&tx_dma_);
  return (remaining > n) ? 0U : (n - remaining);
#else
  (void)timeout;
  return 0U;
#endif
}

inline size_t Datagram::write(const uint8_t *buffer, size_t n) {
  return writeTimeout(buffer, n, TIME_INFINITE);
}

inline size_t Datagram::readTimeout(uint8_t *buffer, size_t n, sysinterval_t timeout) {
  if ((buffer == nullptr) || (n == 0U)) {
    return 0U;
  }
#if STM32_DMA_USE_WAIT == TRUE
  const msg_t msg = dmaTransfertTimeout(&rx_dma_, &driver().usart->RDR,
                                        buffer, n, timeout);
  if (msg == MSG_OK) {
    return n;
  }
  const size_t remaining = dmaGetTransactionCounter(&rx_dma_);
  return (remaining > n) ? 0U : (n - remaining);
#else
  (void)timeout;
  return 0U;
#endif
}

inline size_t Datagram::read(uint8_t *buffer, size_t n) {
  return readTimeout(buffer, n, TIME_INFINITE);
}

inline bool Datagram::txBusy() const {
  return (dmaGetState(const_cast<DMADriver *>(&tx_dma_)) == DMA_ACTIVE);
}

inline bool Datagram::rxBusy() const {
  return (dmaGetState(const_cast<DMADriver *>(&rx_dma_)) == DMA_ACTIVE);
}

inline void Datagram::initDmaCfg(DMAConfig &dst, const DmaUserConfig &src, bool is_rx) {
  dst = {};
  dst.stream = src.stream;
#if STM32_DMA_SUPPORTS_DMAMUX
  dst.dmamux = src.dmamux;
#else
  dst.channel = src.channel;
#endif
  dst.inc_peripheral_addr = false;
  dst.inc_memory_addr = true;
  dst.op_mode = DMA_ONESHOT;
  dst.end_cb = nullptr;
  dst.error_cb = src.error_cb;
#if STM32_DMA_USE_ASYNC_TIMOUT
  dst.timeout = src.timeout;
#endif
  dst.direction = is_rx ? DMA_DIR_P2M : DMA_DIR_M2P;
  dst.dma_priority = src.dma_priority;
  dst.irq_priority = src.irq_priority;
  dst.psize = src.psize;
  dst.msize = src.msize;
#if __DCACHE_PRESENT
  dst.activate_dcache_sync = src.activate_dcache_sync;
#endif
#if STM32_DMA_ADVANCED
  dst.pburst = src.pburst;
  dst.mburst = src.mburst;
  dst.fifo = src.fifo;
  dst.periph_inc_size_4 = src.periph_inc_size_4;
  dst.transfert_end_ctrl_by_periph = src.transfert_end_ctrl_by_periph;
#endif
}

template <size_t DBS, size_t FD>
inline Continuous<DBS, FD>::Continuous(const ContinuousConfig &cfg)
    : Base(BaseConfig{cfg.driver, cfg.sio_config}),
      rx_dma_{},
      tx_dma_{},
      rx_cfg_(nullptr),
      tx_cfg_(nullptr),
      rx_cfg_storage_{},
      tx_cfg_storage_{},
      rx_cb_(cfg.rx_half_cb),
      rx_user_(cfg.rx_user),
      tx_cb_(nullptr),
      tx_user_(nullptr),
      rx_thread_name_(cfg.rx_thread_name),
      rx_thread_prio_(cfg.rx_thread_prio),
      rx_thread_wa_size_(cfg.rx_thread_wa_size),
      rx_thread_(nullptr),
      rx_dropped_(0U),
      rx_fifo_() {

  if (config_ != nullptr) {
    config_storage_.cr3 |= (USART_CR3_DMAR | USART_CR3_DMAT);
  }

  auto init_dma_cfg = [this](DmaCfgWithOwner &dst, const DmaUserConfig &src, const bool is_rx) {
    dst.cfg = {};
    dst.cfg.stream = src.stream;
#if STM32_DMA_SUPPORTS_DMAMUX
    dst.cfg.dmamux = src.dmamux;
#else
    dst.cfg.channel = src.channel;
#endif
    dst.cfg.inc_peripheral_addr = false;
    dst.cfg.inc_memory_addr = true;
    dst.cfg.op_mode = is_rx ? DMA_CONTINUOUS_HALF_BUFFER : DMA_ONESHOT;
    dst.cfg.end_cb = is_rx ? &Continuous::dmaRxCb : &Continuous::dmaTxCb;
    dst.cfg.error_cb = src.error_cb;
#if STM32_DMA_USE_ASYNC_TIMOUT
    dst.cfg.timeout = src.timeout;
#endif
    dst.cfg.direction = is_rx ? DMA_DIR_P2M : DMA_DIR_M2P;
    dst.cfg.dma_priority = src.dma_priority;
    dst.cfg.irq_priority = src.irq_priority;
    dst.cfg.psize = src.psize;
    dst.cfg.msize = src.msize;
#if __DCACHE_PRESENT
    dst.cfg.activate_dcache_sync = src.activate_dcache_sync;
#endif
#if STM32_DMA_ADVANCED
    dst.cfg.pburst = src.pburst;
    dst.cfg.mburst = src.mburst;
    dst.cfg.fifo = src.fifo;
    dst.cfg.periph_inc_size_4 = src.periph_inc_size_4;
    dst.cfg.transfert_end_ctrl_by_periph = src.transfert_end_ctrl_by_periph;
#endif
    dst.owner = this;
  };

  init_dma_cfg(rx_cfg_storage_, cfg.rx_dma_cfg, true);
  rx_cfg_ = &rx_cfg_storage_.cfg;
  init_dma_cfg(tx_cfg_storage_, cfg.tx_dma_cfg, false);
  tx_cfg_ = &tx_cfg_storage_.cfg;
}

template <size_t DBS, size_t FD>
inline msg_t Continuous<DBS, FD>::start() {
  msg_t msg = sioStart(siop_, config_);
  if (msg != HAL_RET_SUCCESS) {
    return msg;
  }

  dmaObjectInit(&rx_dma_);
  dmaObjectInit(&tx_dma_);

  if (!dmaStart(&rx_dma_, rx_cfg_) || !dmaStart(&tx_dma_, tx_cfg_)) {
    dmaStop(&tx_dma_);
    dmaStop(&rx_dma_);
    sioStop(siop_);
    return HAL_RET_HW_FAILURE;
  }

  setCallback(&Continuous::sioIdleCb, this);
  writeEnableFlagsX(SIO_EV_RXIDLE);

  startRx();

  if (rx_thread_ == nullptr) {
    rx_thread_ = chThdCreateFromHeap(nullptr,
                                     rx_thread_wa_size_,
                                     rx_thread_name_,
                                     rx_thread_prio_,
                                     &Continuous::rxWorker,
                                     this);
    if (rx_thread_ == nullptr) {
      stopRx();
      dmaStop(&tx_dma_);
      dmaStop(&rx_dma_);
      setCallback(nullptr, nullptr);
      writeEnableFlagsX(SIO_EV_NONE);
      sioStop(siop_);
      return HAL_RET_NO_RESOURCE;
    }
  }
  return HAL_RET_SUCCESS;
}

template <size_t DBS, size_t FD>
inline void Continuous<DBS, FD>::stop() {
  stopRx();
  dmaStop(&tx_dma_);
  dmaStop(&rx_dma_);
  setCallback(nullptr, nullptr);
  writeEnableFlagsX(SIO_EV_NONE);
  sioStop(siop_);

  if (rx_thread_ != nullptr) {
    if (auto slotOpt = rx_fifo_.takeObject(TIME_IMMEDIATE)) {
      auto &slot = slotOpt->get();
      slot.len = 0U;
      rx_fifo_.sendObject(slot);
    }
    chThdWait(rx_thread_);
    rx_thread_ = nullptr;
  }
}

template <size_t DBS, size_t FD>
inline void Continuous<DBS, FD>::setRxCallback(RxHalfCallback cb, void *user) {
  rx_cb_ = cb;
  rx_user_ = user;
}

template <size_t DBS, size_t FD>
inline void Continuous<DBS, FD>::setTxCallback(TxCallback cb, void *user) {
  tx_cb_ = cb;
  tx_user_ = user;
}

template <size_t DBS, size_t FD>
inline void Continuous<DBS, FD>::startRx() {
  dmaStartTransfert(&rx_dma_, &driver().usart->RDR, sio_rxbuf_, DBS);
}

template <size_t DBS, size_t FD>
inline void Continuous<DBS, FD>::stopRx() {
  dmaStopTransfert(&rx_dma_);
}

template <size_t DBS, size_t FD>
inline bool Continuous<DBS, FD>::writeI(const uint8_t *data, size_t len) {
  if (txBusy()) {
    return false;
  }
  return dmaStartTransfert(&tx_dma_, &driver().usart->TDR,
                           const_cast<uint8_t *>(data), len);
}

template <size_t DBS, size_t FD>
inline msg_t Continuous<DBS, FD>::writeTimeout(const ByteSpan &slice, sysinterval_t timeout) {
  const uint8_t *data = slice.data();
  const size_t len = slice.size();
  if ((data == nullptr) || (len == 0U)) {
    return MSG_OK;
  }

#if STM32_DMA_USE_WAIT == TRUE
  return dmaTransfertTimeout(&tx_dma_, &driver().usart->TDR,
                             const_cast<uint8_t *>(data), len, timeout);
#else
  (void)timeout;
  (void)data;
  (void)len;
  return MSG_RESET;
#endif
}

template <size_t DBS, size_t FD>
inline msg_t Continuous<DBS, FD>::write(const ByteSpan &slice) {
  return writeTimeout(slice, TIME_INFINITE);
}

template <size_t DBS, size_t FD>
inline bool Continuous<DBS, FD>::txBusy() const {
  return (dmaGetState(const_cast<DMADriver *>(&tx_dma_)) == DMA_ACTIVE);
}

template <size_t DBS, size_t FD>
inline uint8_t *Continuous<DBS, FD>::rxBuffer() {
  return sio_rxbuf_;
}

template <size_t DBS, size_t FD>
inline const uint8_t *Continuous<DBS, FD>::rxBuffer() const {
  return sio_rxbuf_;
}

template <size_t DBS, size_t FD>
inline void Continuous<DBS, FD>::dmaRxCb(DMADriver *dmap, void *buffer, const size_t n) {
  if (dmap == nullptr || dmap->config == nullptr) {
    return;
  }

  auto *cfg = reinterpret_cast<const DmaCfgWithOwner *>(dmap->config);
  auto *self = cfg->owner;
  if ((self == nullptr) || (self->rx_cb_ == nullptr)) {
    return;
  }

  chSysLockFromISR();
  auto slotOpt = self->rx_fifo_.takeObjectI();
  chSysUnlockFromISR();

  if (!slotOpt) {
    chSysLockFromISR();
    self->rx_dropped_++;
    chSysUnlockFromISR();
    return;
  }

  auto &slot = slotOpt->get();
  if (n > RxHalfSize) {
    chSysLockFromISR();
    self->rx_fifo_.returnObjectI(slot);
    self->rx_dropped_++;
    chSysUnlockFromISR();
    return;
  }

  std::memcpy(slot.data, buffer, n);
  slot.len = n;

  chSysLockFromISR();
  self->rx_fifo_.sendObjectI(slot);
  chSysUnlockFromISR();
}

template <size_t DBS, size_t FD>
inline void Continuous<DBS, FD>::dmaTxCb(DMADriver *dmap, void *buffer, const size_t n) {
  (void)buffer;
  (void)n;
  if (dmap == nullptr || dmap->config == nullptr) {
    return;
  }

  auto *cfg = reinterpret_cast<const DmaCfgWithOwner *>(dmap->config);
  auto *self = cfg->owner;
  if ((self != nullptr) && (self->tx_cb_ != nullptr)) {
    self->tx_cb_(self->tx_user_);
  }
}

template <size_t DBS, size_t FD>
inline void Continuous<DBS, FD>::sioIdleCb(SIODriver *siop) {
  auto *self = static_cast<Continuous *>(siop->arg);
  if (self != nullptr) {
    //osalDbgAssert((ev & ~SIO_EV_RXIDLE) == 0U, "unexpected SIO events cleared");
    sioevents_t ev = sioGetAndClearEventsX(siop);
    if ((ev & SIO_EV_RXIDLE) && 
	(dmaGetState(&self->rx_dma_) == DMA_ACTIVE)) {
#if STM32_DMA_USE_ASYNC_TIMOUT
      dmaForceHalfBufferFromISR(&self->rx_dma_);
#endif
    }
  }
  sioSetEnableFlagsX(siop, SIO_EV_RXIDLE);
}

template <size_t DBS, size_t FD>
inline void Continuous<DBS, FD>::rxWorker(void *arg) {
  auto *self = static_cast<Continuous *>(arg);
  if (self == nullptr) {
    chThdExit(MSG_RESET);
  }
  chRegSetThreadName(self->rx_thread_name_);

  while (true) {
    auto slotOpt = self->rx_fifo_.receiveObject(TIME_INFINITE);
    if (!slotOpt) {
      continue;
    }
    auto &slot = slotOpt->get();
    if (slot.len == 0U) {
      self->rx_fifo_.returnObject(slot);
      break;
    }
    if (self->rx_cb_ != nullptr) {
      ByteSpan slice(slot.data, slot.len);
      self->rx_cb_(slice, self->rx_user_);
    }
    self->rx_fifo_.returnObject(slot);
  }

  while (true) {
    auto slotOpt = self->rx_fifo_.receiveObject(TIME_IMMEDIATE);
    if (!slotOpt) {
      break;
    }
    auto &slot = slotOpt->get();
    self->rx_fifo_.returnObject(slot);
  }

  chThdExit(MSG_OK);
}

} // namespace SIO
