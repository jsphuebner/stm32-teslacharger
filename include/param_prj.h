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

/* This file contains all parameters used in your project
 * See main.cpp on how to access them.
 * If a parameters unit is of format "0=Choice, 1=AnotherChoice" etc.
 * It will be displayed as a dropdown in the web interface
 * If it is a spot value, the decimal is translated to the name, i.e. 0 becomes "Choice"
 * If the enum values are powers of two, they will be displayed as flags, example
 * "0=None, 1=Flag1, 2=Flag2, 4=Flag3, 8=Flag4" and the value is 5.
 * It means that Flag1 and Flag3 are active -> Display "Flag1 | Flag3"
 *
 * Every parameter/value has a unique ID that must never change. This is used when loading parameters
 * from flash, so even across firmware versions saved parameters in flash can always be mapped
 * back to our list here. If a new value is added, it will receive its default value
 * because it will not be found in flash.
 * The unique ID is also used in the CAN module, to be able to recover the CAN map
 * no matter which firmware version saved it to flash.
 * Make sure to keep track of your ids and avoid duplicates. Also don't re-assign
 * IDs from deleted parameters because you will end up loading some random value
 * into your new parameter!
 * IDs are 16 bit, so 65535 is the maximum
 */

//Define a version string of your firmware here
#define VER 1.17.R

/* Entries must be ordered as follows:
   1. Saveable parameters (id != 0)
   2. Temporary parameters (id = 0)
   3. Display values
 */
//Next param id (increase when adding new parameter!): 23
//Next value Id: 2051
/*              category     name         unit       min     max     default id */
#define PARAM_LIST \
    PARAM_ENTRY(CAT_CHARGER, idclim,      "A",       0,      45,     45,     3   ) \
    PARAM_ENTRY(CAT_CHARGER, iaclim,      "A",       0,      72,     16,     10  ) \
    PARAM_ENTRY(CAT_CHARGER, idcspnt,     "A",       0,      45,     45,     9   ) \
    PARAM_ENTRY(CAT_CHARGER, chargerena,  CHARGERS,  1,      7,      7,      4   ) \
    PARAM_ENTRY(CAT_CHARGER, udcspnt,     "V",       50,     420,    403,    5   ) \
    PARAM_ENTRY(CAT_CHARGER, udclim,      "V",       50,     420,    398,    6   ) \
    PARAM_ENTRY(CAT_CHARGER, timelim,     "minutes", -1,     10000,  -1,     16  ) \
    PARAM_ENTRY(CAT_CHARGER, timedly,     "minutes", -1,     10000,  -1,     22  ) \
    PARAM_ENTRY(CAT_CHARGER, inputype,    INPUTS,    0,      5,      1,      17  ) \
    PARAM_ENTRY(CAT_CHARGER, cancontrol,  OFFON,     0,      1,      0,      14  ) \
    PARAM_ENTRY(CAT_CHARGER, enablepol,   POLARITIES,0,      1,      0,      18  ) \
    PARAM_ENTRY(CAT_CHARGER, idckp,       "",        0,      10000,  1,      20  ) \
    PARAM_ENTRY(CAT_CHARGER, idcki,       "",        0,      10000,  10,     21  ) \
    VALUE_ENTRY(state,       STATES,  2043 ) \
    VALUE_ENTRY(uptime,      "s",     2048 ) \
    VALUE_ENTRY(lasterr,     errorListString,  2002 ) \
    VALUE_ENTRY(uaux,        "V",     2049 ) \
    VALUE_ENTRY(aclim,       "A",     2042 ) \
    VALUE_ENTRY(cablelim,    "A",     2038 ) \
    VALUE_ENTRY(evselim,     "A",     2039 ) \
    VALUE_ENTRY(idc,         "A",     2041 ) \
    VALUE_ENTRY(udc,         "V",     2047 ) \
    VALUE_ENTRY(soc,         "%",     2046 ) \
    VALUE_ENTRY(proximity,   OFFON,   2040 ) \
    VALUE_ENTRY(enable,      OFFON,   2005 ) \
    VALUE_ENTRY(canenable,   OFFON,   2045 ) \
    VALUE_ENTRY(cpuload,     "%",     2004 ) \
    VALUE_ENTRY(version,     VERSTR,  2001 ) \
    VALUE_ENTRY(opmode,      OPMODES, 2000 ) \
    VALUE_ENTRY(hwaclim,     "A",     2050 ) \
    VALUE_ENTRY(c1stt,       "",      2007 ) \
    VALUE_ENTRY(c1flag,      CHFLAGS, 2008 ) \
    VALUE_ENTRY(c1tmp1,      "°C",    2010 ) \
    VALUE_ENTRY(c1tmp2,      "°C",    2011 ) \
    VALUE_ENTRY(c1tmpin,     "°C",    2012 ) \
    VALUE_ENTRY(c1uac,       "V",     2013 ) \
    VALUE_ENTRY(c1iac,       "A",     2014 ) \
    VALUE_ENTRY(c1udc,       "V",     2015 ) \
    VALUE_ENTRY(c1idc,       "A",     2016 ) \
    VALUE_ENTRY(c2stt,       "",      2017 ) \
    VALUE_ENTRY(c2flag,      CHFLAGS, 2018 ) \
    VALUE_ENTRY(c2tmp1,      "°C",    2020 ) \
    VALUE_ENTRY(c2tmp2,      "°C",    2021 ) \
    VALUE_ENTRY(c2tmpin,     "°C",    2022 ) \
    VALUE_ENTRY(c2uac,       "V",     2023 ) \
    VALUE_ENTRY(c2iac,       "A",     2024 ) \
    VALUE_ENTRY(c2udc,       "V",     2025 ) \
    VALUE_ENTRY(c2idc,       "A",     2026 ) \
    VALUE_ENTRY(c3stt,       "",      2027 ) \
    VALUE_ENTRY(c3flag,      CHFLAGS, 2028 ) \
    VALUE_ENTRY(c3tmp1,      "°C",    2030 ) \
    VALUE_ENTRY(c3tmp2,      "°C",    2031 ) \
    VALUE_ENTRY(c3tmpin,     "°C",    2032 ) \
    VALUE_ENTRY(c3uac,       "V",     2033 ) \
    VALUE_ENTRY(c3iac,       "A",     2034 ) \
    VALUE_ENTRY(c3udc,       "V",     2035 ) \
    VALUE_ENTRY(c3idc,       "A",     2036 ) \
    VALUE_ENTRY(test_time,   "s",     2037 ) \


