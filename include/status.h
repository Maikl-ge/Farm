// status.h
#ifndef STATUS_H
#define STATUS_H

#include <Arduino.h>
#include "TimeModule.h"

uint16_t getCurrentTimeInMinutes();
uint16_t convertTimeToMinutes(uint16_t time);
int subtractDates(int date1, int date2); // Добавлено объявление функции
int subtractDatesUsingEpoch(uint16_t date1, uint16_t date2); // Изменено на функцию
uint16_t getCurrentDate(); // Tекущее время
uint16_t readFromEEPROM(int address);
extern uint16_t wateringInterval;
extern uint16_t wateringDraining;
extern uint16_t totalMinutesElapsed;
extern bool pintStatusFarm;

extern uint16_t longPhacse;
extern uint16_t longPhacse1;
extern uint16_t longPhacse2;
extern uint16_t longPhacse3;
extern uint16_t longPhacse4;
extern uint16_t longPhacse5;
extern uint16_t longPhacse6;

extern int phaseToGrowe;

//void CheckStatusFarm();
void currentStatusFarm();
void checkPhaseToGrowe();
void saveUint16ToEEPROM(int address, uint16_t value);
 
#endif // STATUS_H