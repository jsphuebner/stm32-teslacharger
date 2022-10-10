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
}
