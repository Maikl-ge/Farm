#include "TimeModule.h"
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Ds1302.h>
#include <Pinout.h>


// Создаем объект DS1302
Ds1302 rtc(DS1302_CE, DS1302_IO, DS1302_CLK);

// Массив с днями недели для отображения
const static char* WeekDays[] = {
    "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"
};

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
    dt.dow = (epochTime / 86400 + 4) % 7 + 1;       // День недели (1-7)

    // Устанавливаем время в DS1302
    rtc.setDateTime(&dt);
    Serial.println("Time synchronized successfully.");
}

// Вывод текущего времени
void printCurrentTime() {
    Ds1302::DateTime now;
    rtc.getDateTime(&now);  // Получаем текущее время

    static uint8_t last_second = 0;
    if (last_second != now.second) {
        last_second = now.second;

        // Выводим дату и время в формате YYYY-MM-DD DDD HH:MM:SS
        Serial.print("20");
        Serial.print(now.year);    // 00-99
        Serial.print('-');
        if (now.month < 10) Serial.print('0');
        Serial.print(now.month);   // 01-12
        Serial.print('-');
        if (now.day < 10) Serial.print('0');
        Serial.print(now.day);     // 01-31
        Serial.print(' ');
        Serial.print(WeekDays[now.dow - 1]); // День недели
        Serial.print(' ');
        if (now.hour < 10) Serial.print('0');
        Serial.print(now.hour);    // 00-23
        Serial.print(':');
        if (now.minute < 10) Serial.print('0');
        Serial.print(now.minute);  // 00-59
        Serial.print(':');
        if (now.second < 10) Serial.print('0');
        Serial.print(now.second);  // 00-59
        Serial.println();
    }
}