#include <Arduino.h>
#include <pinout.h>
#include <globals.h>
#include <TimeModule.h>
#include <Profile.h>


uint16_t transitionTime = 15 * 60; // Длительность (в секундах) перехода от рассвета ко дню и от дня к закату
uint16_t currentTimeMinutes; // Текущее время в минутах
unsigned long lastUpdateTime = 0; // Последнее обновление яркости
unsigned long brightnessInterval = 0; // Интервал изменения яркости в мс
const float GAMMA = 2.2; // Коэффициент гамма-коррекции

// Глобальные переменные
int currentBrightness = 0; // Текущая яркость (0-100)

// Константы для значений яркости
const int MAX_BRIGHTNESS = 1023;
const int MIN_BRIGHTNESS = 0;
const int pwmChannel = 0;  // Канал PWM для управления светом
const int pwmFrequency = 5000; // Частота PWM
const int pwmResolution = 10;  // Разрешение PWM (10 бит)


void setupLightControl() {
    //Настройка канала PWM
    ledcSetup(pwmChannel, pwmFrequency, pwmResolution);
    // Привязка канала PWM к пину
    ledcAttachPin(LIGHT_PIN, pwmChannel);
    ledcWrite(pwmChannel, MIN_BRIGHTNESS);
    brightnessInterval = (transitionTime * 1000) / MAX_BRIGHTNESS; // Интервал в миллисекундах
}

// Преобразование значения яркости с гамма-коррекцией
int applyGammaCorrection(int rawValue, int maxValue) {
    float normalized = float(rawValue) / maxValue; // Нормализуем значение (0.0 - 1.0)
    float corrected = pow(normalized, GAMMA);     // Применяем гамма-коррекцию
    return corrected * maxValue;                 // Масштабируем обратно
}

// Функция для расчета яркости в зависимости от текущего времени
void updateLightBrightness() {
    printCurrentTime();

    //SUNRISE = (10 * 60) +37;  // Время восхода для теста

    long currentTimeSeconds = (CurrentTime / 10000) * 3600 + ((CurrentTime / 100) % 100) * 60 + (CurrentTime % 100);

    int targetBrightness = 0;

    // Рассвет
    long sunriseStart = (SUNRISE * 60) - (transitionTime / 2);
    long sunriseEnd = SUNRISE * 60;
    // Закат
    long sunsetStart = (SUNSET * 60) - (transitionTime / 2);
    long sunsetEnd = SUNSET * 60;

    // Рассчитать яркость в зависимости от времени
    if (currentTimeSeconds >= sunriseStart && currentTimeSeconds <= sunriseEnd) {
        // Рассвет: от MIN_BRIGHTNESS до MAX_BRIGHTNESS
        float progress = float(currentTimeSeconds - sunriseStart) / (sunriseEnd - sunriseStart);
        targetBrightness = MIN_BRIGHTNESS + progress * (MAX_BRIGHTNESS - MIN_BRIGHTNESS);
    } else if (currentTimeSeconds >= sunsetStart && currentTimeSeconds <= sunsetEnd) {
        // Закат: от MAX_BRIGHTNESS до MIN_BRIGHTNESS
        float progress = float(currentTimeSeconds - sunsetStart) / (sunsetEnd - sunsetStart);
        targetBrightness = MAX_BRIGHTNESS - progress * (MAX_BRIGHTNESS - MIN_BRIGHTNESS);
    } else if (currentTimeSeconds > sunriseEnd && currentTimeSeconds < sunsetStart) {
        // День
        targetBrightness = MAX_BRIGHTNESS;
    } else {
        // Ночь
        targetBrightness = MIN_BRIGHTNESS;
    }

    // Применяем гамма-коррекцию к рассчитанной яркости
    int correctedBrightness = applyGammaCorrection(targetBrightness, MAX_BRIGHTNESS);

    // Обновляем яркость только при изменении
    if (currentBrightness != correctedBrightness) {
        currentBrightness = correctedBrightness;
        ledcWrite(pwmChannel, currentBrightness);
        Serial.print("Яркость (с гамма-коррекцией): ");
        Serial.println(currentBrightness);
    }
}
