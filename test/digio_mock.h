#include "../libopeninv/include/my_fp.h"
#include <stdio.h>
#include "../include/digio_prj.h"
#include "../include/anain_prj.h"
#ifndef TEST_MOCK_H
#define TEST_MOCK_H
class Can {
  public:
   void Send(uint32_t canId, uint32_t data[2]) { Send(canId, data, 8); }
   void Send(uint32_t canId, uint32_t data[2], uint8_t len) {} ;
};
//void Can::Send(uint32_t canId, uint32_t data[2], uint8_t len){ };


class DigIo{
  public:
   #define DIG_IO_ENTRY(name, port, pin, mode) static DigIo name;
   DIG_IO_LIST
   #undef DIG_IO_ENTRY

    bool val = 0;

    bool Get() { return val; }
    void Set() { val = 1;}
    void Clear() { val = 0;}
};

class AnaIn{
  public:
    #define ANA_IN_ENTRY(name, port, pin) static AnaIn name;
    ANA_IN_LIST
    #undef ANA_IN_ENTRY

    bool Get() { return 1; }
    void Set() {}
};


#endif
