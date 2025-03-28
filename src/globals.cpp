#include <Arduino.h>
#include "globals.h"
#include <OneWire.h>
#include <pinout.h>
#include <DallasTemperature.h>

// Глобальные переменные
String messageToSend = "";
String enqueueASK = "empty";   // Флаг подтверждения ACK из очереди
int8_t timeZone = 4; // Часовой пояс
bool sendMessageOK = false;  // Флаг отправки сообщения
long transmitionTime = 0; // Время передачи сообщения
unsigned long timeSlot = 0; // Время Слота передачи сообщения
uint16_t currentVentilation = 0;  // Текущая вентиляция
uint16_t currentCirculation = 0;  // Текущая циркуляция
uint16_t  currentRotation; // Текущая ротация
String currentPhase = ""; // Текущая фаза
float currentTemperatura = 0; // Целевая температура (°C)
float currentHumidity = 0;    // Целевая влажность (%)

// Переменные для управления устройствами On/Off
bool OSMOS_ON = false;          // Подача очищенной воды (ON/OFF) (GPIO32, нога 7)
bool PUMP_WATERING = false;            // Полив (ON/OFF) (GPIO33, нога 8)
bool PUMP_TRANSFER = false;     // Подача в бак полива osmo воды (ON/OFF) (GPIO26, нога 10)
bool WATER_OUT = false;         // Слив (ON/OFF) (GPIO27, нога 11)
bool STEAM_IN = false;          // Парогенератор (ON/OFF) (GPIO3, нога 34)

// Переменные для управления устройствами PWM
int LIGHT = 0;  // Свет (PWM) (GPIO02, нога 24)
int FAN_RACK = 0;     // Циркуляция внутри 1 и 2 полки (PWM) (GPIO15, нога 23)
int FAN_VENT = 0;    // Циркуляция внутри 3 и 4 полки (PWM) (GPIO17, нога 28)
int FAN_CIRC = 0;     // Циркуляция внутри камеры (PWM) (GPIO16, нога 27)
int FAN_INLET = 0;    // Подача воздуха из вне (PWM) (GPIO12, нога 13)
int HITER_AIR = 0;    // Обогрев камеры (PWM) (GPIO13, нога 15)
int HITER_WATER = 0;  // Нагрев воды (PWM) (GPIO14, нога 12)
int FAN_OPTION = 0;   // Опциональный вентилятор (GPIO25, нога 9)

// Переменные для управления шаговым двигателем
int STEP = 0;  // Шаговый двигатель (GPIO1, нога 35)
String DIR = "left";   // Направление (GPIO0, нога 25)
bool ENABLE = false;   // Включение (GPIO0, нога 25)

String id_farm_ACK = "";
String type_msg_ACK = "";  // Инициализация переменной для типа сообщения ACK
String ack_ACK = "";       // Инициализация переменной для подтверждения ACK
int missedPongs = 0;

// Константы для WiFi и WebSocket
const char* ssid = "78A2D8"; // "TORNIKE";       // Название WiFi-сети  "iPhone (M)";  //
const char* password = "mu9fvavddu";  // "20000718";  // Пароль  "dlt654321";   //
const char* ws_server = "ws://207.244.250.144:5001"; // WebSocket сервер

// Создаём объект OneWire
OneWire oneWire(ONE_WIRE_BUS);

// Создаём объект DallasTemperature
DallasTemperature ds18b20(&oneWire);

// Определение адресов датчиков DS18B20
DeviceAddress sensorWaterOsmoAddress = {0x28, 0x8B, 0x63, 0x58, 0x00, 0x00, 0x00, 0x97};
DeviceAddress sensorWateringAddress = {0x28, 0x8B, 0x63, 0x58, 0x00, 0x00, 0x00, 0x97};
DeviceAddress sensorOutdoorAddress = {0x28, 0x8B, 0x63, 0x58, 0x00, 0x00, 0x00, 0x97};
DeviceAddress sensorInletAddress = {0x28, 0x8B, 0x63, 0x58, 0x00, 0x00, 0x00, 0x97};


