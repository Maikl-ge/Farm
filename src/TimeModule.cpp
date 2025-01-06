#include <Arduino.h>
#include "TimeModule.h"
#include <Rtc_Pcf8563.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <Pinout.h>

int8_t timeZone = 0; // Часовой пояс
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
}

// Вывод текущего времени
void printCurrentTime() {
    // Получаем текущее время
    rtc.getDateTime();

    // Формирование даты в формате YYYYMMDD
    CurrentDate = (rtc.getYear() + 2000) * 10000 + rtc.getMonth() * 100 + rtc.getDay();

    // Формирование времени в формате HHMMSS
    CurrentTime = rtc.getHour() * 10000 + rtc.getMinute() * 100 + rtc.getSecond();

    // Вывод даты и времени в Serial
    Serial.print("Дата: ");
    Serial.print(CurrentDate / 10000);       // Год
    Serial.print('-');
    Serial.print((CurrentDate / 100) % 100); // Месяц
    Serial.print('-');
    Serial.print(CurrentDate % 100);         // День
    Serial.print(" Время: ");
    Serial.print(CurrentTime / 10000);       // Часы
    Serial.print(':');
    Serial.print((CurrentTime / 100) % 100); // Минуты
    Serial.print(':');
    Serial.println(CurrentTime % 100);       // Секунды
}