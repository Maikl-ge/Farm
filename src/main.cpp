#include <WiFi.h>
#include "ota_module.h"
#include "globals.h"
#include "DataSender.h"
#include "TimeModule.h"
#include "Profile.h"
#include "CurrentProfile.h"
#include "SensorsModule.h"
#include <Arduino.h>

// Прототипы функций
void updateWebSocket();
void updateSensors();
void sendData();

// Функция для обновления состояния WebSocket
void updateWebSocket() {
    processWebSocket();
}

// Функция для опроса датчиков
void updateSensors() {
    SensorData sensorData = readSensors();
}

// Функция для отправки данных
void sendData() {
    sendDataIfNeeded();
}

void setup() {
    Serial.begin(115200);

    // Подключение к WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi!");
    // Serial.print("IP-адрес ESP32: ");
    // Serial.println(WiFi.localIP());

    // Вызов модуля загрузки настроек
    initializeSettingsModule(); 

    initTimeModule();    // Инициализируем модуль времени
    syncTimeWithNTP();   // Синхронизируем время с NTP

    // Инициализация WebSocket и подключения
    initializeWebSocket();
    // Настройка OTA через модуль
    // setupOTA();

    // Инициализация модуля опроса датчиков
    initializeSensors();
}

void loop() {
    // Обновление OTA через модуль
    // ArduinoOTA.handle();

    // Выводим текущее время
    printCurrentTime();  

    // Обновление состояния WebSocket
    updateWebSocket();

    // Опрос датчиков
    updateSensors();

    // Отправка данных каждые 60 секунд
    sendData();

    delay(1000); // Задержка для предотвращения перегрузки процессора
}