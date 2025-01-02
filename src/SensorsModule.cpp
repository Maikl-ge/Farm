#include <Arduino.h>
#include "SensorsModule.h"
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "Pinout.h" // Подключаем Pinout.h

// Инициализация всех сенсоров
void initializeSensors() {
    // Инициализация кнопок
    pinMode(BUTTON1_PIN, INPUT_PULLUP);
    pinMode(BUTTON2_PIN, INPUT_PULLUP);
    pinMode(BUTTON3_PIN, INPUT_PULLUP);

    // Инициализация датчиков Холла
    pinMode(HALL_SENSOR1_PIN, INPUT);
    pinMode(HALL_SENSOR2_PIN, INPUT);
    pinMode(HALL_SENSOR3_PIN, INPUT);
    pinMode(HALL_SENSOR4_PIN, INPUT);

    // Инициализация датчиков HDC1080
    Wire.begin(SDA_PIN, SCL_PIN);
    // Здесь можно добавить дополнительную настройку для каждого датчика HDC1080

    // Инициализация датчиков DS18B20
    OneWire oneWire(ONE_WIRE_BUS);
    DallasTemperature ds18b20(&oneWire);

    // Инициализация датчика pH
    pinMode(PH_SENSOR_PIN, INPUT);
}

// Опрос кнопок
ButtonState readButtons() {
    ButtonState state;
    state.button1Pressed = digitalRead(BUTTON1_PIN) == LOW;
    state.button2Pressed = digitalRead(BUTTON2_PIN) == LOW;
    state.button3Pressed = digitalRead(BUTTON3_PIN) == LOW;
    return state;
}

// Обработка состояния кнопок
void handleButtonState() {
    ButtonState buttonState = readButtons();

    if (buttonState.button1Pressed) {
        Serial.println("Button 1 pressed");
    }

    if (buttonState.button2Pressed) {
        Serial.println("Button 2 pressed");
    }

    if (buttonState.button3Pressed) {
        Serial.println("Button 3 pressed");
    }
}
// Обновление состояния кнопок
void updateButtonState(unsigned long currentMillis, unsigned long buttonInterval, unsigned long &lastButtonUpdate) {
    if (currentMillis - lastButtonUpdate >= buttonInterval) {
        lastButtonUpdate = currentMillis;
        handleButtonState();
    }
}
void updateSensors() {
    Serial.println("Опрос всех сенсоров");
    return;
}
