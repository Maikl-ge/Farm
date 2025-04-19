#include <Arduino.h>
#include <ArduinoWebsockets.h>
#include "WebSocketHandler.h"
#include "globals.h"
#include <DataSender.h>
#include "Profile.h"
#include <SDCard.h>
#include <TimeLib.h>
#include <queue>
#include <menu.h>
#include <status.h>
#include "TimeModule.h"
#include <EEPROM.h>

std::queue<String> ackQueue;  // Очередь для хранения ACK

#define WEBSOCKETS_MAX_DATA_SIZE 2048 // Максимальный размер данных

using namespace websockets;

// Глобальные переменные для WebSocket клиента 
WebsocketsClient webSocket;
String messageFromServer;
String messageACK;
String id_farm_message = "  ";
String type_msg_message = "  ";
uint16_t currentTimeInMinutes = 0;
bool connectedWebSocket = false;
bool connected = false;
unsigned long lastReconnectAttempt = 0;
const unsigned long RECONNECT_INTERVAL = 5000; // Интервал повторного подключения в мс

// Прототипы функций
void connectWebSocket();
void resetWebSocketState();
void handleWebSocketMessage(const String& message);
void parceMessageFromServer(const String& messageFromServer);
void getCurrentDateToGrowe();

void initializeWebSocket() {
    WiFi.setSleep(false);
    webSocket.onMessage([](WebsocketsMessage message) {
        handleWebSocketMessage(message.data());
    });

    webSocket.onEvent([](WebsocketsEvent event, String data) {
        webSocketEvent(event, data);
    });

    connectWebSocket();
}

void webSocketEvent(WebsocketsEvent event, String data) {
    switch (event) {
        case WebsocketsEvent::ConnectionOpened:
            Serial.println("WebSocket connection opened");
            connected = true;
            break;
        case WebsocketsEvent::ConnectionClosed:
            Serial.println("WebSocket connection closed !!!");
            connectedWebSocket = false;
            processWebSocket();  // Переподключаем
            break;
        case WebsocketsEvent::GotPing:
            webSocket.pong(); // Отправка PONG-сообщения
            //Serial.println("Got a Ping!");
            break;
        case WebsocketsEvent::GotPong:
            missedPongs = 0;   // Сброс счетчика
            Serial.println("Got a Pong!");
            break;
    }
}

