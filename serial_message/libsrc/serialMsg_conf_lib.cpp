#include "serialMsg_conf_lib.hpp"

#ifdef __gnu_linux__
#include <fcntl.h> 
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>


int SystemDependant_posixPipe::fd[2] = {0};

void SystemDependant_posixPipe::initClass(void)
{
  pipe(fd);
}

int SystemDependant_posixSerial::fd = {-1};

static inline constexpr int getLinuxBaudRate(const uint32_t bauds);
bool SystemDependant_posixSerial::initClass(const char* portName, const uint32_t baudRate)
{
  fd = open(portName, O_RDWR | O_NOCTTY | O_SYNC);
  if (fd < 0) {
    std::cerr << "Error opening " <<  portName << std::endl;
    return false;
  }

  /*baudrate 115200, 8 bits, no parity, 1 stop bit */
  if (set_interface_attribs(fd, getLinuxBaudRate(baudRate)) < 0)
    return false;

  fcntl(fd, F_SETFL, 0);
  
  return true;
}


std::thread *SystemDependant_posix::thrPtr = nullptr;
std::mutex   SystemDependant_posix::mtx;

int set_interface_attribs(int fd, int speed)
{
    struct termios tty;

    if (tcgetattr(fd, &tty) < 0) {
        printf("Error from tcgetattr: %s\n", strerror(errno));
        return -1;
    }

    cfsetospeed(&tty, (speed_t)speed);
    cfsetispeed(&tty, (speed_t)speed);

    tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;         /* 8-bit characters */
    tty.c_cflag &= ~PARENB;     /* no parity bit */
    tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
    tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */

    /* setup for non-canonical mode */
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty.c_oflag &= ~OPOST;

    /* fetch bytes as they become available */
    tty.c_cc[VMIN] = 255;
    tty.c_cc[VTIME] = 255;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        printf("Error from tcsetattr: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

static inline constexpr int getLinuxBaudRate(const uint32_t bauds)
{
  switch (bauds) {
  case 50       : return B50     ;    
  case 75       : return B75     ;     
  case 110      : return B110    ;     
  case 134      : return B134    ;    
  case 150      : return B150    ;    
  case 200      : return B200    ;    
  case 300      : return B300    ;     
  case 600      : return B600    ;   
  case 1200     : return B1200   ;   
  case 1800     : return B1800   ;  
  case 2400     : return B2400   ;  
  case 4800     : return B4800   ;  
  case 9600     : return B9600   ;  
  case 19200    : return B19200  ;  
  case 38400    : return B38400  ; 
  case 57600    : return B57600  ; 
  case 115200   : return B115200 ;
  case 230400   : return B230400 ; 
  case 460800   : return B460800 ;
  case 500000   : return B500000 ;
  case 576000   : return B576000 ;
  case 921600   : return B921600 ;
  case 1000000  : return B1000000;
  case 1152000  : return B1152000;
  case 1500000  : return B1500000;
  case 2000000  : return B2000000;
  case 2500000  : return B2500000;
  case 3000000  : return B3000000;
  case 3500000  : return B3500000;
  case 4000000  : return B4000000;
  default :
    std::cerr << "baud rate " << bauds << " not standard, aborting\n";
    abort();
    return 0;
  }
}
#endif

#if defined(__ARM_EABI__) && __ARM_EABI__ 
THD_WORKING_AREA(SystemDependant_chibios::waThread, 4096);
mutex_t  SystemDependant_chibios::mtx = _MUTEX_DATA(mtx);
thread_t *SystemDependant_chibios::thd = nullptr;

#if HAL_USE_SERIAL  
SerialDriver *SystemDependant_chibiosSerial::sd = nullptr;
#endif

#if HAL_USE_UART 
UARTDriver *SystemDependant_chibiosUART::ud = nullptr;
#endif
//uint8_t SystemDependant_chibiosUART::lostBuffer[pipeLen] = {0};
// CircularBuffer SystemDependant_chibiosUART::lost = {
//   .writePointer = 0,
//   .readPointer = 0,
//   .size = pipeLen,
//   .keys = SystemDependant_chibiosUART::lostBuffer
// };

uint8_t SystemDependant_chibiosUART::ringBuffer[pipeLen] = {0};
CircularBuffer SystemDependant_chibiosUART::ring = {
  .writePointer = 0,
  .readPointer = 0,
  .size = pipeLen,
  .keys = SystemDependant_chibiosUART::ringBuffer
};
Pipe SystemDependant_chibiosUART::pipe =  _PIPE_DATA(SystemDependant_chibiosUART::pipe,
						     &SystemDependant_chibiosUART::ring);

#ifndef USART_CR2_STOP1_BITS 
#define USART_CR2_STOP1_BITS    (0 << 12)
#endif

void SystemDependant_chibiosUART::initClass(UARTDriver &_ud, const uint32_t baud)
{
  static THD_WORKING_AREA(waUartPumpThd, 256);
   static const UARTConfig uartConfig =  {
    .txend1_cb =nullptr,
    .txend2_cb = nullptr,
    .rxend_cb = nullptr,
    .rxchar_cb = nullptr,
    // .rxchar_cb = [] (UARTDriver *ud, uint16_t c) {
    //    (void) ud;
    //    const uint8_t c8 = static_cast<uint8_t>(c);
    //    ringBufferEnqueBuffer(&lost, &c8, 1);
    //    palSetLine(LINE_C02_DBG_LED);
    //  },
    .rxerr_cb = nullptr,
    .speed = baud,
    .cr1 = 0,
    .cr2 = USART_CR2_STOP1_BITS | USART_CR2_LINEN,
    .cr3 = 0
  };

 ud = &_ud;
 uartStart(ud, &uartConfig);
   
 chThdCreateStatic(waUartPumpThd, sizeof(waUartPumpThd), NORMALPRIO+1,
		   &uartPumpThd, NULL);
}

void SystemDependant_chibiosUART::uartPumpThd(void *arg)
{
  (void) arg;
  static uint8_t dmaBuffer[pipeLen];
  //  static uint8_t isrBuffer[pipeLen];
  chRegSetThreadName("uartPumpThd");
  while (true) {
    size_t retv = pipeLen;
    //size_t lostBytes;

    // if ((lostBytes = ringBufferUsedSize(&lost)) != 0) {
    //   ringBufferDequeBuffer(&lost, isrBuffer, lostBytes);
    //   pipeWrite(&pipe, isrBuffer, lostBytes);
    //   palClearLine(LINE_C02_DBG_LED);
    // }

    uartReceiveTimeout (ud, &retv, dmaBuffer, TIME_MS2I(10));
    if (retv != 0) {
      pipeWrite(&pipe, dmaBuffer, retv);
    }
  }
}

#endif































