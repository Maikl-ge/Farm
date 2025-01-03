#ifndef CURRENT_CONSTANTS_H
#define CURRENT_CONSTANTS_H

#include <Arduino.h>

// Глобальные переменные профиля

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
                       // 0x45 - E 
                       //-----------
                       // 0x57 - W
                       // 0x4F - O
                       //-----------
                       // 0x45 - E
                       // 0x4E - N

#endif // CURRENT_CONSTANTS_H
