#ifndef DATA_SENDER_H
#define DATA_SENDER_H

#include <ArduinoWebsockets.h>
#include <ArduinoJson.h>

using namespace websockets;

// Объявления функций
void initializeWebSocket();
void processWebSocket();
void sendDataIfNeeded();

#endif
