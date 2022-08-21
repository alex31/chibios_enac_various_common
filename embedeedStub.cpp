#include <ch.h>
#include <hal.h>

extern "C" {
  __attribute__((weak, noreturn))
  void __cxa_pure_virtual(void) {
    osalSysHalt("pure virtual");
    while(true);
  }
}

namespace std {
  __attribute__((weak, noreturn))
  void __throw_bad_function_call() {
    osalSysHalt("bad function call");
    while(true);
  }
}

