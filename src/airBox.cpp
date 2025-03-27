#include <Arduino.h>
#include <airBox.h>
#include <pinout.h>
#include <globals.h>
#include <SensorsModule.h>
#include <status.h>

void CurrentStatusFarm();

// Константы
const int PWM_FREQUENCY = 5000;
const int PWM_RESOLUTION = 10;
const int HITER_AIR_CHANNEL = 2;
const int FAN_INLET_CHANNEL = 3;
const float TEMP_TOLERANCE = 0.5;
const float HUM_TOLERANCE = 1.0;
const int MIN_PWM = 0;
const int MAX_PWM = 1023;

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
    unsigned long currentTime = millis();
    static unsigned long lastUpdateClimatTime = 0;
    if (currentTime - lastUpdateClimatTime < 1000) return; // Интервал 1 секунда

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

    // Логика управления
    if (tempLow || humLow) {
        // Температура ниже заданной
        if (tempLow) {
            newTempOutput = K_TEMP * tempError;
            if (newTempOutput > MAX_PWM) newTempOutput = MAX_PWM;
            if (newTempOutput < 0) newTempOutput = 0;
        }
        // Влажность ниже заданной
        if (humLow) {
            digitalWrite(STEAM_IN_PIN, HIGH);
            STEAM_IN = HIGH;
        } else {
            digitalWrite(STEAM_IN_PIN, LOW);
            STEAM_IN = LOW;
        }
        newFanOutput = MIN_PWM; // Вентилятор выключен
    } else if (tempHigh || humHigh) {
        // Температура или влажность выше заданной
        newTempOutput = 0; // Нагреватель выключен
        // Вентилятор включается по максимальной ошибке (температура или влажность)
        float fanTempOutput = K_FAN_TEMP * fanTempError;
        float fanHumOutput = K_FAN_HUM * fanHumError;
        newFanOutput = max(fanTempOutput, fanHumOutput); // Максимум из двух ошибок
        if (newFanOutput > MAX_PWM) newFanOutput = MAX_PWM;
        if (newFanOutput < MIN_PWM) newFanOutput = MIN_PWM;
        digitalWrite(STEAM_IN_PIN, LOW);
        STEAM_IN = LOW;
    } else {
        // Всё в пределах допуска
        newTempOutput = 0;
        newFanOutput = MIN_PWM;
        digitalWrite(STEAM_IN_PIN, LOW);
        STEAM_IN = LOW;
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

    // Применение значений к выходам
    ledcWrite(HITER_AIR_CHANNEL, tempOutput);
    ledcWrite(FAN_INLET_CHANNEL, fanOutput);

    // Обновление глобальных переменных
    HITER_AIR = tempOutput;
    FAN_INLET = fanOutput;

    lastUpdateClimatTime = currentTime;
}