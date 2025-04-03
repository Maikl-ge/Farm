#include <Arduino.h>
#include "SensorsModule.h"
#include <Profile.h>
#include "water.h"
#include <globals.h>
#include <pinout.h>

// Глобальные переменные
float hysteresis;
float tempRange;

// Инициализация пинов для управления водой
void setupWater() {
    pinMode(OSMOS_ON_PIN, OUTPUT);  // Пии клапана подачи чистой воды
    pinMode(PUMP_TRANSFER_PIN, OUTPUT);

    digitalWrite(OSMOS_ON_PIN, LOW);
    digitalWrite(PUMP_TRANSFER_PIN, LOW);  

    ledcSetup(5, 5000, 10); // Настройка канала PWM для нагрева воды
    ledcAttachPin(HITER_WATER_PIN, 5); // Привязка канала PWM к пину нагрева воды

    hysteresis = 0.2; // Гистерезис ±0.5°C для стабильности
    tempRange = 15;  // Диапазон пропорционального регулирования (в °C)
}

// Обработка состояния датчиков уровня воды
void controlWaterLevel() {
    if (max_osmo_level == 1) {
        digitalWrite(OSMOS_ON_PIN, LOW);  // Выключаем осмос, если бак полный
    } else if (min_osmo_level == 0 && max_osmo_level == 0) {
        digitalWrite(OSMOS_ON_PIN, HIGH); // Включаем осмос, если бак пуст
    }

    if (max_water_level == 1) {
        digitalWrite(PUMP_TRANSFER_PIN, LOW);  // Выключаем насос, если бак полива полный
    } else if (min_water_level == 0 && max_water_level == 0) {
        digitalWrite(PUMP_TRANSFER_PIN, HIGH); // Включаем насос, если бак полива пуст
    }
}

// Управление нагревателем воды
void controlWaterHeater() {
    float tempError = currentWaterTemperatura - water_temperature_osmo;

    if (tempError > hysteresis) {
        // Температура ниже - включаем нагрев воды в баке полива
        ledcWrite(6, 1023);
    } else if (tempError < -hysteresis) {
        // Температура выше - выключаем нагрев воды в баке полива
        ledcWrite(6, 0);
    } else {
        // Пропорциональное управление
        float scaledError = tempError / tempRange; // Масштабируем ошибку
        int pwmValue = map(scaledError * 1023, -1023, 1023, 0, 1023);
        pwmValue = constrain(pwmValue, 0, 1023);
        ledcWrite(6, pwmValue);
    }
}

void updateWater() {
    readPCF8574();
    controlWaterLevel();
    controlWaterHeater();
}
