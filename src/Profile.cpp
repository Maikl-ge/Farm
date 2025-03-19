#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include "globals.h"
#include "Profile.h"
#include "DataSender.h"
#include "WebSocketHandler.h"

//  Константы глобальных переменных
uint8_t ID_FARM = 255;  // ID фермы
String TYPE_MSG = " "; // Тип сообщения
uint16_t LENGTH_MSG = 0; // Длина сообщения

const uint16_t VALUE_READY = 0x5245;  // RE - Ready статус фермы в EEPROM
const uint16_t VALUE_WORK = 0x574F;   // WO - Work статус фермы в EEPROM
const uint16_t VALUE_END = 0x454E;    // EN - End статус фермы в EEPROM
const uint16_t VALUE_ABORT = 0x4142;  // AB - Abort статус фермы в EEPROM

// Константы для адресов и значений
const int EEPROM_START_ADDRESS = 0x30;  // Адрес начала EEPROM

int EEPROM_PHASE_ADDRESS = 0x02;                // Текущая фаза
int EEPROM_STATUS_BOX_ADDRESS = 0x04;           // Текущий статус
int EEPROM_GROWE_START_TIME_ADDRESS = 0x08;  // Адрес 0x2A-0x2B
int EEPROM_GROWE_STOP_DATE_ADDRESS = 0x0A;  // Адрес 0x2C-0x2D
int EEPROM_CULTURE_ADDRESS = 0x0A;     // Текущая культура

String FARM_RES_STATUS = "FRQW";        // Ответ статуса фермы на запрос сервера
String FARM_RES_DATA = "FRQD";          // Ответ с данными фермы на запрос сервера
String FARM_RES_SETTINGS = "FRQS";      // Запрос у сервера настроек и базы данных 
String FARM_RES_PARAMETERS = "FRQP";    // Ответ параметров фермы на запрос сервера
String FARM_RES_PROFILE = "FRQF";       // Ответ профиля фермы на запрос сервера
String FARM_RES_CURRENT = "FRQC";       // Ответ текущих данных фермы

String FARM_DATA_STATUS = "FDST";       // Ответ Статус фермы (например, готова к работе)
String FARM_DATA_TELEMETRY = "FDTL";    // Телеметрические данные фермы
String FARM_DATA_ERROR = "FDER";        // Сообщение об ошибке фермы
String FARM_DATA_EVENT = "FDEV";        // Событие фермы (например, включение устройства)
String FARM_DATA_SETTINGS = "FDSE";     // Настройки фермы на запрос серверу настроек

String FARM_LOG_INFO = "FLIN";          // Информационное сообщение
String FARM_LOG_DEBUG = "FLDB";         // Отладочное сообщение
String FARM_LOG_ERROR = "FLER";         // Ошибка (например, аппаратная или программная)

String SERVER_CMD_START = "SCMD";       // Команда на запуск
String SERVER_CMD_STOP = "SCMS";        // Команда на остановку
String SERVER_CMD_RESTART = "SCMR";     // Команда на перезапуск
String SERVER_CMD_UPDATE = "SCMU";      // Команда на обновление ПО
String SERVER_CMD_SETTINGS = "SCME";    // Команда на обновление настроек

String SERVER_REQ_STATUS = "SRST";      // Запрос о статусе фермы
String SERVER_REQ_DATA = "SRDT";        // Запрос данных фермы
String SERVER_REQ_SETTINGS = "SRSE";    // Запрос о настройках фермы Профиля настроек сохраненных 
String SERVER_REQ_PARAMETERS = "SRPM";  // Запрос о параметрах фермы
String SERVER_REQ_PROFILE = "SRPF";     // Запрос о профиле фермы
String SERVER_REQ_CURRENT = "SRCU";     // Запрос  текущих данных фермы

String SERVER_ERR_INVALID = "SERR";     // Ошибка: недействительный запрос
String SERVER_EVENT_SYNC = "SEVN";  

