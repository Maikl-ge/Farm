#ifndef DATA_SENDER_H
#define DATA_SENDER_H

#include <ArduinoWebsockets.h>
#include <ArduinoJson.h>
#include <Arduino.h>

using namespace websockets;

// Объявления функций
void initializeWebSocket();
void processWebSocket();
void handleWebSocketMessage(WebsocketsMessage message);
void sendDataIfNeeded();
void saveMessageToSDCard(const String& message);
void sendWebSocketMessage(const String& ID_FARM, const String& TYPE_MSG, const String& LENGTH_MSG, const String& jsonMessage);
void setupCDcard();

#endif
