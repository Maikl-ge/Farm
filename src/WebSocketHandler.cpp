#include <Arduino.h>
#include <ArduinoWebsockets.h>
#include "WebSocketHandler.h"
#include "globals.h"
#include <DataSender.h>
#include "Profile.h"
#include <CurrentProfile.h>

//#define WEBSOCKETS_MAX_DATA_SIZE 1024 // Максимальный размер данных

using namespace websockets;

// Глобальные переменные для WebSocket клиента 
WebsocketsClient webSocket;
String messageFromServer;
String messageACK;
bool connectedWebSocket = false;
bool connected = false;
unsigned long lastReconnectAttempt = 0;
const unsigned long RECONNECT_INTERVAL = 1400; // Интервал повторного подключения в мс

// Прототипы функций
void connectWebSocket();
void resetWebSocketState();
void handleWebSocketMessage(const String& message);
void parceMessageFromServer(const String& messageFromServer);

void initializeWebSocket() {
    webSocket.onMessage([](WebsocketsMessage message) {
        handleWebSocketMessage(message.data());
    });

    webSocket.onEvent([](WebsocketsEvent event, String data) {
        webSocketEvent(event, data);
    });

    connectWebSocket();
}

void webSocketEvent(WebsocketsEvent event, String data) {
    switch (event) {
        case WebsocketsEvent::ConnectionOpened:
            Serial.println("WebSocket connection opened");
            connected = true;
            break;
        case WebsocketsEvent::ConnectionClosed:
            Serial.println("WebSocket connection closed !!!");
            connectedWebSocket = false;
            processWebSocket();  // Переподключаем
            break;
        case WebsocketsEvent::GotPing:
            webSocket.pong(); // Отправка PONG-сообщения
            //Serial.println("Got a Ping!");
            break;
        case WebsocketsEvent::GotPong:
            missedPongs = 0;   // Сброс счетчика
            Serial.println("Got a Pong!");
            break;
    }
}

void parceMessageFromServer(const String& messageFromServer) {

    // Обработка сообщения КОМАНДЫ
    if (messageFromServer == SERVER_CMD_START) {    // SCMD Запуск цикла роста
        saveUint16ToEEPROM(EEPROM_WORK_ADDRESS, (0x57 << 8 | 0x4F)); // W O - WORK
        Serial.println("Команда от сервера: START");
    }
    if (messageFromServer == SERVER_CMD_STOP) {      // SCMS Остановка цикла роста
        saveUint16ToEEPROM(EEPROM_WORK_ADDRESS, (0x45 << 8 | 0x4E)); // E N - END
        Serial.println("Команда от сервера: STOP");
    }
    if (messageFromServer == SERVER_CMD_RESTART) {    // SCMR Перезагрузка фермы
        Serial.println("Команда от сервера: RESTART");
        esp_restart();
    }
    if (messageFromServer == SERVER_CMD_UPDATE) {
        Serial.println("Команда от сервера: UPDATE");
    }
    if (messageFromServer == SERVER_CMD_SETTINGS) {         // Получена команда Обновление настроек
        fetchAndSaveSettings();
        Serial.println("Команда от сервера: SETTINGS NEW");
    }
    
    // Обработка сообщения ЗАПРОСЫ
    if (messageFromServer == SERVER_REQ_STATUS) {     // SRST Запрос статуса фермы
        serializeStatus();
        Serial.println("Запрос от сервера: STATUS");
    }
    if (messageFromServer == SERVER_REQ_DATA) {
        // Отправка данных
        sendDataIfNeeded();
        // Отправка статуса
        serializeStatus();
        Serial.println("Запрос от сервера: DATA");
    }
    if (messageFromServer == SERVER_REQ_SETTINGS) {
        Serial.println(readUint16FromEEPROM(EEPROM_WORK_ADDRESS));  // SRSE Запрос на отправку Настройки фермы
        serializeSettings();
        Serial.println("Настройки из EEPROM");
    }
    if (messageFromServer == SERVER_REQ_PARAMETERS) {
        sendDataIfNeeded();
        Serial.println("Запрос от сервера: PARAMETERS");
    }
    if (messageFromServer == SERVER_REQ_PROFILE) {
        Serial.println("Запрос от сервера: PROFILE");
    }
    if (messageFromServer == SERVER_REQ_CURRENT) {
        Serial.println("Запрос от сервера: CURRENT");
    }
    // Обработка сообщения ошибки и 
    if (messageFromServer == SERVER_ERR_INVALID) {
        Serial.println("Ошибка: недействительный запрос");
    }
    if (messageFromServer == SERVER_EVENT_SYNC) {
        Serial.println("Событие синхронизации");
    }
}    

