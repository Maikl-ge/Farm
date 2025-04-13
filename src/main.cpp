#include <Arduino.h>
#include <WiFi.h>
#include <EEPROM.h>
#include "globals.h"
#include "pinout.h"
#include "TimeModule.h"
#include "SensorsModule.h"
#include "DataSender.h"
#include "ota_module.h"
#include "Profile.h"
#include "watering.h"
#include "WebSocketHandler.h"
#include <AccessPoint.h>
#include "menu.h"
#include <FS.h>
#include "SDcard.h"
#include "fanControl.h"
#include "status.h"

//#define WEBSOCKETS_MAX_DATA_SIZE 4096 // Максимальный размер данных

// Размер EEPROM
#define EEPROM_SIZE 2048    

// Прототипы функций
void sendDataTask(void *parameter);
void updatePCF8574Task(void *parameter);
void updateWater(); // Прототип функции
void requestSettings();
void updateButtonState(); 
void initializePins(); // Прототип функции 
void updateLightBrightness(); // Прототип функции
void updateWatering(); // Прототип функции
void updateFanControl(); // Прототип функции
void CurrentStatusFarm();  // Определение текущего статуса фермы
void setupStepper(); // Инициализация шагового двигателя
void updateStepperControl(); // Обновление состояния двигателя
void updateSoakState();
void readAllHTU21D();

// Объявление объекта класса AccessPoint
AccessPoint accessPoint;   

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
                Serial.println(" 4 missed Pongs, reconnecting WebSocket...");
                webSocket.close();  // Закрываем текущий WebSocket
                missedPongs = 0;   // Сброс счетчика
                connectWebSocket(); // Пробуем подключиться заново
                }
                lastPing = currentMillis;  // Обновляем время последнего PING
            }
        }
        // Задержка перед следующим циклом
        vTaskDelay(5000 / portTICK_PERIOD_MS);  // 5 секунда
    }
}

void updateSensorsTask(void *parameter) {
    for (;;) {
        updateSensors();
        if (connected) {
        // Отправка ping каждые 10 секунд
        webSocket.ping();
        } 
        vTaskDelay(10000 / portTICK_PERIOD_MS);  // Задержка 10000 мс
    }
}

void sendDataTask(void *parameter) {
    for (;;) {  
        pintStatusFarm = true;  
        Serial.print("< --- > Тик передачи данных  ");  Serial.println(CurrentTime); 
        CurrentStatusFarm(); // Определение текущего статуса фермы  
        timeSlot = 0;
        unsigned long timeStartSlot = millis(); // Время начала передачи
            if(statusFarm == "Work") {
                sendDataIfNeeded(); // Отправка данных на сервер
                if(!sendMessageOK) {
                    serializeStatus(); // Отправка статуса фермы
                }
            }
            // Отправка данных из очереди               
            if(dequeueIndex > 0 || enqueueIndex > 0) {
                while((millis() - timeStartSlot) < 45000) {  // временное окно для пересылки сообщений из SD 45000 мс
                    if(dequeueIndex == 0 && enqueueIndex == 0) {
                        break;
                    }                    
                    if (!sendMessageOK && connected) {
                        Serial.println("Отправка сообщения из очереди");
                        dequeue(); // Отправка данных из очереди на сервер
                    }

                    delay(1);  // Небольшая задержка чтобы не нагружать процессор
                }
            }                     
        //Serial.println("Время передачи: " + String(timeSlot) + " ms");  
        timeSlot = (millis() - timeStartSlot);      
        //Serial.println("Время слота: " + String(timeSlot) + " ms");   
        vTaskDelay((59999 - timeSlot) / portTICK_PERIOD_MS);  // Задержка 60000 мс          
    }
}

void updateMenuTask(void *parameter) {
    for (;;) {
        updateButtonState();
        vTaskDelay(70 / portTICK_PERIOD_MS);  // Задержка 70 мс
    }
}

void updateWaterTask(void *parameter) {
    for (;;) {
        webSocket.poll(); // Обработка WebSocket событий
        readPCF8574(); // Чтение состояния датчиков холла  

        updateWatering();       
        updateLightBrightness();  
        updateWater();        
        updateFanControl();
        //updateStepperControl(); // Обновление состояния двигателя
        updateClimateControl(); // Обновление климат-контроля
        vTaskDelay(100 / portTICK_PERIOD_MS);  // Задержка 100 мс
    }
}

void setup() {
    Serial.begin(115200);
    Serial.setDebugOutput(false); // Отключение вывода отладочных сообщений
    //ArduinoOTA.begin();

    EEPROM.begin(2048); // Инициализация EEPROM с размером 512 байт

    // Инициализация EEPROM
    if (!EEPROM.begin(EEPROM_SIZE)) {
        Serial.println("Failed to initialize EEPROM");
        return;
    } 

    // Переход в режим Точки доступа, если кнопка MODE нажата в момент включения
    bool executeOnce = true;
    if (executeOnce) {
        if(!analogRead(START_BUTTON_PIN)) {
        Serial.println("Access Point Started");
        accessPoint.start();
        while (executeOnce == true) {

        delay(100);
        }
        executeOnce = false;
        }
    }
    executeOnce = false;


    // Подключение к WiFi
    WiFi.begin(ssid, password);
    
    connectToWiFi();

    initTimeModule();    // Инициализируем модуль времени

    syncTimeWithNTP("pool.ntp.org", timeZone); // Синхронизируем время с NTP

    initializeSettingsModule(); // Инициализация модуля настроек
 
    setupWatering(); // Инициализация модуля полива

    setupLightControl(); // Инициализация модуля управления светом

    initializeSensors();  // Инициализация модуля сенсоров  

    setupStepper(); // Инициализация модуля управления шаговым двигателем

    setupFan(); // Инициализация модуля вентиляции  

    setupClimateControl(); // Инициализация модуля климат-контроля

    initializeWebSocket();  // Инициализация WebSocket

    setupCDcard(); // Инициализация SD карты

    initializeMenu(); // Инициализация модуля меню  

    readAllHTU21D();

    updateSensors(); // Обновление сенсоров

    setupWater(); // Инициализация модуля управления водой

    readAllDS18B20();

    pintStatusFarm = true;   
    CurrentStatusFarm(); // Определение текущего статуса фермы  

    //updateFanControl(); // Обновление вентиляции
    //updateLightBrightness(); // Обновление яркости света

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
        15000,  // Размер стека задачи
        NULL,
        1,
        NULL,
        tskNO_AFFINITY
    );
}

void loop() {
    // ArduinoOTA.handle(); // Обработка OTA обновлений
    // Другие задачи, если есть
    updateStepperControl();
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

