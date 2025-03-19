#include <Arduino.h>
#include <watering.h>
#include <Profile.h>
#include <pinout.h>
#include <TimeModule.h>

// Глобальные переменные
unsigned long lastWateringTimeSeconds = 0;  // Время последнего полива в секундах
unsigned long pumpOnTimeSeconds = 0;        // Время включения насоса в секундах
bool isDayTime = false;                     // День или ночь
bool pumpIsOn = false;                      // Состояние насоса
const unsigned long PUMP_RUN_TIME_SECONDS = 3 * 60; // Время работы насоса (секунды)

// Инициализация пина для управления насосом
void setupWatering() {
    pinMode(PUMP_WATERING_PIN, OUTPUT);   // Пин для насоса полива
    pinMode(WATER_OUT_PIN, OUTPUT);       // Пин для слива
    digitalWrite(PUMP_WATERING_PIN, LOW); // Выключить насос по умолчанию
    digitalWrite(WATER_OUT_PIN, LOW);     // Выключить слив по умолчанию
}

// Функция для обновления состояния полива
void updateWatering() {
    // Вывести текущее время (для отладки)
    printCurrentTime();

    // Преобразование времени в секунды с начала дня
    unsigned long currentTimeSeconds = (CurrentTime / 10000) * 3600 +  // Часы
                                       ((CurrentTime / 100) % 100) * 60 +  // Минуты
                                       (CurrentTime % 100); // Секунды

    // Преобразование времени восхода и заката в секунды
    unsigned long sunriseTimeSeconds = SUNRISE * 60;
    unsigned long sunsetTimeSeconds = SUNSET * 60;

    // Проверка, день или ночь
    if (currentTimeSeconds >= sunriseTimeSeconds && currentTimeSeconds < sunsetTimeSeconds) {
        isDayTime = true;
    } else {
        isDayTime = false;
    }

    // Определение интервала полива (в секундах)
    unsigned long wateringIntervalSeconds = isDayTime ;//? DAY_WATERING_INTERVAL : NIGHT_WATERING_INTERVAL;

    // Проверка, прошло ли достаточно времени с последнего полива
    if (!pumpIsOn && ((currentTimeSeconds + 86400 - lastWateringTimeSeconds) % 86400 >= wateringIntervalSeconds)) {
        // Включение насоса
        digitalWrite(PUMP_WATERING_PIN, HIGH);
        pumpOnTimeSeconds = currentTimeSeconds;
        pumpIsOn = true;
    }

    // Проверка, прошло ли время работы насоса
    if (pumpIsOn && ((currentTimeSeconds + 86400 - pumpOnTimeSeconds) % 86400 >= PUMP_RUN_TIME_SECONDS)) {
        // Выключение насоса
        digitalWrite(PUMP_WATERING_PIN, LOW);
        pumpIsOn = false;
        lastWateringTimeSeconds = currentTimeSeconds;
    }
}
