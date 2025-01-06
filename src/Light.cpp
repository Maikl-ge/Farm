
#include <CurrentProfile.h>
#include <Arduino.h>
#include <pinout.h>
#include <globals.h>
#include <TimeModule.h>
#include <EEPROM.h>

uint16_t sunriseDay; // Время  рассвета 
uint16_t sunsetDay; // Время заката
uint16_t currentDay; // Текущее время суток
uint16_t transitionTime; // Время перехода
uint16_t currentTimeMinutes; // Текущее время в минутах

// Прототип функции
uint16_t readUint16FromEEPROM(int address);

// Глобальные переменные
int currentBrightness = 0; // Текущая яркость (0-100)

void setupLightControl() {
    pinMode(LIGHT_PIN, OUTPUT);
    analogWrite(LIGHT_PIN, 0); // Установить яркость на 0

    // Адрес для чтения байтов состояния Фермы
    int address = 0x1C;
    // Чтение двух байт и объединение в uint16_t
    sunriseDay = readUint16FromEEPROM(address);

    address = 0x1E;
    // Чтение двух байт и объединение в uint16_t
    sunsetDay = readUint16FromEEPROM(address);

    transitionTime = 15;
}

// Определение функции
uint16_t readUint16FromEEPROM(int address) {
    uint16_t value = 0;
    value = EEPROM.read(address); // Читаем младший байт
    value |= EEPROM.read(address + 1) << 8; // Читаем старший байт и сдвигаем в старшую позицию
    return value;
}

// Функция для расчета яркости в зависимости от текущего времени
void updateLightBrightness() {
    currentTimeMinutes = CurrentTime;
    int transitionSteps = transitionTime * 60; // Время перехода в секундах

    if (currentTimeMinutes >= sunriseDay && currentTimeMinutes < sunriseDay + transitionTime) {
        // Восход - плавное увеличение яркости
        int elapsedTime = (currentTimeMinutes - sunriseDay) * 60;
        currentBrightness = map(elapsedTime, 0, transitionSteps, 0, 100);
    } else if (currentTimeMinutes >= sunsetDay && currentTimeMinutes < sunsetDay + transitionTime) {
        // Закат - плавное уменьшение яркости
        int elapsedTime = (currentTimeMinutes - sunsetDay) * 60;
        currentBrightness = map(elapsedTime, 0, transitionSteps, 100, 0);
    } else if (currentTimeMinutes >= sunriseDay + transitionTime && currentTimeMinutes < sunsetDay) {
        // Полный день - яркость 100
        currentBrightness = 100;
    } else {
        // Ночь - яркость 0
        currentBrightness = 0;
    }

    // Применить новую яркость (переводим в диапазон 0-255)
    int pwmValue = map(currentBrightness, 0, 100, 0, 255);
    analogWrite(LIGHT_PIN, pwmValue);
    Serial.print("Current time minutes: ");
    Serial.println(currentTimeMinutes);
    Serial.print("Current brightness: ");
    Serial.println(currentBrightness);
    Serial.print("Sunrise: ");
    Serial.println(sunriseDay);
    Serial.print("Sunset: ");
    Serial.println(sunsetDay);
    Serial.print("Transition time: ");
    Serial.println(transitionTime);
}

