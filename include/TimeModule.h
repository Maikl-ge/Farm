#ifndef TIMEMODULE_H
#define TIMEMODULE_H

#include <Wire.h>
#include <Ds1302.h>
#include <Arduino.h>

//extern Ds1302 rtc;  // Инициализация экземпляра RTC

// Функции для работы с модулем времени
void initTimeModule();
void syncTimeWithNTP(const char* ntpServer = "pool.ntp.org");
void printCurrentTime();

#endif // TIMEMODULE_H