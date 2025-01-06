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

        if (message.data() == "Ok:200") {
            //Serial.println("Сообщение подтверждено сервером.");
        } else {
            //Serial.println("Подтверждение от сервера не получено. Сохраняем сообщение на SD-карту.");
            saveMessageToSDCard(message.data());
        }
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
    // Проверяем состояние соединения
    client.poll();

    if (!connected) {
      //  static unsigned long lastReconnectAttempt = 0;
     //   unsigned long currentTime = millis();

        // Проверяем, прошло ли достаточно времени для повторной попытки подключения
     //   if (currentTime - lastReconnectAttempt >= 5000) { // 5 секунд
     //       Serial.println("Reconnecting to WebSocket server...");
     //       lastReconnectAttempt = currentTime; // Обновляем время последней попытки

            if (client.connect(ws_server)) {
                Serial.println("Reconnected to WebSocket server!");
                connected = true;
            } else {
                Serial.println("Reconnect attempt failed. Waiting 10 seconds before retrying.");
            }
     //   }
    }
}

// Отправка данных
void sendDataIfNeeded() {
    printCurrentTime();
    static unsigned long lastTime = 0;
    unsigned long currentTime = millis();

    StaticJsonDocument<512> doc;
    doc["DF"] = CurrentDate;
    doc["TF"] = CurrentTime;
    doc["start_Button"] = startButtonPressed ? 1 : 0;
    doc["stop_Button"] = stopButtonPressed ? 1 : 0;
    doc["mode_Button"] = modeButtonPressed ? 1 : 0;
    doc["max_osmo_level"] = max_osmo_level ? 1 : 0;
    doc["min_osmo_level"] = min_osmo_level ? 1 : 0;
    doc["max_water_level"] = max_water_level ? 1 : 0;
    doc["min_water_level"] = min_water_level ? 1 : 0;
    doc["T1"] = temperature_1;
    doc["H1"] = humidity_1;
    doc["T2"] = temperature_2;
    doc["H2"] = humidity_2;
    doc["T3"] = temperature_3;
    doc["H3"] = humidity_3;
    doc["T4"] = temperature_4;
    doc["H4"] = humidity_4;
    doc["T5"] = temperature_5;
    doc["H5"] = humidity_5;
    doc["WTO"] = water_temperature_osmo;
    doc["WTW"] = water_temperature_watering;
    doc["ATO"] = air_temperature_outdoor;
    doc["ATI"] = air_temperature_inlet;
    doc["ph"] = ph_osmo;
    doc["tds"] = tds_osmo;
    doc["pm"] = power_monitor ? 1 : 0;

    String jsonMessage;
    serializeJson(doc, jsonMessage);

    if (connected) {
        client.send(jsonMessage);
        Serial.print("Sent: ");
        //Serial.println(jsonMessage);
    } else {
        //Serial.println("Соединение отсутствует. Сообщение не отправлено.");
        saveMessageToSDCard(jsonMessage);
    }
}
void saveMessageToSDCard(const String& message) {
    // Заглушка записи на SD-карту
    Serial.print("Сохраняем сообщение на SD-карту: ");
    //Serial.println(message);
}