// Глобальные переменные
uint16_t PHASE = 0;                // Текущая фаза
uint16_t STATUS_BOX = 0; // Текущее состояние фермы
String CULTURE = "";     // Текущая культура
uint16_t CYCLE = 0; // Адрес 0x22-0x23
uint16_t SUNRISE = 0;    // Время восхода
uint16_t SUNSET = 0;     // Время заката
uint16_t GROWE_START_TIME = 0;  // Время начала цикла роста
uint16_t GROWE_STOP_DATE = 0;  // Дата начала цикла роста
// Определение переменных фаз
uint16_t PHASE1_DURATION = 0;
uint16_t PHASE1_TEMP = 0;
uint16_t PHASE1_HUMIDITY = 0;
uint16_t PHASE1_WATER_TEMP = 0;
uint16_t PHASE1_LIGHT = 0;
uint16_t PHASE1_CIRCULATION = 0;
uint16_t PHASE1_VENTILATION = 0;
uint16_t PHASE1_WATERING = 0;
uint16_t PHASE1_DRAINING = 0;
uint16_t PHASE1_ROTATION = 0;
// Фаза 2
uint16_t PHASE2_DURATION = 0;
uint16_t PHASE2_TEMP = 0;
uint16_t PHASE2_HUMIDITY = 0;
uint16_t PHASE2_WATER_TEMP = 0;
uint16_t PHASE2_LIGHT = 0;
uint16_t PHASE2_CIRCULATION = 0;
uint16_t PHASE2_VENTILATION = 0;
uint16_t PHASE2_WATERING = 0;
uint16_t PHASE2_DRAINING = 0;
uint16_t PHASE2_ROTATION = 0;
// Фаза 3
uint16_t PHASE3_DURATION = 0;
uint16_t PHASE3_TEMP = 0;
uint16_t PHASE3_HUMIDITY = 0;
uint16_t PHASE3_WATER_TEMP = 0;
uint16_t PHASE3_LIGHT = 0;
uint16_t PHASE3_CIRCULATION = 0;
uint16_t PHASE3_VENTILATION = 0;
uint16_t PHASE3_WATERING = 0;
uint16_t PHASE3_DRAINING = 0;
uint16_t PHASE3_ROTATION = 0;
// Фаза 4
uint16_t PHASE4_DURATION = 0;
uint16_t PHASE4_TEMP = 0;
uint16_t PHASE4_HUMIDITY = 0;
uint16_t PHASE4_WATER_TEMP = 0;
uint16_t PHASE4_LIGHT = 0;
uint16_t PHASE4_CIRCULATION = 0;
uint16_t PHASE4_VENTILATION = 0;
uint16_t PHASE4_WATERING = 0;
uint16_t PHASE4_DRAINING = 0;
uint16_t PHASE4_ROTATION = 0;
// Фаза 5
uint16_t PHASE5_DURATION = 0;
uint16_t PHASE5_TEMP = 0;
uint16_t PHASE5_HUMIDITY = 0;
uint16_t PHASE5_WATER_TEMP = 0;
uint16_t PHASE5_LIGHT = 0;
uint16_t PHASE5_CIRCULATION = 0;
uint16_t PHASE5_VENTILATION = 0;
uint16_t PHASE5_WATERING = 0;
uint16_t PHASE5_DRAINING = 0;
uint16_t PHASE5_ROTATION = 0;
// Фаза 6
uint16_t PHASE6_DURATION = 0;
uint16_t PHASE6_TEMP = 0;
uint16_t PHASE6_HUMIDITY = 0;
uint16_t PHASE6_WATER_TEMP = 0;
uint16_t PHASE6_LIGHT = 0;
uint16_t PHASE6_CIRCULATION = 0;
uint16_t PHASE6_VENTILATION = 0;
uint16_t PHASE6_WATERING = 0;
uint16_t PHASE6_DRAINING = 0;
uint16_t PHASE6_ROTATION = 0;




// Сохраняет `uint16_t` значение в EEPROM
void saveUint16ToEEPROM(int address, uint16_t value) {
    EEPROM.write(address, value & 0xFF);       // Младший байт
    EEPROM.write(address + 1, (value >> 8));  // Старший байт
}