void parceMessageFromServer(const String& messageFromServer) {
    Serial.print(" Получена команда от сервера: ");
    Serial.println(messageFromServer);
    CurrentStatusFarm();
    const unsigned long waitDuration = 5000;
    unsigned long waitStart = millis(); 
    // Обработка сообщения КОМАНДЫ
    startButton = false;
    stopButton = false;
    if (messageFromServer == SERVER_CMD_START) {    // SCMD Запуск цикла роста
        while (millis() - waitStart < waitDuration) {   // Ожидаем 5 секунд, пока не нажата нужная комбинация кнопок
            if (startButton || statusFarm == "Work") {
                break;
            }
            delay(10); // лёгкая пауза, чтобы не перегружать цикл
        }
        if (!startButton || statusFarm == "Work") {
            Serial.println("Запуск отменен: кнопка подтверждения не нажата."); 
            return; // Прерываем выполнение команды
        }

        startButton = false;
        currentTimeInMinutes = getCurrentTimeInMinutes();
        totalMinutesElapsed = 0;
        statusFarm = "Work";
        CurrentStatusFarm();
        saveStringToEEPROM(EEPROM_STATUS_BOX_ADDRESS, statusFarm);
        Serial.println("Команда от сервера: START"); 

        GROWE_MODE_TIME = currentTimeInMinutes;
        saveUint16ToEEPROM(EEPROM_GROWE_MODE_TIME_ADDRESS, currentTimeInMinutes);   
        EEPROM.commit();        
        Serial.println("Время начала цикла роста: " + String(GROWE_MODE_TIME));

        getCurrentDateToGrowe();
        saveUint16ToEEPROM(EEPROM_GROWE_MODE_DATE_ADDRESS, GROWE_MODE_DATE);
        EEPROM.commit();
        
        // Распаковываем и выводим дату в читаемом виде
        time_t rawTime = (GROWE_MODE_DATE * 86400); // Перевод дней в секунды
        tmElements_t t;
        breakTime(rawTime, t); // Разбираем в структуру времени
        
        Serial.printf("Dата начала цикла роста: %02d.%02d.%04d\n", t.Day, t.Month, t.Year + 1970);
        totalMinutesElapsed = 0;
    }
    
    if (messageFromServer == SERVER_CMD_STOP) {      // SCMS Остановка цикла роста
        while (millis() - waitStart < waitDuration) {   // Ожидаем 5 секунд, пока не нажата нужная комбинация кнопок
            if (stopButton) {
                break;
            }
            delay(10); // лёгкая пауза, чтобы не перегружать цикл
        }
        if (!stopButton) {
            Serial.println("Остановка отменена: кнопка подтверждения не нажата."); return; // Прерываем выполнение команды
        }

        stopButton = false;
        statusFarm = "Stop";
        CurrentStatusFarm();
        saveStringToEEPROM(EEPROM_STATUS_BOX_ADDRESS, statusFarm);        
        Serial.println("Команда от сервера: STOP");

        getCurrentDateToGrowe();
        saveUint16ToEEPROM(EEPROM_GROWE_MODE_DATE_ADDRESS, GROWE_MODE_DATE);
        EEPROM.commit();

        GROWE_MODE_TIME = currentTimeInMinutes;
        saveUint16ToEEPROM(EEPROM_GROWE_MODE_TIME_ADDRESS, currentTimeInMinutes);   
        EEPROM.commit();      
        Serial.println("Время завершения цикла роста: " + String(GROWE_MODE_TIME));

        getCurrentDateToGrowe();
        time_t rawTime = (GROWE_MODE_DATE * 86400); // Перевод дней в секунды
        tmElements_t t;
        breakTime(rawTime, t); // Разбираем в структуру времени
        
        Serial.printf("Dата завершения цикла роста: %02d.%02d.%04d\n", t.Day, t.Month, t.Year + 1970);
        totalMinutesElapsed = 0;
    }

    if (messageFromServer == SERVER_CMD_RESTART) {    // SCMR Перезагрузка фермы
        Serial.println("Команда от сервера: RESTART");
        esp_restart();
    }
    if (messageFromServer == SERVER_CMD_UPDATE) {      // SCMU Обновление ПО контроллера фермы
        Serial.println("Команда от сервера: UPDATE");
    }
    if (messageFromServer == SERVER_CMD_SETTINGS) {    // Получена команда Обновление настроек
        fetchAndSaveSettings();
        Serial.println("Команда от сервера: SETTINGS NEW");
    }
    
    // Обработка сообщения ЗАПРОСЫ
    if (messageFromServer == SERVER_REQ_STATUS) {     // SRST Запрос статуса фермы
        serializeStatus();       // Отправка статуса
        Serial.println("Запрос от сервера: STATUS");
    }
    if (messageFromServer == SERVER_REQ_DATA) {
        sendDataIfNeeded();    // Отправка данных
        Serial.println("Запрос от сервера: DATA");
    }
    if (messageFromServer == SERVER_REQ_SETTINGS) {  // SRSE Запрос на отправку Настройки фермы
        serializeSettings();  // Отправка настроек на ферму
        Serial.println("Настройки из EEPROM");
    }
    if (messageFromServer == SERVER_REQ_PARAMETERS) {
        sendDataIfNeeded();
        Serial.println("Запрос от сервера: PARAMETERS");
    }
    if (messageFromServer == SERVER_REQ_PROFILE) {
        Serial.println("Запрос от сервера: PROFILE");
    }
    if (messageFromServer == SERVER_REQ_CURRENT) {
        Serial.println("Запрос от сервера: CURRENT");
    }
    // Обработка сообщения ошибки и 
    if (messageFromServer == SERVER_ERR_INVALID) {
        Serial.println("Ошибка: недействительный запрос");
    }
    if (messageFromServer == SERVER_EVENT_SYNC) {
        Serial.println("Событие синхронизации");
    }
}    

void sendWebSocketMessage(const String& messageToSend) {
    if (connected) {
        webSocket.send(messageToSend);
        Serial.print("Send: ");
        Serial.println(TYPE_MSG); // остается от предидущего варианта
    } else {
        Serial.println("Соединение отсутствует. Сообщение не отправлено.");
        enqueue(sd, messageToSend);   
    }
    handleWebSocketMessage(messageToSend);  // Разбор сообщения
    if(connected) {

        // Ожидание ACK с таймаутом
        unsigned long startWait = millis();
        while(millis() - startWait < 4000) {  // ждем 4000 мс
            if (type_msg_ACK == TYPE_MSG && ack_ACK == "ACK") {
                Serial.println("Квитанция ACK " + String(type_msg_ACK) + " получена " +  String(millis() - startWait) + " ms");
                type_msg_ACK = "";
                ack_ACK = "";
                id_farm_ACK = "";
                TYPE_MSG = "";
                sendMessageOK = false;
                enqueueASK = "sendOk";
                timeSlot = timeSlot + (millis() - transmitionTime);
                return;
            }
            delay(1);  // Небольшая задержка чтобы не нагружать процессор
        }
    
        // Если ACK не получен за 4000 мс
        Serial.println("Таймаут ожидания ACK " + String(type_msg_ACK));
        sendMessageOK = false;
        if(enqueueASK != "send" && type_msg_ACK != "FRQS") {
        enqueue(sd, messageToSend);   
        }
        enqueueASK = "empty";
    } 
}

