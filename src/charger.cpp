#include "my_math.h"
#include "errormessage.h"
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
   DigIo::ch3act_out.Clear();
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

bool CheckDelay()
{
   uint32_t now = rtc_get_counter_val();
   uint32_t start = Param::GetInt(Param::timedly) * 60;

   return start <= 0 || (now - startTime) > start;
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

void ResetValuesInOffMode()
{
   if (Param::GetInt(Param::state) == OFF)
   {
      for (int i = Param::c1stt; i <= Param::c3idc; i++)
      {
         Param::SetInt((Param::PARAM_NUM)i, 0);
      }
   }
}

void CalcEnable()
{
   static int recheckCan = 10;
   bool enablePol = Param::GetBool(Param::enablepol);
   bool enable = DigIo::enable_in.Get() ^ enablePol;

   enable &= !Param::GetBool(Param::cancontrol) || Param::GetBool(Param::canenable);

   if (Param::GetBool(Param::cancontrol))
   {
      if (recheckCan == 0)
      {
         if (Param::GetInt(Param::canenable) == 3)
         {
            Param::SetInt(Param::canenable, 0);
            ErrorMessage::Post(ERR_EXTCAN);
         }
         else
         {
            Param::SetInt(Param::canenable, 3); //Must be overwritten by CAN message within the next second
         }
         recheckCan = 10;
      }

      recheckCan--;
   }

   Param::SetInt(Param::enable, enable);
}

// TODO: no unit tests below here (see test_logic.h and implement there)

bool CheckChargerFaults()
{
   const int acPresentThresh = 70;
   const int timeout = 20;
   static int counters[3] = { timeout, timeout, timeout };
   int configuredChargers = Param::GetInt(Param::chargerena);
   bool timeouts[3];
   bool active1 = (configuredChargers & 1) && (Param::GetInt(Param::c1uac) > acPresentThresh);
   bool active2 = (configuredChargers & 2) && (Param::GetInt(Param::c2uac) > acPresentThresh);
   bool active3 = (configuredChargers & 4) && (Param::GetInt(Param::c3uac) > acPresentThresh);

   timeouts[0] = (Param::GetInt(Param::c1flag) & FLAG_CHECK) != 0;
   timeouts[1] = (Param::GetInt(Param::c2flag) & FLAG_CHECK) != 0;
   timeouts[2] = (Param::GetInt(Param::c3flag) & FLAG_CHECK) != 0;

   for (int i = 0; i < 3; i++)
   {
      if (timeouts[i])
      {
         if (counters[i] > 0)
         {
            counters[i]--;
            timeouts[i] = false;
         }
         else
         {
            ErrorMessage::Post(ERR_CHARGERCAN);
         }
      }
      else
      {
         counters[i] = timeout;
      }
   }

   //Set check flag. By the next call this should be deleted by the CAN module
   Param::SetInt(Param::c1flag, Param::GetInt(Param::c1flag) | FLAG_CHECK);
   Param::SetInt(Param::c2flag, Param::GetInt(Param::c2flag) | FLAG_CHECK);
   Param::SetInt(Param::c3flag, Param::GetInt(Param::c3flag) | FLAG_CHECK);

   return (active1 && ((Param::GetInt(Param::c1flag) & FLAG_FAULT) || timeouts[0])) ||
          (active2 && ((Param::GetInt(Param::c2flag) & FLAG_FAULT) || timeouts[1])) ||
          (active3 && ((Param::GetInt(Param::c3flag) & FLAG_FAULT) || timeouts[2]));
}

void CalcAcCurrentLimit()
{
   int configuredChargers = Param::GetInt(Param::chargerena);
   float iacLim = Param::GetFloat(Param::iaclim);
   float hwaclim = Param::GetFloat(Param::hwaclim);
   float evseLim = Param::GetFloat(Param::evselim);
   float cableLim = Param::GetFloat(Param::cablelim);
   int activeModules = ((configuredChargers & 1) > 0) + ((configuredChargers & 2) > 0) + ((configuredChargers & 4) > 0);

   if (IsEvseInput())
   {
      iacLim = MIN(iacLim, MIN(evseLim, cableLim));
   }

   if (Param::GetInt(Param::opmode) == 0)
   {
      dcCurController.ResetIntegrator();
      iacLim = 0;
   }
   else
   {
      dcCurController.SetMinMaxY(0, iacLim);
      iacLim = dcCurController.Run(Param::Get(Param::idc));
   }

   if (Param::GetInt(Param::inputype) == INP_MANUAL ||
       Param::GetInt(Param::inputype) == INP_TYPE1  ||
       Param::GetInt(Param::inputype) == INP_TYPE2  ||
      (Param::GetInt(Param::inputype) == INP_TYPE2_AUTO && !DigIo::threep_in.Get()))
   {
      iacLim /= (float)activeModules;
   }

   iacLim = MIN(iacLim, hwaclim);

   Param::SetFloat(Param::aclim, iacLim);
}

void ChargerStateMachine()
{
   static states state = OFF;
   int configuredChargers = Param::GetInt(Param::chargerena);

   if (!Param::GetBool(Param::enable))
   {
      state = OFF;
   }

   switch (state)
   {
      default:
      case OFF:
         Param::SetInt(Param::opmode, 0);
         DisableAll();

         if (CheckStartCondition())
         {
            startTime = rtc_get_counter_val();
            state = WAITSTART;
         }
         break;
      case WAITSTART:
         if (CheckDelay())
            state = ENABLE;
         break;
      case ENABLE:
         DigIo::hvena_out.Set();
         if (configuredChargers & 1)
            DigIo::ch1ena_out.Set();
         if (configuredChargers & 2)
            DigIo::ch2ena_out.Set();
         if (configuredChargers & 4)
            DigIo::ch3ena_out.Set();
         state = ACTIVATE;
         break;
      case ACTIVATE:
         Param::SetInt(Param::opmode, 1);

         if (configuredChargers & 1)
            DigIo::ch1act_out.Set();
         if (configuredChargers & 2)
            DigIo::ch2act_out.Set();
         if (configuredChargers & 4)
            DigIo::ch3act_out.Set();

         startTime = rtc_get_counter_val();
         state = EVSEACTIVATE;
         break;
      case EVSEACTIVATE:
         DigIo::evseact_out.Set();
         DigIo::acpres_out.Set();

         if (CheckVoltage() || CheckTimeout())
            state = STOP;
         if (CheckUnplugged())
         {
            DigIo::acpres_out.Clear();
            DigIo::evseact_out.Clear();
            state = OFF;
         }
         if (CheckChargerFaults())
         {
            DigIo::acpres_out.Clear();
            state = OFF;
         }
         break;
      case STOP:
         DisableAll();
         Param::SetInt(Param::opmode, 0);

         if (CheckUnplugged())
            state = OFF;
         break;
   }

   Param::SetInt(Param::state, state);
}
