#include "../libopeninv/include/my_fp.h"
#include "../libopeninv/include/my_math.h"
#include "../libopeninv/include/params.h"
#include "test.h"
#include "test_can.h"
#include "test_io.h"
#include "test_state.h"

extern void Param::Change(Param::PARAM_NUM ParamNum){ };

// To add a test:
// 1) write void function test_x (see existing includes)
// 2) register function & name to test_cases below

int main() {
   //Test Case register
   TestCase tests[] = {
      (TestCase){"test_is_evse_input", test_is_evse_input},
      (TestCase){"test_check_unplugged", test_check_unplugged},
      (TestCase){"test_disable_all", test_disable_all},
      // Example:
      //(TestCase){"Your tests name", function_name_without_brackets}
      //...
   };

   run_suite(tests, sizeof(tests)/sizeof(TestCase));
   return 0;
}
