#pragma once
#include <cstdint>
#include <cstddef>
#include <array>
#include <type_traits>


// our checksum function
template <size_t N>
uint16_t fletcher16(const std::array<uint8_t, N> &buffer,  const size_t len=0UL);


#if defined(_CHIBIOS_RT_)
#include "stdutil.h"
#endif


/*
#                 _        _                                  
#                | |      (_)                                 
#                | |       _    _ __    _   _  __  __         
#                | |      | |  | '_ \  | | | | \ \/ /         
#                | |____  | |  | | | | | |_| |  >  <          
#                |______| |_|  |_| |_|  \__,_| /_/\_\         
*/
#ifdef __gnu_linux__
#include <thread>
#include <mutex>
#include <iostream>
#include <unistd.h>
#include <assert.h>

int set_interface_attribs(int fd, int speed);
/*
#                 _____    _    _ __                 
#                |  __ \  (_)  | '_ \                
#                | |__) |  _   | |_) |   ___         
#                |  ___/  | |  | .__/   / _ \        
#                | |      | |  | |     |  __/        
#                |_|      |_|  |_|      \___|        
*/

class SystemDependant_posixPipe {
public:
// static class; object cannot be instanced
  SystemDependant_posixPipe() = delete;

  // just need to get the type of the fucntion return value
  // in case of custom chekcsum function, just put the type
  using Checksum_t = std::result_of<decltype(fletcher16<1>) &
				    (std::array<uint8_t, 1>, size_t)>::type;

  // member method
  static void initClass(void);
  
  template <size_t N>
  static void write(const std::array<uint8_t, N> &wbuffer, size_t len=0UL);
  
  template <size_t N>
  static void read(std::array<uint8_t, N> &rbuffer, size_t len=0UL);

  // we use an already defined checksum function, but feel free to implement yours instead of
  // fletcher16
  template <size_t N>
  static Checksum_t checksum(const std::array<uint8_t, N> &buffer,  const size_t len=0UL) {
    return fletcher16(buffer, len);
  };
  

  // member variable
  static int fd[2];
  constexpr static const int& fdread = fd[0];
  constexpr static const int& fdwrite = fd[1];
};

template <size_t N>
void SystemDependant_posixPipe::write(const std::array<uint8_t, N> &wbuffer, size_t len)
{
  len = len ? std::min(len, N) : N;
  //  std::cout << "really send " << len << " on interface\n";
  const ssize_t retv = ::write(fdwrite, wbuffer.data(), len);
  if (retv != static_cast<ssize_t>(len)) {
    std::cerr << "error write " << retv << " bytes instead of " << len << std::endl;
  }
}

template <size_t N>
void SystemDependant_posixPipe::read(std::array<uint8_t, N> &rbuffer, size_t len)
{
  len = len ? std::min(len, N) : N;
  //  std::cout << "really read " << len << " on interface\n";
  const ssize_t retv = ::read(fdread, rbuffer.data(), len);
  if (retv !=  static_cast<ssize_t>(len)) {
    std::cerr << "error read " << retv << " bytes instead of " << len << std::endl;
  }
}

/*
#                 ______                 _            _          
#                /  ____|               (_)          | |         
#                | (___     ___   _ __   _     __ _  | |         
#                 \___ \   / _ \ | '__| | |   / _` | | |         
#                .____) | |  __/ | |    | |  | (_| | | |         
#                \_____/   \___| |_|    |_|   \__,_| |_|         
*/
class SystemDependant_posixSerial {
public:
// static class; object cannot be instanced
  SystemDependant_posixSerial() = delete;

  // just need to get the type of the fucntion return value
  // in case of custom chekcsum function, just put the type
  using Checksum_t = std::result_of<decltype(fletcher16<1>) &
				    (std::array<uint8_t, 1>, size_t)>::type;

  // member method
  static bool initClass(const char* portName, const uint32_t baudRate);
  
  template <size_t N>
  static void write(const std::array<uint8_t, N> &wbuffer, size_t len=0UL);
  
  template <size_t N>
  static void read(std::array<uint8_t, N> &rbuffer, size_t len=0UL);

