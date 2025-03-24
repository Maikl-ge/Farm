#include <Arduino.h>
#include <watering.h>
#include <Profile.h>
#include <globals.h>
#include <pinout.h>
#include <TimeModule.h>
#include <status.h>

// Глобальные переменные
unsigned long lastWateringTimeSeconds = 0;  // Время последнего полива в секундах
unsigned long pumpOnTimeSeconds = 0;        // Время включения насоса в секундах
unsigned long drainDelayTimeSeconds = 0;    // Время задержки перед включением клапана слива
unsigned long drainOnTimeSeconds = 0;       // Время включения клапана слива
bool pumpIsOn = false;                      // Состояние насоса
bool drainIsOn = false;                     // Состояние клапана слива
const unsigned long PUMP_RUN_TIME_SECONDS = 2 * 60; // Время работы насоса (2 минуты)
const unsigned long DRAIN_OPEN_TIME = 57;   // Время работы клапана слива (57 секунд)
const unsigned long SECONDS_IN_DAY = 86400; // Секунд в сутках

void setupWatering() {
    pinMode(PUMP_WATERING_PIN, OUTPUT);
    pinMode(WATER_OUT_PIN, OUTPUT);
    digitalWrite(PUMP_WATERING_PIN, LOW);
    digitalWrite(WATER_OUT_PIN, LOW);
}

void updateWatering() {
    printCurrentTime();

    // Преобразование времени в секунды с начала дня
    unsigned long currentTimeSeconds = (CurrentTime / 10000) * 3600 +  
                                       ((CurrentTime / 100) % 100) * 60 +  
                                       (CurrentTime % 100);

    // Интервал полива в секундах
    unsigned long wateringIntervalSeconds = (wateringInterval * 60);

    // Коррекция для перехода через полночь
    unsigned long timeSinceLastWatering;
    if (currentTimeSeconds >= lastWateringTimeSeconds) {
        timeSinceLastWatering = currentTimeSeconds - lastWateringTimeSeconds;
    } else {
        timeSinceLastWatering = (SECONDS_IN_DAY - lastWateringTimeSeconds) + currentTimeSeconds;
    }

    // Отладочные сообщения
    // Serial.print("Current Time (s): "); Serial.println(currentTimeSeconds);
    // Serial.print("Last Watering (s): "); Serial.println(lastWateringTimeSeconds);
    // Serial.print("Time Since Last (s): "); Serial.println(timeSinceLastWatering);
    // Serial.print("Watering Interval (s): "); Serial.println(wateringIntervalSeconds);

    // Проверка на включение насоса
    if (!pumpIsOn && timeSinceLastWatering >= wateringIntervalSeconds) {
        digitalWrite(PUMP_WATERING_PIN, HIGH);
        Serial.println(" -------- Pump is ON");
        PUMP_WATERING = true;
        pumpOnTimeSeconds = currentTimeSeconds;
        pumpIsOn = true;
    }

    // Проверка на выключение насоса
    if (pumpIsOn) {
        unsigned long pumpRunTime = (currentTimeSeconds >= pumpOnTimeSeconds) 
            ? (currentTimeSeconds - pumpOnTimeSeconds) 
            : (SECONDS_IN_DAY - pumpOnTimeSeconds + currentTimeSeconds);
        
        if (pumpRunTime >= PUMP_RUN_TIME_SECONDS) {
            digitalWrite(PUMP_WATERING_PIN, LOW);
            Serial.println(" -------- Pump is OFF");
            PUMP_WATERING = false;
            pumpIsOn = false;
            lastWateringTimeSeconds = currentTimeSeconds;
            drainDelayTimeSeconds = currentTimeSeconds + (wateringDraining * 60);
            Serial.print("Drain Delay Set To (s): "); Serial.println(drainDelayTimeSeconds);
        }
    }

    // Проверка на включение слива
    if (!drainIsOn && drainDelayTimeSeconds > 0) {
        unsigned long timeSincePumpOff = (currentTimeSeconds >= lastWateringTimeSeconds) 
            ? (currentTimeSeconds - lastWateringTimeSeconds) 
            : (SECONDS_IN_DAY - lastWateringTimeSeconds + currentTimeSeconds);
        
        if (timeSincePumpOff >= (wateringDraining * 60)) {
            digitalWrite(WATER_OUT_PIN, HIGH);
            Serial.println(" --------> Drain is ON -------");
            WATER_OUT = true;
            drainIsOn = true;
            drainOnTimeSeconds = currentTimeSeconds;
        }
    }

    // Проверка на выключение слива
    if (drainIsOn) {
        unsigned long drainRunTime = (currentTimeSeconds >= drainOnTimeSeconds) 
            ? (currentTimeSeconds - drainOnTimeSeconds) 
            : (SECONDS_IN_DAY - drainOnTimeSeconds + currentTimeSeconds);
        
        if (drainRunTime >= DRAIN_OPEN_TIME) {
            digitalWrite(WATER_OUT_PIN, LOW);
            Serial.println(" --------> Drain is OFF -------");
            WATER_OUT = false;
            drainIsOn = false;
            drainDelayTimeSeconds = 0; // Сброс задержки
        }
    }
}