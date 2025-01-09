#include <Arduino.h>
#include <WiFi.h>
#include <EEPROM.h>
#include "globals.h"
#include "TimeModule.h"
#include "SensorsModule.h"
#include "DataSender.h"
#include "ota_module.h"
#include "Profile.h"
#include "CurrentProfile.h"
#include "watering.h"
#include "WebSocketHandler.h"

// Размер EEPROM
#define EEPROM_SIZE 0x2A

// Прототипы функций
void sendDataTask(void *parameter);
void updatePCF8574Task(void *parameter);
void updateButtonWaterTask(void *parameter);
void updateButtonWater(); // Прототип функции
void requestSettings();

// Задачи для FreeRTOS

void updateWebSocketTask(void *parameter) {
    for (;;) {
        processWebSocket();
        vTaskDelay(10000 / portTICK_PERIOD_MS);  // Задержка 10000 мс
    }
}

void updateSensorsTask(void *parameter) {
    for (;;) {
        updateSensors();
        updateLightBrightness();
        vTaskDelay(5000 / portTICK_PERIOD_MS);  // Задержка 5000 мс
    }
}

void sendDataTask(void *parameter) {
    for (;;) {
        sendDataIfNeeded();
        vTaskDelay(60000 / portTICK_PERIOD_MS);  // Задержка 60000 мс
    }
}

void updatePCF8574Task(void *parameter) {
    for (;;) {
        readPCF8574();
        vTaskDelay(1000 / portTICK_PERIOD_MS);  // Задержка 1000 мс
    }
}

void updateButtonWaterTask(void *parameter) {
    for (;;) {
        updateButtonWater();
        webSocket.poll(); // Обработка WebSocket событий
        vTaskDelay(250 / portTICK_PERIOD_MS);  // Задержка 250 мс
    }
}

void setup() {
    Serial.begin(115200);
    Serial.setDebugOutput(false); // Отключение вывода отладочных сообщений

    // Инициализация EEPROM
    if (!EEPROM.begin(EEPROM_SIZE)) {
        Serial.println("Failed to initialize EEPROM");
        return;
    }
    setupWatering(); // Инициализация модуля полива

    setupLightControl(); // Инициализация модуля управления светом

    setupWater(); // Инициализация модуля управления водой

    // Подключение к WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi!");

    // Инициализация WebSocket
    initializeWebSocket();

    initTimeModule();    // Инициализируем модуль времени
    syncTimeWithNTP("pool.ntp.org"); // Синхронизируем время с NTP

    //requestSettings(); // Отправка запроса "Settings" и получение ответа

    setupOTA();  // Настройка OTA через модуль

    initializeSensors();  // Инициализация модуля сенсоров

    // Создание задач
    xTaskCreatePinnedToCore(
        updateWebSocketTask,   // Функция задачи
        "Update WebSocket",    // Название задачи
        10000,                 // Размер стека задачи
        NULL,                  // Параметры задачи
        1,                     // Приоритет задачи
        NULL,                  // Дескриптор задачи
        0                      // Ядро, на котором будет выполняться задача (0 или 1)
    );

    xTaskCreatePinnedToCore(
        updateSensorsTask,
        "Update Sensors",
        10000,
        NULL,
        1,
        NULL,
        tskNO_AFFINITY
    );

    xTaskCreatePinnedToCore(
        sendDataTask,
        "Send Data",
        10000,
        NULL,
        1,
        NULL,
        0
    );

    xTaskCreatePinnedToCore(
        updatePCF8574Task,
        "Update PCF8574",
        10000,
        NULL,
        1,
        NULL,
        tskNO_AFFINITY
    );

    xTaskCreatePinnedToCore(
        updateButtonWaterTask,
        "Update Button Water",
        10000,
        NULL,
        1,
        NULL,
        tskNO_AFFINITY
    );
}

void loop() {
    ArduinoOTA.handle(); // Обработка OTA обновлений
    // Другие задачи, если есть
}

// Функция для отправки запроса "Settings" и получения ответа
void requestSettings() {
    // Отправка запроса на сервер через WebSocket
    sendWebSocketMessage("Settings");
    Serial.println("Запрос 'Settings' отправлен.");
}
