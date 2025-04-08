#include <Arduino.h>
#include <watering.h>
#include <Profile.h>
#include <SensorsModule.h>
#include <globals.h>
#include <pinout.h>
#include <TimeModule.h>
#include <status.h>

// Глобальные переменные
unsigned long lastWateringStartTimeSeconds = 0; // Время начала последнего полива 
unsigned long pumpOnTimeSeconds = 0;        // Время включения насоса в секундах
unsigned long drainDelayTimeSeconds = 0;    // Время задержки перед включением клапана слива
unsigned long drainOnTimeSeconds = 0;       // Время включения клапана слива
bool pumpIsOn = false;                      // Состояние насоса
bool drainIsOn = false;                     // Состояние клапана слива
bool pumpIsOffLevel = false;

const unsigned long PUMP_RUN_TIME_SECONDS = 3 * 60; // Время работы насоса (3 минуты)
const unsigned long DRAIN_OPEN_TIME = 2 * 60;   // Время работы клапана слива (120 секунд)
const unsigned long SECONDS_IN_DAY = 86400; // Секунд в сутках
bool max_watering_level = 1;   // Датчик уровня полива воды в боксе
unsigned long currentTimeSeconds;

void stopPump(unsigned long now, const char* reason);

void setupWatering() {
    pinMode(PUMP_WATERING_PIN, OUTPUT);
    pinMode(WATER_OUT_PIN, OUTPUT);
    digitalWrite(PUMP_WATERING_PIN, LOW);
    digitalWrite(WATER_OUT_PIN, LOW);

    pinMode(WATERING_BOX_PIN, INPUT);
    max_watering_level = digitalRead(WATERING_BOX_PIN);
}

void updateWatering() {
    if(statusFarm == "Stop" || statusFarm == "End") {
        return;
    }

    printCurrentTime();

    // Текущее время в секундах от начала суток
    currentTimeSeconds = (CurrentTime / 10000) * 3600 +  
                         ((CurrentTime / 100) % 100) * 60 +  
                         (CurrentTime % 100);

    // Время с момента последнего старта насоса
    unsigned long timeSinceLastStart = (currentTimeSeconds >= lastWateringStartTimeSeconds)
        ? currentTimeSeconds - lastWateringStartTimeSeconds
        : SECONDS_IN_DAY - lastWateringStartTimeSeconds + currentTimeSeconds;

    // Чтение датчика уровня
    max_watering_level = digitalRead(WATERING_BOX_PIN);

    // === ВКЛЮЧЕНИЕ НАСОСА ===
    bool wateringReady = !pumpIsOn && timeSinceLastStart >= (wateringInterval * 60) && pumpIsOffLevel == 0;
    if (wateringReady) {
        digitalWrite(PUMP_WATERING_PIN, HIGH);
        Serial.print("💧 Насос ВКЛЮЧЕН  ");  Serial.println(CurrentTime);
        ph_osmo = true;

        pumpOnTimeSeconds = currentTimeSeconds;
        lastWateringStartTimeSeconds = currentTimeSeconds;  // фиксируем начало полива
        pumpIsOn = true;
        PUMP_WATERING = true;
    }

    // === ВЫКЛЮЧЕНИЕ НАСОСА по уровню ===
    if (pumpIsOn && max_watering_level == 0) {
        stopPump(currentTimeSeconds, "по датчику");
        pumpIsOffLevel = 1;
    }

    // === ВЫКЛЮЧЕНИЕ НАСОСА по таймеру ===
    if (pumpIsOn && pumpIsOffLevel == 0) {
        unsigned long pumpRunTime = (currentTimeSeconds >= pumpOnTimeSeconds)
            ? currentTimeSeconds - pumpOnTimeSeconds
            : SECONDS_IN_DAY - pumpOnTimeSeconds + currentTimeSeconds;

        if (pumpRunTime >= PUMP_RUN_TIME_SECONDS) {
            stopPump(currentTimeSeconds, "по таймеру");
            pumpIsOffLevel = 0;
        }
    }

// === ВКЛЮЧЕНИЕ СЛИВА ===
if (!drainIsOn && drainDelayTimeSeconds > 0) {
    bool drainTimeReached = (currentTimeSeconds >= drainDelayTimeSeconds) ||
                            (drainDelayTimeSeconds > SECONDS_IN_DAY - 600 && currentTimeSeconds < 600); // учёт перехода через полночь

    if (drainTimeReached) {
        digitalWrite(WATER_OUT_PIN, HIGH);
        tds_osmo = true;
        Serial.print("🚰 Слив ВКЛЮЧЕН  ");  Serial.println(CurrentTime);
        drainIsOn = true;
        drainOnTimeSeconds = currentTimeSeconds;
        WATER_OUT = true;
        drainDelayTimeSeconds = 0;
        pumpIsOffLevel = 0;
    }
}

    // === ВЫКЛЮЧЕНИЕ СЛИВА ===
    if (drainIsOn) {
        unsigned long drainRunTime = (currentTimeSeconds >= drainOnTimeSeconds)
            ? currentTimeSeconds - drainOnTimeSeconds
            : SECONDS_IN_DAY - drainOnTimeSeconds + currentTimeSeconds;

        if (drainRunTime >= DRAIN_OPEN_TIME) {
            tds_osmo = false;
            digitalWrite(WATER_OUT_PIN, LOW);
            Serial.print("🚫 Слив ВЫКЛЮЧЕН  ");  Serial.println(CurrentTime);
            drainIsOn = false;
            WATER_OUT = false;
        }
    }
}

// === ФУНКЦИЯ ОСТАНОВКИ НАСОСА ===
void stopPump(unsigned long now, const char* reason) {
    digitalWrite(PUMP_WATERING_PIN, LOW);
    ph_osmo = false;
    Serial.print("🛑 Насос ВЫКЛЮЧЕН (");
    Serial.print(reason);
    Serial.println(")  ");   Serial.println(CurrentTime);
    pumpIsOn = false;
    PUMP_WATERING = false; 
    drainDelayTimeSeconds = now + (wateringDraining * 60);
}