  // we use an already defined checksum function, but feel free to implement yours instead of
  // fletcher16
  template <size_t N>
  static Checksum_t checksum(const std::array<uint8_t, N> &buffer,  const size_t len=0UL) {
    return fletcher16(buffer, len);
  };
  

  // member variable
  static int fd;
};

template <size_t N>
void SystemDependant_posixSerial::write(const std::array<uint8_t, N> &wbuffer, size_t len)
{
  len = len ? std::min(len, N) : N;
  //  std::cout << "really send " << len << " on interface\n";
  const ssize_t retv = ::write(fd, wbuffer.data(), len);
  if (retv != static_cast<ssize_t>(len)) {
    std::cerr << "error write " << retv << " bytes instead of " << len << std::endl;
  }
}

template <size_t N>
void SystemDependant_posixSerial::read(std::array<uint8_t, N> &rbuffer, size_t len)
{
  len = len ? std::min(len, N) : N;
  //  std::cout << "really read " << len << " on interface\n";
  const ssize_t retv = ::read(fd, rbuffer.data(), len);
  if (retv !=  static_cast<ssize_t>(len)) {
    std::cerr << "error read " << retv << " bytes instead of " << len << std::endl;
  }
}



/*                                                                                          
#                 _____                   _                                                 
#                |  __ \                 (_)                                                
#                | |__) |   ___    ___    _   __  __                                        
#                |  ___/   / _ \  / __|  | |  \ \/ /         
#                | |      | (_) | \__ \  | |   >  <          
#                |_|       \___/  |___/  |_|  /_/\_\         
*/

class SystemDependant_posix {
  using ThreadEntryFn_t = void (*)(void);
public:
  // static class; object cannot be instanced
  SystemDependant_posix() = delete;

  // member method
  static void initClass(void) {};
  static void launch(ThreadEntryFn_t fn) { thrPtr = new std::thread(fn);};
  static void lock(void) {mtx.lock();};
  static void unlock(void)  {mtx.unlock();};
  static void abort(const char* abortMsg)  {(void) abortMsg; ::abort();};
  static void assertm(const bool cond, const char* abortMsg)  {
    if (not cond) {
      std::cerr << abortMsg << std::endl;
      exit(-1);
    };
  };
private:
  // member variable
  static std::thread *thrPtr;
  static std::mutex mtx;
};
#endif


template <size_t N>
uint16_t fletcher16(const std::array<uint8_t, N> &buffer, const size_t len) {
  uint16_t sum1 = 0xff, sum2 = 0xff;
  
  size_t bytes = len ? std::min(len, N) : N;
  auto data = buffer.begin();
  
  while (bytes) {
    size_t tlen = bytes > 20 ? 20 : bytes;
    bytes -= tlen;
    do {
      sum1 = (uint16_t) (sum1 + *data++);
      sum2 =  (uint16_t) (sum2 + sum1);
    } while (--tlen);
    sum1 = (uint16_t) ((sum1 & 0xff) + (sum1 >> 8));
    sum2 = (uint16_t) ((sum2 & 0xff) + (sum2 >> 8));
  }
  /* Second reduction step to reduce sums to 8 bits */
  sum1 = (uint16_t) ((sum1 & 0xff) + (sum1 >> 8));
  sum2 = (uint16_t) ((sum2 & 0xff) + (sum2 >> 8));
  return (uint16_t) ((sum2 % 0xff) << 8) | (sum1 % 0xff);
};

/*
#                  _____   _       _    _       _                         
#                 / ____| | |     (_)  | |     (_)                        
#                | |      | |__    _   | |__    _     ___    ___          
#                | |      | '_ \  | |  | '_ \  | |   / _ \  / __|         
#                | |____  | | | | | |  | |_) | | |  | (_) | \__ \         
#                 \_____| |_| |_| |_|  |_.__/  |_|   \___/  |___/         
*/
#if defined(__ARM_EABI__) && __ARM_EABI__ 
#include "ch.h"
#include "hal.h"
#include "pipe.h"
/*
#                 _____    _______         
#                |  __ \  |__   __|        
#                | |__) |    | |           
#                |  _  /     | |           
#                | | \ \     | |           
#                |_|  \_\    |_|           
*/

