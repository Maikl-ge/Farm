#include <Arduino.h>
#include <pinout.h>
#include <status.h>
#include <globals.h>
#include <SensorsModule.h>

// Константы
const float WATER_HYSTERESIS = 0.25;       // Гистерезис температуры воды (±0.5°C)
const float WATER_TEMP_RANGE = 2.0;       // Диапазон пропорционального управления (±2°C)
const int PWM_CHANNEL = 5;                // Канал PWM для нагревателя
const int PWM_FREQ = 21000;                // Частота PWM (21 кГц)
const int PWM_RESOLUTION = 10;            // Разрешение PWM (10 бит, 0-1023)
const int PWM_MIN = 0;                    // Минимальное значение PWM
const int PWM_MAX = 1023;                 // Максимальное значение PWM
const unsigned long HEATER_ON_DELAY = 5000;  // Задержка перед включением нагревателя (мс)
const unsigned long HEATER_OFF_DELAY = 5; // Задержка перед выключением нагревателя (мс)
const int MIN_EFFECTIVE_PWM = 20;          // Минимальное эффективное значение PWM для нагрева

// Глобальные переменные таймеров
unsigned long heaterSafeTimerStart = 0;   // Время начала безопасного состояния (мс)
unsigned long heaterUnsafeTimerStart = 0; // Время начала небезопасного состояния (мс)
bool heaterReadyToTurnOn = false;         // Флаг готовности нагревателя к включению

void setupWater() {

    // Инициализация пина нагревателя
    pinMode(HITER_WATER_PIN, OUTPUT);
    digitalWrite(HITER_WATER_PIN, LOW);

    // Настройка PWM для нагревателя
    ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(HITER_WATER_PIN, PWM_CHANNEL);
    ledcWrite(PWM_CHANNEL, PWM_MIN);
}

void controlWaterLevel() {
    // Управление осмосом
min_osmo_level = 1;  // установил в 1 для теста
    if (max_osmo_level == 1) {
        digitalWrite(OSMOS_ON_PIN, LOW);  // Выключаем осмос, если бак полный
    }
    else if (min_osmo_level == 0 && max_osmo_level == 0) {
        digitalWrite(OSMOS_ON_PIN, HIGH); // Включаем осмос, если бак пуст
    }

    // Управление насосом подачи
    if (max_water_level == 1) {
        digitalWrite(PUMP_TRANSFER_PIN, LOW);  // Выключаем насос, если бак полива полный
    }
    else if (min_water_level == 0 && max_water_level == 0) {
        digitalWrite(PUMP_TRANSFER_PIN, HIGH); // Включаем насос, если бак полива пуст
    }
}

void controlWaterHeater() {
    // Проверка условий безопасности
    bool safeWaterLevel = (min_osmo_level == 1) && (max_osmo_level == 1 || max_osmo_level == 0);
    bool safeTempSensor = ((currentWaterTemperatura) > 0) && ((currentWaterTemperatura) < 50);

    // Управление таймерами безопасности
    if (safeWaterLevel && safeTempSensor) {
        heaterUnsafeTimerStart = 0;
        if (!heaterReadyToTurnOn) {
            if (heaterSafeTimerStart == 0) {
                heaterSafeTimerStart = millis();
            }
            else if (millis() - heaterSafeTimerStart >= HEATER_ON_DELAY) {
                heaterReadyToTurnOn = true;
            }
        }
    }
    else {
        heaterSafeTimerStart = 0;
        if (heaterReadyToTurnOn) {
            if (heaterUnsafeTimerStart == 0) {
                heaterUnsafeTimerStart = millis();
            }
            else if (millis() - heaterUnsafeTimerStart >= HEATER_OFF_DELAY) {
                heaterReadyToTurnOn = false;
                ledcWrite(PWM_CHANNEL, PWM_MIN);
                HITER_WATER = PWM_MIN;
                return;
            }
        }
        else {
            ledcWrite(PWM_CHANNEL, PWM_MIN);
            HITER_WATER = PWM_MIN;
            return;
        }
    }

    // Ожидание безопасных условий
    if (!heaterReadyToTurnOn) {
        ledcWrite(PWM_CHANNEL, PWM_MIN);
        HITER_WATER = PWM_MIN;
        return;
    }

    // Управление нагревом
    float tempError = (currentWaterTemperatura) - water_temperature_osmo;
    int pwmValue;

    if (tempError > WATER_HYSTERESIS) {
        pwmValue = PWM_MAX;
    }
    else if (tempError < -WATER_HYSTERESIS) {
        pwmValue = PWM_MIN;
    }
    else {
        float scaledError = tempError / WATER_TEMP_RANGE;
        pwmValue = map(scaledError * PWM_MAX, -PWM_MAX, PWM_MAX, PWM_MIN, PWM_MAX);
        pwmValue = constrain(pwmValue, PWM_MIN, PWM_MAX);
        if (pwmValue > PWM_MIN && pwmValue < MIN_EFFECTIVE_PWM) {
            pwmValue = MIN_EFFECTIVE_PWM;
        }
    }

    ledcWrite(PWM_CHANNEL, pwmValue);
    HITER_WATER = pwmValue;
    // Serial.print("Heater PWM: ");
    // Serial.println(pwmValue);
    digitalWrite(PUMP_TRANSFER_PIN, HIGH);   
    PUMP_TRANSFER = 1;    
    if(pwmValue == 0) {
        digitalWrite(PUMP_TRANSFER_PIN, LOW);
        PUMP_TRANSFER = 0;
    }
}

void updateWater() {
    readPCF8574(); // Раскомментировать для обновления данных с датчиков
    controlWaterLevel();
    controlWaterHeater();
}