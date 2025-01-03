#include "DataSender.h"
#include "globals.h"
#include "TimeModule.h"
#include "SensorsModule.h"

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
    printCurrentTime();
    static unsigned long lastTime = 0;
    // Формируем JSON
    JsonDocument doc;
// Состояние кнопок
    doc["CurrentDate"] = CurrentDate;
    doc["CurrentTime"] = CurrentTime;
    doc["start_Button"] = start_Button;
    doc["stop_Button"] = stop_Button;
    doc["mode_Button"] = mode_Button;

    // Состояние датчиков уровня воды
    doc["max_osmo_level"] = max_osmo_level;
    doc["min_osmo_level"] = min_osmo_level;
    doc["max_water_level"] = max_water_level;
    doc["min_water_level"] = min_water_level;

    // Данные с датчиков HTU21D
    doc["temperature_1"] = temperature_1;
    doc["humidity_1"] = humidity_1;
    doc["temperature_2"] = temperature_2;
    doc["humidity_2"] = humidity_2;
    doc["temperature_3"] = temperature_3;
    doc["humidity_3"] = humidity_3;
    doc["temperature_4"] = temperature_4;
    doc["humidity_4"] = humidity_4;
    doc["temperature_5"] = temperature_5;
    doc["humidity_5"] = humidity_5;

    // Данные с датчиков температуры
    doc["water_temperature_osmo"] = water_temperature_osmo;
    doc["water_temperature_watering"] = water_temperature_watering;
    doc["air_temperature_outdoor"] = air_temperature_outdoor;
    doc["air_temperature_inlet"] = air_temperature_inlet;

    // Данные качества воды
    doc["ph_osmo"] = ph_osmo;
    doc["tds_osmo"] = tds_osmo;

    // Мониторинг питающей сети
    doc["power_monitor"] = power_monitor;

        String jsonMessage;
        serializeJson(doc, jsonMessage);

        // Отправляем сообщение
        client.send(jsonMessage);
        lastTime = millis();

        Serial.print("Sent: ");
        Serial.println(jsonMessage);    
}
