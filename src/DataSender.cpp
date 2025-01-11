#include "DataSender.h"
#include "globals.h"
#include "TimeModule.h"
#include "SensorsModule.h"
#include "WebSocketHandler.h"
#include "CurrentProfile.h"

// Отправка данных
void sendDataIfNeeded() {
    printCurrentTime();
    static unsigned long lastTime = 0;
    unsigned long currentTime = millis();

    DynamicJsonDocument doc(512);
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
    
    // Добавление ID фермы и типа сообщения и длинны перед JSON, разделенные пробелом
    TYPE_MSG = FARM_LOG_INFO; // Тип сообщения "FLIN" - данные от фермы на сервер данные
    ID_FARM = 255;  // ID фермы
    LENGTH_MSG = jsonMessage.length(); // Длина JSON сообщения
    String messageToSend = String(ID_FARM) + " " + TYPE_MSG + " " + String(LENGTH_MSG) + " " + jsonMessage;

    // Отправка сообщения
    sendWebSocketMessage(messageToSend);
}

void saveMessageToSDCard(const String& message) {
    // Заглушка записи на SD-карту
    Serial.print("Сохраняем сообщение на SD-карту: ");
    Serial.println(message);
}

