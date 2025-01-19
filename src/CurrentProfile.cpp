#include "CurrentProfile.h"

// Инициализация глобальных переменных
uint8_t ID_FARM = 255;  // ID фермы
String TYPE_MSG = " "; // Тип сообщения
uint16_t LENGTH_MSG = 0; // Длина сообщения

uint16_t DAY_CIRCULATION = 0; // Адрес 0x00-0x01
uint16_t DAY_HUMIDITY_START = 0; // Адрес 0x02-0x03
uint16_t DAY_HUMIDITY_END = 0;  // Адрес 0x04-0x05
uint16_t DAY_TEMPERATURE_START = 0; // Адрес 0x06-0x07
uint16_t DAY_TEMPERATURE_END = 0; // Адрес 0x08-0x09
uint16_t DAY_VENTILATION = 0; // Адрес 0x0A-0x0B
uint16_t DAY_WATERING_INTERVAL = 0; // Адрес 0x0C-0x0D

uint16_t NIGHT_CIRCULATION = 0; // Адрес 0x0E-0x0F
uint16_t NIGHT_HUMIDITY_START = 0; // Адрес 0x10-0x11
uint16_t NIGHT_HUMIDITY_END = 0; // Адрес 0x12-0x13
uint16_t NIGHT_TEMPERATURE_START = 0; // Адрес 0x14-0x15
uint16_t NIGHT_TEMPERATURE_END = 0; // Адрес 0x16-0x17
uint16_t NIGHT_VENTILATION = 0; // Адрес 0x18-0x19
uint16_t NIGHT_WATERING_INTERVAL = 0; // Адрес 0x1A-0x1B

uint16_t SUNRISE = 0; // Адрес 0x1C-0x1D
uint16_t SUNSET = 0; // Адрес 0x1E-0x1F


uint16_t WATER_TEMPERATURE = 0; // Адрес 0x20-0x21
uint16_t CYCLE = 0; // Адрес 0x22-0x23
uint16_t WORK = 0;  // Адрес 0x24-0x25
uint16_t GROWE_START = 0;  // Адрес 0x26-0x29
uint16_t GROWE_START_TIME = 0;  // Адрес 0x2A-0x2B
uint16_t GROWE_START_DATE = 0;  // Адрес 0x2C-0x2D




int EEPROM_DAY_CIRCULATION_ADDRESS = 0x00; // Адрес 0x00-0x01
int EEPROM_DAY_HUMIDITY_START_ADDRESS = 0x02; // Адрес 0x02-0x03
int EEPROM_DAY_HUMIDITY_END_ADDRESS = 0x04;  // Адрес 0x04-0x05
int EEPROM_DAY_TEMPERATURE_START_ADDRESS = 0x06; // Адрес 0x06-0x07
int EEPROM_DAY_TEMPERATURE_END_ADDRESS = 0x08; // Адрес 0x08-0x09
int EEPROM_DAY_VENTILATION_ADDRESS = 0x0A; // Адрес 0x0A-0x0B
int EEPROM_DAY_WATERING_INTERVAL_ADDRESS = 0x0C; // Адрес 0x0C-0x0D
int EEPROM_NIGHT_CIRCULATION_ADDRESS = 0x0E; // Адрес 0x0E-0x0F
int EEPROM_NIGHT_HUMIDITY_START_ADDRESS = 0x10; // Адрес 0x10-0x11
int EEPROM_NIGHT_HUMIDITY_END_ADDRESS = 0x12; // Адрес 0x12-0x13
int EEPROM_NIGHT_TEMPERATURE_START_ADDRESS = 0x14; // Адрес 0x14-0x15
int EEPROM_NIGHT_TEMPERATURE_END_ADDRESS = 0x16; // Адрес 0x16-0x17
int EEPROM_NIGHT_VENTILATION_ADDRESS = 0x18; // Адрес 0x18-0x19
int EEPROM_NIGHT_WATERING_INTERVAL_ADDRESS = 0x1A; // Адрес 0x1A-0x1B
int EEPROM_SUNRISE_ADDRESS = 0x1C; // Адрес 0x1C-0x1D
int EEPROM_SUNSET_ADDRESS = 0x1E; // Адрес 0x1E-0x1F
int EEPROM_WATER_TEMPERATURE_ADDRESS = 0x20; // Адрес 0x20-0x21
int EEPROM_CYCLE_ADDRESS = 0x22; // Адрес 0x22-0x23
int EEPROM_WORK_ADDRESS = 0x24;  // Адрес 0x24-0x25
int EEPROM_GROWE_START_ADDRESS = 0x26;  // Адрес 0x26-0x29
int EEPROM_GROWE_START_TIME_ADDRESS = 0x2A;  // Адрес 0x2A-0x2B
int EEPROM_GROWE_START_DATE_ADDRESS = 0x2C;  // Адрес 0x2C-0x2D


// Типы сообщений от фермы серверу
// Типы запросов
String FARM_RES_STATUS = "FRQW";        // Ответ статуса фермы на запрос сервера
String FARM_RES_DATA = "FRQD";          // Ответ с данными фермы на запрос сервера
String FARM_RES_SETTINGS = "FRQS";      // Запрос у сервера настроек и базы данных 
String FARM_RES_PARAMETERS = "FRQP";    // Ответ параметров фермы на запрос сервера
String FARM_RES_PROFILE = "FRQF";       // Ответ профиля фермы на запрос сервера
String FARM_RES_CURRENT = "FRQC";       // Ответ текущих данных фермы

// Типы данных
String FARM_DATA_STATUS = "FDST";       // Ответ Статус фермы (например, готова к работе)
String FARM_DATA_TELEMETRY = "FDTL";    // Телеметрические данные фермы
String FARM_DATA_ERROR = "FDER";        // Сообщение об ошибке фермы
String FARM_DATA_EVENT = "FDEV";        // Событие фермы (например, включение устройства)
String FARM_DATA_SETTINGS = "FDSE";     // Настройки фермы на запрос серверу настроек

// Логи и отладка
String FARM_LOG_INFO = "FLIN";          // Информационное сообщение
String FARM_LOG_DEBUG = "FLDB";         // Отладочное сообщение
String FARM_LOG_ERROR = "FLER";         // Ошибка (например, аппаратная или программная)


// Типы сообщений от сервера ферме
// Команды управления
String SERVER_CMD_START = "SCMD";       // Команда на запуск
String SERVER_CMD_STOP = "SCMS";        // Команда на остановку
String SERVER_CMD_RESTART = "SCMR";     // Команда на перезапуск
String SERVER_CMD_UPDATE = "SCMU";      // Команда на обновление ПО
String SERVER_CMD_SETTINGS = "SCME";    // Команда на обновление настроек

// Ответы на запросы
String SERVER_REQ_STATUS = "SRST";      // Запрос о статусе фермы
String SERVER_REQ_DATA = "SRDT";        // Запрос данных фермы
String SERVER_REQ_SETTINGS = "SRSE";    // Запрос о настройках фермы Профиля настроек сохраненных 
String SERVER_REQ_PARAMETERS = "SRPM";  // Запрос о параметрах фермы
String SERVER_REQ_PROFILE = "SRPF";     // Запрос о профиле фермы
String SERVER_REQ_CURRENT = "SRCU";     // Запрос  текущих данных фермы

// Ошибки и события
String SERVER_ERR_INVALID = "SERR";     // Ошибка: недействительный запрос
String SERVER_EVENT_SYNC = "SEVN";      // Событие синхронизации



