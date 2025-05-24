#include <Arduino.h>
#include "TimeModule.h"
#include <Rtc_Pcf8563.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <pinout.h>
#include <TimeLib.h>
#include <Profile.h>
#include <globals.h>


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
bool syncTimeWithNTP(const char* ntpServer, int8_t timeZone);
// Список альтернативных NTP-серверов
const char* ntpServers[] = {"pool.ntp.org", "time.google.com", "time.windows.com"};
const int numNtpServers = sizeof(ntpServers) / sizeof(ntpServers[0]);

void checkRtcPresence() {
    Wire.beginTransmission(0x51); // Адрес RTC PCF8563
    if (Wire.endTransmission() == 0) {
        Serial.println("RTC detected on I2C bus.");
    } else {
        Serial.println("RTC not detected on I2C bus.");
    }
}

int8_t getTimeZoneOffset(int year, int month, int day) {
    if ((month > 3 && month < 10) || 
        (month == 3 && day >= 31 - ((5 * year / 4 + 4) % 7)) || 
        (month == 10 && day < 31 - ((5 * year / 4 + 1) % 7))) {
        return timeZone +1; // Летнее время (UTC+3)
    } else {
        return timeZone; // Зимнее время (UTC+2)
    }
}

bool isRtcValid() {
    rtc.getDateTime();
    int year = rtc.getYear() + 2000;
    int month = rtc.getMonth();
    int day = rtc.getDay();
    int hour = rtc.getHour();
    int minute = rtc.getMinute();
    int second = rtc.getSecond();

    // Проверяем, что значения находятся в разумных пределах
    if (year >= 2020 && year <= 2100 && // Ожидаемый диапазон годов
        month >= 1 && month <= 12 &&    // Месяцы
        day >= 1 && day <= 31 &&        // Дни
        hour >= 0 && hour <= 23 &&      // Часы
        minute >= 0 && minute <= 59 &&  // Минуты
        second >= 0 && second <= 59) {  // Секунды
        return true;
    }
    return false;
}

void initTimeModule() {
    Wire.begin(SDA_PIN, SCL_PIN); // Инициализация I2C
    checkRtcPresence(); // Проверка наличия RTC на шине I2C
    
    // Инициализируем RTC только если она не содержит валидных данных
    if (!isRtcValid()) {
        Serial.println("RTC data invalid or not set, initializing to default.");
        rtc.initClock(); // Устанавливаем начальное значение только если RTC пустая
    } else {
        Serial.println("RTC data valid, skipping initialization.");
    }
    
    timeClient.begin(); // Запуск NTP клиента
    printCurrentTime(); // Обновляем глобальные переменные из RTC при старте
}

bool syncTimeWithNTP(const char* ntpServer, int8_t timeZone) {
    timeClient.setPoolServerName(ntpServer);
    Serial.print("Synchronizing time with NTP server: ");
    Serial.println(ntpServer);

    const int maxAttempts = 10; // Максимум попыток
    int attempts = 0;

    while (attempts < maxAttempts && !timeClient.update()) {
        delay(500);
        attempts++;
        Serial.print(".");
    }
    Serial.println();

    if (attempts >= maxAttempts) {
        Serial.println("Failed to sync with NTP server.");
        return false;
    }

    unsigned long epochTime = timeClient.getEpochTime();
    struct tm timeInfo;
    gmtime_r((time_t*)&epochTime, &timeInfo);

    timeZone = getTimeZoneOffset(timeInfo.tm_year + 1900, timeInfo.tm_mon + 1, timeInfo.tm_mday);
    epochTime += timeZone * 3600;
    gmtime_r((time_t*)&epochTime, &timeInfo);

    // Записываем в RTC только при успешной синхронизации
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
    if (timeInfo.tm_hour == 0) {
        timeInfo.tm_hour = 24;
    }

    Serial.println("Time synchronized successfully.");
    CurrentDate = (timeInfo.tm_year + 1900) * 10000 + (timeInfo.tm_mon + 1) * 100 + timeInfo.tm_mday;
    CurrentTime = timeInfo.tm_hour * 10000 + timeInfo.tm_min * 100 + timeInfo.tm_sec;

    Serial.printf("Current Date (YYYYMMDD): %lu\n", CurrentDate);
    Serial.printf("Current Time (HHMMSS): %06lu\n", CurrentTime);
    return true;
}

bool syncTimeWithNTPServers(int8_t timeZone) {
    bool syncSuccess = false;
    for (int i = 0; i < numNtpServers && !syncSuccess; i++) {
        syncSuccess = syncTimeWithNTP(ntpServers[i], timeZone);
        if (!syncSuccess) {
            Serial.println("Trying next NTP server...");
        }
    }
    if (!syncSuccess) {
        Serial.println("All NTP sync attempts failed. Using RTC as fallback.");
        printCurrentTime(); // Обновляем глобальные переменные из RTC
        if (CurrentDate == 0 || CurrentTime == 0) {
            Serial.println("RTC not initialized previously, time may be incorrect.");
        }
    }
    return syncSuccess;
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
// Глобальная переменная для хранения предыдущего времени в минутах
uint16_t PreviousTimeInMinutes = 0;

uint16_t getCurrentTimeInMinutes() {
    // Временные переменные
    uint8_t h1, m1, h2, m2, h3, m3;

    // Тройное чтение RTC с паузами
    rtc.getDateTime();
    h1 = rtc.getHour(); m1 = rtc.getMinute();
    delay(10);

    rtc.getDateTime();
    h2 = rtc.getHour(); m2 = rtc.getMinute();
    delay(10);

    rtc.getDateTime();
    h3 = rtc.getHour(); m3 = rtc.getMinute();

    // Проверка совпадения времени
    bool timeMatch = (h1 == h2 && h2 == h3) && (m1 == m2 && m2 == m3);

    if (timeMatch) {
        // Если совпало — обновляем предыдущее значение и возвращаем новое
        PreviousTimeInMinutes = h1 * 60 + m1;
        return PreviousTimeInMinutes;
    } else {
        // Иначе возвращаем предыдущее значение
        Serial.println("RTC minute read mismatch — using previous time.");
        return PreviousTimeInMinutes;
    }
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

