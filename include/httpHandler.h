#ifndef HTTPHANDLER_H
#define HTTPHANDLER_H

#include <Arduino.h>
#include <cJSON.h>
#include <esp_http_client.h>
#include <HTTPClient.h>
#include "globals.h"

#define URL_REPORT "http://207.244.250.144:8080/report"

class HttpHandler {
public:
    HttpHandler();
    bool sendJson(const char* url, cJSON* json);
    bool sendText(const char* url, const char* payload);
    cJSON* receiveJson(const char* url);
};

bool sendHttpJsonLikeSocket(const char* url, const char* messageToSend);
void sendHttpJson();
void sendHttpJson(const String& jsonString);
bool sendHttpCommand(const char* CMDtoFarm, const char* url);
bool sendHttpCommand(const String& CMDtoFarm);  // перегрузка с одним аргументом

#endif // HTTPHANDLER_H