// Функция для загрузки настроек с сервера и сохранения в глобальные переменные
void fetchAndSaveSettings() {
        // Парсим JSON-ответ
        DynamicJsonDocument doc(2048);
        DeserializationError error = deserializeJson(doc, ack_ACK);

        if (error) {
            Serial.print("Ошибка парсинга JSON: ");
            Serial.println(error.c_str());
            return;
        }

        // Проверяем, что значение "nameProfile" существует и не пустое
        if (doc.containsKey("nameprofile") && !doc["nameprofile"].isNull()) {
            CULTURE = doc["nameprofile"].as<String>();
        } else {
            Serial.println("Ошибка: значение 'nameprofile' отсутствует или пустое");
            CULTURE = "Unknown"; // Устанавливаем значение по умолчанию
        }

        //CULTURE = doc["nameprofile"].as<String>();
        CYCLE = doc["cycle"];
        SUNRISE = doc["sunrise"];
        SUNSET = doc["sunset"];
        // Фаза 1
        PHASE1_DURATION = doc["phase1_duration"];    
        PHASE1_TEMP =  static_cast<uint16_t>(doc["phase1_temp"].as<float>() * 10);     
        PHASE1_HUMIDITY =  static_cast<uint16_t>(doc["phase1_hum"].as<float>() * 10);     
        PHASE1_WATER_TEMP = static_cast<uint16_t>(doc["phase1_water_temp"].as<float>() * 10);   
        PHASE1_LIGHT =  doc["phase1_light"];            
        PHASE1_CIRCULATION =  doc["phase1_circ"];  
        PHASE1_VENTILATION = doc["phase1_vent"];  
        PHASE1_WATERING = doc["phase1_watering"];     
        PHASE1_DRAINING = doc["phase1_draining"];     
        PHASE1_ROTATION = doc["phase1_rot"];  
        // Фаза 2
        PHASE2_DURATION = doc["phase2_duration"];
        PHASE2_TEMP =  static_cast<uint16_t>(doc["phase1_temp"].as<float>() * 10);     
        PHASE2_HUMIDITY =  static_cast<uint16_t>(doc["phase1_hum"].as<float>() * 10);     
        PHASE2_WATER_TEMP = static_cast<uint16_t>(doc["phase1_water_temp"].as<float>() * 10); 
        PHASE2_LIGHT = doc["phase2_light"];
        PHASE2_CIRCULATION = doc["phase2_circ"];
        PHASE2_VENTILATION = doc["phase2_vent"];
        PHASE2_WATERING = doc["phase2_watering"];
        PHASE2_DRAINING = doc["phase2_draining"];
        PHASE2_ROTATION = doc["phase2_rot"];
        // Фаза 3
        PHASE3_DURATION = doc["phase3_duration"];
        PHASE3_TEMP =  static_cast<uint16_t>(doc["phase1_temp"].as<float>() * 10);     
        PHASE3_HUMIDITY =  static_cast<uint16_t>(doc["phase1_hum"].as<float>() * 10);     
        PHASE3_WATER_TEMP = static_cast<uint16_t>(doc["phase1_water_temp"].as<float>() * 10); 
        PHASE3_LIGHT = doc["phase3_light"];
        PHASE3_CIRCULATION = doc["phase3_circ"];
        PHASE3_VENTILATION = doc["phase3_vent"];
        PHASE3_WATERING = doc["phase3_watering"];
        PHASE3_DRAINING = doc["phase3_draining"];
        PHASE3_ROTATION = doc["phase3_rot"];
        // Фаза 4
        PHASE4_DURATION = doc["phase4_duration"];
        PHASE4_TEMP =  static_cast<uint16_t>(doc["phase1_temp"].as<float>() * 10);     
        PHASE4_HUMIDITY =  static_cast<uint16_t>(doc["phase1_hum"].as<float>() * 10);     
        PHASE4_WATER_TEMP = static_cast<uint16_t>(doc["phase1_water_temp"].as<float>() * 10); 
        PHASE4_LIGHT = doc["phase4_light"];
        PHASE4_CIRCULATION = doc["phase4_circ"];
        PHASE4_VENTILATION = doc["phase4_vent"];
        PHASE4_WATERING = doc["phase4_watering"];
        PHASE4_DRAINING = doc["phase4_draining"];
        PHASE4_ROTATION = doc["phase4_rot"];
        // Фаза 5
        PHASE5_DURATION = doc["phase5_duration"];
        PHASE5_TEMP =  static_cast<uint16_t>(doc["phase1_temp"].as<float>() * 10);     
        PHASE5_HUMIDITY =  static_cast<uint16_t>(doc["phase1_hum"].as<float>() * 10);     
        PHASE5_WATER_TEMP = static_cast<uint16_t>(doc["phase1_water_temp"].as<float>() * 10); 
        PHASE5_LIGHT = doc["phase5_light"];
        PHASE5_CIRCULATION = doc["phase5_circ"];
        PHASE5_VENTILATION = doc["phase5_vent"];
        PHASE5_WATERING = doc["phase5_watering"];
        PHASE5_DRAINING = doc["phase5_draining"];
        PHASE5_ROTATION = doc["phase5_rot"];
        // Фаза 6
        PHASE6_DURATION = doc["phase6_duration"];
        PHASE6_TEMP =  static_cast<uint16_t>(doc["phase1_temp"].as<float>() * 10);     
        PHASE6_HUMIDITY =  static_cast<uint16_t>(doc["phase1_hum"].as<float>() * 10);     
        PHASE6_WATER_TEMP = static_cast<uint16_t>(doc["phase1_water_temp"].as<float>() * 10); 
        PHASE6_LIGHT = doc["phase6_light"];
        PHASE6_CIRCULATION = doc["phase6_circ"];
        PHASE6_VENTILATION = doc["phase6_vent"];
        PHASE6_WATERING = doc["phase6_watering"];
        PHASE6_DRAINING = doc["phase6_draining"];
        PHASE6_ROTATION = doc["phase6_rot"];  

        Serial.println("Все настройки сохранены в глобальные переменные.");
        // Сохраняем настройки в EEPROM
        saveSettingsToEEPROM();
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
    uint16_t value = readUint16FromEEPROM(STATUS_BOX);

    // Проверка значения
    if (value == VALUE_READY || value == VALUE_END || value == VALUE_WORK || value == VALUE_ABORT) {  // Проверка статусов
        EEPROMRead();  // Чтение настроек из EEPROM    
        Serial.println("Настройки из EEPROM загружены");

        // Определение статуса фермы
        if (value == VALUE_READY) {
            Serial.println("Статус фермы: READY");
        } else if (value == VALUE_WORK) {
            Serial.println("Статус фермы: WORK");
        } else if (value == VALUE_END) {
            Serial.println("Статус фермы: END");
        } else if (value == VALUE_ABORT) {
            Serial.println("Статус фермы: ABORT");
        }
    } else {
        Serial.println("EEPROM не содержит корректных настроек.");
    }
}

