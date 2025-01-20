#ifndef SENSORS_MODULE_H
#define SENSORS_MODULE_H

#include <Arduino.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "Pinout.h"

// Объявление глобальной переменной
extern uint8_t sensorState;

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

// Объявление переменных для хранения состояния датчиков температуры и влажности
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

// Объявление переменных для датчиков температуры
extern float water_temperature_osmo;
extern float water_temperature_watering;
extern float air_temperature_outdoor;
extern float air_temperature_inlet;

// Объявление переменных для датчиков качества воды
extern float CO2;
extern float ph_osmo;
extern float tds_osmo;

// Объявление переменных для мониторинга питающей сети
extern float power_monitor;
extern uint8_t buttonState;

// Функции модуля
void initializeSensors(); // Инициализация всех сенсоров
void updateSensors(); // Обновление состояния всех сенсоров
uint8_t readPCF8574(); // Чтение состояния датчиков холла
void handleButtonState(); // Обработка состояния кнопок
SensorData readHTU21D(uint8_t address); // Чтение данных с датчика HTU21D
void readAllHTU21D(); // Чтение данных с пяти датчиков HTU21D
void readAllDS18B20(); // Чтение данных с четырех датчиков DS18B20

#endif // SENSORS_MODULE_H