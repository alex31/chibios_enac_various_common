/**
 * @file sioWrapper.hpp
 * @brief C++23 SIO wrapper interfaces.
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>
#include <algorithm>

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
  const SIOConfig &sio_config;    ///< Mandatory SIO configuration (copied).
};

/** @brief Common SIO wrapper base class (non-copyable). */
class Base {
public:
  /** @brief Construct base with driver and mandatory config. */
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
  /** @brief Access the wrapped low-level SIODriver. */
  SIODriver &rawDriver();
  /** @brief Access the wrapped low-level SIODriver (const). */
  const SIODriver &rawDriver() const;

  /** @brief Acquire the SIO bus mutex (caller-managed mutual exclusion). */
  void acquireBus();
  /** @brief Release the SIO bus mutex. */
  void releaseBus();
  /** @brief ISR-context callback signature for user-selected SIO events. */
  using EventCallbackI = void (*)(sioevents_t events, void *user);

protected:
  /** @brief Protected virtual destructor (no public delete). */
  virtual ~Base();

  /** @brief Access the wrapped driver (mutable). */
  SIODriver &driver();
  /** @brief Access the wrapped driver (const). */
  const SIODriver &driver() const;
  /** @brief Access the copied SIOConfig. */
  const SIOConfig &config() const;

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
  SIOConfig config_{};       ///< Local copy of mandatory SIO configuration.
  mutex_t bus_mutex_;        ///< Bus mutex for caller-managed exclusion.

  /** @brief Configure user event dispatch mask and callback (ISR context). */
  void configureEventDispatch(sioevents_t mask, EventCallbackI cb, void *user);
  /** @brief Get currently configured user event mask. */
  sioevents_t configuredEventMask() const;
  /** @brief Dispatch selected events to the user callback (ISR context). */
  void dispatchConfiguredEventsI(sioevents_t events) const;

private:
  sioevents_t event_mask_ = SIO_EV_NONE; ///< User-selected events to report.
  EventCallbackI event_cb_ = nullptr;    ///< User callback for selected events.
  void *event_user_ = nullptr;           ///< User callback context.
};


/** @brief Configuration for SIO::Buffered. */
struct BufferedConfig {
  SIODriver &driver;              ///< Low-level SIO driver.
  const SIOConfig &sio_config;    ///< Mandatory SIO configuration.
};

/** @brief Read-only byte view used for RX/TX spans. */
using ByteSpan = std::span<const uint8_t>;
/** @brief RX half-buffer callback (thread context). */
using RxHalfCallback = void (*)(const ByteSpan &slice, void *user);
/** @brief Generic callback used by TX/RX DMA completion hooks. */
using TxCallback = void (*)(void *user);
/** @brief ISR-context callback used for selected SIO events. */
using EventCallbackI = Base::EventCallbackI;

/** @brief User-facing reduced DMA configuration. */
struct DmaUserConfig {
  dmaerrorcallback_t error_cb = nullptr; ///< Optional DMA error callback.
  sysinterval_t timeout = TIME_INFINITE; ///< Optional DMA timeout.
  uint8_t stream = STM32_DMA_STREAM_ID_ANY; ///< DMA stream identifier.
#if STM32_DMA_SUPPORTS_DMAMUX
  uint8_t dmamux; ///< DMAMUX request identifier.
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
#if STM32_DMA_ADVANCED
  uint8_t pburst = 0; ///< Peripheral burst size.
  uint8_t mburst = 0; ///< Memory burst size.
  uint8_t fifo = 0;   ///< FIFO threshold.
  bool periph_inc_size_4 = false; ///< Peripheral increment by 4 bytes.
  bool transfert_end_ctrl_by_periph = false; ///< Peripheral flow controller.
#endif
#if __DCACHE_PRESENT
  bool activate_dcache_sync = false; ///< Enable D-cache sync on transfer.
#endif
};

