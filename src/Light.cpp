#include <CurrentProfile.h>
#include <Arduino.h>
#include <pinout.h>
#include <globals.h>
#include <TimeModule.h>


uint16_t transitionTime = 15; // лительность перехода от рассвета ко дню и от дня к закату
uint16_t currentTimeMinutes; // Текущее время в минутах
unsigned long lastUpdateTime = 0; // Последнее обновление яркости
unsigned long brightnessInterval = 0; // Интервал изменения яркости в мс
bool isDayTransition = false; // Переход к дню
bool isNightTransition = false; // Переход к ночи

// Глобальные переменные
int currentBrightness = 0; // Текущая яркость (0-100)

// Константы для значений яркости
const int MAX_BRIGHTNESS = 1023;
const int MIN_BRIGHTNESS = 0;
const int pwmChannel = 0;
const int pwmFrequency = 5000; // Частота PWM
const int pwmResolution = 10;  // Разрешение PWM (10 бит)


void setupLightControl() {
    // Настройка канала PWM
    ledcSetup(pwmChannel, pwmFrequency, pwmResolution);
    // Привязка канала PWM к пину
    ledcAttachPin(LIGHT_PIN, pwmChannel);
    ledcWrite(pwmChannel, MIN_BRIGHTNESS);
    brightnessInterval = ((transitionTime * 60 ) * 1000) / MAX_BRIGHTNESS; // Интервал в миллисекундах
}

// Функция для расчета яркости в зависимости от текущего времени
void updateLightBrightness() {
    // Получить текущее время в минутах
    int currentTimeMinutes = CurrentTime;

    // Проверяем, начался ли восход
    if (currentTimeMinutes == SUNRISE) {
        isDayTransition = true;
        isNightTransition = false;
        currentBrightness = MIN_BRIGHTNESS;
    }
    // Проверяем, начался ли закат
    else if (currentTimeMinutes == SUNSET) {
        isDayTransition = false;
        isNightTransition = true;
        currentBrightness = MAX_BRIGHTNESS;
    }

    // Проверяем, закончился ли переход
    if (isDayTransition && currentBrightness >= MAX_BRIGHTNESS) {
        isDayTransition = false;
    }
    if (isNightTransition && currentBrightness <= MIN_BRIGHTNESS) {
        isNightTransition = false;
    }

    // Увеличение/уменьшение яркости при переходе
    if ((isDayTransition || isNightTransition) && millis() - lastUpdateTime >= brightnessInterval) {
        if (isDayTransition) {
            currentBrightness++;
        } else if (isNightTransition) {
            currentBrightness--;
        }

        // Применить новую яркость
        ledcWrite(pwmChannel, currentBrightness);

        // Обновить время последнего изменения яркости
        lastUpdateTime = millis();
    }
}

