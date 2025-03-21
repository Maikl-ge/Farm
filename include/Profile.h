#ifndef PROFILE_H
#define PROFILE_H

// Инициализация модуля настроек
void initializeSettingsModule();
void saveUint16ToEEPROM();
void EEPROMRead();
void saveSettingsToEEPROM();
uint16_t readUint16FromEEPROM(int address);
void serializeSettings();
void fetchAndSaveSettings();
void printEEPROMValues(int startAddress, int endAddress);
void saveStringToEEPROM(int address, const String& value);
String readStringFromEEPROM(int address, int maxLength);
void updateCurrentTimeInMinutes();

// Глобальные переменные профиля

extern uint8_t ID_FARM;  // ID фермы
extern String TYPE_MSG; // Тип сообщения
extern uint16_t LENGTH_MSG; // Длина сообщения

// Параметры времени
extern uint16_t SUNRISE;  // Время восхода
extern uint16_t SUNSET;   // Время заката

// Цикл работы
extern uint16_t CYCLE;
extern uint16_t STATUS_BOX; // 0x52 - R Адрес 0x24-0x25
                            // 0x45 - E  Ready
                            //--------------------------------
                            // 0x57 - W  Work
                            // 0x4F - O
                            //--------------------------------
                            // 0x45 - E  End
                            // 0x4E - N
                            //--------------------------------
                            // 0x41 - A  Abort
                            // 0x42 - B
extern String statusFarm; // Статус фермы    
extern uint16_t currentTimeInMinutes; // Текущее время в минутах                     
extern uint16_t GROWE_START_TIME;  // Время начала цикла роста
extern uint16_t GROWE_STOP_DATE;  // Дата начала цикла роста 

extern int EEPROM_PHASE_ADDRESS;
extern int EEPROM_CULTURE_ADDRESS;
extern int EEPROM_CYCLE_ADDRESS;
extern int EEPROM_STATUS_BOX_ADDRESS;
extern int EEPROM_GROWE_START_TIME_ADDRESS;
extern int EEPROM_GROWE_STOP_DATE_ADDRESS;

//Типы сообщений от фермы серверу
// Типы запросов
extern String FARM_RES_STATUS;       // Запрос статуса фермы
extern String FARM_RES_DATA;         // Запрос данных фермы
extern String FARM_RES_SETTINGS;     // Запрос настроек фермы
extern String FARM_RES_PARAMETERS;   // Запрос параметров фермы
extern String FARM_RES_PROFILE;      // Запрос профиля фермы
extern String FARM_RES_CURRENT;      // Запрос текущих данных фермы

// Типы данных
extern String FARM_DATA_STATUS;       // Статус фермы
extern String FARM_DATA_TELEMETRY;    // Телеметрические данные фермы
extern String FARM_DATA_ERRORS;       // Ошибки фермы
extern String FARM_DATA_EVENT;        // Событие фермы (например, включение устройства)

// Логи и отладка
extern String FARM_LOG_INFO;          // Информационное сообщение
extern String FARM_LOG_DEBUG;         // Отладочное сообщение
extern String FARM_LOG_ERROR;         // Ошибка (например, аппаратная или программная)

//Типы сообщений от сервера ферме
// Команды управления
extern String SERVER_CMD_START;       // Запуск работы
extern String SERVER_CMD_STOP;        // Остановка работы
extern String SERVER_CMD_RESTART;     // Перезапуск
extern String SERVER_CMD_UPDATE;      // Обновление прошивки
extern String SERVER_CMD_SETTINGS;    // Команда на обновление настроек

// Ответы на запросы
extern String SERVER_REQ_STATUS;      // Запрос статуса
extern String SERVER_REQ_DATA;        // Запрос данных
extern String SERVER_REQ_SETTINGS;    // Запрос настроек
extern String SERVER_REQ_PARAMETERS;  // Запрос параметров
extern String SERVER_REQ_PROFILE;     // Запрос профиля
extern String SERVER_REQ_CURRENT;     // Запрос текущих данных

// Ошибки и события
extern String SERVER_ERR_INVALID;       // Ошибка: недействительный запрос
extern String SERVER_EVENT_SYNC;        // Событие синхронизации

