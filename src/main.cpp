#include <WiFi.h>
#include "ota_module.h"
#include "globals.h"
#include "DataSender.h"
#include "TimeModule.h"
#include "Profile.h"
#include "CurrentProfile.h"
#include "SensorsModule.h"
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Прототипы функций
void webSocketTask(void *pvParameters);
void sensorTask(void *pvParameters);
void sendDataTask(void *pvParameters);

// Функция для обновления состояния WebSocket
void webSocketTask(void *pvParameters) {
    for (;;) {
        processWebSocket();
        vTaskDelay(5000 / portTICK_PERIOD_MS); // Выполняем каждые 500 мс (5 секунд)
    }
}

// Функция для опроса датчиков
void sensorTask(void *pvParameters) {
    for (;;) {
        SensorData sensorData = readSensors();
        vTaskDelay(200 / portTICK_PERIOD_MS); // Выполняем каждые 2000 мс (0.2 секунды)
    }
}

// Функция для отправки данных
void sendDataTask(void *pvParameters) {
    for (;;) {
        sendDataIfNeeded();
        vTaskDelay(120000 / portTICK_PERIOD_MS); // Выполняем каждые 60000 мс (60 секунд)
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

        // Создание задач
    xTaskCreate(webSocketTask, "WebSocket Task", 1024, NULL, 1, NULL);
    xTaskCreate(sensorTask, "Sensor Task", 1024, NULL, 1, NULL);
    xTaskCreate(sendDataTask, "Send Data Task", 1024, NULL, 1, NULL);

    // Запуск планировщика FreeRTOS
    vTaskStartScheduler();
    
}

void loop() {
    // Обновление OTA через модуль
    // ArduinoOTA.handle();

    // Выводим текущее время
    printCurrentTime();  

    // Обновление состояния WebSocket
    processWebSocket();

        // Опрос датчиков
    SensorData sensorData = readSensors();

    // Отправка данных каждые 60 секунд
    sendDataIfNeeded();
}