void sendWebSocketMessage(const String& ID_FARM, const String& TYPE_MSG, const String& LENGTH_MSG, const String& jsonMessage) {
    String messageToSend = String(ID_FARM) + " " + TYPE_MSG + " " + String(LENGTH_MSG) + " " + jsonMessage;
    if (connected) {
        webSocket.send(messageToSend);
        Serial.print("Sent: ");
        Serial.println(TYPE_MSG);
        //Serial.println(messageToSend);
    } else {
        Serial.println("Соединение отсутствует. Сообщение не отправлено.");
        saveMessageToSDCard(messageToSend);
    }
    //parseMessageACK();
}

void processWebSocket() {
    static unsigned long lastPing = 0;
    unsigned long currentMillis = millis();

    if (!connected) {
        // Проверка состояния Wi-Fi
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("Wi-Fi disconnected, attempting to reconnect...");
            WiFi.reconnect();
            delay(1000); // Небольшая задержка для стабилизации
            return;      // Не пытаемся подключиться к WebSocket, пока Wi-Fi не восстановится
        }

        // Если Wi-Fi в порядке, проверяем необходимость переподключения WebSocket
        if (currentMillis - lastReconnectAttempt >= RECONNECT_INTERVAL) {
           lastReconnectAttempt = currentMillis;
            connectWebSocket();
        }
    } 
}

void resetWebSocketState() {
    Serial.println("Resetting WebSocket state...");

    // Очистка буферов сообщений
    messageFromServer = "";
    messageACK = "";
    
    // Очистка переменных ACK
    type_msg_ACK = "";
    ack_ACK = "";
    id_farm_ACK = "";
    
    // Сброс счётчиков
    missedPongs = 0;
    lastReconnectAttempt = 0;

    // Очистка флагов
    connected = false;

    // Очистка дополнительных буферов или очередей
    //  pendingMessages.clear();  // Если используется очередь сообщений
    
    try {
        // Закрытие WebSocket соединения
        Serial.println("Закрываем соединение, если оно есть: ");
        webSocket.close();  // Закрываем соединение, если оно есть
    } catch (const std::exception& e) {
        Serial.print("Exception during WebSocket close: ");
        Serial.println(e.what());
    }

    Serial.println("WebSocket state has been reset");
}

void connectWebSocket() {
    static bool isConnecting = false;
    if (isConnecting) return;  // Если уже идет подключение, то выходим

    Serial.println("Attempting to connect to WebSocket...");

    // Очищаем состояние перед новым подключением    
    resetWebSocketState();

    isConnecting = true;
    
    try {
        connected = webSocket.connect(ws_server);
        if (connected) {
            Serial.println("WebSocket connected");
            connectedWebSocket = true;
        } else {
            Serial.println("WebSocket connection failed");
        }
    } catch (const std::exception& e) {
        Serial.print("Exception during WebSocket connection: ");
        Serial.println(e.what());
    }

    isConnecting = false;
} 

// Функция для разбора сообщения сервера на три переменные
void handleWebSocketMessage(const String& message) {
    Serial.print("Received WebSocket message: ");
    Serial.println(message);
    messageFromServer = message;
    messageACK = message;

    // Проверяем, что сообщение не пустое
    if (messageACK.isEmpty()) {
        //Serial.println("Ошибка: пустое сообщение ACK");
        return;
    }

    // Находим позиции пробелов
    int firstSpace = messageACK.indexOf(' ');  // Позиция первого пробела
    int secondSpace = messageACK.indexOf(' ', firstSpace + 1);  // Позиция второго пробела

    // Проверяем корректность структуры сообщения
    if (firstSpace == -1 || secondSpace == -1) {
        Serial.println("Ошибка разбора сообщения: недостаточно частей");
        return;
    }

    // Извлечение частей
    id_farm_ACK = messageACK.substring(0, firstSpace);                 // Первая часть: номер фермы
    type_msg_ACK = messageACK.substring(firstSpace + 1, secondSpace); // Вторая часть: тип сообщения
    ack_ACK = messageACK.substring(secondSpace + 1);                  // Третья часть: квитанция

    // Проверка корректности ID фермы
    if (id_farm_ACK == String(ID_FARM)) {
        parceMessageFromServer(type_msg_ACK);
        messageFromServer = type_msg_ACK;
    } 
}
