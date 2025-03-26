#include <Arduino.h>
#include <airBox.h>
#include <pinout.h>
#include <globals.h>
#include <PID_v1.h>
#include <SensorsModule.h>
#include <status.h>

void CurrentStatusFarm();

// Константы
const int PWM_FREQUENCY = 5000;
const int PWM_RESOLUTION = 10;
const int HITER_AIR_CHANNEL = 0;
const int FAN_INLET_CHANNEL = 3;
const float TEMP_TOLERANCE = 0.5;
const float HUM_TOLERANCE = 1.0;
const int MIN_PWM = 100;  // Обновлено: теперь вентилятор всегда стартует
const int MAX_PWM = 1023;

// PID параметры (оптимизированные)
const double KP_TEMP = 40.0, KI_TEMP = 0.5, KD_TEMP = 5.0;
const double KP_FAN = 50.0, KI_FAN = 0.5, KD_FAN = 10.0;

// Переменные PID
double tempInput = 0.0, tempOutput = 0.0, tempSetpoint = 0.0;
double fanInput = 0.0, fanOutput = 0.0, fanSetpoint = 0.0;

// PID объекты
PID tempPID(&tempInput, &tempOutput, &tempSetpoint, KP_TEMP, KI_TEMP, KD_TEMP, DIRECT);
PID fanPID(&fanInput, &fanOutput, &fanSetpoint, KP_FAN, KI_FAN, KD_FAN, DIRECT);

unsigned long lastUpdateClimatTime = 0;
const unsigned long UPDATE_INTERVAL = 1000;

void setupClimateControl() {
    Serial.println("AIR BOX Init");

    ledcSetup(HITER_AIR_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(HITER_AIR_PIN, HITER_AIR_CHANNEL);
    ledcWrite(HITER_AIR_CHANNEL, 0);

    ledcSetup(FAN_INLET_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(FAN_INLET_PIN, FAN_INLET_CHANNEL);
    ledcWrite(FAN_INLET_CHANNEL, MIN_PWM);

    pinMode(STEAM_IN_PIN, OUTPUT);
    digitalWrite(STEAM_IN_PIN, LOW);

    tempPID.SetMode(AUTOMATIC);
    tempPID.SetOutputLimits(0, MAX_PWM);
    fanPID.SetMode(AUTOMATIC);
    fanPID.SetOutputLimits(MIN_PWM, MAX_PWM);
}

void updateClimateControl() {
    unsigned long currentTime = millis();
    if (currentTime - lastUpdateClimatTime < UPDATE_INTERVAL) return;

    // Устанавливаем целевые значения
    tempSetpoint = currentTemperatura;
    fanSetpoint = currentHumidity;  // Теперь вентилятор управляется по влажности

    // Читаем текущие показания
    tempInput = temperature_1;
    fanInput = humidity_1;  // Теперь PID работает по влажности, а не температуре

    // Определение состояний
    bool tempHigh = tempInput > tempSetpoint + TEMP_TOLERANCE;
    bool tempLow = tempInput < tempSetpoint - TEMP_TOLERANCE;
    bool humHigh = fanInput > fanSetpoint + HUM_TOLERANCE;
    bool humLow = fanInput < fanSetpoint - HUM_TOLERANCE;

    // Отладочная информация
    Serial.print("tempHigh: "); Serial.print(tempHigh);
    Serial.print(" | tempLow: "); Serial.print(tempLow);
    Serial.print(" | humHigh: "); Serial.print(humHigh);
    Serial.print(" | humLow: "); Serial.println(humLow);

    Serial.print("Fan Error: "); Serial.println(fanInput - fanSetpoint);

    // Логика управления
    if (tempLow || humLow) {
        fanPID.SetMode(MANUAL);
        fanOutput = MIN_PWM;  // Принудительный старт вентилятора

        if (tempLow) {
            tempPID.SetMode(AUTOMATIC);
            tempPID.Compute();
            Serial.print("Temp PID Output: "); Serial.println(tempOutput);
        } else {
            tempPID.SetMode(MANUAL);
            tempOutput = 0;
        }

        if (humLow) {
            digitalWrite(STEAM_IN_PIN, HIGH);
            STEAM_IN = HIGH;
        } else {
            digitalWrite(STEAM_IN_PIN, LOW);
            STEAM_IN = LOW;
        }
    } else if (tempHigh || humHigh) {
        tempPID.SetMode(MANUAL);
        tempOutput = 0;
    
        fanPID.SetMode(AUTOMATIC);
        fanPID.SetTunings(KP_FAN, KI_FAN, KD_FAN);  // Принудительно обновляем PID
        fanPID.Compute();
    
        // Минимальное значение для работы вентилятора
        if (fanOutput < 200) fanOutput = 200;
    
        Serial.print("Fan PID Output: "); Serial.println(fanOutput);
    
        digitalWrite(STEAM_IN_PIN, LOW);
        STEAM_IN = LOW;
    } else {
        tempPID.SetMode(MANUAL);
        fanPID.SetMode(MANUAL);
        tempOutput = 0;
        fanOutput = MIN_PWM;
        digitalWrite(STEAM_IN_PIN, LOW);
        STEAM_IN = LOW;
    }

    // Применение значений к выходам
    ledcWrite(HITER_AIR_CHANNEL, (int)tempOutput >= MIN_PWM ? (int)tempOutput : 0);
    ledcWrite(FAN_INLET_CHANNEL, fanOutput < 100 ? 100 : fanOutput);  // Защита от нестарта

    // Обновление глобальных переменных
    HITER_AIR = (int)tempOutput;
    FAN_RACK = (int)fanOutput;

    // Вывод в Serial Monitor
    Serial.print("Hiter PWM: "); Serial.print((int)tempOutput);
    Serial.print(" | Fan PWM: "); Serial.print((int)fanOutput);
    Serial.print(" | Steam State: "); Serial.print(digitalRead(STEAM_IN_PIN) ? "ON" : "OFF");
    Serial.print(" | temperature_1: "); Serial.print(temperature_1);
    Serial.print(" | humidity_1: "); Serial.print(humidity_1);
    Serial.print(" | Target Temp: "); Serial.print(currentTemperatura);
    Serial.print(" | Target Hum: "); Serial.println(currentHumidity);

    lastUpdateClimatTime = currentTime;
}