namespace detail {

template <size_t>
inline constexpr bool always_false_v = false;

template <typename Owner>
struct DmaCfgWithOwner {
  DMAConfig cfg{};         ///< Full DMA configuration.
  Owner *owner = nullptr;  ///< Back-pointer for callbacks.
};

inline void initDmaConfig(DMAConfig &dst,
                          const DmaUserConfig &src,
                          dmacallback_t end_cb,
                          const bool is_rx,
                          const dmaopmode_t op_mode) {
  dst = {};
  dst.stream = src.stream;
#if STM32_DMA_SUPPORTS_DMAMUX
  dst.dmamux = src.dmamux;
#else
  dst.channel = src.channel;
#endif
  dst.inc_peripheral_addr = false;
  dst.inc_memory_addr = true;
  dst.op_mode = op_mode;
  dst.end_cb = end_cb;
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

template <typename Owner>
inline Owner *ownerFromDma(DMADriver *dmap) {
  if ((dmap == nullptr) || (dmap->config == nullptr)) {
    return nullptr;
  }
  const auto *cfg = reinterpret_cast<const DmaCfgWithOwner<Owner> *>(dmap->config);
  return cfg->owner;
}

} // namespace detail

/**
 * @brief Buffered SIO wrapper.
 * @note  Best fit for low-throughput links where latency and simple byte-stream
 *        semantics matter more than peak throughput.
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
  const SIOConfig &sio_config;    ///< Mandatory SIO configuration.
  TxCallback txend_cb = nullptr; ///< End of TX buffer callback (DMA end).
  void *txend_user = nullptr;    ///< User context for txend_cb.
  TxCallback rxend_cb = nullptr;  ///< RX buffer filled callback (DMA end).
  void *rxend_user = nullptr;     ///< User context for rxend_cb.
  bool enable_rx_idle = false; ///< Enable RX idle detection to stop DMA early.
};

/**
 * @brief DMA oneshot SIO wrapper (UART-like semantics).
 * @note  Best fit for framed exchanges requiring explicit read/write operations.
 *        It can be fast but has per-transaction DMA rearm overhead.
 */
class Datagram : public Base {
public:
  /** @brief Configuration type alias. */
  using Config = DatagramConfig;
  /** @brief Construct datagram wrapper with config. */
#if STM32_DMA_USE_ASYNC_TIMOUT
  explicit Datagram(const Config &cfg);
#else
  explicit Datagram(const Config &cfg) = delete;
#endif

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

  /** @brief Start asynchronous TX DMA (I-class). Returns true if started.
   *  @note  Caller must ensure DMA is ready; DMA layer asserts on misuse.
   */
  bool writeI(const uint8_t *buffer, size_t n);
  /** @brief Start asynchronous RX DMA (I-class). Returns true if started.
   *  @note  Caller must ensure DMA is ready; DMA layer asserts on misuse.
   */
  bool readI(uint8_t *buffer, size_t n);

  /** @brief Check if TX DMA is active. */
  bool txBusy() const;
  /** @brief Check if RX DMA is active. */
  bool rxBusy() const;

private:
  /** @brief DMAConfig plus back-pointer to owner. */
  using DmaCfgWithOwner = detail::DmaCfgWithOwner<Datagram>;
  static_assert(offsetof(DmaCfgWithOwner, cfg) == 0,
                "DMAConfig must be the first member of DmaCfgWithOwner for unsafe cast");

  /** @brief RX DMA completion callback (ISR). */
  static void dmaRxCb(DMADriver *dmap, void *buffer, const size_t n);
  /** @brief TX DMA completion callback (ISR). */
  static void dmaTxCb(DMADriver *dmap, void *buffer, const size_t n);
  /** @brief SIO event callback (ISR). */
  static void sioIdleCb(SIODriver *siop);

