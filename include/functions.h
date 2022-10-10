#include "../libopeninv/include/params.h"

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
