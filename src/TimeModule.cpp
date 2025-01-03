#include "TimeModule.h"
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Ds1302.h>
#include <Pinout.h>

uint32_t CurrentDate = 0; // Текущая дата фермы
uint32_t CurrentTime = 0; // Текущее время фермы

// Создаем объект DS1302
Ds1302 rtc(DS1302_CE, DS1302_IO, DS1302_CLK);

// Настройки NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000); // Сдвиг времени = 0 (UTC)

// Инициализация модуля времени
void initTimeModule() {
    rtc.init();           // Инициализация DS1302
    timeClient.begin();   // Запуск NTP клиента
}

// Синхронизация времени с NTP сервером
void syncTimeWithNTP(const char* ntpServer) { // Значение по умолчанию только в .h!
    timeClient.setPoolServerName(ntpServer); // Установка адреса NTP сервера

    Serial.println("Synchronizing time with NTP server...");
    while (!timeClient.update()) {
        timeClient.forceUpdate(); // Принудительное обновление времени
        delay(500);
    }

    // Получение времени эпохи
    unsigned long epochTime = timeClient.getEpochTime();

    // Преобразуем время в структуру DateTime для DS1302
    Ds1302::DateTime dt;
    dt.year = (epochTime / 31557600) + 1970 - 2000;  // Преобразуем в формат 00-99
    dt.month = (epochTime / 2629746) % 12 + 1;      // Месяц
    dt.day = (epochTime / 86400) % 31 + 1;          // День
    dt.hour = (epochTime / 3600) % 24;              // Час
    dt.minute = (epochTime / 60) % 60;              // Минуты
    dt.second = epochTime % 60;                     // Секунды

    // Устанавливаем время в DS1302
    rtc.setDateTime(&dt);
    Serial.println("Time synchronized successfully.");
}

// Вывод текущего времени
void printCurrentTime() {
    Ds1302::DateTime now;
    rtc.getDateTime(&now);  // Получаем текущее время

    // Формирование даты в формате YYYYMMDD
    CurrentDate = (now.year * 10000) + (now.month * 100) + now.day;

    // Формирование времени в формате HHMMSS
    CurrentTime = (now.hour * 10000) + (now.minute * 100) + now.second;

    // Вывод даты и времени в Serial
    // Serial.print("Дата: ");
    // Serial.print(CurrentDate / 10000);       // Год
    // Serial.print('-');
    // Serial.print((CurrentDate / 100) % 100); // Месяц
    // Serial.print('-');
    // Serial.println(CurrentDate % 100);      // День

    // Serial.print("Время: ");
    // Serial.print(CurrentTime / 10000);       // Часы
    // Serial.print(':');
    // Serial.print((CurrentTime / 100) % 100); // Минуты
    // Serial.print(':');
    // Serial.println(CurrentTime % 100);      // Секунды
    // Serial.print("Дата: ");
    // Serial.print(CurrentDate);       // Год
    // Serial.print(" Время: ");
    // Serial.print(CurrentTime);       // Часы
}