  DMADriver rx_dma_;                ///< RX DMA driver instance.
  DMADriver tx_dma_;                ///< TX DMA driver instance.
  DmaCfgWithOwner rx_cfg_storage_;  ///< RX DMA config storage.
  DmaCfgWithOwner tx_cfg_storage_;  ///< TX DMA config storage.
  const DMAConfig *rx_cfg_;         ///< RX DMA config pointer.
  const DMAConfig *tx_cfg_;         ///< TX DMA config pointer.
  bool rx_idle_enabled_;            ///< Enable RX idle detection.
  TxCallback txend_cb_;            ///< TX end-of-buffer callback.
  void *txend_user_;               ///< User context for txend_cb.
  TxCallback rxend_cb_;             ///< RX buffer filled callback.
  void *rxend_user_;                ///< User context for rxend_cb.

protected:
  /** @brief Protected destructor. */
  ~Datagram() override = default;
};


/** @brief Configuration for SIO::Continuous. */
struct ContinuousConfig {
  SIODriver &driver;              ///< Low-level SIO driver.
  DmaUserConfig rx_dma_cfg;       ///< User RX DMA settings.
  DmaUserConfig tx_dma_cfg;       ///< User TX DMA settings.
  const SIOConfig &sio_config;    ///< Mandatory SIO configuration.
  RxHalfCallback rx_half_cb = nullptr; ///< RX half-buffer callback (thread context).
  void *rx_user = nullptr;        ///< User context for RX callback.
  const char *rx_thread_name = "sio-rx"; ///< RX worker thread name.
  tprio_t rx_thread_prio = NORMALPRIO;   ///< RX worker thread priority.
  size_t rx_thread_wa_size = THD_WORKING_AREA_SIZE(512); ///< RX worker stack size.
  bool enable_rx_idle = true;     ///< Enable RX-idle flush support.
  sioevents_t event_mask = SIO_EV_NONE; ///< Additional SIO events to report (except RXIDLE).
  EventCallbackI event_cb = nullptr; ///< ISR-context callback for selected SIO events.
  void *event_user = nullptr;     ///< User context for event_cb.
};

/**
 * @brief Continuous DMA SIO wrapper (RX circular + TX oneshot).
 * @note  Best fit for sustained high-throughput RX streams because RX DMA stays
 *        active continuously.
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
  /** @brief Zero-copy RX lease descriptor (valid until releaseRx). */
  struct RxLease {
    const uint8_t *data = nullptr;
    size_t len = 0U;

  private:
    friend class Continuous<DBS, FD>;
    void *cookie = nullptr;
  };
  /** @brief Construct continuous wrapper with config. */
  explicit Continuous(const ContinuousConfig &cfg);

  /** @brief Start SIO + DMA + worker thread. */
  msg_t start() override;
  /** @brief Stop DMA, driver, and worker thread. */
  void stop() override;
  /** @brief Update SIO config while preserving DMA request bits. */
  void setConfig(const SIOConfig &cfg);

  /** @brief Set RX callback and context. */
  void setRxCallback(RxHalfCallback cb, void *user);
  /** @brief Receive one queued RX slice without copy; release with releaseRx(). */
  bool receiveRx(RxLease &lease, sysinterval_t timeout = TIME_INFINITE);
  /** @brief Release a previously acquired RX lease. */
  void releaseRx(RxLease &lease);
  /** @brief Read bytes into dst with timeout; returns copied bytes. */
  size_t readTimeout(std::span<uint8_t> dst, sysinterval_t timeout = TIME_INFINITE);
  /** @brief Read bytes into dst, waiting indefinitely for first byte. */
  size_t read(std::span<uint8_t> dst);

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
  using DmaCfgWithOwner = detail::DmaCfgWithOwner<Continuous<DBS, FD>>;
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
  bool rx_idle_enabled_;            ///< Enable RXIDLE-triggered DMA flush.
  thread_t *rx_thread_;             ///< RX worker thread handle.
  size_t rx_dropped_;               ///< Dropped RX slices count.
  ObjectFifo<RxItem, FD> rx_fifo_;  ///< RX FIFO storage.
  RxItem *rx_read_item_;            ///< Current partially consumed RX slot (pull API).
  size_t rx_read_offset_;           ///< Offset in current pull slot.
  alignas(4) uint8_t sio_rxbuf_[DBS]; ///< RX DMA buffer.

protected:
  /** @brief Protected destructor. */
  ~Continuous() override = default;
};

