#ifndef TEST_STATE_H
#define TEST_STATE_H
#endif
#include <assert.h>
#include "test_common.h"
#include "functions.h"

void test_is_evse_input()
{

  bool res = false;

  res = IsEvseInput();
  assert(res);

  Param::SetInt(Param::inputype, INP_MANUAL);
  res = IsEvseInput();
  assert(!res);
}

void test_check_unplugged()
{
  bool res = false;

  Param::SetInt(Param::inputype, INP_TYPE1);
  //TODO: surprising behaviour with Param::Set(Param::proximity, true); // as it expects s32fp

  Param::SetInt(Param::proximity, 1);
  res = CheckUnplugged();
  assert(!res);

  Param::SetInt(Param::proximity, 0);
  res = CheckUnplugged();
  assert(res);
}
