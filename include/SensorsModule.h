#ifndef SENSORS_MODULE_H
#define SENSORS_MODULE_H

#include <Arduino.h>

// Пины подключения датчиков
#define ANALOG_SENSOR_PIN 34 // Пин для аналогового датчика
#define DIGITAL_SENSOR_PIN 27 // Пин для цифрового датчика

// Интервал опроса датчиков в миллисекундах
#define SENSOR_READ_INTERVAL 1000

// Структура для хранения данных с датчиков
struct SensorData {
    int analogValue;
    bool digitalState;
};

// Функции модуля
void initializeSensors(); // Инициализация датчиков
SensorData readSensors(); // Опрос датчиков

#endif // SENSORS_MODULE_H
