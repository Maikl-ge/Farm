#include "CurrentProfile.h"

// Инициализация глобальных переменных
uint8_t ID_FARM = 0;  // ID фермы
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
uint16_t GROWE_START_DATA = 0;  // Адрес 0x2C-0x2D