// Функция для разбора сообщения сервера на три переменные
void handleWebSocketMessage(const String& message) {
    Serial.print("Received WebSocket message: ");
    Serial.println(message);
    messageFromServer = message;
    messageACK = message;

    // Проверяем, что сообщение не пустое
    if (messageACK.isEmpty()) {
        Serial.println("Ошибка: пустое сообщение ACK");
        return;
    }

    // Находим позиции пробелов
    int firstSpace = messageACK.indexOf(' ');  // Позиция первого пробела
    int secondSpace = messageACK.indexOf(' ', firstSpace + 1);  // Позиция второго пробела

    // Проверяем корректность структуры сообщения
    if (firstSpace == -1 || secondSpace == -1) {
        Serial.println("Ошибка разбора сообщения: недостаточно частей");
        return;
    }

    // Извлечение частей
    id_farm_ACK = messageACK.substring(0, firstSpace);                // Первая часть: номер фермы
    type_msg_ACK = messageACK.substring(firstSpace + 1, secondSpace); // Вторая часть: тип сообщения
    ack_ACK = messageACK.substring(secondSpace + 1);                  // Третья часть: квитанция
    parceMessageFromServer(type_msg_ACK);
    if(enqueueASK == "send") {
        TYPE_MSG = type_msg_ACK;
    }
}

void processWebSocket() {
    static unsigned long lastPing = 0;
    unsigned long currentMillis = millis();

    if (!connected) {
        // Проверка состояния Wi-Fi
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("Wi-Fi disconnected, attempting to reconnect...");
            WiFi.reconnect();
            delay(1000); // Небольшая задержка для стабилизации
            return;      // Не пытаемся подключиться к WebSocket, пока Wi-Fi не восстановится
        }

        // Если Wi-Fi в порядке, проверяем необходимость переподключения WebSocket
        if (currentMillis - lastReconnectAttempt >= RECONNECT_INTERVAL) {
           lastReconnectAttempt = currentMillis;
            connectWebSocket();
        }
    } 
}

void resetWebSocketState() {
    Serial.println("Resetting WebSocket state...");

    // Очистка буферов сообщений
    messageFromServer = "";
    messageACK = "";
    
    // Очистка переменных ACK
    type_msg_ACK = "";
    ack_ACK = "";
    id_farm_ACK = "";
    
    // Сброс счётчиков
    missedPongs = 0;
    lastReconnectAttempt = 0;

    // Очистка флагов
    connected = false;

    // Очистка дополнительных буферов или очередей
    //  pendingMessages.clear();  // Если используется очередь сообщений
    
    try {
        // Закрытие WebSocket соединения
        Serial.println("Закрываем соединение, если оно есть: ");
        webSocket.close();  // Закрываем соединение, если оно есть
    } catch (const std::exception& e) {
        Serial.print("Exception during WebSocket close: ");
        Serial.println(e.what());
    }

    Serial.println("WebSocket state has been reset");
}

void connectWebSocket() {
    static bool isConnecting = false;
    if (isConnecting) return;  // Если уже идет подключение, то выходим

    Serial.println("Attempting to connect to WebSocket...");

    // Очищаем состояние перед новым подключением    
    resetWebSocketState();

    isConnecting = true;
    
    try {
        connected = webSocket.connect(ws_server);
        if (connected) {
            Serial.println("WebSocket connected");
            connectedWebSocket = true;
        } else {
            Serial.println("WebSocket connection failed");
        }
    } catch (const std::exception& e) {
        Serial.print("Exception during WebSocket connection: ");
        Serial.println(e.what());
    }

    isConnecting = false;
} 
void saveStringToEEPROM(int address, String& statusFarm) {
    int len = statusFarm.length();
    for (int i = 0; i < len; i++) {
        EEPROM.write(address + i, statusFarm[i]);
    }
    EEPROM.write(address + len, '\0'); // Добавление терминального нуля для завершения строки    
    EEPROM.commit(); // Сохранение изменений в EEPROM
}