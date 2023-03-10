#ifndef TEST_LOGIC_H
#define TEST_LOGIC_H
#endif
#include <assert.h>
#include "test_common.h"

void test_calc_totals()
{

  // simple idc addition test
  Param::SetInt(Param::c1idc, 12);
  Param::SetInt(Param::c2idc, 13);
  Param::SetInt(Param::c3idc, 11);

  CalcTotals();

  s32fp idc = Param::Get(Param::idc);
  assert(36 == FP_TOINT(idc));

  // udcmax test
  Param::SetInt(Param::c1udc, 365);
  Param::SetInt(Param::c2udc, 364);
  Param::SetInt(Param::c3udc, 366);

  CalcTotals();

  s32fp udcmax = Param::Get(Param::udc);
  assert(366 == FP_TOINT(udcmax));
}

// TIMER NEEDED
void test_check_timeout()
{
  bool res;
  startTime = 11;
  
  //above limit
  Param::SetInt(Param::test_time, 14 * 60);
  Param::SetInt(Param::timelim, 13);
  res = CheckTimeout();
  assert(res);

  //below limit
  Param::SetInt(Param::test_time, 12 * 60);
  res = CheckTimeout();
  assert(!res);

  //start time changes result
  startTime = 1100;
  res = CheckTimeout();
  assert(res);
}

void test_check_delay()
{
  bool res;
  startTime = 11;
  
  //after delay
  Param::SetInt(Param::test_time, 14 * 60);
  Param::SetInt(Param::timedly, 13);
  res = CheckDelay();
  assert(res);

  //before delay 
  Param::SetInt(Param::test_time, 12 * 60);
  res = CheckDelay();
  assert(!res);

  //start time changes result
  startTime = 1100;
  res = CheckDelay();
  assert(res);
}


void test_evse_read()
{
  // INP_MANUAL
  Param::SetInt(Param::inputype, INP_MANUAL);
  EvseRead();
  assert(0 == Param::GetInt(Param::proximity));
  assert(32 == Param::GetInt(Param::cablelim));

  // INP_TYPE1
  //under threshold
  Param::SetInt(Param::inputype, INP_TYPE1);
  EvseRead();
  assert(1 == Param::GetInt(Param::proximity));
  assert(40 == Param::GetInt(Param::cablelim));

  //above
  Param::SetInt(Param::inputype, INP_TYPE1);
  AnaIn::cablelim.Set(2222);
  EvseRead();
  assert(0 == Param::GetInt(Param::proximity));
  assert(0 == Param::GetInt(Param::cablelim));

  // INP_TYPE2 and variations
  //above threshold
  Param::SetInt(Param::inputype, INP_TYPE2);
  AnaIn::cablelim.Set(4222);
  EvseRead();
  assert(0 == Param::GetInt(Param::proximity));
  assert(0 == Param::GetInt(Param::cablelim));

  // under threshold (example)
  Param::SetInt(Param::inputype, INP_TYPE2);
  AnaIn::cablelim.Set(2822);
  EvseRead();
  assert(1 == Param::GetInt(Param::proximity));
  assert(20 == Param::GetInt(Param::cablelim));
}

void test_check_start_condition()
{
  bool res;
  //not met by default
  Param::SetInt(Param::inputype, INP_TYPE1);
  Param::SetInt(Param::cablelim, 0);
  Param::SetInt(Param::proximity, 0);
  assert(IsEvseInput());
  res = CheckStartCondition();
  assert(!res);

  //enable override not in EVSE type
  Param::SetInt(Param::enable, 1);
  assert(IsEvseInput());
  res = CheckStartCondition();
  assert(!res);

  //enable override in manual type
  Param::SetInt(Param::inputype, INP_MANUAL);
  assert(!IsEvseInput());
  res = CheckStartCondition();
  assert(res);

  //EVSE start condition met
  Param::SetInt(Param::inputype, INP_TYPE2_3P);
  Param::SetInt(Param::enable, 1);
  Param::SetInt(Param::proximity, 1);
  Param::SetInt(Param::cablelim, 32);
  res = CheckStartCondition();
  assert(res);

  //EVSE start condition not met cablelim
  Param::SetInt(Param::inputype, INP_TYPE2_3P);
  Param::SetInt(Param::enable, 1);
  Param::SetInt(Param::proximity, 1);
  Param::SetInt(Param::cablelim, 0);
  res = CheckStartCondition();
  assert(!res);
}

void test_check_voltage(){
  bool res;
  Param::SetInt(Param::udc, 400);
  Param::SetInt(Param::udclim, 390);

  // tolerate up to ten one off spikes
  res = CheckVoltage();
  assert(!res);

  // drop out if received more than ten 
  for(int i =0; i < 15;i++){
    res = CheckVoltage();
  }
  assert(res);
}

void test_reset_values_in_off_mode(){
  Param::SetInt(Param::c1flag, 1);
  Param::SetInt(Param::c2tmp2, 15);
  Param::SetInt(Param::c3stt, 15);
  Param::SetInt(Param::c3uac, 239);
  assert(1 == Param::GetInt(Param::c1flag));
  ResetValuesInOffMode();
  assert(0 == Param::GetInt(Param::c1flag));
  assert(0 == Param::GetInt(Param::c2tmp2));
  assert(0 == Param::GetInt(Param::c3stt));
  assert(0 == Param::GetInt(Param::c3uac));
}

void test_calc_enable(){
  
  int enable;

  //default off
  Param::SetInt(Param::enable, 0);
  Param::SetInt(Param::enablepol, 0); // polarity switch for DigIo
  Param::SetInt(Param::cancontrol, 0);
  Param::SetInt(Param::canenable, 0);
  DigIo::enable_in.Clear();
  CalcEnable();
  enable = Param::GetInt(Param::enable);
  assert(!enable);

  //only DigIo high -> on
  Param::SetInt(Param::enable, 0);
  Param::SetInt(Param::enablepol, 0);
  Param::SetInt(Param::cancontrol, 0);
  Param::SetInt(Param::canenable, 0);
  DigIo::enable_in.Set();
  CalcEnable();
  enable = Param::GetInt(Param::enable);
  assert(enable);

  //DigIo and enablepol high -> off
  Param::SetInt(Param::enable, 0);
  Param::SetInt(Param::enablepol, 1);
  Param::SetInt(Param::cancontrol, 0);
  Param::SetInt(Param::canenable, 0);
  DigIo::enable_in.Set();
  CalcEnable();
  enable = Param::GetInt(Param::enable);
  assert(!enable);


  // DigIo & canenable -> on
  Param::SetInt(Param::enable, 0);
  Param::SetInt(Param::enablepol, 0);
  Param::SetInt(Param::cancontrol, 0);
  Param::SetInt(Param::canenable, 2);
  DigIo::enable_in.Set();
  CalcEnable();
  enable = Param::GetInt(Param::enable);
  assert(enable);

  //recheck can after 10+ calls
  // TODO unfinished test
  //int canenable = Param::GetInt(Param::canenable);
  //Param::SetInt(Param::cancontrol, 1);
  //for(int i =0; i < 15;i++){
  //  CalcEnable();
  //  enable = Param::GetInt(Param::enable);
  //  assert(enable);
  //}
  //assert(3 == canenable);
}

// TODO: test_charger_state_machine(){}
// TODO: test_calc_ac_current_limit(){}
// TODO: test_check_charger_faults(){}
