#include "../libopeninv/include/params.h"
#ifdef TEST_COMMON_H
#include "../test/digio_mock.h"
#endif

static bool IsEvseInput();
static void DisableAll();

static bool IsEvseInput()
{
   enum inputs input = (enum inputs)Param::GetInt(Param::inputype);
   return input == INP_TYPE1 || input == INP_TYPE2 || input == INP_TYPE2_3P || input == INP_TYPE2_AUTO;
}

static bool CheckUnplugged()
{
   return IsEvseInput() && !Param::GetBool(Param::proximity);
}

static void DisableAll()
{
   DigIo::hvena_out.Clear();
   DigIo::acpres_out.Clear();
   DigIo::evseact_out.Clear();
   DigIo::ch1act_out.Clear();
   DigIo::ch2act_out.Clear();
   DigIo::ch2act_out.Clear();
   DigIo::ch1ena_out.Clear();
   DigIo::ch2ena_out.Clear();
   DigIo::ch3ena_out.Clear();
}
