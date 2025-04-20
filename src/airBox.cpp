#include <Arduino.h>
#include <airBox.h>
#include <pinout.h>
#include <globals.h>
#include <SensorsModule.h>
#include <status.h>

// Константы
const int PWM_FREQUENCY = 5000;
const int PWM_RESOLUTION = 10;
const int HITER_AIR_CHANNEL = 3;
const int FAN_INLET_CHANNEL = 4;
const float TEMP_TOLERANCE = 0.25;
const float HUM_TOLERANCE = 1.0;
const int MIN_PWM = 0;
const int MAX_PWM = 1000;
bool steamActive = LOW;
// Коэффициенты пропорционального управления
const float K_TEMP = 100.0; // Коэффициент для нагревателя (PWM на °C ошибки)
const float K_FAN_TEMP = 150.0; // Коэффициент для вентилятора по температуре (PWM на °C)
const float K_FAN_HUM = 10.0;  // Коэффициент для вентилятора по влажности (PWM на %)

// Коэффициент сглаживания (0.0–1.0, чем меньше, тем плавнее)
const float ALPHA = 0.5;

void setupClimateControl() {

    ledcSetup(HITER_AIR_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(HITER_AIR_PIN, HITER_AIR_CHANNEL);
    ledcWrite(HITER_AIR_CHANNEL, 0);

    ledcSetup(FAN_INLET_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(FAN_INLET_PIN, FAN_INLET_CHANNEL);
    ledcWrite(FAN_INLET_CHANNEL, MIN_PWM);

    pinMode(STEAM_IN_PIN, OUTPUT);
    digitalWrite(STEAM_IN_PIN, LOW);
}

void updateClimateControl() {
    if(statusFarm == "Work" || statusFarm == "Pause") {
        unsigned long currentTime = millis();
        static unsigned long lastUpdateClimatTime = 0;
        if (currentTime - lastUpdateClimatTime < 250) return; // Интервал 1 секунда

        // Установка текущих и целевых значений
        float tempInput = temperature_1;
        float tempSetpoint = currentTemperatura;
        float humInput = humidity_1;
        float humSetpoint = currentHumidity;

        // Проверка условий
        bool tempHigh = tempInput > tempSetpoint + TEMP_TOLERANCE;
        bool tempLow = tempInput < tempSetpoint - TEMP_TOLERANCE;
        bool humHigh = humInput > humSetpoint + HUM_TOLERANCE;
        bool humLow = humInput < humSetpoint - HUM_TOLERANCE;

        // Расчёт ошибок
        float tempError = tempSetpoint - tempInput; // Ошибка для нагревателя
        float fanTempError = tempInput - tempSetpoint; // Ошибка температуры для вентилятора
        float fanHumError = humInput - humSetpoint;   // Ошибка влажности для вентилятора

        // Переменные для управления (рассчитанные значения)
        float newTempOutput = 0.0; // Новое значение для нагревателя
        float newFanOutput = MIN_PWM; // Новое значение для вентилятора

        // Статические переменные для хранения предыдущих значений
        static float smoothedTempOutput = 0.0;
        static float smoothedFanOutput = MIN_PWM;

        // === Комбинированная логика на основе таблицы ===

        if (tempHigh && humHigh) {
            // ↑ ↑ : Вентиляция
            newFanOutput = max(K_FAN_TEMP * fanTempError, K_FAN_HUM * fanHumError);
        }

        else if (tempHigh && !humHigh && !humLow) {
            // ↑ = : Вентиляция
            newFanOutput = K_FAN_TEMP * fanTempError;
        }

        else if (tempHigh && humLow) {
            // ↑ ↓ : Только увлажнение + ограниченная вентиляция
            steamActive = true;
        
            float fanTempOutput = K_FAN_TEMP * fanTempError;
            float limitedFanOutput = constrain(fanTempOutput, MIN_PWM, MAX_PWM * 0.5);
            newFanOutput = limitedFanOutput;
        }

        else if (!tempHigh && !tempLow && humHigh) {
            // = ↑ : Вентиляция
            newFanOutput = K_FAN_HUM * fanHumError;
        }

        else if (!tempHigh && !tempLow && !humHigh && !humLow) {
            // = = : Idle
            newFanOutput = MIN_PWM; newTempOutput = MIN_PWM; steamActive = false;
        }

        else if (!tempHigh && !tempLow && humLow) {
            // = ↓ : Только увлажнение
            steamActive = true;
            newFanOutput = MIN_PWM;
        }

        else if (tempLow && humHigh) {
            // ↓ ↑ : Нагрев + Вентиляция (без увлажнения)
            newTempOutput = K_TEMP * tempError;
            newFanOutput = K_FAN_HUM * fanHumError;
        }

        else if (tempLow && !humHigh && !humLow) {
            // ↓ = : Только нагрев
            newTempOutput = K_TEMP * tempError;
        }

        else if (tempLow && humLow) {
            // ↓ ↓ : Нагрев + увлажнение
            newTempOutput = K_TEMP * tempError;
            steamActive = true;
        }

        // Применение сглаживания
        smoothedTempOutput = ALPHA * newTempOutput + (1.0 - ALPHA) * smoothedTempOutput;
        smoothedFanOutput = ALPHA * newFanOutput + (1.0 - ALPHA) * smoothedFanOutput;

        // Ограничение значений после сглаживания
        if (smoothedTempOutput > MAX_PWM) smoothedTempOutput = MAX_PWM;
        if (smoothedTempOutput < MIN_PWM) smoothedTempOutput = MIN_PWM;
        if (smoothedFanOutput > MAX_PWM) smoothedFanOutput = MAX_PWM;
        if (smoothedFanOutput < MIN_PWM) smoothedFanOutput = MIN_PWM;

        // Приведение к целому типу для ledcWrite
        int tempOutput = (int)smoothedTempOutput;
        int fanOutput = (int)smoothedFanOutput;

        // Обновление глобальных переменных
        HITER_AIR = tempOutput;
        FAN_INLET = fanOutput;
        STEAM_IN = steamActive;

        lastUpdateClimatTime = currentTime;
        // Применение значений к выходам
        digitalWrite(STEAM_IN_PIN, steamActive);
        ledcWrite(HITER_AIR_CHANNEL, tempOutput);
        ledcWrite(FAN_INLET_CHANNEL, fanOutput);         
    }

}
