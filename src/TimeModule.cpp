#include <Arduino.h>
#include "TimeModule.h"
#include <Rtc_Pcf8563.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <Pinout.h>
#include <TimeLib.h>
#include <Profile.h>


//int8_t timeZone = 3; // Часовой пояс
uint32_t CurrentDate = 0; // Текущая дата фермы
uint32_t CurrentTime = 0; // Текущее время фермы
uint16_t CurrentTimeInMinutes = 0; // Текущее время в минутах
uint16_t CurrentDataShort = 0;  // Время начала цикла роста
uint16_t daysEopch = 0;  // Число дней начала цикла роста от 1 января 1970

// Инициализация экземпляра RTC
Rtc_Pcf8563 rtc;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -10800, 60000);

void checkRtcPresence() {
    Wire.beginTransmission(0x51); // Адрес RTC PCF8563
    if (Wire.endTransmission() == 0) {
        Serial.println("RTC detected on I2C bus.");
    } else {
        Serial.println("RTC not detected on I2C bus.");
    }
}

void initTimeModule() {
    Wire.begin(SDA_PIN, SCL_PIN); // Инициализация I2C
    //checkRtcPresence(); // Проверка наличия RTC на шине I2C
    rtc.initClock(); // Инициализация RTC
    timeClient.begin(); // Запуск NTP клиента
}

void syncTimeWithNTP(const char* ntpServer, int8_t timeZone) {
    timeClient.setPoolServerName(ntpServer); // Установка адреса NTP сервера
    Serial.println("Synchronizing time with NTP server...");
    while (!timeClient.update()) {
        delay(500);
    }

    unsigned long epochTime = timeClient.getEpochTime();
    // Учёт часового пояса
    epochTime += timeZone * 3600; // Сдвиг времени на основе часового пояса
    // Преобразуем время в структуру tm
    struct tm timeInfo;
    gmtime_r((time_t*)&epochTime, &timeInfo); // Используем gmtime_r для потокобезопасности

    rtc.setDateTime(
        timeInfo.tm_mday, 
        timeInfo.tm_wday, 
        timeInfo.tm_mon + 1, 
        false, // Укажите значение для century
        timeInfo.tm_year % 100, 
        timeInfo.tm_hour, 
        timeInfo.tm_min, 
        timeInfo.tm_sec
    );
    if(timeInfo.tm_hour == 00) {
        timeInfo.tm_hour = 24;
    }
    Serial.println("Time synchronized successfully.");
    CurrentDate = (timeInfo.tm_year + 1900) * 10000 + (timeInfo.tm_mon + 1) * 100 + timeInfo.tm_mday;
    CurrentTime = timeInfo.tm_hour * 10000 + timeInfo.tm_min * 100 + timeInfo.tm_sec;
    // Вывод даты и времени для проверки
    Serial.printf("Current Date (YYYYMMDD): %lu\n", CurrentDate);
    Serial.printf("Current Time (HHMMSS): %06lu\n", CurrentTime); // Форматирование с ведущими нулями
}

// Вывод текущего времени
void printCurrentTime() {
    // Получаем текущее время
    rtc.getDateTime();

    // Формирование даты в формате YYYYMMDD
    CurrentDate = (rtc.getYear() + 2000) * 10000 + rtc.getMonth() * 100 + rtc.getDay();
    //CurrentDateShort = timeInfo.tm_mday * 10000 + (timeInfo.tm_mon + 1) * 100 + (timeInfo.tm_year % 100); // Формат DDMMYY
    // Формирование времени в формате HHMMSS
    CurrentTime = rtc.getHour() * 10000 + rtc.getMinute() * 100 + rtc.getSecond();
}

// Получение текущего времени в минутах
uint16_t getCurrentTimeInMinutes() {
    rtc.getDateTime();
    uint16_t currentTimeInMinutes = rtc.getHour() * 60 + rtc.getMinute();
    return currentTimeInMinutes;
}

// Сохранение текущей даты в GROWE_MODE_DATE как число дней с 1 января 1970
void getCurrentDateToGrowe() {
    rtc.getDateTime();  // Обновляем время перед считыванием
    uint8_t day = rtc.getDay();
    uint8_t month = rtc.getMonth();
    uint16_t year = rtc.getYear() + 2000;  // Полный год (например, 2025)
    daysEopch = dateToDaysSinceEpoch(year, month, day);
    GROWE_MODE_DATE = daysEopch;  // Сохраняем дату начала цикла роста
}

// Получение текущей даты как числа дней с 1 января 1970
uint16_t getCurrentDate() {
    rtc.getDateTime();  // Обновляем время перед считыванием
    uint8_t day = rtc.getDay();
    uint8_t month = rtc.getMonth();
    uint16_t year = rtc.getYear() + 2000;  // Полный год (например, 2025)
    daysEopch = dateToDaysSinceEpoch(year, month, day);    
    return daysEopch;
}

// Функция преобразования даты в количество дней с 1 января 1970
uint16_t dateToDaysSinceEpoch(uint16_t year, uint8_t month, uint8_t day) {
    static const int daysBeforeMonth[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
    if (year < 1970 || month < 1 || month > 12 || day < 1 || day > 31) {
        Serial.println("Ошибка: некорректная дата! " + String(year) + "-" + String(month) + "-" + String(day));
        return 0;  // Вернём 0 в случае ошибки
    }
    uint16_t days = (year - 1970) * 365 + (year - 1969) / 4 - (year - 1901) / 100 + (year - 1601) / 400;
    days += daysBeforeMonth[month - 1];
    days += day - 1;
    if (month > 2 && ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))) {
        days++;
    }
    return days;
}

