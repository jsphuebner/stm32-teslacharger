#ifndef CHARGER_H
#define CHARGER_H
#include "../libopeninv/include/picontroller.h"
#include "../libopeninv/include/params.h"
extern uint32_t startTime;
extern PiController dcCurController;
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
void CalcEnable();
bool CheckChargerFaults();
void ChargerStateMachine();
void CalcAcCurrentLimit();
void ChargerStateMachine();
#endif
