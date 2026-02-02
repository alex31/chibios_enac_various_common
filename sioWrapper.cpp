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

} // namespace SIO
