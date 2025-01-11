#include <Arduino.h>
#include <ArduinoWebsockets.h>
#include "WebSocketHandler.h"
#include "globals.h"
#include <DataSender.h>
#include "Profile.h"
#include <CurrentProfile.h>

#define WEBSOCKETS_MAX_DATA_SIZE 1024 // Максимальный размер данных

using namespace websockets;

// Глобальные переменные для WebSocket клиента 
WebsocketsClient webSocket;
String messageFromServer;
bool connected = false;
unsigned long lastReconnectAttempt = 0;
const unsigned long RECONNECT_INTERVAL = 20000; // Интервал повторного подключения в мс

void initializeWebSocket() {
    webSocket.onMessage([](WebsocketsMessage message) {
        handleWebSocketMessage(message.data());
    });

    webSocket.onEvent([](WebsocketsEvent event, String data) {
        webSocketEvent(event, data);
    });

    connectWebSocket();
}

void connectWebSocket() {
    connected = webSocket.connect(ws_server);

    if (connected) {
        Serial.println("WebSocket connected");
    } else {
        Serial.println("WebSocket connection failed");
    }
}

void webSocketEvent(WebsocketsEvent event, String data) {
    switch (event) {
        case WebsocketsEvent::ConnectionOpened:
            Serial.println("WebSocket connection opened");
            connected = true;
            break;
        case WebsocketsEvent::ConnectionClosed:
            Serial.println("WebSocket connection closed, attempting reconnect...");
            Serial.println(data);
            connected = false;
            connectWebSocket(); // processWebSocket(); // Попытка повторного соединения
            break;
        case WebsocketsEvent::GotPing:
            Serial.println("Got a Ping!");
            break;
        case WebsocketsEvent::GotPong:
            Serial.println("Got a Pong!");
            break;
    }
}


void handleWebSocketMessage(const String& message) {
    Serial.print("Received WebSocket message: ");
    Serial.println(message);
    messageFromServer = message;
    // Обработка сообщения КОМАНДЫ
    if (messageFromServer == SERVER_CMD_START) {
        Serial.println("Команда от сервера: START");
    }
    if (messageFromServer == SERVER_CMD_STOP) {
        Serial.println("Команда от сервера: STOP");
    }
    if (messageFromServer == SERVER_CMD_RESTART) {
        Serial.println("Команда от сервера: RESTART");
    }
    if (messageFromServer == SERVER_CMD_UPDATE) {
        Serial.println("Команда от сервера: UPDATE");
    }
    // Обработка сообщения ЗАПРОСЫ
    if (messageFromServer == SERVER_REQ_STATUS) {
        Serial.println("Запрос от сервера: STATUS");
    }
    if (messageFromServer == SERVER_REQ_DATA) {
        Serial.println("Запрос от сервера: DATA");
    }
    if (messageFromServer == SERVER_REQ_SETTINGS) {
        Serial.println(readUint16FromEEPROM(EEPROM_WORK_ADDRESS));
        serializeSettings();
        Serial.println("Настройки из EEPROM");
    }
    if (messageFromServer == SERVER_REQ_PARAMETERS) {
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

void sendWebSocketMessage(const String& message) {
    if (connected) {
        webSocket.send(message);
        Serial.print("Sent: ");
        Serial.println(message);
    } else {
        Serial.println("Соединение отсутствует. Сообщение не отправлено.");
        saveMessageToSDCard(message);
    }
}

void processWebSocket() {
    static unsigned long lastPing = 0;
    unsigned long currentMillis = millis();
    //webSocket.ping();
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

