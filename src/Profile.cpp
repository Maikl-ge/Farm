#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include "globals.h"
#include "CurrentProfile.h"
#include "Profile.h"
#include "DataSender.h"

// Константы для адресов и значений
const int EEPROM_START_ADDRESS = 0x00;
const int EEPROM_END_ADDRESS = 0x2D;
const int EEPROM_CHECK_ADDRESS = 0x24;
const uint16_t VALUE_READY = 0x5245;  // RE - Ready статус фермы в EEPROM
const uint16_t VALUE_WORK = 0x574F;  // WO - Work статус фермы в EEPROM
const uint16_t VALUE_END = 0x454E;  // EN - End статус фермы в EEPROM


// Сохраняет `uint16_t` значение в EEPROM
void saveUint16ToEEPROM(int address, uint16_t value) {
    EEPROM.write(address, value & 0xFF);       // Младший байт
    EEPROM.write(address + 1, (value >> 8));  // Старший байт
}

// Функция для загрузки настроек с сервера и сохранения в глобальные переменные
void fetchAndSaveSettings() {
    HTTPClient http;
    http.begin(http_server);
    int httpResponseCode = http.GET();

    if (httpResponseCode == 200) {
        String response = http.getString();
        Serial.println("Ответ получен:");
//        Serial.println(response);

        // Парсим JSON-ответ
        DynamicJsonDocument doc(512);
        DeserializationError error = deserializeJson(doc, response);

        if (error) {
            Serial.print("Ошибка парсинга JSON: ");
            Serial.println(error.c_str());
            return;
        }

        // Присваиваем значения глобальным переменным
        DAY_CIRCULATION = doc["dayCirculation"];
        DAY_HUMIDITY_START = static_cast<uint16_t>(doc["dayHumidityStart"].as<float>() * 10);
        DAY_HUMIDITY_END = static_cast<uint16_t>(doc["dayHumidityEnd"].as<float>() * 10);
        DAY_TEMPERATURE_START = static_cast<uint16_t>(doc["dayTemperatureStart"].as<float>() * 10);
        DAY_TEMPERATURE_END = static_cast<uint16_t>(doc["dayTemperatureEnd"].as<float>() * 10);
        DAY_VENTILATION = doc["dayVentilation"];
        DAY_WATERING_INTERVAL = doc["dayWateringInterval"];

        NIGHT_CIRCULATION = doc["nightCirculation"];
        NIGHT_HUMIDITY_START = static_cast<uint16_t>(doc["nightHumidityStart"].as<float>() * 10);
        NIGHT_HUMIDITY_END = static_cast<uint16_t>(doc["nightHumidityEnd"].as<float>() * 10);
        NIGHT_TEMPERATURE_START = static_cast<uint16_t>(doc["nightTemperatureStart"].as<float>() * 10);
        NIGHT_TEMPERATURE_END = static_cast<uint16_t>(doc["nightTemperatureEnd"].as<float>() * 10);
        NIGHT_VENTILATION = doc["nightVentilation"];
        NIGHT_WATERING_INTERVAL = doc["nightWateringInterval"];

        SUNRISE = doc["sunrise"];
        SUNSET = doc["sunset"];
        WATER_TEMPERATURE = static_cast<uint16_t>(doc["waterTemperature"].as<float>() * 10);

        CYCLE = doc["cycle"];

        Serial.println("Все настройки сохранены в глобальные переменные.");

        // Сохраняем настройки в EEPROM
        saveSettingsToEEPROM();
    } else {
        Serial.printf("Ошибка HTTP-запроса. Код: %d\n", httpResponseCode);
    }

    http.end();
}

// Функция для чтения двух байт из EEPROM и объединения их в uint16_t
uint16_t readUint16FromEEPROM(int address) {
    return EEPROM.read(address) | (EEPROM.read(address + 1) << 8);
}

// Функция для вывода значений из EEPROM
void printEEPROMValues(int startAddress, int endAddress) {
    for (int address = startAddress; address <= endAddress; address += 2) {
        uint16_t value = readUint16FromEEPROM(address);
        Serial.printf("Адрес 0x%02X-0x%02X: %u\n", address, address + 1, value);
    }
}

// Инициализация модуля
void initializeSettingsModule() {
    // Чтение двух байт и объединение в uint16_t
    uint16_t value = readUint16FromEEPROM(EEPROM_CHECK_ADDRESS);

    Serial.printf("Прочитанное значение: 0x%04X\n", value);

    // Проверка значения
    if (value == VALUE_READY) {  // Проверка статуса RE "Ready"
        fetchAndSaveSettings();   
        EEPROMRead(); // Чтение настроек из EEPROM
        return; // Завершаем выполнение функции
    } 
    if (value == VALUE_WORK || VALUE_END) {  // Проверка статуса WO "Work"    
        EEPROMRead(); // Чтение настроек из EEPROM
    }
    if (value == VALUE_END) {  // Проверка статуса EN "End"
        Serial.println("EEPROM не содержит настроек.");
    }
}    