// Сохраняет строку в EEPROM
void saveStringToEEPROM(int address, const String& value) {
    int len = value.length();
    for (int i = 0; i < len; i++) {
        EEPROM.write(address + i, value[i]);
    }
    EEPROM.write(address + len, '\0'); // Добавление терминального нуля для завершения строки   
}
    // Сохраняет настройки в EEPROM
void saveSettingsToEEPROM() {
    int address = EEPROM_START_ADDRESS;

    saveUint16ToEEPROM(address, CYCLE); address += 2;
    saveUint16ToEEPROM(address, SUNRISE); address += 2;
    saveUint16ToEEPROM(address, SUNSET);  address += 2;
    // Сохранение параметров фазы 1
    saveUint16ToEEPROM(address, PHASE1_DURATION); address += 2;
    saveUint16ToEEPROM(address, PHASE1_TEMP); address += 2;
    saveUint16ToEEPROM(address, PHASE1_HUMIDITY); address += 2;
    saveUint16ToEEPROM(address, PHASE1_WATER_TEMP); address += 2;
    saveUint16ToEEPROM(address, PHASE1_LIGHT); address += 2;
    saveUint16ToEEPROM(address, PHASE1_CIRCULATION); address += 2;
    saveUint16ToEEPROM(address, PHASE1_VENTILATION); address += 2;
    saveUint16ToEEPROM(address, PHASE1_WATERING); address += 2;
    saveUint16ToEEPROM(address, PHASE1_DRAINING); address += 2;
    saveUint16ToEEPROM(address, PHASE1_ROTATION); address += 2;
    // Сохранение параметров фазы 2
    saveUint16ToEEPROM(address, PHASE2_DURATION); address += 2;
    saveUint16ToEEPROM(address, PHASE2_TEMP); address += 2;
    saveUint16ToEEPROM(address, PHASE2_HUMIDITY); address += 2;
    saveUint16ToEEPROM(address, PHASE2_WATER_TEMP); address += 2;
    saveUint16ToEEPROM(address, PHASE2_LIGHT); address += 2;
    saveUint16ToEEPROM(address, PHASE2_CIRCULATION); address += 2;
    saveUint16ToEEPROM(address, PHASE2_VENTILATION); address += 2;
    saveUint16ToEEPROM(address, PHASE2_WATERING); address += 2;
    saveUint16ToEEPROM(address, PHASE2_DRAINING); address += 2;
    saveUint16ToEEPROM(address, PHASE2_ROTATION); address += 2;
    // Сохранение параметров фазы 3
    saveUint16ToEEPROM(address, PHASE3_DURATION); address += 2;
    saveUint16ToEEPROM(address, PHASE3_TEMP); address += 2;
    saveUint16ToEEPROM(address, PHASE3_HUMIDITY); address += 2;
    saveUint16ToEEPROM(address, PHASE3_WATER_TEMP); address += 2;
    saveUint16ToEEPROM(address, PHASE3_LIGHT); address += 2;
    saveUint16ToEEPROM(address, PHASE3_CIRCULATION); address += 2;
    saveUint16ToEEPROM(address, PHASE3_VENTILATION); address += 2;
    saveUint16ToEEPROM(address, PHASE3_WATERING); address += 2;
    saveUint16ToEEPROM(address, PHASE3_DRAINING); address += 2;
    saveUint16ToEEPROM(address, PHASE3_ROTATION); address += 2;
    // Сохранение параметров фазы 4
    saveUint16ToEEPROM(address, PHASE4_DURATION); address += 2;
    saveUint16ToEEPROM(address, PHASE4_TEMP); address += 2;
    saveUint16ToEEPROM(address, PHASE4_HUMIDITY); address += 2;
    saveUint16ToEEPROM(address, PHASE4_WATER_TEMP); address += 2;
    saveUint16ToEEPROM(address, PHASE4_LIGHT); address += 2;
    saveUint16ToEEPROM(address, PHASE4_CIRCULATION); address += 2;
    saveUint16ToEEPROM(address, PHASE4_VENTILATION); address += 2;
    saveUint16ToEEPROM(address, PHASE4_WATERING); address += 2;
    saveUint16ToEEPROM(address, PHASE4_DRAINING); address += 2;
    saveUint16ToEEPROM(address, PHASE4_ROTATION); address += 2;
    // Сохранение параметров фазы 5
    saveUint16ToEEPROM(address, PHASE5_DURATION); address += 2;
    saveUint16ToEEPROM(address, PHASE5_TEMP); address += 2;
    saveUint16ToEEPROM(address, PHASE5_HUMIDITY); address += 2;
    saveUint16ToEEPROM(address, PHASE5_WATER_TEMP); address += 2;
    saveUint16ToEEPROM(address, PHASE5_LIGHT); address += 2;
    saveUint16ToEEPROM(address, PHASE5_CIRCULATION); address += 2;
    saveUint16ToEEPROM(address, PHASE5_VENTILATION); address += 2;
    saveUint16ToEEPROM(address, PHASE5_WATERING); address += 2;
    saveUint16ToEEPROM(address, PHASE5_DRAINING); address += 2;
    saveUint16ToEEPROM(address, PHASE5_ROTATION); address += 2;
    // Сохранение параметров фазы 6
    saveUint16ToEEPROM(address, PHASE6_DURATION); address += 2;
    saveUint16ToEEPROM(address, PHASE6_TEMP); address += 2;
    saveUint16ToEEPROM(address, PHASE6_HUMIDITY); address += 2;
    saveUint16ToEEPROM(address, PHASE6_WATER_TEMP); address += 2;
    saveUint16ToEEPROM(address, PHASE6_LIGHT); address += 2;
    saveUint16ToEEPROM(address, PHASE6_CIRCULATION); address += 2;
    saveUint16ToEEPROM(address, PHASE6_VENTILATION); address += 2;
    saveUint16ToEEPROM(address, PHASE6_WATERING); address += 2;
    saveUint16ToEEPROM(address, PHASE6_DRAINING); address += 2;
    saveUint16ToEEPROM(address, PHASE6_ROTATION); address += 2;
   
    saveStringToEEPROM(EEPROM_CULTURE_ADDRESS, CULTURE);

    // saveUint16ToEEPROM(EEPROM_STATUS_BOX_ADDRESS, '57'); address += 1;
    // saveUint16ToEEPROM(EEPROM_STATUS_BOX_ADDRESS, '4F');    

    EEPROM.commit(); // Запись изменений в EEPROM
    Serial.println("Настройки сохранены в EEPROM.");
}

