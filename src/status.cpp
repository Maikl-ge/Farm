// status.cpp
#include <Arduino.h>
#include "status.h"
#include "TimeModule.h"
#include "Profile.h"

// Определение текущего статуса фермы
void currentStatusFarm() {
    uint16_t currentTimeInMinutes = getCurrentTimeInMinutes();

    Serial.println("Время в минутах: " + String(currentTimeInMinutes));
    uint16_t elapsedTime = subtractTimes(currentTimeInMinutes, GROWE_MODE_TIME);
    Serial.println("Прошло " + String(elapsedTime) + " минут с начала цикла роста");
}

// Функция для сложения двух значений времени в формате HHMM
uint16_t addTimes(uint16_t time1, uint16_t time2) {
    // Разбиваем первое время на часы и минуты
    uint8_t hours1 = time1 / 100;
    uint8_t minutes1 = time1 % 100;

    // Разбиваем второе время на часы и минуты
    uint8_t hours2 = time2 / 100;
    uint8_t minutes2 = time2 % 100;

    // Складываем минуты и часы
    uint8_t totalMinutes = minutes1 + minutes2;
    uint8_t totalHours = hours1 + hours2 + totalMinutes / 60;

    // Приводим значения к формату HHMM
    totalMinutes %= 60;
    uint16_t result = totalHours * 100 + totalMinutes;
    return result;
}

// Функция для вычитания двух значений времени в формате HHMM
uint16_t subtractTimes(uint16_t time1, uint16_t time2) {
    // Разбиваем первое время на часы и минуты
    uint8_t hours1 = time1 / 100;
    uint8_t minutes1 = time1 % 100;

    // Разбиваем второе время на часы и минуты
    uint8_t hours2 = time2 / 100;
    uint8_t minutes2 = time2 % 100;

    // Вычитаем минуты и часы
    int totalMinutes = (hours1 * 60 + minutes1) - (hours2 * 60 + minutes2);

    // Приводим значения к формату HHMM
    uint8_t totalHours = totalMinutes / 60;
    totalMinutes %= 60;
    uint16_t result = totalHours * 100 + totalMinutes;
    return result;
}