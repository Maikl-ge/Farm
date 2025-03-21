#ifndef TIMEMODULE_H
#define TIMEMODULE_H

#include <Rtc_Pcf8563.h>

// Инициализация экземпляра RTC
extern Rtc_Pcf8563 rtc;
extern uint32_t CurrentDate; // Текущая дата фермы
extern uint32_t CurrentTime; // Текущее время фермы
extern uint16_t currentTimeInMinutes; // Текущее время в минутах

extern uint16_t GROWE_START_TIME;  // Время начала цикла роста

// Функции для работы с модулем времени
void initTimeModule();
void syncTimeWithNTP(const char* ntpServer, int8_t timeZone);
void printCurrentTime();
uint16_t getCurrentTimeInMinutes();
void saveCurrentDateToGroweStopDate(); // Tекущая дата
#endif // TIMEMODULE_H