class SystemDependant_chibios {
  using ThreadEntryFn_t = void (*)(void);
public:
  // static class; object cannot be instanced
  SystemDependant_chibios() = delete;

  // member method
  static void initClass(void) {};
  static void launch(ThreadEntryFn_t fn) {
    thd = chThdCreateStatic(waThread, sizeof(waThread), NORMALPRIO,
			    [] (void *arg) {
			       chRegSetThreadName("mill messages");
			      reinterpret_cast<ThreadEntryFn_t>(arg)();
			    }
			    , reinterpret_cast<void *>(fn));
  };
  static void lock(void) {chMtxLock(&mtx);};
  static void unlock(void) {chMtxUnlock(&mtx);};
  static void abort(const char* abortMsg)  {chSysHalt(abortMsg);};
  static void assertm(const bool cond, const char* abortMsg)  {
    (void) abortMsg;
    chDbgAssert(cond, abortMsg);
  };
  
private:
  // member variable
  static THD_WORKING_AREA(waThread, 4096);
  static mutex_t  mtx;
  static thread_t *thd;
};
/*
#                 ______                 _            _          
#                /  ____|               (_)          | |         
#                | (___     ___   _ __   _     __ _  | |         
#                 \___ \   / _ \ | '__| | |   / _` | | |         
#                .____) | |  __/ | |    | |  | (_| | | |         
#                \_____/   \___| |_|    |_|   \__,_| |_|         
*/
#if HAL_USE_SERIAL
class SystemDependant_chibiosSerial {
public:
// static class; object cannot be instanced
  SystemDependant_chibiosSerial() = delete;

  // just need to get the type of the fucntion return value
  // in case of custom chekcsum function, just put the type
  using Checksum_t = std::result_of<decltype(fletcher16<1>) &
				    (std::array<uint8_t, 1>, size_t)>::type;

  // member method
  static void initClass(SerialDriver &_sd) {sd = &_sd;};
  
  template <size_t N>
  static void write(const std::array<uint8_t, N> &wbuffer, size_t len=0UL);
  
  template <size_t N>
  static void read(std::array<uint8_t, N> &rbuffer, size_t len=0UL);

  // we use an already defined checksum function, but feel free to implement yours instead of
  // fletcher16
  template <size_t N>
  static Checksum_t checksum(const std::array<uint8_t, N> &buffer,  const size_t len=0UL) {
    return fletcher16(buffer, len);
  };
  

  // member variable
  static  SerialDriver *sd;
};

template <size_t N>
void SystemDependant_chibiosSerial::write(const std::array<uint8_t, N> &wbuffer, size_t len)
{
  len = len ? std::min(len, N) : N;
  //  std::cout << "really send " << len << " on interface\n";
  const size_t retv = sdWrite(sd, wbuffer.data(), len);
  if (retv != len) {
    chSysHalt("SystemDependant_chibiosSerial::write error");
  }
}

template <size_t N>
void SystemDependant_chibiosSerial::read(std::array<uint8_t, N> &rbuffer, size_t len)
{
  len = len ? std::min(len, N) : N;
  //  std::cout << "really read " << len << " on interface\n";
  const size_t retv = sdRead(sd, rbuffer.data(), len);
  if (retv != len) {
    chSysHalt("SystemDependant_chibiosSerial::read error");
  }
}
#endif // HAL_USE_SERIAL

#if HAL_USE_SERIAL_USB
#include "usb_serial.h"
class SystemDependant_chibiosUsbSerial {
public:
// static class; object cannot be instanced
  SystemDependant_chibiosUsbSerial() = delete;

  // just need to get the type of the fucntion return value
  // in case of custom chekcsum function, just put the type
  using Checksum_t = std::result_of<decltype(fletcher16<1>) &
				    (std::array<uint8_t, 1>, size_t)>::type;

  // member method
  static void initClass(SerialUSBDriver *_sdu, USBDriver& usbd) {
    sdu = _sdu;
    usbSerialInit(sdu, &usbd);
      while (not isUsbConnected()) {
    chThdSleepMilliseconds(1);
  }
  };
  
