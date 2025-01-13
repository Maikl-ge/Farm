#include "DataSender.h"
#include "globals.h"
#include "TimeModule.h"
#include "SensorsModule.h"
#include "WebSocketHandler.h"
#include "CurrentProfile.h"

// Отправка данных
void sendDataIfNeeded() {
    printCurrentTime();
    updateSensors();
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

    // Отправка сообщения
    sendWebSocketMessage(String(ID_FARM), String(TYPE_MSG), String(LENGTH_MSG), jsonMessage);
    
    // Ожидание ACK с таймаутом
    unsigned long startWait = millis();
    while(millis() - startWait < 300) {  // ждем 300 мс
        if (type_msg_ACK == TYPE_MSG && ack_ACK == "ACK") {
            Serial.println("Квитанция ACK получена");
            type_msg_ACK = "";
            ack_ACK = "";
            id_farm_ACK = "";
            return;
        }
        delay(1);  // Небольшая задержка чтобы не нагружать процессор
    }

    // Если ACK не получен за 300 мс
    Serial.println("Таймаут ожидания ACK");
    if(connected) {
        saveMessageToSDCard(jsonMessage);
    }
}

void saveMessageToSDCard(const String& message) {
    // Заглушка записи на SD-карту
    Serial.print("Сохраняем сообщение на SD-карту: ");
    Serial.println(message);
}

