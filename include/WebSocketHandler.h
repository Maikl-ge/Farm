#ifndef WEBSOCKETHANDLER_H
#define WEBSOCKETHANDLER_H

#include <ArduinoWebsockets.h>

extern websockets::WebsocketsClient webSocket;
extern bool connected;
extern String messageFromServer;


void initializeWebSocket();
void webSocketEvent(websockets::WebsocketsEvent event, String data);
void handleWebSocketMessage(const String& message);
void sendWebSocketMessage(const String& message);
void processWebSocket();
void connectWebSocket();
void parseMessageACK();

#endif // WEBSOCKETHANDLER_H