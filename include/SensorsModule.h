#ifndef SENSORS_MODULE_H
#define SENSORS_MODULE_H

#include <Arduino.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "Pinout.h" // Подключаем Pinout.h

// Прототипы функций опроса кнопок
void readButtons();
// Объявления переменных (extern) состояния кнопок
extern bool start_Button;
extern bool stop_Button;
extern bool mode_Button;

// Прототипы функций опроса датчиков холла
void readHallSensors();

// Объявления переменных для хранения состояния датчиков холла
extern bool max_osmo_level;
extern bool min_osmo_level;
extern bool max_water_level;
extern bool min_water_level;

// Структура для хранения данных с датчиков HTU21D
    struct SensorData {
    float temperature;
    float humidity;
};

//  Объявление переменных для хранения состояния датчиков температуры и влажности
extern float temperature_1;
extern float humidity_1;
extern float temperature_2;
extern float humidity_2;
extern float temperature_3;
extern float humidity_3;
extern float temperature_4;
extern float humidity_4;
extern float temperature_5;
extern float humidity_5;

// Объявление переменных для датчиков температыры 
extern float water_temperature_osmo;
extern float water_temperature_watering;
extern float air_temperature_outdoor;
extern float air_temperature_inlet;

// Объявление переменных для датчиков качества воды
extern float ph_osmo;
extern float tds_osmo;

// Объявление переменных для мониторинга питающей сети
extern bool power_monitor;

// Функции модуля
void initializeSensors(); // Инициализация кнопок
void updateSensors(); // Обновление состояния всех сенсоров
void readButtons(); // Опрос кнопок
void readHallSensors(); // Чтение состояния датчиков холла
void handleButtonState(); // Обработка состояния кнопок
void updateButtonState(unsigned long currentMillis, unsigned long buttonInterval, unsigned long &lastButtonUpdate); // Обновление состояния кнопок
SensorData readHTU21D(uint8_t address); // Чтение данных с датчика HTU21D
void readAllHTU21D(); // Чтение данных с пяти датчиков HTU21D
void readAllDS18B20(); // Чтение данных с четырех датчиков DS18B20

#endif // SENSORS_MODULE_H