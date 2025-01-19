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
#include <SPI.h>
#include <SD.h>
#include "menu.h"

#define WEBSOCKETS_MAX_DATA_SIZE 2048 // Максимальный размер данных

// Размер EEPROM
#define EEPROM_SIZE 512

// Прототипы функций
void sendDataTask(void *parameter);
void updatePCF8574Task(void *parameter);
void updateWater(); // Прототип функции
void requestSettings();
void updateButtonState();  

// Задачи для FreeRTOS

void updateWebSocketTask(void *parameter) {
    static unsigned long lastPing = 0; // Время последнего отправленного PING
    // Если пропущено 11 Pong подряд, убиваем текущий сокет и подключаем новый

    for (;;) {
        unsigned long currentMillis = millis();
        // Если соединение потеряно, пробуем переподключиться

        if (!connected) {
            Serial.println("Reconnected...");          
            connectWebSocket();
        } else {
            // Если соединение активно, отправляем PING каждые 5 секунд
            if (currentMillis - lastPing >= 5000) {  // Проверка интервала
                missedPongs++;  // Увеличиваем счетчик пропущенных Pong
                if (missedPongs >= 4) {
                Serial.println("3 missed Pongs, reconnecting WebSocket...");
                webSocket.close();  // Закрываем текущий WebSocket
                missedPongs = 0;   // Сброс счетчика
                connectWebSocket(); // Пробуем подключиться заново
                }
                lastPing = currentMillis;  // Обновляем время последнего PING
            }
        }
        // Задержка перед следующим циклом
        vTaskDelay(3000 / portTICK_PERIOD_MS);  // 3 секунда
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
        // Отправка параметров
        sendDataIfNeeded();
        // Отправка статуса
        //serializeStatus();
        vTaskDelay(60000 / portTICK_PERIOD_MS);  // Задержка 60000 мс             
    }
}

void updateMenuTask(void *parameter) {
    for (;;) {
        updateButtonState();
        webSocket.poll(); // Обработка WebSocket событий
        vTaskDelay(50 / portTICK_PERIOD_MS);  // Задержка 50 мс
    }
}

void updateWaterTask(void *parameter) {
    for (;;) {
        updateWater();
        vTaskDelay(1000 / portTICK_PERIOD_MS);  // Задержка 1000 мс
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

    initializeSettingsModule(); // Инициализация модуля настроек

    initializeMenu(); // Инициализация модуля меню

    setupWatering(); // Инициализация модуля полива

    setupLightControl(); // Инициализация модуля управления светом

    setupWater(); // Инициализация модуля управления водой

    // Подключение к WiFi
    WiFi.begin(ssid, password);
    
    connectToWiFi();

    initializeWebSocket();  // Инициализация WebSocket

    initTimeModule();    // Инициализируем модуль времени

    syncTimeWithNTP("pool.ntp.org", timeZone); // Синхронизируем время с NTP

    initializeSensors();  // Инициализация модуля сенсоров
    
    setupOTA();  // Настройка OTA через модуль

    setupCDcard(); // Инициализация SD карты
    
    if (connected) {
        Serial.println("WebSocket connected started");    
    } else {
        Serial.println("WebSocket connection not started");
    }

    // Создание задач
    xTaskCreatePinnedToCore(
        updateWebSocketTask,   // Функция задачи
        "Update WebSocket",    // Название задачи
        10000,                 // Размер стека задачи
        NULL,                  // Параметры задачи
        2,                     // Приоритет задачи
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
        updateMenuTask,
        "Update Menu",
        10000,
        NULL,
        1,
        NULL,
        tskNO_AFFINITY
    );

    xTaskCreatePinnedToCore(
        updateWaterTask,
        "Update Water",
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
    sendWebSocketMessage("FRQS");
    Serial.println("Запрос 'Settings' отправлен серверу.");
}
// Функция для подключения к WiFi
void connectToWiFi() {
    const int maxAttempts = 3;            // Количество попыток подключения
    const unsigned long attemptTimeout = 5000; // Время ожидания каждой попытки (в миллисекундах)

    for (int attempt = 1; attempt <= maxAttempts; ++attempt) {
        Serial.printf("Attempt %d of %d to connect to WiFi...\n", attempt, maxAttempts);
        unsigned long startAttemptTime = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < attemptTimeout) {
            delay(100); // Небольшая задержка для освобождения процессора
            Serial.print(".");
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nConnected to WiFi!");
            Serial.print("IP Address: ");
            Serial.println(WiFi.localIP());
            return; // Выходим из функции, если подключение успешно
        }

        Serial.println("\nFailed to connect. Retrying...");
    }

    // Если не удалось подключиться за три попытки
    Serial.println("Failed to connect to WiFi after 3 attempts.");
    // Здесь можно добавить дополнительные действия, например, включение режима AP
}

