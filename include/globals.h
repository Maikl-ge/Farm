#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
// Глобальные переменные
extern String id_farm_ACK;
extern String type_msg_ACK;
extern String ack_ACK;

// Константы для WiFi и WebSocket
extern const char* ssid;
extern const char* password;
extern const char* ws_server;
extern const char* http_server;

// Функции для работы с модулями
void setupWater(); // Инициализация модуля управления водой
void setupLightControl(); // Инициализация модуля управления светом
void updateLightBrightness(); // Обновление яркости света

#endif // GLOBALS_H
