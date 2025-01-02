#ifndef SENSORS_MODULE_H
#define SENSORS_MODULE_H

#include <Arduino.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "Pinout.h" // Подключаем Pinout.h

// Объявления переменных (extern)
extern bool start_Button;
extern bool stop_Button;
extern bool mode_Button;

// Прототипы функций
void readButtons();

// Структура для хранения состояния датчиков холла
struct HallSensorState {
    bool sensor1;
    bool sensor2;
    bool sensor3;
    bool sensor4;
};

// Функции модуля
void initializeSensors(); // Инициализация кнопок
void updateSensors(); // Обновление состояния всех сенсоров
void readButtons(); // Опрос кнопок
HallSensorState readHallSensors(); // Чтение состояния датчиков холла
void handleButtonState(); // Обработка состояния кнопок
void updateButtonState(unsigned long currentMillis, unsigned long buttonInterval, unsigned long &lastButtonUpdate); // Обновление состояния кнопок

#endif // SENSORS_MODULE_H