#include <Arduino.h>
#include "SensorsModule.h"
#include "Pinout.h" // Подключаем Pinout.h
#include <Wire.h> // Для работы с I2C
#include <DallasTemperature.h> // Для работы с DS18B20
#include <OneWire.h> // Для работы с 1-Wire

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

    // Настройки для DS18B20
    OneWire oneWire(ONE_WIRE_BUS); // Шина OneWire
    DallasTemperature sensors(&oneWire); // Библиотека DallasTemperature

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

// Чтение состояния датчиков холла
HallSensorState readHallSensors() {
    HallSensorState state;
    state.sensor1 = digitalRead(HALL_SENSOR1_PIN);
    state.sensor2 = digitalRead(HALL_SENSOR2_PIN);
    state.sensor3 = digitalRead(HALL_SENSOR3_PIN);
    state.sensor4 = digitalRead(HALL_SENSOR4_PIN);
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
// Обновление состояния датчиков
void updateSensors() {
    Serial.println("Опрос всех сенсоров");
    return;
}