// Сохраняет настройки в EEPROM
void saveSettingsToEEPROM() {
    int address = 0;

    saveUint16ToEEPROM(address, DAY_CIRCULATION);        address += 2;
    saveUint16ToEEPROM(address, DAY_HUMIDITY_START);    address += 2;
    saveUint16ToEEPROM(address, DAY_HUMIDITY_END);      address += 2;
    saveUint16ToEEPROM(address, DAY_TEMPERATURE_START); address += 2;
    saveUint16ToEEPROM(address, DAY_TEMPERATURE_END);   address += 2;
    saveUint16ToEEPROM(address, DAY_VENTILATION);       address += 2;
    saveUint16ToEEPROM(address, DAY_WATERING_INTERVAL); address += 2;

    saveUint16ToEEPROM(address, NIGHT_CIRCULATION);        address += 2;
    saveUint16ToEEPROM(address, NIGHT_HUMIDITY_START);    address += 2;
    saveUint16ToEEPROM(address, NIGHT_HUMIDITY_END);      address += 2;
    saveUint16ToEEPROM(address, NIGHT_TEMPERATURE_START); address += 2;
    saveUint16ToEEPROM(address, NIGHT_TEMPERATURE_END);   address += 2;
    saveUint16ToEEPROM(address, NIGHT_VENTILATION);       address += 2;
    saveUint16ToEEPROM(address, NIGHT_WATERING_INTERVAL); address += 2;

    saveUint16ToEEPROM(address, SUNRISE); address += 2;
    saveUint16ToEEPROM(address, SUNSET);  address += 2;
    saveUint16ToEEPROM(address, WATER_TEMPERATURE); address += 2;

    saveUint16ToEEPROM(address, CYCLE); address += 2;
    saveUint16ToEEPROM(address, WORK); address += 2;
    saveUint16ToEEPROM(address, GROWE_START); address += 2; 
    
    EEPROM.commit(); // Запись изменений в EEPROM
    Serial.println("Настройки сохранены в EEPROM.");
}

    void EEPROMRead() {
            // Присвоение переменным считанных значений
        int address = EEPROM_START_ADDRESS;
        DAY_CIRCULATION = readUint16FromEEPROM(address); address += 2;
        DAY_HUMIDITY_START = readUint16FromEEPROM(address); address += 2;
        DAY_HUMIDITY_END = readUint16FromEEPROM(address); address += 2;
        DAY_TEMPERATURE_START = readUint16FromEEPROM(address); address += 2;
        DAY_TEMPERATURE_END = readUint16FromEEPROM(address); address += 2;
        DAY_VENTILATION = readUint16FromEEPROM(address); address += 2;
        DAY_WATERING_INTERVAL = readUint16FromEEPROM(address); address += 2;
        NIGHT_CIRCULATION = readUint16FromEEPROM(address); address += 2;
        NIGHT_HUMIDITY_START = readUint16FromEEPROM(address); address += 2;
        NIGHT_HUMIDITY_END = readUint16FromEEPROM(address); address += 2;
        NIGHT_TEMPERATURE_START = readUint16FromEEPROM(address); address += 2;
        NIGHT_TEMPERATURE_END = readUint16FromEEPROM(address); address += 2;
        NIGHT_VENTILATION = readUint16FromEEPROM(address); address += 2;
        NIGHT_WATERING_INTERVAL = readUint16FromEEPROM(address); address += 2;
        SUNRISE = readUint16FromEEPROM(address); address += 2;
        SUNSET = readUint16FromEEPROM(address); address += 2;
        WATER_TEMPERATURE = readUint16FromEEPROM(address); address += 2;
        CYCLE = readUint16FromEEPROM(address); address += 2;
        WORK = readUint16FromEEPROM(address); address += 2;
        GROWE_START = readUint16FromEEPROM(address); address += 2;
}

// Сереализация настроек считанных из EEPROM и отправка на сервер
void serializeSettings() {
    EEPROMRead();
    DynamicJsonDocument doc(512);
    doc["dayCirculation"] = DAY_CIRCULATION;
    doc["dayHumidityStart"] = DAY_HUMIDITY_START / 10.0;
    doc["dayHumidityEnd"] = DAY_HUMIDITY_END / 10.0;
    doc["dayTemperatureStart"] = DAY_TEMPERATURE_START / 10.0;
    doc["dayTemperatureEnd"] = DAY_TEMPERATURE_END / 10.0;
    doc["dayVentilation"] = DAY_VENTILATION;
    doc["dayWateringInterval"] = DAY_WATERING_INTERVAL;

    doc["nightCirculation"] = NIGHT_CIRCULATION;
    doc["nightHumidityStart"] = NIGHT_HUMIDITY_START / 10.0;
    doc["nightHumidityEnd"] = NIGHT_HUMIDITY_END / 10.0;
    doc["nightTemperatureStart"] = NIGHT_TEMPERATURE_START / 10.0;
    doc["nightTemperatureEnd"] = NIGHT_TEMPERATURE_END / 10.0;
    doc["nightVentilation"] = NIGHT_VENTILATION;
    doc["nightWateringInterval"] = NIGHT_WATERING_INTERVAL;

    doc["sunrise"] = SUNRISE;
    doc["sunset"] = SUNSET;
    doc["waterTemperature"] = WATER_TEMPERATURE / 10.0;

    doc["cycle"] = CYCLE;
    doc["work"] = WORK;
    doc["groweStart"] = GROWE_START;

    // Сериализация в строку
    String jsonSettings;
    serializeJson(doc, jsonSettings);
    Serial.println(jsonSettings);

    // Добавление ID фермы и типа сообщения и длинны перед JSON, разделенные пробелом
    TYPE_MSG = FARM_RES_SETTINGS; // Тип сообщения "FRQS" - данные от фермы на сервер данные
    ID_FARM = 255;  // ID фермы
    LENGTH_MSG = jsonSettings.length(); // Длина JSON сообщения
    String messageToSend = String(ID_FARM) + " " + TYPE_MSG + " " + String(LENGTH_MSG) + " " + jsonSettings;

    // Отправка сообщения
    sendWebSocketMessage(messageToSend);
}
