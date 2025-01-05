#include <WiFi.h>
#include "ota_module.h"
#include "globals.h"
#include "DataSender.h"
#include "TimeModule.h"
#include "Profile.h"
#include "CurrentProfile.h"
#include "SensorsModule.h"
#include <Arduino.h>

// Задачи для FreeRTOS

void updateWebSocketTask(void *parameter) {
    for (;;) {
        processWebSocket();
        vTaskDelay(2000 / portTICK_PERIOD_MS);  // Задержка 2000 мс
    }
}

void updateSensorsTask(void *parameter) {
    for (;;) {
        updateSensors();
        vTaskDelay(5000 / portTICK_PERIOD_MS);  // Задержка 5000 мс
    }
}

void sendDataTask(void *parameter) {
    for (;;) {
        sendDataIfNeeded();
        vTaskDelay(60000 / portTICK_PERIOD_MS);  // Задержка 60000 мс
    }
}

void updateButtonTask(void *parameter) {
    for (;;) {
        updateButtonState();
        vTaskDelay(300 / portTICK_PERIOD_MS);  // Задержка 300 мс
    }
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

    initializeSettingsModule(); // Вызов модуля загрузки настроек
    initTimeModule();    // Инициализируем модуль времени
    syncTimeWithNTP();   // Синхронизируем время с NTP
    initializeWebSocket(); // Инициализация WebSocket и подключения

    // setupOTA();  // Настройка OTA через модуль

    initializeSensors();  // Инициализация кнопок

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
        updateButtonTask,
        "Update Button",
        10000,
        NULL,
        1,
        NULL,
        tskNO_AFFINITY
    );
}

void loop() {
    // Пусто, так как все задачи выполняются в контексте FreeRTOS
}