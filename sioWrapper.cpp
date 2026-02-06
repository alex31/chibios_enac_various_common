#include "sioWrapper.hpp"

namespace SIO {

Base::Base(const BaseConfig &cfg)
    : siop_(&cfg.driver),
      config_(nullptr),
      bus_mutex_{} {
  if (cfg.sio_config != nullptr) {
    config_storage_ = *cfg.sio_config;
    config_ = &config_storage_;
  }
  chMtxObjectInit(&bus_mutex_);
}

Base::~Base() = default;

SIODriver &Base::driver() { return *siop_; }

const SIODriver &Base::driver() const { return *siop_; }

const SIOConfig *Base::config() const { return config_; }

void Base::acquireBus() {
  chMtxLock(&bus_mutex_);
}

void Base::releaseBus() {
  chMtxUnlock(&bus_mutex_);
}

void Base::setConfig(const SIOConfig &cfg) {
  config_storage_ = cfg;
  config_ = &config_storage_;
}

void Base::configureEventDispatch(sioevents_t mask, EventCallbackI cb, void *user) {
  event_mask_ = (cb != nullptr) ? mask : SIO_EV_NONE;
  event_cb_ = cb;
  event_user_ = user;
}

sioevents_t Base::configuredEventMask() const {
  return event_mask_;
}

void Base::dispatchConfiguredEventsI(sioevents_t events) const {
  if ((event_cb_ == nullptr) || (events == SIO_EV_NONE)) {
    return;
  }
  const sioevents_t selected = events & event_mask_;
  if (selected != SIO_EV_NONE) {
    event_cb_(selected, event_user_);
  }
}

void Base::setCallback(siocb_t cb, void *arg) {
  siop_->arg = arg;
  sioSetCallbackX(siop_, cb);
}

void Base::writeEnableFlags(sioevents_t mask) {
  sioWriteEnableFlags(siop_, mask);
}

void Base::writeEnableFlagsX(sioevents_t mask) {
  sioWriteEnableFlagsX(siop_, mask);
}

void Base::setEnableFlags(sioevents_t mask) {
  sioSetEnableFlags(siop_, mask);
}

void Base::setEnableFlagsX(sioevents_t mask) {
  sioSetEnableFlagsX(siop_, mask);
}

void Base::clearEnableFlags(sioevents_t mask) {
  sioClearEnableFlags(siop_, mask);
}

void Base::clearEnableFlagsX(sioevents_t mask) {
  sioClearEnableFlagsX(siop_, mask);
}

sioevents_t Base::getAndClearErrors() {
  return sioGetAndClearErrors(siop_);
}

sioevents_t Base::getAndClearEvents() {
  return sioGetAndClearEvents(siop_);
}

sioevents_t Base::getEvents() const {
  return sioGetEvents(siop_);
}

size_t Base::asyncRead(uint8_t *buffer, size_t n) {
  return sioAsyncRead(siop_, buffer, n);
}

size_t Base::asyncWrite(const uint8_t *buffer, size_t n) {
  return sioAsyncWrite(siop_, buffer, n);
}

msg_t Base::synchronizeRX(sysinterval_t timeout) {
  return sioSynchronizeRX(siop_, timeout);
}

msg_t Base::synchronizeRXIdle(sysinterval_t timeout) {
  return sioSynchronizeRXIdle(siop_, timeout);
}

msg_t Base::synchronizeTX(sysinterval_t timeout) {
  return sioSynchronizeTX(siop_, timeout);
}

msg_t Base::synchronizeTXEnd(sysinterval_t timeout) {
  return sioSynchronizeTXEnd(siop_, timeout);
}

Datagram::Datagram(const Config &cfg)
    : Base(BaseConfig{
          .driver = cfg.driver,
          .sio_config = cfg.sio_config,
      }),
      rx_dma_{},
      tx_dma_{},
      rx_cfg_storage_{},
      tx_cfg_storage_{},
      rx_cfg_(nullptr),
      tx_cfg_(nullptr),
      rx_idle_enabled_(cfg.enable_rx_idle),
      txend_cb_(cfg.txend_cb),
      txend_user_(cfg.txend_user),
      rxend_cb_(cfg.rxend_cb),
      rxend_user_(cfg.rxend_user) {
  if (config_ != nullptr) {
    config_storage_.cr3 |= (USART_CR3_DMAR | USART_CR3_DMAT);
  }
  detail::initDmaConfig(rx_cfg_storage_.cfg, cfg.rx_dma_cfg, &Datagram::dmaRxCb,
                        true, DMA_ONESHOT);
  rx_cfg_storage_.owner = this;
  rx_cfg_ = &rx_cfg_storage_.cfg;
  detail::initDmaConfig(tx_cfg_storage_.cfg, cfg.tx_dma_cfg, &Datagram::dmaTxCb,
                        false, DMA_ONESHOT);
  tx_cfg_storage_.owner = this;
  tx_cfg_ = &tx_cfg_storage_.cfg;
}

msg_t Datagram::start() {
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

  sioevents_t mask = SIO_EV_ALL_ERRORS;
  if (rx_idle_enabled_) {
    mask |= SIO_EV_RXIDLE;
  }
  if (mask != SIO_EV_NONE) {
    setCallback(&Datagram::sioIdleCb, this);
    writeEnableFlagsX(mask);
  }

  return HAL_RET_SUCCESS;
}

void Datagram::stop() {
  setCallback(nullptr, nullptr);
  writeEnableFlagsX(SIO_EV_NONE);
  dmaStop(&tx_dma_);
  dmaStop(&rx_dma_);
  sioStop(siop_);
}

void Datagram::setConfig(const SIOConfig &cfg) {
  config_storage_ = cfg;
  config_storage_.cr3 |= (USART_CR3_DMAR | USART_CR3_DMAT);
  config_ = &config_storage_;
}

size_t Datagram::writeTimeout(const uint8_t *buffer, size_t n, sysinterval_t timeout) {
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

size_t Datagram::write(const uint8_t *buffer, size_t n) {
  return writeTimeout(buffer, n, TIME_INFINITE);
}

size_t Datagram::readTimeout(uint8_t *buffer, size_t n, sysinterval_t timeout) {
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

size_t Datagram::read(uint8_t *buffer, size_t n) {
  return readTimeout(buffer, n, TIME_INFINITE);
}

bool Datagram::writeI(const uint8_t *buffer, size_t n) {
  osalDbgCheckClassI();
  osalDbgCheck((buffer != nullptr) && (n > 0U));
  osalDbgAssert(!txBusy(), "TX DMA busy");
  if ((buffer == nullptr) || (n == 0U) || txBusy()) {
    return false;
  }
  return dmaStartTransfertI(&tx_dma_, &driver().usart->TDR,
                            const_cast<uint8_t *>(buffer), n);
}

bool Datagram::readI(uint8_t *buffer, size_t n) {
  osalDbgCheckClassI();
  osalDbgCheck((buffer != nullptr) && (n > 0U));
  osalDbgAssert(!rxBusy(), "RX DMA busy");
  if ((buffer == nullptr) || (n == 0U) || rxBusy()) {
    return false;
  }
  return dmaStartTransfertI(&rx_dma_, &driver().usart->RDR, buffer, n);
}

bool Datagram::txBusy() const {
  return (dmaGetState(const_cast<DMADriver *>(&tx_dma_)) == DMA_ACTIVE);
}

bool Datagram::rxBusy() const {
  return (dmaGetState(const_cast<DMADriver *>(&rx_dma_)) == DMA_ACTIVE);
}

void Datagram::dmaRxCb(DMADriver *dmap, void *buffer, const size_t n) {
  (void)buffer;
  (void)n;
  auto *self = detail::ownerFromDma<Datagram>(dmap);
  if ((self != nullptr) && (self->rxend_cb_ != nullptr)) {
    self->rxend_cb_(self->rxend_user_);
  }
}

void Datagram::dmaTxCb(DMADriver *dmap, void *buffer, const size_t n) {
  (void)buffer;
  (void)n;
  auto *self = detail::ownerFromDma<Datagram>(dmap);
  if ((self != nullptr) && (self->txend_cb_ != nullptr)) {
    self->txend_cb_(self->txend_user_);
  }
}

void Datagram::sioIdleCb(SIODriver *siop) {
  auto *self = static_cast<Datagram *>(siop->arg);
  if (self != nullptr) {
    const sioevents_t ev = sioGetAndClearEventsX(siop);
    if (((ev & SIO_EV_RXIDLE) != 0U) && self->rx_idle_enabled_) {
      if (dmaGetState(&self->rx_dma_) == DMA_ACTIVE) {
        chSysLockFromISR();
        dmaStopTransfertI(&self->rx_dma_);
        chSysUnlockFromISR();
      }
    }
    if ((ev & SIO_EV_ALL_ERRORS) != 0U) {
      if (dmaGetState(&self->rx_dma_) == DMA_ACTIVE) {
        chSysLockFromISR();
        dmaStopTransfertI(&self->rx_dma_);
        chSysUnlockFromISR();
      } else {
        // Generic fallback: drain stale bytes through SIO API only.
        uint8_t sink[16];
        while (!sioIsRXEmptyX(siop)) {
          const size_t drained = sioAsyncReadX(siop, sink, sizeof(sink));
          if (drained == 0U) {
            break;
          }
        }
      }
    }
    sioevents_t mask = SIO_EV_ALL_ERRORS;
    if (self->rx_idle_enabled_) {
      mask |= SIO_EV_RXIDLE;
    }
    sioSetEnableFlagsX(siop, mask);
  }
}

} // namespace SIO