  template <size_t N>
  static void write(const std::array<uint8_t, N> &wbuffer, size_t len=0UL);
  
  template <size_t N>
  static void read(std::array<uint8_t, N> &rbuffer, size_t len=0UL);

  // we use an already defined checksum function, but feel free to implement yours instead of
  // fletcher16
  template <size_t N>
  static Checksum_t checksum(const std::array<uint8_t, N> &buffer,  const size_t len=0UL) {
    return fletcher16(buffer, len);
  };
  

  // member variable
  static  SerialUSBDriver *sdu;
};

template <size_t N>
void SystemDependant_chibiosUsbSerial::write(const std::array<uint8_t, N> &wbuffer, size_t len)
{
  len = len ? std::min(len, N) : N;
  //  std::cout << "really send " << len << " on interface\n";
  const size_t retv = chnWrite(sdu, wbuffer.data(), len);
  if (retv != len) {
    chSysHalt("SystemDependant_chibiosUsbSerial::write error");
  }
}

template <size_t N>
void SystemDependant_chibiosUsbSerial::read(std::array<uint8_t, N> &rbuffer, size_t len)
{
  len = len ? std::min(len, N) : N;
  //  std::cout << "really read " << len << " on interface\n";
  const size_t retv = chnRead(sdu, rbuffer.data(), len);
  if (retv != len) {
    chSysHalt("SystemDependant_chibiosUsbSerial::read error");
  }
}
#endif // HAL_USE_SERIAL

/*
#                 _    _                  _            
#                | |  | |                | |           
#                | |  | |   __ _   _ __  | |_          
#                | |  | |  / _` | | '__| | __|         
#                | |__| | | (_| | | |    \ |_          
#                 \____/   \__,_| |_|     \__|         
*/
#if HAL_USE_UART
 class SystemDependant_chibiosUART {
public:
// static class; object cannot be instanced
  SystemDependant_chibiosUART() = delete;

  // just need to get the type of the fucntion return value
  // in case of custom chekcsum function, just put the type
  using Checksum_t = std::result_of<decltype(fletcher16<1>) &
				    (std::array<uint8_t, 1>, size_t)>::type;

  // member method
  static void initClass(UARTDriver &_ud, const uint32_t baud);
  
  template <size_t N>
  static void write(const std::array<uint8_t, N> &wbuffer, size_t len=0UL);
  
  template <size_t N>
  static void read(std::array<uint8_t, N> &rbuffer, size_t len=0UL);

  // we use an already defined checksum function, but feel free to implement yours instead of
  // fletcher16
  template <size_t N>

  static Checksum_t checksum(const std::array<uint8_t, N> &buffer,  const size_t len=0UL) {
    return fletcher16(buffer, len);
  };
  
  
  // member variable
private:
  static  UARTDriver *ud;
  static constexpr size_t pipeLen = 512U;
  //  static uint8_t lostBuffer[pipeLen];
  //  static CircularBuffer lost;

  static uint8_t ringBuffer[pipeLen];
  static CircularBuffer ring;
  static Pipe pipe;

  // member method
  static void uartPumpThd(void *arg);
};


template <size_t N>
void SystemDependant_chibiosUART::write(const std::array<uint8_t, N> &wbuffer, size_t len)
{
  len = len ? std::min(len, N) : N;
  //  std::cout << "really send " << len << " on interface\n";
  //  DebugTrace("send %u b.", len);
  size_t retv = len;
  const msg_t status = uartSendTimeout (ud, &retv, wbuffer.data(), TIME_INFINITE);
  if ((status != MSG_OK) or (retv != len)) {
    chSysHalt("SystemDependant_chibiosUART::write error");
  }
}

template <size_t N>
void SystemDependant_chibiosUART::read(std::array<uint8_t, N> &rbuffer, size_t len)
{
  len = len ? std::min(len, N) : N;
  //  std::cout << "really read " << len << " on interface\n";
  const size_t retv = pipeRead (&pipe, rbuffer.data(), len);
  if (retv != len) {
    chSysHalt("SystemDependant_chibiosUART::read error");
  }
}
#endif  // HAL_USE_UART

#endif

