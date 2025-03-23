#ifndef TIMEMODULE_H
#define TIMEMODULE_H

#include <Rtc_Pcf8563.h>

// Инициализация экземпляра RTC
extern Rtc_Pcf8563 rtc;
extern uint32_t CurrentDate; // Текущая дата фермы
extern uint32_t CurrentTime; // Текущее время фермы
extern uint16_t CurrentDataShort; // Текущее время в минутах
extern uint16_t currentTimeInMinutes; // Текущее время в минутах
extern uint16_t GROWE_MODE_TIME;  // Время начала цикла роста
extern uint16_t GROWE_MODE_DATA;  // Дата начала цикла роста
extern uint16_t daysEopch;

// Функции для работы с модулем времени
void initTimeModule();
void syncTimeWithNTP(const char* ntpServer, int8_t timeZone);
void printCurrentTime();
uint16_t getCurrentTimeInMinutes();
String getReadableDate(uint16_t date);
void getCurrentDateToGrowe(); // Tекущее время
uint16_t getCurrentDate(); // Tекущее время
uint16_t dateToDaysSinceEpoch(uint16_t year, uint8_t month, uint8_t day);

#endif // TIMEMODULE_H
