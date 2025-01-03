#ifndef SENSORS_MODULE_H
#define SENSORS_MODULE_H

#include <Arduino.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "Pinout.h" // Подключаем Pinout.h

// Объявления переменных (extern) состояния кнопок
extern bool start_Button;
extern bool stop_Button;
extern bool mode_Button;

// Прототипы функций опроса кнопок
void readButtons();
void readHallSensors();

// Структура для хранения состояния датчиков холла

extern bool max_osmo_level;
extern bool min_osmo_level;
extern bool max_water_level;
extern bool min_water_level;

// Функции модуля
void initializeSensors(); // Инициализация кнопок
void updateSensors(); // Обновление состояния всех сенсоров
void readButtons(); // Опрос кнопок
void readHallSensors(); // Чтение состояния датчиков холла
void handleButtonState(); // Обработка состояния кнопок
void updateButtonState(unsigned long currentMillis, unsigned long buttonInterval, unsigned long &lastButtonUpdate); // Обновление состояния кнопок

#endif // SENSORS_MODULE_H