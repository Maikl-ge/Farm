#include <Arduino.h>
#include "SensorsModule.h"
#include "water.h"
#include <CurrentProfile.h>

// Глобальные переменные
float hysteresis;
float tempRange;

// Инициализация пинов для управления водой
void setupWater() {
    pinMode(OSMOS_ON_PIN, OUTPUT);
    pinMode(PUMP_2_PIN, OUTPUT);
    pinMode(HITER_WATER_PIN, OUTPUT);

    digitalWrite(OSMOS_ON_PIN, LOW);
    digitalWrite(PUMP_2_PIN, LOW);
    digitalWrite(HITER_WATER_PIN, LOW);

    hysteresis = 0.5; // Гистерезис ±0.5°C для стабильности
    tempRange = 10;  // Диапазон пропорционального регулирования (в °C)
}

// Обработка нажатия кнопок
void updateButtonState() {
    if (modeButtonPressed && startButtonPressed) {
        modeButtonPressed = false; // Сброс флага
        startButtonPressed = false; // Сброс флага
        Serial.println("START");
    }   
    if (modeButtonPressed && stopButtonPressed) {
        modeButtonPressed = false; // Сброс флага
        stopButtonPressed = false; // Сброс флага
        Serial.println("STOP");
    }  
    if (startButtonPressed) {
        startButtonPressed = false; // Сброс флага
        Serial.println("Start button pressed");
    }
    if (stopButtonPressed) {
        stopButtonPressed = false; // Сброс флага
        Serial.println("Stop button pressed");
    }  
}

void sendMessagetoStatus() {
    Serial.println("Power monitor: ERROR");
}

//Обработка состояния датчиков холла
void controlWaterLevel() {
    if (max_osmo_level == 1 && min_osmo_level == 0) {
       digitalWrite (OSMOS_ON_PIN, LOW);
    }

    if (min_osmo_level == 0 && max_osmo_level == 1) {
        digitalWrite (OSMOS_ON_PIN, HIGH);
    }
    
    if (max_water_level == 1 && min_water_level == 0) {
        digitalWrite (PUMP_2_PIN, LOW);
    }
    
    if (min_water_level == 0 && max_water_level == 1) {
        digitalWrite (PUMP_2_PIN, HIGH);
    }
}

// Функция мониторинга питания
void powerMonitor() {
    // Чтение состояния мониторинга питающей сети
    if (power_monitor == 0) {
        sendMessagetoStatus();
    }
}

void updateButtonWater() {
    readPCF8574();
    controlWaterLevel();
    updateButtonState();
    powerMonitor();
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
        int pwmValue = 255 * (tempError + hysteresis) / (2 * hysteresis + tempRange);
        pwmValue = constrain(pwmValue, 0, 255); // Ограничение значения PWM
        analogWrite(HITER_WATER_PIN, pwmValue);
    }
}