// Считывает строку из EEPROM
String readStringFromEEPROM(int address, int maxLength) {
    char data[maxLength + 1];
    int len = 0;
    unsigned char k;
    k = EEPROM.read(address);
    while (k != '\0' && len < maxLength) {
        k = EEPROM.read(address + len);
        data[len] = k;
        len++;
    }
    data[len] = '\0';
    return String(data);
}

    void EEPROMRead() {
            // Присвоение переменным считанных значений
        int address = EEPROM_START_ADDRESS;
        CYCLE = readUint16FromEEPROM(address); address += 2;
        SUNRISE = readUint16FromEEPROM(address); address += 2;
        SUNSET = readUint16FromEEPROM(address); address += 2;

        // Считывание параметров фазы 1
        PHASE1_DURATION = readUint16FromEEPROM(address); address += 2;
        PHASE1_TEMP = readUint16FromEEPROM(address); address += 2;
        PHASE1_HUMIDITY = readUint16FromEEPROM(address); address += 2;
        PHASE1_WATER_TEMP = readUint16FromEEPROM(address); address += 2;
        PHASE1_LIGHT = readUint16FromEEPROM(address); address += 2;
        PHASE1_CIRCULATION = readUint16FromEEPROM(address); address += 2;
        PHASE1_VENTILATION = readUint16FromEEPROM(address); address += 2;
        PHASE1_WATERING = readUint16FromEEPROM(address); address += 2;
        PHASE1_DRAINING = readUint16FromEEPROM(address); address += 2;
        PHASE1_ROTATION = readUint16FromEEPROM(address); address += 2;
        // Считывание параметров фазы 2
        PHASE2_DURATION = readUint16FromEEPROM(address); address += 2;
        PHASE2_TEMP = readUint16FromEEPROM(address); address += 2;
        PHASE2_HUMIDITY = readUint16FromEEPROM(address); address += 2;
        PHASE2_WATER_TEMP = readUint16FromEEPROM(address); address += 2;
        PHASE2_LIGHT = readUint16FromEEPROM(address); address += 2;
        PHASE2_CIRCULATION = readUint16FromEEPROM(address); address += 2;
        PHASE2_VENTILATION = readUint16FromEEPROM(address); address += 2;
        PHASE2_WATERING = readUint16FromEEPROM(address); address += 2;
        PHASE2_DRAINING = readUint16FromEEPROM(address); address += 2;
        PHASE2_ROTATION = readUint16FromEEPROM(address); address += 2;
        // Считывание параметров фазы 3
        PHASE3_DURATION = readUint16FromEEPROM(address); address += 2;
        PHASE3_TEMP = readUint16FromEEPROM(address); address += 2;
        PHASE3_HUMIDITY = readUint16FromEEPROM(address); address += 2;
        PHASE3_WATER_TEMP = readUint16FromEEPROM(address); address += 2;
        PHASE3_LIGHT = readUint16FromEEPROM(address); address += 2;
        PHASE3_CIRCULATION = readUint16FromEEPROM(address); address += 2;
        PHASE3_VENTILATION = readUint16FromEEPROM(address); address += 2;
        PHASE3_WATERING = readUint16FromEEPROM(address); address += 2;
        PHASE3_DRAINING = readUint16FromEEPROM(address); address += 2;
        PHASE3_ROTATION = readUint16FromEEPROM(address); address += 2;
        // Считывание параметров фазы 4
        PHASE4_DURATION = readUint16FromEEPROM(address); address += 2;
        PHASE4_TEMP = readUint16FromEEPROM(address); address += 2;
        PHASE4_HUMIDITY = readUint16FromEEPROM(address); address += 2;
        PHASE4_WATER_TEMP = readUint16FromEEPROM(address); address += 2;
        PHASE4_LIGHT = readUint16FromEEPROM(address); address += 2;
        PHASE4_CIRCULATION = readUint16FromEEPROM(address); address += 2;
        PHASE4_VENTILATION = readUint16FromEEPROM(address); address += 2;
        PHASE4_WATERING = readUint16FromEEPROM(address); address += 2;
        PHASE4_DRAINING = readUint16FromEEPROM(address); address += 2;
        PHASE4_ROTATION = readUint16FromEEPROM(address); address += 2;
        // Считывание параметров фазы 5
        PHASE5_DURATION = readUint16FromEEPROM(address); address += 2;
        PHASE5_TEMP = readUint16FromEEPROM(address); address += 2;
        PHASE5_HUMIDITY = readUint16FromEEPROM(address); address += 2;
        PHASE5_WATER_TEMP = readUint16FromEEPROM(address); address += 2;
        PHASE5_LIGHT = readUint16FromEEPROM(address); address += 2;
        PHASE5_CIRCULATION = readUint16FromEEPROM(address); address += 2;
        PHASE5_VENTILATION = readUint16FromEEPROM(address); address += 2;
        PHASE5_WATERING = readUint16FromEEPROM(address); address += 2;
        PHASE5_DRAINING = readUint16FromEEPROM(address); address += 2;
        PHASE5_ROTATION = readUint16FromEEPROM(address); address += 2;
        // Считывание параметров фазы 6
        PHASE6_DURATION = readUint16FromEEPROM(address); address += 2;
        PHASE6_TEMP = readUint16FromEEPROM(address); address += 2;
        PHASE6_HUMIDITY = readUint16FromEEPROM(address); address += 2;
        PHASE6_WATER_TEMP = readUint16FromEEPROM(address); address += 2;
        PHASE6_LIGHT = readUint16FromEEPROM(address); address += 2;
        PHASE6_CIRCULATION = readUint16FromEEPROM(address); address += 2;
        PHASE6_VENTILATION = readUint16FromEEPROM(address); address += 2;
        PHASE6_WATERING = readUint16FromEEPROM(address); address += 2;
        PHASE6_DRAINING = readUint16FromEEPROM(address); address += 2;
        PHASE6_ROTATION = readUint16FromEEPROM(address); address += 2;

        // Назавание профиля старт стоп и статус
        GROWE_START_TIME = readUint16FromEEPROM(EEPROM_GROWE_START_TIME_ADDRESS);
        GROWE_STOP_DATE = readUint16FromEEPROM(EEPROM_GROWE_STOP_DATE_ADDRESS);
        STATUS_BOX = readUint16FromEEPROM(EEPROM_STATUS_BOX_ADDRESS); 
        PHASE = readUint16FromEEPROM(EEPROM_PHASE_ADDRESS);
        CULTURE = readStringFromEEPROM(EEPROM_CULTURE_ADDRESS, 20);
        Serial.println(CULTURE);
        Serial.println(STATUS_BOX);
}

