#include <CurrentProfile.h>
#include <Arduino.h>
#include <pinout.h>
#include <globals.h>
#include <TimeModule.h>


uint16_t transitionTime; // Время перехода
uint16_t currentTimeMinutes; // Текущее время в минутах

// Глобальные переменные
int currentBrightness = 0; // Текущая яркость (0-100)

// Константы для значений яркости
const int MAX_BRIGHTNESS = 255;
const int MIN_BRIGHTNESS = 0;

void setupLightControl() {
    pinMode(LIGHT_PIN, OUTPUT);
    analogWrite(LIGHT_PIN, 0); // Установить яркость на 0

    // Длительность перехода от рассвета ко дню и от дня к закату
    transitionTime = 15;
}

// Функция для расчета яркости в зависимости от текущего времени
void updateLightBrightness() {
    int currentTimeMinutes = CurrentTime; // Предполагается, что CurrentTime определен глобально
    int transitionSteps = transitionTime * 60; // Время перехода в секундах

    if (currentTimeMinutes >= SUNRISE && currentTimeMinutes < SUNRISE + transitionTime) {
        // Восход - плавное увеличение яркости
        int elapsedTime = (currentTimeMinutes - SUNRISE) * 60;
        currentBrightness = map(elapsedTime, 0, transitionSteps, MIN_BRIGHTNESS, MAX_BRIGHTNESS);
    } else if (currentTimeMinutes >= SUNSET && currentTimeMinutes < SUNSET + transitionTime) {
        // Закат - плавное уменьшение яркости
        int elapsedTime = (currentTimeMinutes - SUNSET) * 60;
        currentBrightness = map(elapsedTime, 0, transitionSteps, MAX_BRIGHTNESS, MIN_BRIGHTNESS);
    } else if (currentTimeMinutes >= SUNRISE + transitionTime && currentTimeMinutes < SUNSET) {
        // Полный день - яркость 255
        currentBrightness = MAX_BRIGHTNESS;
    } else {
        // Ночь - яркость 0
        currentBrightness = MIN_BRIGHTNESS;
    }

    // Применить новую яркость (переводим в диапазон 0-255)
    int pwmValue = currentBrightness;
    analogWrite(LIGHT_PIN, pwmValue);

}

