#pragma once
#include <ch.h>
#include <hal.h>



template<size_t WSS, typename T>
class SingletonWorkerThread
{
public:
  enum ExitCode {ERROR_IN_INIT=-10, ERROR_IN_LOOP=-11};

  SingletonWorkerThread(const char *m_name, const tprio_t m_prio) : name(m_name), prio(m_prio) {};
  bool run(sysinterval_t time);
  SingletonWorkerThread& terminate();
  SingletonWorkerThread& join();
  virtual ~SingletonWorkerThread() {terminate().join();};
  
protected:
  static void threadFunc(void *o);
private:

  // this is called in origin thread context
  virtual bool init(void) = 0;
  // this is called in newly created thread context
  virtual bool initInThreadContext(void) {return true;};
  virtual bool loop(void) = 0;

  const char *name;
  const tprio_t prio;
  sysinterval_t timeInLoop;
  
  static thread_t *handle;

  static THD_WORKING_AREA(ws, WSS);
};


  
template<size_t WSS, typename T>
bool SingletonWorkerThread<WSS, T>::run(sysinterval_t m_timeInLoop)
{
  // this will force that only one process can be run
  // in the same time
  if (handle != nullptr)
    return false;
  
  if (init() == false)
    return false;
  
  timeInLoop = m_timeInLoop;
  handle = chThdCreateStatic(ws, sizeof(ws), prio, threadFunc, this);
  chRegSetThreadNameX (handle, name);
  return true;
}

template<size_t WSS, typename T>
SingletonWorkerThread<WSS, T>& SingletonWorkerThread<WSS, T>::join(void)
{
  // this will force that only one process can be run
  // in the same time
  if (handle != nullptr) {
    chThdWait(handle);
    handle = nullptr;
  }
  return *this;
}

template<size_t WSS, typename T>
SingletonWorkerThread<WSS, T>& SingletonWorkerThread<WSS, T>::terminate(void)
{
  // this will force that only one process can be run
  // in the same time, not really a singleton but close. 
  if (handle != nullptr) 
    chThdTerminate(handle);
  
  return *this;
  
}


template<size_t WSS, typename T>
void SingletonWorkerThread<WSS, T>::threadFunc(void *o) {
  T * const self = static_cast<T*>(o);
  if (self->initInThreadContext() == false)
    chThdExit(ERROR_IN_INIT);


  while (!chThdShouldTerminateX()) {
    const systime_t now = chVTGetSystemTimeX();
    const systime_t then = chTimeAddX(now, self->timeInLoop);
    if (self->loop() == false)
      break;
    if (self->timeInLoop)
      chThdSleepUntilWindowed(now, then);
  }
  chThdExit(ERROR_IN_LOOP);
}

template<size_t WSS, typename T>
ALIGNED_VAR(32) stkalign_t SingletonWorkerThread<WSS, T>::ws[THD_WORKING_AREA_SIZE(WSS) /
						    sizeof(stkalign_t)];

template<size_t WSS, typename T>
thread_t *SingletonWorkerThread<WSS, T>::handle = nullptr;
