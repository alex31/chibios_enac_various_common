#include "../windowsAverage.hpp"
#include <cstdio>
#include <cstdint>
#include <iostream>

int main(int argc, char *argv[])
{
  {
    ResizableWindowMedianAverage<int32_t, 6> wa(1);
    for (ssize_t i=0; i<wa.size()+2; i++) {
      wa.push(1100-(i*100));
    }
    for (ssize_t i=0; i<wa.size(); i++) {
      printf("wa[%ld] = %d\n", i, wa[i]);
    }


    wa.resize(4);

    printf ("DEBUG> wa.size() = %ld\n", wa.size());
  
    // for (ssize_t i=0; i<wa.size(); i++) {
    //   wa.push(i);
    // }
    for (ssize_t i=0; i<wa.size(); i++) {
      printf("wa[%ld] = %d\n", i, wa[i]);
    }
    printf("\n\n");
    //  std::for_each(wa.begin(), wa.end(), [](const auto & v) {printf("wa[x] = %d\n", v);});

    wa.resize(6);
    printf ("DEBUG> wa.size() = %ld\n", wa.size());
    for (ssize_t i=0; i<wa.size(); i++) {
      printf("wa[%ld] = %d\n", i, wa[i]);
    }
    wa.push(1000);
    printf ("DEBUG> push 1000\n");
    for (ssize_t i=0; i<wa.size(); i++) {
      printf("wa[%ld] = %d\n", i, wa[i]);
    }
   
    printf ("DEBUG> sum=%d, mean=%d\n", wa.getSum(), wa.getMean());
  }
  
  {
    WindowMedianAverage<int32_t, 6, 1> wa;
    for (ssize_t i=0; i<wa.size()+2; i++) {
      wa.push(1100-(i*100));
    }
    for (ssize_t i=0; i<wa.size(); i++) {
      printf("wa[%ld] = %d\n", i, wa[i]);
    }


    printf ("DEBUG> wa.size() = %ld\n", wa.size());
  
    // for (ssize_t i=0; i<wa.size(); i++) {
    //   wa.push(i);
    // }
    for (ssize_t i=0; i<wa.size(); i++) {
      printf("wa[%ld] = %d\n", i, wa[i]);
    }
    printf("\n\n");
    //  std::for_each(wa.begin(), wa.end(), [](const auto & v) {printf("wa[x] = %d\n", v);});

    printf ("DEBUG> wa.size() = %ld\n", wa.size());
    for (ssize_t i=0; i<wa.size(); i++) {
      printf("wa[%ld] = %d\n", i, wa[i]);
    }
    wa.push(1000);
    printf ("DEBUG> push 1000\n");
    for (ssize_t i=0; i<wa.size(); i++) {
      printf("wa[%ld] = %d\n", i, wa[i]);
    }
   
    printf ("DEBUG> sum=%d, mean=%d\n", wa.getSum(), wa.getMean());
  }
}
