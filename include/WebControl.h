#pragma once

#include <WebServer.h>

// Инициализировать сервер
void setupWebServer();

// Объявить сервер (глобально для модуля)
extern WebServer server;
