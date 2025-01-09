#include <Arduino.h>
#include <ArduinoWebsockets.h>
#include "WebSocketHandler.h"
#include "globals.h"
#include "DataSender.h"

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
            connectWebSocket(); // Попытка повторного соединения
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

    if (message == "Settings Farm") {
        Serial.println("Сделат чтение настроек из EEPROM");
    }
    if (message == "Work Start") {
        Serial.println("Начало цикла выращивания");
    }
    if (message == "test") {
        Serial.println("Конец цикла выращивания");
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
    webSocket.ping();
    if (!connected) {
        if (currentMillis - lastReconnectAttempt >= RECONNECT_INTERVAL) {
            lastReconnectAttempt = currentMillis;
            connectWebSocket();
        }
    }
}

