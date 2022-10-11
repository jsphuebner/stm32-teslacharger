#include "my_math.h"
#include "charger.h"
#ifdef TEST_COMMON_H
#include "../test/test_common.h"
#include "../test/digio_mock.h"
#include "../test/timer_mock.h"
#else
#include "digio.h"
#include "anain.h"
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/rtc.h>
#endif

bool IsEvseInput()
{
   enum inputs input = (enum inputs)Param::GetInt(Param::inputype);
   return input == INP_TYPE1 || input == INP_TYPE2 || input == INP_TYPE2_3P || input == INP_TYPE2_AUTO;
}

bool CheckUnplugged()
{
   return IsEvseInput() && !Param::GetBool(Param::proximity);
}

void DisableAll()
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

void CalcTotals()
{
   s32fp totalCurrent = Param::Get(Param::c1idc) + Param::Get(Param::c2idc) + Param::Get(Param::c3idc);

   Param::SetFixed(Param::idc, totalCurrent);
   s32fp u1 = Param::Get(Param::c1udc);
   s32fp u2 = Param::Get(Param::c2udc);
   s32fp u3 = Param::Get(Param::c3udc);

   s32fp udcmax = MAX(u1, MAX(u2, u3));
   Param::SetFixed(Param::udc, udcmax);
}

bool CheckStartCondition()
{
   return (IsEvseInput() && Param::GetBool(Param::proximity) && Param::Get(Param::cablelim) > FP_FROMFLT(1.4) && Param::GetBool(Param::enable)) ||
         (!IsEvseInput() && Param::GetBool(Param::enable));
}

bool CheckVoltage()
{
   static int timeout = 0;

   if (Param::Get(Param::udc) > Param::Get(Param::udclim))
   {
      timeout++;
   }
   else
   {
      timeout = 0;
   }

   return timeout > 10;
}


bool CheckTimeout()
{
   uint32_t now = rtc_get_counter_val();
   uint32_t timeout = Param::GetInt(Param::timelim);

   timeout *= 60;

   return timeout > 0 && (now - startTime) > timeout;
}

void EvseRead()
{
   const int threshProxType1 = 2200;
   const int threshProx = 3700;
   const int thresh13A = 3200;
   const int thresh20A = 2800;
   const int thresh32A = 1800;
   const int thresh63A = 1000;
   int val = AnaIn::cablelim.Get();

   if (timer_get_flag(TIM3, TIM_SR_CC2IF))
   {
      //The relationship between duty cycle and maximum current is linear
      //until 85% = 51A. Above that it becomes non-linear but that is not
      //relevant for our 10kW charger.
      float evselim = timer_get_ic_value(TIM3, TIM_IC2) / 10;
      evselim *= 0.666666f;
      Param::SetFloat(Param::evselim, evselim);
   }
   else
   {
      //If no PWM detected, set limit to 0
      Param::SetInt(Param::evselim, 0);
   }

   if (Param::GetInt(Param::inputype) == INP_TYPE2 ||
       Param::GetInt(Param::inputype) == INP_TYPE2_3P ||
       Param::GetInt(Param::inputype) == INP_TYPE2_AUTO)
   {
      if (val > threshProx)
      {
         Param::SetInt(Param::proximity, 0);
         Param::SetInt(Param::cablelim, 0);
      }
      else
      {
         Param::SetInt(Param::proximity, 1);

         if (val > thresh13A)
         {
            Param::SetInt(Param::cablelim, 13);
         }
         else if (val > thresh20A)
         {
            Param::SetInt(Param::cablelim, 20);
         }
         else if (val > thresh32A)
         {
            Param::SetInt(Param::cablelim, 32);
         }
         else if (val > thresh63A)
         {
            Param::SetInt(Param::cablelim, 63);
         }
      }
   }
   else if (Param::GetInt(Param::inputype) == INP_TYPE1)
   {
      if (val > threshProxType1)
      {
         Param::SetInt(Param::proximity, 0);
         Param::SetInt(Param::cablelim, 0);
      }
      else
      {
         Param::SetInt(Param::proximity, 1);
         Param::SetInt(Param::cablelim, 40);
      }
   }
   else
   {
      Param::SetInt(Param::proximity, 0);
      Param::SetInt(Param::cablelim, 32);
   }
}