/***** Enum String definitions *****/
#define OPMODES      "0=Off, 1=Run"
#define CHARGERS     "1=Charger1, 2=Charger2, 4=Charger3"
#define OFFON        "0=Off, 1=On"
#define CHFLAGS      "0=None, 1=Enabled, 2=Fault, 4=CheckAlive"
#define STATES       "0=Off, 1=WaitStart, 2=Enable, 3=Activate, 4=Run, 5=Stop"
#define INPUTS       "0=Type2, 1=Type2-3P, 2=Type1, 3=Manual, 4=Manual-3P, 5=Type2-Auto"
#define POLARITIES   "0=ActiveHigh, 1=ActiveLow"
#define CAT_TEST     "Testing"
#define CAT_CHARGER  "Charger"
#define CAT_COMM     "Communication"

#define VERSTR STRINGIFY(4=VER)

/***** enums ******/

enum inputs
{
   INP_TYPE2,
   INP_TYPE2_3P,
   INP_TYPE1,
   INP_MANUAL,
   INP_MANUAL_3P,
   INP_TYPE2_AUTO
};

enum states
{
   OFF,
   WAITSTART,
   ENABLE,
   ACTIVATE,
   EVSEACTIVATE,
   STOP
};

enum _canspeeds
{
   CAN_PERIOD_100MS = 0,
   CAN_PERIOD_10MS,
   CAN_PERIOD_LAST
};

enum _modes
{
   MOD_OFF = 0,
   MOD_RUN,
   MOD_LAST
};

enum _chflags
{
   FLAG_NONE = 0,
   FLAG_ENABLED = 1,
   FLAG_FAULT = 2,
   FLAG_CHECK = 4
};

//Generated enum-string for possible errors
extern const char* errorListString;

