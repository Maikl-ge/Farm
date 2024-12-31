#include "globals.h"

// Глобальные переменные
float temperature = 0.0;
float humidity = 0.0;
float waterLevel = 0.0;

// Константы для WiFi и WebSocket
const char* ssid = "TORNIKE";       // Название WiFi-сети
const char* password = "20000718";  // Пароль
const char* ws_server = "ws://207.244.250.144:5001"; // WebSocket сервер
const char* http_server = "http://207.244.250.144:5000/settings"; // Http сервер