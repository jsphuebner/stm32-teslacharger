#ifndef CHARGER_H
#define CHARGER_H
#include "../libopeninv/include/params.h"
extern uint32_t startTime;
bool IsEvseInput();
bool CheckUnplugged();
bool CheckTimeout();
void CalcTotals();
void DisableAll();
void EvseRead();
#endif
