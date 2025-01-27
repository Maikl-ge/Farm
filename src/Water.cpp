#include <Arduino.h>
#include "SensorsModule.h"
#include "water.h"
#include <CurrentProfile.h>
#include <globals.h>
#include <pinout.h>

// Глобальные переменные
float hysteresis;
float tempRange;

// Инициализация пинов для управления водой
void setupWater() {
    pinMode(OSMOS_ON_PIN, OUTPUT);
    pinMode(PUMP_TRANSFER_PIN, OUTPUT);
    pinMode(HITER_WATER_PIN, OUTPUT);

    digitalWrite(OSMOS_ON_PIN, LOW);
    digitalWrite(PUMP_TRANSFER_PIN, LOW);
    digitalWrite(HITER_WATER_PIN, LOW);

    hysteresis = 0.5; // Гистерезис ±0.5°C для стабильности
    tempRange = 10;  // Диапазон пропорционального регулирования (в °C)
}

//Обработка состояния датчиков холла
void controlWaterLevel() {
    if (max_osmo_level == 1 && min_osmo_level == 0) {
       digitalWrite (OSMOS_ON_PIN, LOW);  // Выключаем установку обратного осмоса
    }

    if (min_osmo_level == 0 && max_osmo_level == 1) {
        digitalWrite (OSMOS_ON_PIN, HIGH);  // Включаем установку обратного осмоса
    }
    
    if (max_water_level == 1 && min_water_level == 0) {
        digitalWrite (PUMP_TRANSFER_PIN, LOW);  // Выключаем насос перекачки в бак полива
    }
    
    if (min_water_level == 0 && max_water_level == 1) {
        digitalWrite (PUMP_TRANSFER_PIN, HIGH);  // Включаем насос перекачки в бак полива
    }
}

// Функция управления нагревом воды
void controlWaterHeater() {
    // Вычисление ошибки температуры
    float tempError = WATER_TEMPERATURE - water_temperature_osmo;

    // Управление нагревателем
    if (tempError > hysteresis) {
        // Температура ниже диапазона - включаем нагрев на максимум
        analogWrite(HITER_WATER_PIN, 255);
    } else if (tempError < -hysteresis) {
        // Температура выше диапазона - полностью выключаем нагрев
        analogWrite(HITER_WATER_PIN, 0);
    } else {
        // Пропорциональное регулирование внутри диапазона
        int pwmValue = map(tempError, -hysteresis, hysteresis, 0, 255);
        pwmValue = constrain(pwmValue, 0, 255); // Ограничение значения PWM
        analogWrite(HITER_WATER_PIN, pwmValue);
    }
}

void updateWater() {
    readPCF8574();
    controlWaterLevel();
    controlWaterHeater();
}





