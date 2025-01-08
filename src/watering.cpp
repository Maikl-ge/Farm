#include <Arduino.h>
#include <watering.h>
#include <CurrentProfile.h>
#include <pinout.h>
#include <TimeModule.h>

// Глобальные переменные
unsigned long lastWateringTime = 0;
unsigned long pumpOnTime = 0;
bool isDayTime = false;
bool pumpIsOn = false;
const unsigned long PUMP_RUN_TIME = 1000; // Время работы насоса в миллисекундах

// Инициализация пина для управления насосом
void setupWatering() {
    pinMode(PUMP_WATERING_PIN, OUTPUT);
    digitalWrite(PUMP_WATERING_PIN, LOW); // Выключить насос по умолчанию
}

// Функция для обновления состояния полива
void updateWatering() {
    unsigned long currentTime = millis();
    int currentTimeMinutes = CurrentTime; // Предполагается, что CurrentTime определен глобально

    // Определение времени дня/ночи
    if (currentTimeMinutes >= SUNRISE && currentTimeMinutes < SUNSET) {
        isDayTime = true;
    } else {
        isDayTime = false;
    }

    // Определение интервала полива
    unsigned long wateringInterval = isDayTime ? DAY_WATERING_INTERVAL : NIGHT_WATERING_INTERVAL;

    // Проверка, прошло ли достаточно времени с последнего полива
    if (!pumpIsOn && (currentTime - lastWateringTime >= wateringInterval)) {
        // Включение насоса
        digitalWrite(PUMP_WATERING_PIN, HIGH);
        pumpOnTime = currentTime;
        pumpIsOn = true;
    }

    // Проверка, прошло ли время работы насоса
    if (pumpIsOn && (currentTime - pumpOnTime >= PUMP_RUN_TIME)) {
        // Выключение насоса
        digitalWrite(PUMP_WATERING_PIN, LOW);
        pumpIsOn = false;
        lastWateringTime = currentTime;
    }
}