//Типы сообщений от фермы серверу
// Типы запросов
extern String FARM_RES_STATUS;       // Запрос статуса фермы
extern String FARM_RES_DATA;         // Запрос данных фермы
extern String FARM_RES_SETTINGS;     // Запрос настроек фермы
extern String FARM_RES_PARAMETERS;   // Запрос параметров фермы
extern String FARM_RES_PROFILE;      // Запрос профиля фермы
extern String FARM_RES_CURRENT;      // Запрос текущих данных фермы

// Типы данных
extern String FARM_DATA_STATUS;       // Статус фермы
extern String FARM_DATA_TELEMETRY;    // Телеметрические данные фермы
extern String FARM_DATA_ERRORS;       // Ошибки фермы
extern String FARM_DATA_EVENT;        // Событие фермы (например, включение устройства)

// Логи и отладка
extern String FARM_LOG_INFO;          // Информационное сообщение
extern String FARM_LOG_DEBUG;         // Отладочное сообщение
extern String FARM_LOG_ERROR;         // Ошибка (например, аппаратная или программная)

//Типы сообщений от сервера ферме
// Команды управления
extern String SERVER_CMD_START;       // Запуск работы
extern String SERVER_CMD_STOP;        // Остановка работы
extern String SERVER_CMD_RESTART;     // Перезапуск
extern String SERVER_CMD_UPDATE;      // Обновление прошивки
extern String SERVER_CMD_SETTINGS;    // Команда на обновление настроек

// Ответы на запросы
extern String SERVER_REQ_STATUS;      // Запрос статуса
extern String SERVER_REQ_DATA;        // Запрос данных
extern String SERVER_REQ_SETTINGS;    // Запрос настроек
extern String SERVER_REQ_PARAMETERS;  // Запрос параметров
extern String SERVER_REQ_PROFILE;     // Запрос профиля
extern String SERVER_REQ_CURRENT;     // Запрос текущих данных

// Ошибки и события
extern String SERVER_ERR_INVALID;       // Ошибка: недействительный запрос
extern String SERVER_EVENT_SYNC;        // Событие синхронизации

// Переменные
extern uint16_t PHASE;       // Текущая фаза    
extern uint16_t STATUS_BOX;  // Текущее состояние фермы       
extern String CULTURE;     // Текущая культура
extern uint16_t CYCLE;       // Адрес 0x22-0x23
extern uint16_t SUNRISE;     // Время восхода
extern uint16_t SUNSET;      // Время заката
extern uint16_t GROWE_START_TIME; // Время начала цикла роста
extern uint16_t GROWE_STOP_DATE ;  // Дата начала цикла роста      

// Переменные 6 фаз
extern uint16_t PHASE1_DURATION;     
extern uint16_t PHASE1_TEMP;         
extern uint16_t PHASE1_HUMIDITY;     
extern uint16_t PHASE1_WATER_TEMP;   
extern uint16_t PHASE1_LIGHT;        
extern uint16_t PHASE1_CIRCULATION;  
extern uint16_t PHASE1_VENTILATION;  
extern uint16_t PHASE1_WATERING;     
extern uint16_t PHASE1_DRAINING;     
extern uint16_t PHASE1_ROTATION;     

extern uint16_t PHASE2_DURATION;
extern uint16_t PHASE2_TEMP;
extern uint16_t PHASE2_HUMIDITY;
extern uint16_t PHASE2_WATER_TEMP;
extern uint16_t PHASE2_LIGHT;
extern uint16_t PHASE2_CIRCULATION;
extern uint16_t PHASE2_VENTILATION;
extern uint16_t PHASE2_WATERING;
extern uint16_t PHASE2_DRAINING;
extern uint16_t PHASE2_ROTATION;

extern uint16_t PHASE3_DURATION;
extern uint16_t PHASE3_TEMP;
extern uint16_t PHASE3_HUMIDITY;
extern uint16_t PHASE3_WATER_TEMP;
extern uint16_t PHASE3_LIGHT;
extern uint16_t PHASE3_CIRCULATION;
extern uint16_t PHASE3_VENTILATION;
extern uint16_t PHASE3_WATERING;
extern uint16_t PHASE3_DRAINING;
extern uint16_t PHASE3_ROTATION;

extern uint16_t PHASE4_DURATION;
extern uint16_t PHASE4_TEMP;
extern uint16_t PHASE4_HUMIDITY;
extern uint16_t PHASE4_WATER_TEMP;
extern uint16_t PHASE4_LIGHT;
extern uint16_t PHASE4_CIRCULATION;
extern uint16_t PHASE4_VENTILATION;
extern uint16_t PHASE4_WATERING;
extern uint16_t PHASE4_DRAINING;
extern uint16_t PHASE4_ROTATION;

