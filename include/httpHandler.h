
#ifndef HTTP_HANDLER_H
#define HTTP_HANDLER_H

#pragma once
#include <cJSON.h>
#include <globals.h>

class HttpHandler {
public:
    HttpHandler();
    bool sendJson(const char* url, cJSON* json);
    cJSON* receiveJson(const char* url);
};

void sendHttpJson();
void sendHttpJson(const String& jsonString);
bool sendHttpCommand(const char* CMDtoFarm, const char* url);
bool sendHttpCommand(const String& CMDtoFarm);  // перегрузка с одним аргументом

#endif