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


SerialDriver *SystemDependant_chibiosSerial::sd = nullptr;


#endif































