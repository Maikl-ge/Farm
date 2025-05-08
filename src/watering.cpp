#include <Arduino.h>
#include <watering.h>
#include <Profile.h>
#include <SensorsModule.h>
#include <globals.h>
#include <pinout.h>
#include <TimeModule.h>
#include <status.h>

// Глобальные переменные
unsigned long lastWateringMillis = 0;
unsigned long pumpOnTimeMillis = 0;
unsigned long drainDelayTimeMillis = 0;
unsigned long drainOnTimeMillis = 0;

bool pumpIsOn = false;
bool drainIsOn = false;
bool pumpIsOffLevel = false;
bool drainPending = false;

const unsigned long PUMP_RUN_TIME_SECONDS = 1 * 60;  // Время работы насоса - 1 минута
const unsigned long DRAIN_OPEN_TIME = 0.5 * 60;      // Время открытия клапана слива - 1.5 минуты

bool max_watering_level = 1;

void stopPump(const char* reason);
void checkDrainDelayTime();

void setupWatering() {
    pinMode(PUMP_WATERING_PIN, OUTPUT);
    pinMode(WATER_OUT_PIN, OUTPUT);
    digitalWrite(PUMP_WATERING_PIN, LOW);
    digitalWrite(WATER_OUT_PIN, LOW);

    pinMode(WATERING_LEVEL_BOX_PIN, INPUT);
    max_watering_level = digitalRead(WATERING_LEVEL_BOX_PIN);
}

void updateWatering() {
    if (statusFarm == "Stop" || statusFarm == "End" || statusFarm == "Abort") {
        digitalWrite(PUMP_WATERING_PIN, LOW);
        digitalWrite(WATER_OUT_PIN, LOW);
        return;
    }

    printCurrentTime();

    bool timeStartWatering = false;
    unsigned long nowMillis = millis();
    uint16_t wateringMinutesElapsed = totalMinutesElapsed - 1;

    // Фазы полива
    if (wateringMinutesElapsed < longPhacse1 ||
        (wateringMinutesElapsed >= longPhacse1 && wateringMinutesElapsed < longPhacse2) ||
        (wateringMinutesElapsed >= longPhacse2 && wateringMinutesElapsed < longPhacse3) ||
        (wateringMinutesElapsed >= longPhacse3 && wateringMinutesElapsed < longPhacse4) ||
        (wateringMinutesElapsed >= longPhacse4 && wateringMinutesElapsed < longPhacse5) ||
        (wateringMinutesElapsed >= longPhacse5 && wateringMinutesElapsed < longPhacse6)) {

        if ((wateringMinutesElapsed % wateringInterval) == 0 && (nowMillis - lastWateringMillis >= 75000)) { // 75 секунд защитная задержка
            timeStartWatering = true;
            lastWateringMillis = nowMillis;
            Serial.print("🟢 Полив запущен  ");
            Serial.println(CurrentTime);
        }
    }

    max_watering_level = digitalRead(WATERING_LEVEL_BOX_PIN);
    if (timeStartWatering && !pumpIsOn &&  max_watering_level == 1) {
        digitalWrite(PUMP_WATERING_PIN, HIGH);
        Serial.print("🔵 Насос ВКЛЮЧЕН  ");
        Serial.println(CurrentTime);

        pumpOnTimeMillis = nowMillis;
        pumpIsOn = true;
        PUMP_WATERING = true;
        pumpIsOffLevel = false;
    }

    // === Выключение по уровню ===
    max_watering_level = digitalRead(WATERING_LEVEL_BOX_PIN);
    if (pumpIsOn && max_watering_level == 0) {
        stopPump("по датчику");
        pumpIsOffLevel = true;
    }

    // === Выключение по таймеру ===
    if (pumpIsOn && !pumpIsOffLevel) {
        unsigned long pumpRunTimeMillis = nowMillis - pumpOnTimeMillis;
        if (pumpRunTimeMillis >= PUMP_RUN_TIME_SECONDS * 1000UL) {
            stopPump("по таймеру");
            pumpIsOffLevel = true;
        }
    }

    // === Проверка отложенного запуска слива ===
    checkDrainDelayTime();

    // === Включение слива ===
    if (!drainIsOn && drainDelayTimeMillis > 0 && nowMillis >= drainDelayTimeMillis) {
        digitalWrite(WATER_OUT_PIN, HIGH);
        Serial.print("🟢 Слив ВКЛЮЧЕН  ");
        Serial.println(CurrentTime);

        drainIsOn = true;
        drainOnTimeMillis = nowMillis;
        WATER_OUT = true;
        drainDelayTimeMillis = 0;
    }

    // === Выключение слива ===
    if (drainIsOn) {
        unsigned long drainRunTimeMillis = nowMillis - drainOnTimeMillis;
        if (drainRunTimeMillis >= DRAIN_OPEN_TIME * 1000UL) {
            digitalWrite(WATER_OUT_PIN, LOW);
            Serial.print("🚫 Слив ВЫКЛЮЧЕН  ");
            Serial.println(CurrentTime);
            drainIsOn = false;
            WATER_OUT = false;
        }
    }
}

// === Остановка насоса ===
void stopPump(const char* reason) {
    digitalWrite(PUMP_WATERING_PIN, LOW);
    Serial.print("🛑 Насос ВЫКЛЮЧЕН (");
    Serial.print(reason);
    Serial.print(")  ");
    Serial.println(CurrentTime);

    pumpIsOn = false;
    PUMP_WATERING = false;

    // Планируем слив через N минут
    drainDelayTimeMillis = millis() + wateringDraining * 60UL * 1000UL;
    drainPending = true;
}

// === Проверка наступления времени слива ===
void checkDrainDelayTime() {
    if (drainPending && millis() >= drainDelayTimeMillis) {
        drainPending = false;
    }
}