// Сереализация настроек считанных из EEPROM и отправка на сервер
void serializeSettings() {
    EEPROMRead();
    DynamicJsonDocument doc(2048);

    doc["nameprofile"] = CULTURE;
    doc["sunrise"] = SUNRISE;
    doc["sunset"] = SUNSET;
    doc["cycle"] = CYCLE;
    doc["work"] = STATUS_BOX;
    doc["groweStart"] = WiFi.localIP();   // GROWE_START;
    doc["groweStartTime"] = GROWE_START_TIME;
    doc["groweStoptDate"] = GROWE_STOP_DATE;
    // Фаза 1
    doc["phase1_duration"] = PHASE1_DURATION;
    doc["phase1_temp"] = PHASE1_TEMP / 10.0;
    doc["phase1_hum"] = PHASE1_HUMIDITY / 10.0;
    doc["phase1_water_temp"] = PHASE1_WATER_TEMP / 10.0;
    doc["phase1_light"] = PHASE1_LIGHT;
    doc["phase1_circ"] = PHASE1_CIRCULATION;
    doc["phase1_vent"] = PHASE1_VENTILATION;
    doc["phase1_watering"] = PHASE1_WATERING;
    doc["phase1_draining"] = PHASE1_DRAINING;
    doc["phase1_rot"] = PHASE1_ROTATION;
    // Фаза 2
    doc["phase2_duration"] = PHASE2_DURATION;
    doc["phase2_temp"] = PHASE2_TEMP / 10.0;
    doc["phase2_hum"] = PHASE2_HUMIDITY / 10.0;
    doc["phase2_water_temp"] = PHASE2_WATER_TEMP / 10.0;
    doc["phase2_light"] = PHASE2_LIGHT;
    doc["phase2_circ"] = PHASE2_CIRCULATION;
    doc["phase2_vent"] = PHASE2_VENTILATION;
    doc["phase2_watering"] = PHASE2_WATERING;
    doc["phase2_draining"] = PHASE2_DRAINING;
    doc["phase2_rot"] = PHASE2_ROTATION;
    // Фаза 3
    doc["phase3_duration"] = PHASE3_DURATION;
    doc["phase3_temp"] = PHASE3_TEMP / 10.0;
    doc["phase3_hum"] = PHASE3_HUMIDITY / 10.0;
    doc["phase3_water_temp"] = PHASE3_WATER_TEMP / 10.0;
    doc["phase3_light"] = PHASE3_LIGHT;
    doc["phase3_circ"] = PHASE3_CIRCULATION;
    doc["phase3_vent"] = PHASE3_VENTILATION;
    doc["phase3_watering"] = PHASE3_WATERING;
    doc["phase3_draining"] = PHASE3_DRAINING;
    doc["phase3_rot"] = PHASE3_ROTATION;
    // Фаза 4
    doc["phase4_duration"] = PHASE4_DURATION;
    doc["phase4_temp"] = PHASE4_TEMP / 10.0;
    doc["phase4_hum"] = PHASE4_HUMIDITY / 10.0;
    doc["phase4_water_temp"] = PHASE4_WATER_TEMP / 10.0;
    doc["phase4_light"] = PHASE4_LIGHT;
    doc["phase4_circ"] = PHASE4_CIRCULATION;
    doc["phase4_vent"] = PHASE4_VENTILATION;
    doc["phase4_watering"] = PHASE4_WATERING;
    doc["phase4_draining"] = PHASE4_DRAINING;
    doc["phase4_rot"] = PHASE4_ROTATION;
    // Фаза 5
    doc["phase5_duration"] = PHASE5_DURATION;
    doc["phase5_temp"] = PHASE5_TEMP / 10.0;
    doc["phase5_hum"] = PHASE5_HUMIDITY / 10.0;
    doc["phase5_water_temp"] = PHASE5_WATER_TEMP / 10.0;
    doc["phase5_light"] = PHASE5_LIGHT;
    doc["phase5_circ"] = PHASE5_CIRCULATION;
    doc["phase5_vent"] = PHASE5_VENTILATION;
    doc["phase5_watering"] = PHASE5_WATERING;
    doc["phase5_draining"] = PHASE5_DRAINING;
    doc["phase5_rot"] = PHASE5_ROTATION;
    // Фаза 6
    doc["phase6_duration"] = PHASE6_DURATION;
    doc["phase6_temp"] = PHASE6_TEMP / 10.0;
    doc["phase6_hum"] = PHASE6_HUMIDITY / 10.0;
    doc["phase6_water_temp"] = PHASE6_WATER_TEMP / 10.0;
    doc["phase6_light"] = PHASE6_LIGHT;
    doc["phase6_circ"] = PHASE6_CIRCULATION;
    doc["phase6_vent"] = PHASE6_VENTILATION;
    doc["phase6_watering"] = PHASE6_WATERING;
    doc["phase6_draining"] = PHASE6_DRAINING;
    doc["phase6_rot"] = PHASE6_ROTATION;

    // Сериализация в строку
    String jsonSettings;
    serializeJson(doc, jsonSettings);
    //Serial.println(jsonSettings);

    // Добавление ID фермы и типа сообщения и длинны перед JSON, разделенные пробелом
    TYPE_MSG = FARM_RES_SETTINGS; // Тип сообщения "FRQS" - данные от фермы на сервер данные
    ID_FARM = 255;  // ID фермы
    LENGTH_MSG = jsonSettings.length(); // Длина JSON сообщения
    messageToSend = String(ID_FARM) + " " + TYPE_MSG + " " + String(LENGTH_MSG) + " " + jsonSettings;
    // Отправка сообщения
    Serial.print("Настройки: ");
    transmitionTime = millis();  // Запоминаем время отправки
    sendWebSocketMessage(messageToSend);
}
