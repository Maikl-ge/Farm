#include "DataSender.h"
#include "globals.h"
#include "TimeModule.h"
#include "SensorsModule.h"
#include "WebSocketHandler.h"
#include "CurrentProfile.h"
#include <ArduinoJson.h>
#include <pinout.h>
#include <SPI.h>
#include <SD.h>

// Отправка данных
void sendDataIfNeeded() {
    printCurrentTime();
    updateSensors();
    static unsigned long lastTime = 0;
    unsigned long currentTime = millis();
    sendMessageOK = false;
    
    DynamicJsonDocument doc(512);
    doc["DF"] = CurrentDate;
    doc["TF"] = CurrentTime;
    doc["start_Button"] = 1; //startButtonPressed ? 1 : 0;
    doc["stop_Button"] = 0; //stopButtonPressed ? 1 : 0;
    doc["mode_Button"] = 0; //modeButtonPressed ? 1 : 0;
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
    doc["ph"] = CO2;
    doc["ph"] = ph_osmo;
    doc["tds"] = tds_osmo;
    doc["pm"] = power_monitor ? 1 : 0;

    String jsonMessage;
    serializeJson(doc, jsonMessage);
    
    // Добавление ID фермы и типа сообщения и длинны перед JSON, разделенные пробелом
    TYPE_MSG = FARM_LOG_INFO; // Тип сообщения "FLIN" - данные от фермы на сервер данные
    //ID_FARM = 255;  // ID фермы
    LENGTH_MSG = jsonMessage.length(); // Длина JSON сообщения

    // Отправка сообщения
    sendWebSocketMessage(String(ID_FARM), String(TYPE_MSG), String(LENGTH_MSG), jsonMessage);
    
    // Ожидание ACK с таймаутом
    unsigned long startWait = millis();
    while(millis() - startWait < 4000) {  // ждем 4000 мс
        if (type_msg_ACK == TYPE_MSG && ack_ACK == "ACK") {
            Serial.println("Квитанция ACK Параметров получена " +  String(millis() - startWait) + " ms");
            type_msg_ACK = "";
            ack_ACK = "";
            id_farm_ACK = "";
            sendMessageOK = true;
            return;
        }
        delay(1);  // Небольшая задержка чтобы не нагружать процессор
    }

    // Если ACK не получен за 4000 мс
    Serial.println("Таймаут ожидания ACK Параметров");
    sendMessageOK = true;
    saveMessageToSDCard(jsonMessage);
}

// Функция для сериализации переменных в JSON
void serializeStatus() {
    // Создаем объект JSON
    DynamicJsonDocument doc(512);

    // Заполняем объект данными
    doc["OSMOS_ON"] = OSMOS_ON ? 1 : 0;           // Подача очищенной воды (ON/OFF) (GPIO32, нога 7)
    doc["PUMP_WATERING"] = PUMP_WATERING ? 1 : 0; // Полив (ON/OFF) (GPIO33, нога 8)
    doc["PUMP_TRANSFER"] = PUMP_TRANSFER ? 1 : 0; // Подача в бак полива osmo воды (ON/OFF) (GPIO26, нога 10)
    doc["WATER_OUT"] = WATER_OUT ? 1 : 0;         // Слив (ON/OFF) (GPIO27, нога 11)
    doc["STEAM_IN"] = STEAM_IN ? 1 : 0;           // Парогенератор (ON/OFF) (GPIO3, нога 34)

    doc["LIGHT"] = LIGHT;                // Свет (PWM) (GPIO02, нога 24)
    doc["FAN_RACK"] = FAN_RACK;          // Циркуляция внутри 1 и 2 полки (PWM) (GPIO15, нога 23)
    doc["FAN_SHELF"] = FAN_SHELF;        // Циркуляция внутри 3 и 4 полки (PWM) (GPIO17, нога 28)
    doc["FAN_CIRC"] = FAN_CIRC;          // Циркуляция внутри камеры (PWM) (GPIO16, нога 27)
    doc["FAN_INLET"] = FAN_INLET;        // Подача воздуха из вне (PWM) (GPIO12, нога 13)
    doc["HITER_AIR"] = HITER_AIR;        // Обогрев камеры (PWM) (GPIO13, нога 15)
    doc["HITER_WATER"] = HITER_WATER;    // Нагрев воды (PWM) (GPIO14, нога 12)
    doc["FAN_OPTION"] = FAN_OPTION;      // Опциональный вентилятор (GPIO25, нога 9)

    doc["STEP"] = STEP;                 // Шаговый двигатель (GPIO1, нога 35)
    doc["DIR"] = DIR;                   // Направление (GPIO0, нога 25)
    doc["ENABLE"] = ENABLE ? 1 : 0;     // Включение (GPIO0, нога 25)

    // Сериализуем в строку JSON
    String jsonStatus;
    serializeJson(doc, jsonStatus);

    // Добавление ID фермы и типа сообщения и длинны перед JSON, разделенные пробелом
    TYPE_MSG = FARM_DATA_STATUS; // Тип сообщения "FDST" - Статус от фермы на сервер данные
    LENGTH_MSG = jsonStatus.length(); // Длина JSON сообщения

    // Отправка сообщения Статуса фермы
    sendWebSocketMessage(String(ID_FARM), String(TYPE_MSG), String(LENGTH_MSG), jsonStatus);

      // Ожидание ACK с таймаутом
    unsigned long startWait = millis();
    while(millis() - startWait < 4000) {  // ждем 4000 мс
        if (type_msg_ACK == TYPE_MSG && ack_ACK == "ACK") {
            Serial.println("Квитанция ACK Статуса получена " +  String(millis() - startWait) + " ms");
            type_msg_ACK = "";
            ack_ACK = "";
            id_farm_ACK = "";
            sendMessageOK = false;
            return;
        }
        delay(1);  // Небольшая задержка чтобы не нагружать процессор
    }
    // Если ACK не получен за 4000 мс
    Serial.println("Таймаут ожидания ACK Статуса");
    sendMessageOK = false;
    saveMessageToSDCard(jsonStatus);
}

void saveMessageToSDCard(const String& message) {
    // Заглушка записи на SD-карту
    Serial.print("Сохраняем сообщение на SD-карту: ");
    Serial.println(message);
}

void setupCDcard() {
  Serial.println("Инициализация SD-карты...");

  // Настройка SPI с указанием кастомных пинов
  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

  // Инициализация SD-карты
  if (!SD.begin(SD_CS)) {
    Serial.println("Не удалось инициализировать SD-карту!");
    return;
  }
  Serial.println("SD-карта успешно инициализирована.");

  // Вывод содержимого SD-карты
}