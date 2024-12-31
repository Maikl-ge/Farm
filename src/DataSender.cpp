#include "DataSender.h"
#include "globals.h"
#include "TimeModule.h"

WebsocketsClient client;
bool connected = false;

// Инициализация WebSocket
void initializeWebSocket() {
    client.onMessage([](WebsocketsMessage message) {
        Serial.print("Received: ");
        Serial.println(message.data());
    });

    client.onEvent([](WebsocketsEvent event, String data) {
        if (event == WebsocketsEvent::ConnectionOpened) {
            Serial.println("WebSocket connection opened");
            connected = true;
        } else if (event == WebsocketsEvent::ConnectionClosed) {
            Serial.println("WebSocket connection closed");
            connected = false;
        }
    });

    if (client.connect(ws_server)) {
        Serial.println("Connected to WebSocket server!");
        connected = true;
    } else {
        Serial.println("Failed to connect to WebSocket server");
    }
}

// Обновление состояния WebSocket
void processWebSocket() {
    client.poll();

    if (!connected) {
        Serial.println("Reconnecting to WebSocket server...");
        if (client.connect(ws_server)) {
            Serial.println("Reconnected to WebSocket server!");
            connected = true;
        }
    }
}

// Отправка данных
void sendDataIfNeeded() {
    static unsigned long lastTime = 0;

    if (connected && millis() - lastTime > 60000) {
        // Обновляем данные
        temperature = random(1800, 3000) / 100.0;
        humidity = random(300, 800) / 10.0;
        waterLevel = random(300, 800) / 5.0;

        // Формируем JSON
        JsonDocument doc;
        doc["температура"] = temperature;
        doc["влажность"] = humidity;
        doc["уровень воды"] = waterLevel;

        String jsonMessage;
        serializeJson(doc, jsonMessage);

        // Отправляем сообщение
        client.send(jsonMessage);
        lastTime = millis();

        Serial.print("Sent: ");
        Serial.println(jsonMessage);
        // Выводим текущее время
        Serial.println("Время");
        printCurrentTime();  
    }
}
