#ifndef TEST_IO_H
#define TEST_IO_H
#endif
#include <assert.h>
#include "test_common.h"

void test_disable_all()
{
  DigIo::hvena_out.Set();
  DigIo::acpres_out.Set();
  DigIo::evseact_out.Set();
  DigIo::ch1act_out.Set();
  DigIo::ch2act_out.Set();
  DigIo::ch2act_out.Set();
  DigIo::ch1ena_out.Set();
  DigIo::ch2ena_out.Set();
  DigIo::ch3ena_out.Set();

  assert(DigIo::hvena_out.Get());
  assert(DigIo::hvena_out.Get());
  assert(DigIo::acpres_out.Get());
  assert(DigIo::evseact_out.Get());
  assert(DigIo::ch1act_out.Get());
  assert(DigIo::ch2act_out.Get());
  assert(DigIo::ch2act_out.Get());
  assert(DigIo::ch1ena_out.Get());
  assert(DigIo::ch2ena_out.Get());
  assert(DigIo::ch3ena_out.Get());

  DisableAll();

  assert(!DigIo::hvena_out.Get());
  assert(!DigIo::hvena_out.Get());
  assert(!DigIo::acpres_out.Get());
  assert(!DigIo::evseact_out.Get());
  assert(!DigIo::ch1act_out.Get());
  assert(!DigIo::ch2act_out.Get());
  assert(!DigIo::ch2act_out.Get());
  assert(!DigIo::ch1ena_out.Get());
  assert(!DigIo::ch2ena_out.Get());
  assert(!DigIo::ch3ena_out.Get());

}
