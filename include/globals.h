#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
// Глобальные переменные
extern String id_farm_ACK;
extern String type_msg_ACK;
extern String ack_ACK;
extern int missedPongs;
extern int8_t timeZone; // Часовой пояс

// Переменные для управления устройствами On/Off
extern bool OSMOS_ON;          // Подача очищенной воды (ON/OFF) (GPIO32, нога 7)
extern bool PUMP_WATERING;            // Полив (ON/OFF) (GPIO33, нога 8)
extern bool PUMP_TRANSFER ;     // Подача в бак полива osmo воды (ON/OFF) (GPIO26, нога 10)
extern bool WATER_OUT;         // Слив (ON/OFF) (GPIO27, нога 11)
extern bool STEAM_IN;          // Парогенератор (ON/OFF) (GPIO3, нога 34)

// Переменные для управления устройствами PWM
extern int LIGHT;  // Свет (PWM) (GPIO02, нога 24)
extern int FAN_RACK;     // Циркуляция внутри 1 и 2 полки (PWM) (GPIO15, нога 23)
extern int FAN_SHELF;    // Циркуляция внутри 3 и 4 полки (PWM) (GPIO17, нога 28)
extern int FAN_CIRC;     // Циркуляция внутри камеры (PWM) (GPIO16, нога 27)
extern int FAN_INLET;    // Подача воздуха извне (PWM) (GPIO12, нога 13)
extern int HITER_AIR;    // Обогрев камеры (PWM) (GPIO13, нога 15)
extern int HITER_WATER;  // Нагрев воды (PWM) (GPIO14, нога 12)
extern int FAN_OPTION;   // Опциональный вентилятор (GPIO25, нога 9)

// Переменные для управления шаговым двигателем
extern int STEP;  // Шаговый двигатель (GPIO1, нога 35)
extern String DIR;   // Направление (GPIO0, нога 25)
extern bool ENABLE;   // Включение (GPIO0, нога 25)


// Константы для WiFi и WebSocket
extern const char* ssid;
extern const char* password;
extern const char* ws_server;

// Функции для работы с модулями
void setupWater(); // Инициализация модуля управления водой
void setupLightControl(); // Инициализация модуля управления светом
void updateLightBrightness(); // Обновление яркости света
void connectToWiFi(); // Подключение к WiFi
void saveUint16ToEEPROM(int address, uint16_t value); 
void serializeStatus(); // Сериализация и отправка Статуса фермы серверу

#endif // GLOBALS_H
