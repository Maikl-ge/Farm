#include <Arduino.h>
#include <airBox.h>
#include <pinout.h>
#include <globals.h>
#include <SensorsModule.h>
#include <status.h>

// Константы
const int PWM_FREQUENCY = 20000;
const int PWM_RESOLUTION = 10;
const int HITER_AIR_CHANNEL = 3;
const int FAN_INLET_CHANNEL = 4;
const float TEMP_TOLERANCE = 0.25;
const float HUM_TOLERANCE = 3;
const int MIN_PWM = 0;
const int MAX_PWM = 1000;
float HUN_CORRECTION = 0.0; // Коррекция влажности
bool steamActive = LOW;
unsigned long currentInletTime = 0;
static unsigned long lastUpdatInletTime = 0;
// Переменные принудительной вентиляции
unsigned long lastFanOnTime = 0;      // Последнее включение вентилятора
unsigned long fanOnDuration = 5 * 1000;     // 30 секунд
unsigned long fanMinInterval = 16 * 60 * 1000; // 16 минут
unsigned long fanTriggerInterval = 15 * 60 * 1000; // каждые 15 минут
bool fanForced = false;               // Флаг принудительного включения
unsigned long fanForcedStartTime = 0; // Время старта принудительного включения
int fanOutput;
unsigned long lastFanTriggerTime = 0;
unsigned long lastFanActualOnTime = 0;

// Коэффициенты пропорционального управления
const float K_TEMP = 2000.0; // Коэффициент для нагревателя (PWM на °C ошибки)
const float K_FAN_TEMP = 500.0; // Коэффициент для вентилятора по температуре (PWM на °C)
const float K_FAN_HUM = 100.0;  // Коэффициент для вентилятора по влажности (PWM на %)

// Статические переменные для хранения предыдущих значений
static float smoothedTempOutput = 0.0;
static float smoothedFanOutput = MIN_PWM;

// Переменные для управления (рассчитанные значения)
float newTempOutput = 0.0; // Новое значение для нагревателя
float newFanOutput = MIN_PWM; // Новое значение для вентилятора

// Коэффициент сглаживания (0.0–1.0, чем меньше, тем плавнее)
const float ALPHA = 0.5;

// Флаги 
bool pauseInlet = false; // Флаг для паузы подачи воздуха

void setupClimateControl() {

    ledcSetup(HITER_AIR_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(HITER_AIR_PIN, HITER_AIR_CHANNEL);
    ledcWrite(HITER_AIR_CHANNEL, 0);

    ledcSetup(FAN_INLET_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(FAN_INLET_PIN, FAN_INLET_CHANNEL);
    ledcWrite(FAN_INLET_CHANNEL, MIN_PWM);

    pinMode(STEAM_IN_PIN, OUTPUT);
    digitalWrite(STEAM_IN_PIN, LOW);
    lastUpdatInletTime = 60000;
}

void updateClimateControl() {
    if(statusFarm == "Work" || statusFarm == "Pause") {
        currentInletTime = millis();

        // Установка текущих и целевых значений
        float tempInput = temperatureInBox;
        float tempSetpoint = currentTemperatura + TEMP_TOLERANCE;
        float humInput = (humidityInBox + HUN_CORRECTION);
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

        // === Комбинированная логика на основе таблицы ===

        if (tempHigh && humHigh) {
            // ↑ ↑ : Вентиляция
            newFanOutput = max(K_FAN_TEMP * fanTempError, K_FAN_HUM * fanHumError);
            steamActive = false;  // Парогенератор выключен
            newTempOutput = MIN_PWM; // Сброс нагревателя
        }

        else if (tempHigh && !humHigh) {
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
            newTempOutput = MIN_PWM; 
            newFanOutput = K_FAN_HUM * fanHumError;
            steamActive = true;
        }
        // Температура и влажность в норме
        else if (!tempHigh && !tempLow && !humHigh && !humLow) {
            // = = : Idle
            newFanOutput = MIN_PWM; 
            newTempOutput = MIN_PWM; 
            steamActive = false;
        }
        // Температура нормальная, влажность низкая
        else if (!tempHigh && !tempLow && humLow) {
            // = ↓ : Только увлажнение
            steamActive = true;
            newTempOutput = MIN_PWM;
            newFanOutput = MIN_PWM;
        }
        // Холодно и влажно
        else if (tempLow && humHigh) {
            // ↓ ↑ : Нагрев + Вентиляция (без увлажнения)
            newTempOutput = K_TEMP * tempError;
            newFanOutput = K_FAN_HUM * fanHumError;
            steamActive = false;
        }
        // Холодно, влажность в норме
        else if (tempLow && !humHigh && !humLow) {
            // ↓ = : Только нагрев
            newTempOutput = K_TEMP * tempError;
            steamActive = false;
        }
        // Холодно и сухо 
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
        fanOutput = (int)smoothedFanOutput;

        if(tempSetpoint < tempInput) {
           tempOutput =  MIN_PWM;
        }
        if((humSetpoint - HUM_TOLERANCE) > humInput) {
            steamActive = HIGH;   
        }

        if(humSetpoint + HUM_TOLERANCE < humInput) {
            steamActive = LOW;   
        }

        if(fanOutput <= 200) {
            fanOutput = 200;
        }
        if(tempSetpoint + TEMP_TOLERANCE >= tempInput && humInput <= humSetpoint + HUM_TOLERANCE) {
            fanOutput = 0;
        }
        // Обновление глобальных переменных
        HITER_AIR = tempOutput;
        FAN_INLET = fanOutput;
        STEAM_IN = steamActive;

        // lastUpdateClimatTime = currentTime;
        // Применение значений к выходам
        digitalWrite(STEAM_IN_PIN, steamActive);
        ledcWrite(HITER_AIR_CHANNEL, tempOutput);

        if(!fanForced) {
//fanOutput = 0;   // Удалить при добалении охлаждения
        ledcWrite(FAN_INLET_CHANNEL, fanOutput);  
        }
    }

    unsigned long now = millis();

    // Принудительное включение вентилятора каждые 15 минут,
    // если он не работал последние 16 минут
    if (!fanForced &&
        (now - lastFanTriggerTime >= fanTriggerInterval) &&
        (now - lastFanActualOnTime >= fanMinInterval)) {

        fanForced = true;
        fanForcedStartTime = now;
        lastFanTriggerTime = now;
        lastFanActualOnTime = now;

        int fanOutputInlet = 700;
        FAN_INLET = fanOutputInlet;
        ledcWrite(FAN_INLET_CHANNEL, fanOutputInlet);
    }

    // Выключение через 30 секунд
    if (fanForced && (now - fanForcedStartTime >= fanOnDuration)) {
        fanForced = false;

        int fanOutputInlet = 0;
        FAN_INLET = fanOutputInlet;
        ledcWrite(FAN_INLET_CHANNEL, fanOutputInlet);
    }        
}
