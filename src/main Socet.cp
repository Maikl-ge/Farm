#include <WiFi.h>
#include <ArduinoWebsockets.h>
#include <ArduinoJson.h>  // Библиотека для работы с JSON
#include "globals.h"

float temperature = 0;  // Примерная температура
float humidity = 0;     // Примерная влажность
float waterLevel = 0;

using namespace websockets;

// Данные для подключения к WiFi
const char* ssid = "TORNIKE";       // Название WiFi-сети
const char* password = "20000718"; // Пароль

// Адрес и порт WebSocket-сервера
const char* ws_server = "ws://207.244.250.144:5001"; // Заменить на ваш WebSocket сервер

// Инициализация WebSocket клиента
WebsocketsClient client;
bool connected = false; // Переменная для отслеживания состояния подключения

void setup() {
    // Инициализация серийного порта
    Serial.begin(115200);

    // Подключение к WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi!");

    // Подключение к WebSocket серверу
    client.onMessage([](WebsocketsMessage message) {
        Serial.print("Received: ");
        Serial.println(message.data());
    });

    client.onEvent([](WebsocketsEvent event, String data) {
        if (event == WebsocketsEvent::ConnectionOpened) {
            Serial.println("WebSocket connection opened");
            connected = true; // Обновляем состояние подключения
        } else if (event == WebsocketsEvent::ConnectionClosed) {
            Serial.println("WebSocket connection closed");
            connected = false; // Обновляем состояние подключения
        } else if (event == WebsocketsEvent::GotPing) {
            Serial.println("Received Ping!");
        } else if (event == WebsocketsEvent::GotPong) {
            Serial.println("Received Pong!");
        }
    });

    if (client.connect(ws_server)) {
        Serial.println("Connected to WebSocket server!");
        connected = true; // Устанавливаем состояние подключения
    } else {
        Serial.println("Failed to connect to WebSocket server");
    }
}

void loop() {
    client.poll();

    // Проверяем, подключены ли мы
    if (!connected) {
        Serial.println("Reconnecting to WebSocket server...");
        if (client.connect(ws_server)) {
            Serial.println("Reconnected to WebSocket server!");
            connected = true; // Устанавливаем состояние подключения
        }
    }

    // Обновляем значения переменных
    temperature = random(1800, 3000) / 100.0;
    humidity = random(300, 800) / 10.0;
    waterLevel = random(300, 800) / 5.0;

    // Используем JsonDocument
    JsonDocument doc;

    doc["температура"] = temperature;  // Используем глобальную переменную температуры
    doc["влажность"] = humidity;       // Используем глобальную переменную влажности
    doc["уровень воды"] = waterLevel;  // Используем глобальную переменную уровня воды

    // Сериализация JSON в строку
    String jsonMessage;
    serializeJson(doc, jsonMessage);

    // Отправляем JSON каждые 10 секунд
    static unsigned long lastTime = 0;
    if (connected && millis() - lastTime > 60000) {
        client.send(jsonMessage);
        lastTime = millis();
        Serial.print("Sent: ");
        Serial.println(jsonMessage);  // Печатаем отправленный JSON
    }
}