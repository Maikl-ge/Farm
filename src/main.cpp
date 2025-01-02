#include <WiFi.h>
#include "ota_module.h"
#include "globals.h"
#include "DataSender.h"
#include "TimeModule.h"
#include "Profile.h"
#include "CurrentProfile.h"
#include "SensorsModule.h"
#include <Arduino.h>

ButtonState readButtons();

// Переменные для хранения времени последнего выполнения функций
unsigned long lastWebSocketUpdate = 0;
unsigned long lastSensorUpdate = 0;
unsigned long lastDataSend = 0;
unsigned long lastButtonUpdate = 0;

// Интервалы обновления в миллисекундах
const unsigned long webSocketInterval = 10000;
const unsigned long sensorInterval = 1000;
const unsigned long dataSendInterval = 60000;
const unsigned long buttonInterval = 100; // Интервал опроса кнопок

void setup() {
    Serial.begin(115200);

    // Подключение к WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi!");

    // Вызов модуля загрузки настроек
    initializeSettingsModule(); 

    initTimeModule();    // Инициализируем модуль времени
    syncTimeWithNTP();   // Синхронизируем время с NTP

    // Инициализация WebSocket и подключения
    initializeWebSocket();
    // Настройка OTA через модуль
    // setupOTA();

    // Инициализация кнопок
    initializeSensors();
}

void loop() {
    unsigned long currentMillis = millis();

    // Обновление состояния WebSocket
    if (currentMillis - lastWebSocketUpdate >= webSocketInterval) {
        lastWebSocketUpdate = currentMillis;
        processWebSocket(); 
    }

    // Опрос датчиков
    if (currentMillis - lastSensorUpdate >= sensorInterval) {
        lastSensorUpdate = currentMillis;
        updateSensors(); // Удалите или закомментируйте, если не нужно
    }

    // Отправка данных каждые 60 секунд
    if (currentMillis - lastDataSend >= dataSendInterval) {
        lastDataSend = currentMillis;
        sendDataIfNeeded(); 
    }

    // Опрос кнопок
    updateButtonState(currentMillis, buttonInterval, lastButtonUpdate);

    // Выводим текущее время
    printCurrentTime();
}