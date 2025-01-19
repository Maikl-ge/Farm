#ifndef CURRENT_CONSTANTS_H
#define CURRENT_CONSTANTS_H

#include <Arduino.h>

// Глобальные переменные профиля

extern uint8_t ID_FARM;  // ID фермы
extern String TYPE_MSG; // Тип сообщения
extern uint16_t LENGTH_MSG; // Длина сообщения

// Дневные параметры
extern uint16_t DAY_CIRCULATION;
extern uint16_t DAY_HUMIDITY_START;
extern uint16_t DAY_HUMIDITY_END;
extern uint16_t DAY_TEMPERATURE_START;
extern uint16_t DAY_TEMPERATURE_END;
extern uint16_t DAY_VENTILATION;
extern uint16_t DAY_WATERING_INTERVAL;

// Ночные параметры
extern uint16_t NIGHT_CIRCULATION;
extern uint16_t NIGHT_HUMIDITY_START;
extern uint16_t NIGHT_HUMIDITY_END;
extern uint16_t NIGHT_TEMPERATURE_START;
extern uint16_t NIGHT_TEMPERATURE_END;
extern uint16_t NIGHT_VENTILATION;
extern uint16_t NIGHT_WATERING_INTERVAL;

// Параметры времени
extern uint16_t SUNRISE;
extern uint16_t SUNSET;

// Температура воды
extern uint16_t WATER_TEMPERATURE;

// Цикл работы
extern uint16_t CYCLE;
extern uint16_t GROWE_START;   //  Адрес 0x26-0x27
extern uint16_t WORK;  // 0x52 - R Адрес 0x24-0x25
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

extern uint16_t GROWE_START_TIME;  // Время начала цикла роста
extern uint16_t GROWE_START_DATE;  // Дата начала цикла роста 

extern int EEPROM_DAY_CIRCULATION_ADDRESS; // Адрес начала EEPROM
extern int EEPROM_DAY_HUMIDITY_START_ADDRESS; 
extern int EEPROM_DAY_HUMIDITY_END_ADDRESS;
extern int EEPROM_DAY_TEMPERATURE_START_ADDRESS;
extern int EEPROM_DAY_TEMPERATURE_END_ADDRESS;
extern int EEPROM_DAY_VENTILATION_ADDRESS;
extern int EEPROM_DAY_WATERING_INTERVAL_ADDRESS;
extern int EEPROM_NIGHT_CIRCULATION_ADDRESS;
extern int EEPROM_NIGHT_HUMIDITY_START_ADDRESS;
extern int EEPROM_NIGHT_HUMIDITY_END_ADDRESS;
extern int EEPROM_NIGHT_TEMPERATURE_START_ADDRESS;
extern int EEPROM_NIGHT_TEMPERATURE_END_ADDRESS;
extern int EEPROM_NIGHT_VENTILATION_ADDRESS;
extern int EEPROM_NIGHT_WATERING_INTERVAL_ADDRESS;
extern int EEPROM_SUNRISE_ADDRESS;
extern int EEPROM_SUNSET_ADDRESS;
extern int EEPROM_WATER_TEMPERATURE_ADDRESS;
extern int EEPROM_CYCLE_ADDRESS;
extern int EEPROM_WORK_ADDRESS;
extern int EEPROM_GROWE_START_ADDRESS;
extern int EEPROM_GROWE_START_TIME_ADDRESS;
extern int EEPROM_GROWE_START_DATE_ADDRESS;


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

#endif // CURRENT_CONSTANTS_H
