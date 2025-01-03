#include <Arduino.h>
#include "SensorsModule.h"
#include "Pinout.h" // Подключаем Pinout.h
#include <Wire.h> // Для работы с I2C
#include <DallasTemperature.h> // Для работы с DS18B20
#include <OneWire.h> // Для работы с 1-Wire

// Определение переменных состояния кнопок
bool start_Button = false;
bool stop_Button = false;
bool mode_Button = false;

// Определение переменных состояния датчиков уровня воды
bool max_osmo_level = false;
bool min_osmo_level = false;
bool max_water_level = false;
bool min_water_level = false;

// Инициализация всех сенсоров
void initializeSensors() {
    // Инициализация кнопок
    pinMode(BUTTON1_PIN, INPUT_PULLUP);
    pinMode(BUTTON2_PIN, INPUT_PULLUP);
    pinMode(BUTTON3_PIN, INPUT_PULLUP);

    // Инициализация датчиков Холла
    pinMode(HALL_SENSOR1_PIN, INPUT_PULLUP);
    pinMode(HALL_SENSOR2_PIN, INPUT_PULLUP);
    pinMode(HALL_SENSOR3_PIN, INPUT_PULLUP);
    pinMode(HALL_SENSOR4_PIN, INPUT_PULLUP);

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
void readButtons() {
    start_Button = digitalRead(BUTTON1_PIN) == LOW;
    stop_Button = digitalRead(BUTTON2_PIN) == LOW;
    mode_Button = digitalRead(BUTTON3_PIN) == LOW;
}

// Чтение состояния датчиков уровня воды
void readHallSensors() {
    max_osmo_level = !digitalRead(HALL_SENSOR1_PIN); // A3144: LOW = магнит обнаружен
    min_osmo_level = !digitalRead(HALL_SENSOR2_PIN); // A3144: LOW = магнит обнаружен
    max_water_level = !digitalRead(HALL_SENSOR3_PIN); // A3144: LOW = магнит обнаружен
    min_water_level = !digitalRead(HALL_SENSOR4_PIN); // A3144: LOW = магнит обнаружен
    //return state;
}

// Обработка состояния кнопок
void handleButtonState() {
    readButtons();

    if (start_Button) {
        Serial.println("Start button pressed");
    }
    if (stop_Button) {
        Serial.println("Stop button pressed");
    }
    if (mode_Button) {
        Serial.println("Mode button pressed");
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
    readHallSensors();
    Serial.println("Опрос всех сенсоров");
    return;
}
