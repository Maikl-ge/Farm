#include <Arduino.h>
#include "TimeModule.h"
#include <Rtc_Pcf8563.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <Pinout.h>

int8_t timeZone = 3; // Часовой пояс
uint32_t CurrentDate = 0; // Текущая дата фермы
uint32_t CurrentTime = 0; // Текущее время фермы

// Инициализация экземпляра RTC
Rtc_Pcf8563 rtc;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);

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

void syncTimeWithNTP(const char* ntpServer) {
    timeClient.setPoolServerName(ntpServer); // Установка адреса NTP сервера

    Serial.println("Synchronizing time with NTP server...");
    while (!timeClient.update()) {
        delay(500);
    }

    unsigned long epochTime = timeClient.getEpochTime();

    // Преобразуем время в структуру tm
    struct tm *ptm = gmtime((time_t *)&epochTime);
    rtc.setDateTime(ptm->tm_mday, ptm->tm_wday, ptm->tm_mon + 1, 0, ptm->tm_year % 100, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

    Serial.println("Time synchronized successfully.");

CurrentDate = (ptm->tm_year + 1900) * 10000 + (ptm->tm_mon + 1) * 100 + ptm->tm_mday;
CurrentTime = ptm->tm_hour * 10000 + ptm->tm_min * 100 + ptm->tm_sec;
// Вывод даты и времени для проверки
Serial.printf("Current Date (YYYYMMDD): %lu\n", CurrentDate);
Serial.printf("Current Time (HHMMSS): %lu\n", CurrentTime);
Serial.println("Time synchronized successfully.");

}

// Вывод текущего времени
void printCurrentTime() {
    // Получаем текущее время
    rtc.getDateTime();

    // Формирование даты в формате YYYYMMDD
    CurrentDate = (rtc.getYear() + 2000) * 10000 + rtc.getMonth() * 100 + rtc.getDay();

    // Формирование времени в формате HHMMSS
    CurrentTime = rtc.getHour() * 10000 + rtc.getMinute() * 100 + rtc.getSecond();

    // // Вывод даты и времени в Serial
    // Serial.print("Current time: ");
    // Serial.println(CurrentTime);
    // Serial.print("Current date: ");
    // Serial.println(CurrentDate);

}