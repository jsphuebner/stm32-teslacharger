#include "timer_mock.h"

uint32_t rtc_get_counter_val(){
  return Param::Get(Param::test_time) >> 5;
}