extern uint16_t PHASE5_DURATION;
extern uint16_t PHASE5_TEMP;
extern uint16_t PHASE5_HUMIDITY;
extern uint16_t PHASE5_WATER_TEMP;
extern uint16_t PHASE5_LIGHT;
extern uint16_t PHASE5_CIRCULATION;
extern uint16_t PHASE5_VENTILATION;
extern uint16_t PHASE5_WATERING;
extern uint16_t PHASE5_DRAINING;
extern uint16_t PHASE5_ROTATION;

extern uint16_t PHASE6_DURATION;
extern uint16_t PHASE6_TEMP;
extern uint16_t PHASE6_HUMIDITY;
extern uint16_t PHASE6_WATER_TEMP;
extern uint16_t PHASE6_LIGHT;
extern uint16_t PHASE6_CIRCULATION;
extern uint16_t PHASE6_VENTILATION;
extern uint16_t PHASE6_WATERING;
extern uint16_t PHASE6_DRAINING;
extern uint16_t PHASE6_ROTATION;


#endif //PROFILE_H

// // Инициализация глобальных переменных
// uint8_t ID_FARM = 255;  // ID фермы
// String TYPE_MSG = " "; // Тип сообщения
// uint16_t LENGTH_MSG = 0; // Длина сообщения

// uint16_t DAY_CIRCULATION = 0; // Адрес 0x00-0x01
// uint16_t DAY_HUMIDITY_START = 0; // Адрес 0x02-0x03
// uint16_t DAY_HUMIDITY_END = 0;  // Адрес 0x04-0x05
// uint16_t DAY_TEMPERATURE_START = 0; // Адрес 0x06-0x07
// uint16_t DAY_TEMPERATURE_END = 0; // Адрес 0x08-0x09
// uint16_t DAY_VENTILATION = 0; // Адрес 0x0A-0x0B
// uint16_t DAY_WATERING_INTERVAL = 0; // Адрес 0x0C-0x0D

// uint16_t NIGHT_CIRCULATION = 0; // Адрес 0x0E-0x0F
// uint16_t NIGHT_HUMIDITY_START = 0; // Адрес 0x10-0x11
// uint16_t NIGHT_HUMIDITY_END = 0; // Адрес 0x12-0x13
// uint16_t NIGHT_TEMPERATURE_START = 0; // Адрес 0x14-0x15
// uint16_t NIGHT_TEMPERATURE_END = 0; // Адрес 0x16-0x17
// uint16_t NIGHT_VENTILATION = 0; // Адрес 0x18-0x19
// uint16_t NIGHT_WATERING_INTERVAL = 0; // Адрес 0x1A-0x1B

// uint16_t SUNRISE = 0; // Адрес 0x1C-0x1D
// uint16_t SUNSET = 0; // Адрес 0x1E-0x1F


// uint16_t WATER_TEMPERATURE = 0; // Адрес 0x20-0x21
// uint16_t CYCLE = 0; // Адрес 0x22-0x23
// uint16_t WORK = 0;  // Адрес 0x24-0x25
// uint16_t GROWE_START_TIME = 0;  // Адрес 0x2A-0x2B
// uint16_t GROWE_START_DATE = 0;  // Адрес 0x2C-0x2D




