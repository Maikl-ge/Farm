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
String readStringFromEEPROM(int address, int maxLength);
void updateCurrentTimeInMinutes();
void saveUint16ToEEPROM();
void saveUint16ToEEPROM(int address, uint16_t value);
// Глобальные переменные профиля

extern uint8_t ID_FARM;  // ID фермы
extern String TYPE_MSG; // Тип сообщения 
extern uint16_t LENGTH_MSG; // Длина сообщения

// Параметры времени
extern uint16_t SUNRISE;  // Время восхода
extern uint16_t SUNSET;   // Время заката

// Цикл работы
extern uint16_t CYCLE;
extern String statusFarm; // Статус фермы    
extern uint16_t currentTimeInMinutes; // Текущее время в минутах                     
extern uint16_t GROWE_MODE_TIME;  // Время начала цикла роста
extern uint16_t GROWE_MODE_DATE;  // Дата начала цикла роста 

extern int EEPROM_PHASE_ADDRESS;
extern int EEPROM_CULTURE_ADDRESS;
extern int EEPROM_CYCLE_ADDRESS;
extern int EEPROM_STATUS_BOX_ADDRESS;
extern int EEPROM_GROWE_MODE_TIME_ADDRESS;
extern int EEPROM_GROWE_MODE_DATE_ADDRESS;

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
extern String  STATUS_BOX;   // Текущее состояние фермы       
extern String CULTURE;       // Текущая культура
extern uint16_t CYCLE;       // Адрес 0x22-0x23
extern uint16_t SUNRISE;     // Время восхода
extern uint16_t SUNSET;      // Время заката
extern uint16_t GROWE_MODE_TIME; // Время начала цикла роста
extern uint16_t GROWE_MODE_DATE ;  // Дата начала цикла роста      

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
