#ifndef CHARGER_H
#define CHARGER_H
#include "../libopeninv/include/params.h"
extern uint32_t startTime;
bool IsEvseInput();
bool CheckUnplugged();
bool CheckTimeout();
bool CheckDelay();
void CalcTotals();
void DisableAll();
void EvseRead();
bool CheckStartCondition();
bool CheckVoltage();
void ResetValuesInOffMode();
#endif