// int EEPROM_DAY_CIRCULATION_ADDRESS = 0x00; // Адрес 0x00-0x01
// int EEPROM_DAY_HUMIDITY_START_ADDRESS = 0x02; // Адрес 0x02-0x03
// int EEPROM_DAY_HUMIDITY_END_ADDRESS = 0x04;  // Адрес 0x04-0x05
// int EEPROM_DAY_TEMPERATURE_START_ADDRESS = 0x06; // Адрес 0x06-0x07
// int EEPROM_DAY_TEMPERATURE_END_ADDRESS = 0x08; // Адрес 0x08-0x09
// int EEPROM_DAY_VENTILATION_ADDRESS = 0x0A; // Адрес 0x0A-0x0B
// int EEPROM_DAY_WATERING_INTERVAL_ADDRESS = 0x0C; // Адрес 0x0C-0x0D
// int EEPROM_NIGHT_CIRCULATION_ADDRESS = 0x0E; // Адрес 0x0E-0x0F
// int EEPROM_NIGHT_HUMIDITY_START_ADDRESS = 0x10; // Адрес 0x10-0x11
// int EEPROM_NIGHT_HUMIDITY_END_ADDRESS = 0x12; // Адрес 0x12-0x13
// int EEPROM_NIGHT_TEMPERATURE_START_ADDRESS = 0x14; // Адрес 0x14-0x15
// int EEPROM_NIGHT_TEMPERATURE_END_ADDRESS = 0x16; // Адрес 0x16-0x17
// int EEPROM_NIGHT_VENTILATION_ADDRESS = 0x18; // Адрес 0x18-0x19
// int EEPROM_NIGHT_WATERING_INTERVAL_ADDRESS = 0x1A; // Адрес 0x1A-0x1B
// int EEPROM_SUNRISE_ADDRESS = 0x1C; // Адрес 0x1C-0x1D
// int EEPROM_SUNSET_ADDRESS = 0x1E; // Адрес 0x1E-0x1F
// int EEPROM_WATER_TEMPERATURE_ADDRESS = 0x20; // Адрес 0x20-0x21
// int EEPROM_CYCLE_ADDRESS = 0x22; // Адрес 0x22-0x23
// int EEPROM_WORK_ADDRESS = 0x24;  // Адрес 0x24-0x25
// int EEPROM_GROWE_START_TIME_ADDRESS = 0x2A;  // Адрес 0x2A-0x2B
// int EEPROM_GROWE_START_DATE_ADDRESS = 0x2C;  // Адрес 0x2C-0x2D


// // Типы сообщений от фермы серверу
// // Типы запросов
// String FARM_RES_STATUS = "FRQW";        // Ответ статуса фермы на запрос сервера
// String FARM_RES_DATA = "FRQD";          // Ответ с данными фермы на запрос сервера
// String FARM_RES_SETTINGS = "FRQS";      // Запрос у сервера настроек и базы данных 
// String FARM_RES_PARAMETERS = "FRQP";    // Ответ параметров фермы на запрос сервера
// String FARM_RES_PROFILE = "FRQF";       // Ответ профиля фермы на запрос сервера
// String FARM_RES_CURRENT = "FRQC";       // Ответ текущих данных фермы

// // Типы данных
// String FARM_DATA_STATUS = "FDST";       // Ответ Статус фермы (например, готова к работе)
// String FARM_DATA_TELEMETRY = "FDTL";    // Телеметрические данные фермы
// String FARM_DATA_ERROR = "FDER";        // Сообщение об ошибке фермы
// String FARM_DATA_EVENT = "FDEV";        // Событие фермы (например, включение устройства)
// String FARM_DATA_SETTINGS = "FDSE";     // Настройки фермы на запрос серверу настроек

// // Логи и отладка
// String FARM_LOG_INFO = "FLIN";          // Информационное сообщение
// String FARM_LOG_DEBUG = "FLDB";         // Отладочное сообщение
// String FARM_LOG_ERROR = "FLER";         // Ошибка (например, аппаратная или программная)


// // Типы сообщений от сервера ферме
// // Команды управления
// String SERVER_CMD_START = "SCMD";       // Команда на запуск
// String SERVER_CMD_STOP = "SCMS";        // Команда на остановку
// String SERVER_CMD_RESTART = "SCMR";     // Команда на перезапуск
// String SERVER_CMD_UPDATE = "SCMU";      // Команда на обновление ПО
// String SERVER_CMD_SETTINGS = "SCME";    // Команда на обновление настроек

// // Ответы на запросы
// String SERVER_REQ_STATUS = "SRST";      // Запрос о статусе фермы
// String SERVER_REQ_DATA = "SRDT";        // Запрос данных фермы
// String SERVER_REQ_SETTINGS = "SRSE";    // Запрос о настройках фермы Профиля настроек сохраненных 
// String SERVER_REQ_PARAMETERS = "SRPM";  // Запрос о параметрах фермы
// String SERVER_REQ_PROFILE = "SRPF";     // Запрос о профиле фермы
// String SERVER_REQ_CURRENT = "SRCU";     // Запрос  текущих данных фермы

// // Ошибки и события
// String SERVER_ERR_INVALID = "SERR";     // Ошибка: недействительный запрос
// String SERVER_EVENT_SYNC = "SEVN";      // Событие синхронизации
