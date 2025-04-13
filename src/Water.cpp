#include <Arduino.h>
#include <pinout.h>
#include <status.h>
#include <globals.h>
#include <SensorsModule.h>
#include <math.h>
// Константы
const float WATER_HYSTERESIS = 0.25;       // Гистерезис температуры воды (±0.5°C)
const float WATER_TEMP_RANGE = 1.0;       // Диапазон пропорционального управления (±2°C)
const int PWM_CHANNEL = 5;                // Канал PWM для нагревателя
const int PWM_FREQ = 21000;                // Частота PWM (21 кГц)
const int PWM_RESOLUTION = 10;            // Разрешение PWM (10 бит, 0-1023)
const int PWM_MIN = 0;                    // Минимальное значение PWM
const int PWM_MAX = 1023;                 // Максимальное значение PWM
const int MIN_EFFECTIVE_PWM = 0;          // Минимальное эффективное значение PWM для нагрева
// Задержки нагревателя
const unsigned long HEATER_ON_DELAY = 500;  // Задержка перед включением нагревателя (мс)
const unsigned long HEATER_OFF_DELAY = 5; // Задержка перед выключением нагревателя (мс)

// Глобальные переменные таймеров нагревателя
unsigned long heaterSafeTimerStart = 0;   // Время начала безопасного состояния (мс)
unsigned long heaterUnsafeTimerStart = 0; // Время начала небезопасного состояния (мс)
bool heaterReadyToTurnOn = false;         // Флаг готовности нагревателя к включению

// Тайминги насоса перемешивания
unsigned long intervalPump = 30 * 1000;  // каждые 25 секунд
unsigned long durationPump = 10 * 1000;  // насос включен на 25 секунд
bool isPumpOn = false;               // Состояние включения насоса
unsigned long lastToggleTime = 0;    // Время последнего переключения

void setupWater() {
    // Инициализация пина нагревателя
    pinMode(HITER_WATER_PIN, OUTPUT);
    digitalWrite(HITER_WATER_PIN, LOW);

    pinMode(PUMP_TRANSFER_PIN, OUTPUT);
    digitalWrite(PUMP_TRANSFER_PIN, LOW);
    PUMP_TRANSFER = 0;

    // Настройка PWM для нагревателя
    ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(HITER_WATER_PIN, PWM_CHANNEL);
    ledcWrite(PWM_CHANNEL, PWM_MIN);
    HITER_WATER = PWM_MIN;
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

    if(max_osmo_level == 1 && min_osmo_level == 0) {
        Serial.println("Alert sensors OSMS water");
    }
    // // Управление насосом подачи
    // if (max_water_level == 1) {
    //     digitalWrite(PUMP_TRANSFER_PIN, LOW);  // Выключаем насос, если бак полива полный
    // }
    // else if (min_water_level == 0 && max_water_level == 0) {
    //     digitalWrite(PUMP_TRANSFER_PIN, HIGH); // Включаем насос, если бак полива пуст
    // }
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
    // Управление нагревом с корректировкой
    float tempError = water_temperature_osmo - currentWaterTemperatura; // Инвертируем для корректного расчёта
    int pwmValue;

    //if (tempError > WATER_HYSTERESIS) {
        if (water_temperature_osmo >= currentWaterTemperatura + WATER_HYSTERESIS) {    
            pwmValue = PWM_MIN; // Выключаем нагреватель, если измеренная температура выше верхней границы
        } else if (water_temperature_osmo <= currentWaterTemperatura - WATER_HYSTERESIS) {
            pwmValue = PWM_MAX; // Включаем нагреватель на максимум, если измеренная температура ниже нижней границы
        } else {
            // В зоне гистерезиса сохраняем текущее состояние
            pwmValue = HITER_WATER; // Держим предыдущее значение PWM
        }
    // Устанавливаем состояние на основе PWM
    ledcWrite(PWM_CHANNEL, pwmValue);
    HITER_WATER = pwmValue;

    if (HITER_WATER > 2) { 
        // Нагреватель включён — насос постоянно включён
        digitalWrite(PUMP_TRANSFER_PIN, HIGH);   
        PUMP_TRANSFER = HIGH;    
        isPumpOn = true;
    } else {
        unsigned long currentTime = millis();  // Текущее время
        // Интервалы включения: intervalPump мс (выключен), durationPump мс (включён)
        unsigned long toggleInterval = isPumpOn ? intervalPump : durationPump;

        // Проверяем, нужно ли переключить состояние
        if (currentTime - lastToggleTime >= toggleInterval) {
            lastToggleTime = currentTime; // Обновляем время
            isPumpOn = !isPumpOn; // Инвертируем состояние
            digitalWrite(PUMP_TRANSFER_PIN, isPumpOn ? HIGH : LOW); // Устанавливаем пин
            PUMP_TRANSFER = isPumpOn ? HIGH : LOW; // Обновляем состояние
        }
    }
}

void updateWater() {
    readPCF8574(); // Раскомментировать для обновления данных с датчиков
    controlWaterLevel();
    controlWaterHeater();
}