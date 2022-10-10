/*
 * This file is part of the stm32-template project.
 *
 * Copyright (C) 2020 Johannes Huebner <dev@johanneshuebner.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdint.h>
#include <libopencm3/stm32/usart.h>
#ifdef TEST_COMMON_H
#include "../test/timer_mock.h"
#else
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/rtc.h>
#endif
#include <libopencm3/stm32/can.h>
#include <libopencm3/stm32/iwdg.h>
#include <libopencm3/stm32/crc.h>
#include "stm32_can.h"
#include "terminal.h"
#include "params.h"
#include "hwdefs.h"
#include "digio.h"
#include "hwinit.h"
#include "anain.h"
#include "param_save.h"
#include "my_math.h"
#include "errormessage.h"
#include "printf.h"
#include "stm32scheduler.h"
#include "picontroller.h"
#include "chargercan.h"
#include "charger.h"

static Stm32Scheduler* scheduler;
static Can* can;
static PiController dcCurController;

static void EvseRead()
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



static bool CheckStartCondition()
{
   return (IsEvseInput() && Param::GetBool(Param::proximity) && Param::Get(Param::cablelim) > FP_FROMFLT(1.4) && Param::GetBool(Param::enable)) ||
         (!IsEvseInput() && Param::GetBool(Param::enable));
}

static bool CheckVoltage()
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

static bool CheckChargerFaults()
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

static bool CheckDelay()
{
   uint32_t now = rtc_get_counter_val();
   uint32_t start = Param::GetInt(Param::timedly) * 60;

   return start <= 0 || (now - startTime) > start;
}

static void CalcAcCurrentLimit()
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

static void ChargerStateMachine()
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

static void CalcEnable()
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

static void ResetValuesInOffMode()
{
   if (Param::GetInt(Param::state) == OFF)
   {
      for (int i = Param::c1stt; i <= Param::c3idc; i++)
      {
         Param::SetInt((Param::PARAM_NUM)i, 0);
      }
   }
}

//sample 100ms task
static void Ms100Task(void)
{
   DigIo::led_out.Toggle();
   //The boot loader enables the watchdog, we have to reset it
   //at least every 2s or otherwise the controller is hard reset.
   iwdg_reset();
   //Calculate CPU load. Don't be surprised if it is zero.
   float cpuLoad = scheduler->GetCpuLoad() / 10.0f;
   //This sets a fixed point value WITHOUT calling the parm_Change() function
   Param::SetFloat(Param::cpuload, cpuLoad);
   //Set timestamp of error message
   ErrorMessage::SetTime(rtc_get_counter_val());
   Param::SetInt(Param::uptime, rtc_get_counter_val());
   Param::SetFloat(Param::uaux, AnaIn::uaux.Get() / 223.418f);

   ResetValuesInOffMode();
   CalcTotals();
   CalcEnable();
   CalcAcCurrentLimit();
   ChargerStateMachine();

   EvseRead();

   can->SendAll();
}

static void MapChargerMessages()
{
   uint32_t dummyId;
   uint8_t dummyOfs, dummyLen;
   float dummyGain;
   bool dummyrx;

   //check sample value, if it is mapped assume valid CAN map
   if (can->FindMap(Param::hwaclim, dummyId, dummyOfs, dummyLen, dummyGain, dummyrx)) return;

   can->Clear();

   ChargerCAN::MapMessages(can);

   can->Save();
}

/** This function is called when the user changes a parameter */
void Param::Change(Param::PARAM_NUM paramNum)
{
   s32fp spnt;

   switch (paramNum)
   {
      case Param::idckp:
      case Param::idcki:
         dcCurController.SetGains(Param::GetInt(Param::idckp), Param::GetInt(Param::idcki));
         break;
      case Param::idclim:
      case Param::idcspnt:
         spnt = MIN(Param::Get(Param::idcspnt), Param::Get(Param::idclim));
         dcCurController.SetRef(spnt);
         break;
      default:
         //Handle general parameter changes here. Add paramNum labels for handling specific parameters
         break;
   }
}

//Whichever timer(s) you use for the scheduler, you have to
//implement their ISRs here and call into the respective scheduler
extern "C" void tim2_isr(void)
{
   scheduler->Run();
}

extern "C" int main(void)
{
   extern const TERM_CMD termCmds[];

   clock_setup(); //Must always come first
   rtc_setup();
   ANA_IN_CONFIGURE(ANA_IN_LIST);
   DIG_IO_CONFIGURE(DIG_IO_LIST);
   AnaIn::Start(); //Starts background ADC conversion via DMA
   write_bootloader_pininit(); //Instructs boot loader to initialize certain pins
   gpio_primary_remap(AFIO_MAPR_SWJ_CFG_JTAG_OFF_SW_ON, AFIO_MAPR_CAN1_REMAP_PORTB);
   tim_setup(); //Use timer3 for sampling pilot PWM
   nvic_setup(); //Set up some interrupts
   parm_load(); //Load stored parameters
   Param::Change(Param::idckp); //Call callback once for parameter propagation
   Param::Change(Param::idclim); //Call callback once for parameter propagation

   Stm32Scheduler s(TIM2); //We never exit main so it's ok to put it on stack
   scheduler = &s;
   //Initialize CAN1, including interrupts. Clock must be enabled in clock_setup()
   Can c(CAN1, Can::Baud500, true);
   c.SetNodeId(5);
   //store a pointer for easier access
   can = &c;
   Terminal t3(USART3, termCmds);
   Terminal t1(USART1, termCmds);

   MapChargerMessages();
   dcCurController.SetCallingFrequency(10);

   //Up to four tasks can be added to each timer scheduler
   //AddTask takes a function pointer and a calling interval in milliseconds.
   //The longest interval is 655ms due to hardware restrictions
   //You have to enable the interrupt (int this case for TIM2) in nvic_setup()
   //There you can also configure the priority of the scheduler over other interrupts
   s.AddTask(Ms100Task, 100);

   //backward compatibility, version 4 was the first to support the "stream" command
   Param::SetInt(Param::version, 4);

   //In version 1.11 this changed from mV to V
   if (Param::GetInt(Param::udcspnt) > 420)
   {
      Param::SetFloat(Param::udcspnt, Param::GetFloat(Param::udcspnt) / 1000);
   }

   //Now all our main() does is running the terminal
   //All other processing takes place in the scheduler or other interrupt service routines
   //The terminal has lowest priority, so even loading it down heavily will not disturb
   //our more important processing routines.
   while(1)
   {
      t1.Run();
      t3.Run();
   }


   return 0;
}

