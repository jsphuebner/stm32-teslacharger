#include "timer_mock.h"

uint32_t rtc_get_counter_val(){
  return Param::Get(Param::test_time) >> 5;
}

bool timer_get_flag(uint32_t timer_peripheral, uint32_t flag){
  return Param::Get(Param::test_timer_flag);
}
float timer_get_ic_value(uint32_t timer_peripheral, enum tim_ic_id ic_id){
  return Param::GetFloat(Param::test_timer_icvalue);
}