template <size_t N>
inline Buffered<N>::Buffered(const Config &cfg)
    : Base(BaseConfig{
          .driver = cfg.driver,
          .sio_config = cfg.sio_config,
      }) {
  config_.cr3 &= static_cast<uint32_t>(~(USART_CR3_DMAR | USART_CR3_DMAT));
  bsioObjectInit(&bsiop_, &cfg.driver,
                 sio_rxbuf_, sizeof(sio_rxbuf_),
                 sio_txbuf_, sizeof(sio_txbuf_));
}

template <size_t N>
inline msg_t Buffered<N>::start() {
  return bsioStart(&bsiop_, &config_);
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

template <size_t DBS, size_t FD>
inline Continuous<DBS, FD>::Continuous(const ContinuousConfig &cfg)
    : Base(BaseConfig{
          .driver = cfg.driver,
          .sio_config = cfg.sio_config,
      }),
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
      rx_idle_enabled_(cfg.enable_rx_idle),
      rx_thread_(nullptr),
      rx_dropped_(0U),
      rx_fifo_(),
      rx_read_item_(nullptr),
      rx_read_offset_(0U) {
#if !STM32_DMA_USE_ASYNC_TIMOUT
  static_assert(detail::always_false_v<DBS>,
                "SIO::Continuous requires STM32_DMA_USE_ASYNC_TIMOUT");
#endif

  config_.cr3 |= (USART_CR3_DMAR | USART_CR3_DMAT);

  detail::initDmaConfig(rx_cfg_storage_.cfg, cfg.rx_dma_cfg, &Continuous::dmaRxCb,
                        true, DMA_CONTINUOUS_HALF_BUFFER);
  rx_cfg_storage_.owner = this;
  rx_cfg_ = &rx_cfg_storage_.cfg;
  detail::initDmaConfig(tx_cfg_storage_.cfg, cfg.tx_dma_cfg, &Continuous::dmaTxCb,
                        false, DMA_ONESHOT);
  tx_cfg_storage_.owner = this;
  tx_cfg_ = &tx_cfg_storage_.cfg;

  // RXIDLE is managed separately by enable_rx_idle.
  configureEventDispatch(cfg.event_mask & static_cast<sioevents_t>(~SIO_EV_RXIDLE),
                         cfg.event_cb, cfg.event_user);
}

template <size_t DBS, size_t FD>
inline msg_t Continuous<DBS, FD>::start() {
  msg_t msg = sioStart(siop_, &config_);
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

  sioevents_t ev_mask = configuredEventMask();
  if (rx_idle_enabled_) {
    ev_mask |= SIO_EV_RXIDLE;
  }
  if (ev_mask != SIO_EV_NONE) {
    setCallback(&Continuous::sioIdleCb, this);
    writeEnableFlagsX(ev_mask);
  } else {
    setCallback(nullptr, nullptr);
    writeEnableFlagsX(SIO_EV_NONE);
  }

  startRx();

  if ((rx_cb_ != nullptr) && (rx_thread_ == nullptr)) {
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
  if (rx_read_item_ != nullptr) {
    rx_fifo_.returnObject(*rx_read_item_);
    rx_read_item_ = nullptr;
    rx_read_offset_ = 0U;
  }
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
inline bool Continuous<DBS, FD>::receiveRx(RxLease &lease, sysinterval_t timeout) {
  if ((rx_cb_ != nullptr) || (lease.cookie != nullptr) || (rx_read_item_ != nullptr)) {
    return false;
  }
  auto slotOpt = rx_fifo_.receiveObject(timeout);
  if (!slotOpt) {
    return false;
  }
  auto &slot = slotOpt->get();
  lease.data = slot.data;
  lease.len = slot.len;
  lease.cookie = &slot;
  return true;
}

template <size_t DBS, size_t FD>
inline void Continuous<DBS, FD>::releaseRx(RxLease &lease) {
  if (lease.cookie == nullptr) {
    return;
  }
  auto *slot = static_cast<RxItem *>(lease.cookie);
  rx_fifo_.returnObject(*slot);
  lease.data = nullptr;
  lease.len = 0U;
  lease.cookie = nullptr;
}

template <size_t DBS, size_t FD>
inline size_t Continuous<DBS, FD>::readTimeout(std::span<uint8_t> dst, sysinterval_t timeout) {
  if ((rx_cb_ != nullptr) || dst.empty()) {
    return 0U;
  }

  size_t copied = 0U;
  while (copied < dst.size()) {
    if (rx_read_item_ == nullptr) {
      const sysinterval_t wait = (copied == 0U) ? timeout : TIME_IMMEDIATE;
      auto slotOpt = rx_fifo_.receiveObject(wait);
      if (!slotOpt) {
        break;
      }
      rx_read_item_ = &slotOpt->get();
      rx_read_offset_ = 0U;
    }

    auto &slot = *rx_read_item_;
    if (rx_read_offset_ >= slot.len) {
      rx_fifo_.returnObject(slot);
      rx_read_item_ = nullptr;
      rx_read_offset_ = 0U;
      continue;
    }

    const size_t available = slot.len - rx_read_offset_;
    const size_t to_copy = std::min(available, dst.size() - copied);
    std::memcpy(dst.data() + copied, slot.data + rx_read_offset_, to_copy);
    copied += to_copy;
    rx_read_offset_ += to_copy;

    if (rx_read_offset_ == slot.len) {
      rx_fifo_.returnObject(slot);
      rx_read_item_ = nullptr;
      rx_read_offset_ = 0U;
    }
  }

  return copied;
}

template <size_t DBS, size_t FD>
inline size_t Continuous<DBS, FD>::read(std::span<uint8_t> dst) {
  return readTimeout(dst, TIME_INFINITE);
}

template <size_t DBS, size_t FD>
inline void Continuous<DBS, FD>::setConfig(const SIOConfig &cfg) {
  config_ = cfg;
  config_.cr3 |= (USART_CR3_DMAR | USART_CR3_DMAT);
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
  auto *self = detail::ownerFromDma<Continuous<DBS, FD>>(dmap);
  if (self == nullptr) {
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
  auto *self = detail::ownerFromDma<Continuous<DBS, FD>>(dmap);
  if ((self != nullptr) && (self->tx_cb_ != nullptr)) {
    self->tx_cb_(self->tx_user_);
  }
}

template <size_t DBS, size_t FD>
inline void Continuous<DBS, FD>::sioIdleCb(SIODriver *siop) {
  auto *self = static_cast<Continuous *>(siop->arg);
  sioevents_t rearm_mask = SIO_EV_NONE;
  if (self != nullptr) {
    sioevents_t ev = sioGetAndClearEventsX(siop);
    if (((ev & SIO_EV_RXIDLE) != 0U) &&
        self->rx_idle_enabled_ &&
        (dmaGetState(&self->rx_dma_) == DMA_ACTIVE)) {
#if STM32_DMA_USE_ASYNC_TIMOUT
      dmaForceHalfBufferFromISR(&self->rx_dma_);
#endif
    }

    self->dispatchConfiguredEventsI(ev);

    rearm_mask = self->configuredEventMask();
    if (self->rx_idle_enabled_) {
      rearm_mask |= SIO_EV_RXIDLE;
    }
  }
  sioSetEnableFlagsX(siop, rearm_mask);
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
