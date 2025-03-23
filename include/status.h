// status.h
#ifndef STATUS_H
#define STATUS_H

#include <Arduino.h>
#include "TimeModule.h"

void currentStatusFarm();

uint16_t getCurrentTimeInMinutes();
uint16_t convertTimeToMinutes(uint16_t time);
int subtractDates(int date1, int date2); // Добавлено объявление функции
int subtractDatesUsingEpoch(uint16_t date1, uint16_t date2); // Изменено на функцию
uint16_t getCurrentDate(); // Tекущее время
uint16_t readFromEEPROM(int address);
uint16_t currentDate;
void saveUint16ToEEPROM(int address, uint16_t value);

#endif // STATUS_H