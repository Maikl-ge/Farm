#ifndef TIMEMODULE_H
#define TIMEMODULE_H

#include <Rtc_Pcf8563.h>

// Инициализация экземпляра RTC
extern Rtc_Pcf8563 rtc;
extern uint32_t CurrentDate; // Текущая дата фермы
extern uint32_t CurrentTime; // Текущее время фермы

// Функции для работы с модулем времени
void initTimeModule();
void syncTimeWithNTP(const char* ntpServer, int8_t timeZone);
void printCurrentTime();

#endif // TIMEMODULE_H
