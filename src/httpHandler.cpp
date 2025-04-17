#include "httpHandler.h"
#include <esp_http_client.h>
#include <Arduino.h>
#include <globals.h>

#define API_BASE_URL "http://207.244.250.144:8080"
#define COMMAND_ENDPOINT "/command/send_cmd"
#define DATA_ENDPOINT "/api/command"

HttpHandler::HttpHandler() {}

bool HttpHandler::sendJson(const char* url, cJSON* json) {
    if (!json || !url) {
        Serial.println("Error: Invalid URL or JSON");
        return false;
    }
    char* payload = cJSON_PrintUnformatted(json);
    if (!payload) {
        Serial.println("Error: Failed to serialize JSON");
        return false;
    }
    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_POST,
        .timeout_ms = 5000,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        Serial.println("Error: Failed to init HTTP client");
        free(payload);
        return false;
    }
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, payload, strlen(payload));
    esp_err_t err = esp_http_client_perform(client);
    int status_code = esp_http_client_get_status_code(client);
    esp_http_client_cleanup(client);
    free(payload);
    if (err != ESP_OK) {
        Serial.printf("HTTP POST failed: %s\n", esp_err_to_name(err));
        return false;
    }
    if (status_code < 200 || status_code >= 300) {
        Serial.printf("HTTP error: %d\n", status_code);
        return false;
    }
    return true;
}

cJSON* HttpHandler::receiveJson(const char* url) {
    if (!url) {
        Serial.println("Error: Invalid URL");
        return nullptr;
    }
    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_GET,
        .timeout_ms = 5000,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        Serial.println("Error: Failed to init HTTP client");
        return nullptr;
    }
    esp_err_t err = esp_http_client_perform(client);
    int status_code = esp_http_client_get_status_code(client);
    if (err != ESP_OK || status_code != 200) {
        Serial.printf("HTTP GET failed: %s, status: %d\n", esp_err_to_name(err), status_code);
        esp_http_client_cleanup(client);
        return nullptr;
    }
    int content_length = esp_http_client_get_content_length(client);
    int buffer_size = content_length > 0 ? content_length + 1 : 4096;
    char* buffer = (char*)calloc(1, buffer_size);
    if (!buffer) {
        Serial.println("Error: Failed to allocate buffer");
        esp_http_client_cleanup(client);
        return nullptr;
    }
    int len = esp_http_client_read_response(client, buffer, buffer_size - 1);
    esp_http_client_cleanup(client);
    if (len <= 0) {
        Serial.println("Error: Empty response");
        free(buffer);
        return nullptr;
    }
    buffer[len] = '\0';
    cJSON* json = cJSON_Parse(buffer);
    if (!json) {
        Serial.printf("JSON parse error: %s\n", cJSON_GetErrorPtr());
        free(buffer);
        return nullptr;
    }
    free(buffer);
    return json;
}

bool sendHttpJson(const char* url, const char* device, float temperature, float humidity) {
    HttpHandler http;
    cJSON* root = cJSON_CreateObject();
    if (!root) {
        Serial.println("Error: Failed to create JSON object");
        return false;
    }
    cJSON_AddStringToObject(root, "device", device);
    cJSON_AddNumberToObject(root, "temperature", temperature);
    cJSON_AddNumberToObject(root, "humidity", humidity);
    bool result = http.sendJson(url, root);
    Serial.printf("JSON send %s\n", result ? "successful" : "failed");
    cJSON_Delete(root);
    return result;
}

bool sendHttpCommand(const String& CMDtoFarm) {
    const char* defaultUrl = "http://207.244.250.144:8080/command/send_cmd";
    return sendHttpCommand(CMDtoFarm.c_str(), defaultUrl);
}

bool sendHttpCommand(const char* CMDtoFarm, const char* url) {
    HttpHandler http;
    cJSON* root = cJSON_CreateObject();
    if (!root) {
        Serial.println("Error: Failed to create JSON object");
        return false;
    }
    cJSON_AddStringToObject(root, "id", "255");
    cJSON_AddStringToObject(root, "command", CMDtoFarm);
    cJSON_AddStringToObject(root, "ask_comm", "12");

    bool result = http.sendJson(url, root);
    if (result) {
        char* json_str = cJSON_PrintUnformatted(root);
        Serial.printf("Command sent: %s\n", json_str);
        free(json_str);
    } else {
        Serial.println("Command send failed");
    }

    cJSON_Delete(root);
    return result;
}


// #include "httpHandler.h"
// #undef INADDR_NONE
// #include <esp_http_client.h>
// #include <Arduino.h>
// #include <globals.h>
// #include <HTTPClient.h>

// // Конструктор класса HttpHandler, пока без инициализации
// HttpHandler::HttpHandler() {}
// String jsonString = "";

// // Отправка JSON-объекта по HTTP POST-запросу на указанный URL
// bool HttpHandler::sendJson(const char* url, cJSON* json) {
//     // Проверка на нулевые указатели
//     if (!json || !url) return false;

//     // Преобразование JSON-объекта в неформатированную строку
//     char* payload = cJSON_PrintUnformatted(json);
//     if (!payload) return false;

//     // Настройка конфигурации клиента
//     esp_http_client_config_t config = {};
//     config.url = url;
//     config.method = HTTP_METHOD_POST;

//     // Инициализация HTTP клиента с конфигурацией
//     esp_http_client_handle_t client = esp_http_client_init(&config);
//     if (!client) {
//         free(payload);  // Очистка памяти в случае ошибки
//         return false;
//     }

//     // Установка заголовка Content-Type как JSON
//     esp_http_client_set_header(client, "Content-Type", "application/json");

//     // Установка тела запроса (данные JSON) и его длины
//     esp_http_client_set_post_field(client, payload, strlen(payload));

//     // Выполнение запроса
//     esp_err_t err = esp_http_client_perform(client);

//     // Очистка ресурсов клиента и освобождение памяти
//     esp_http_client_cleanup(client);
//     free(payload);

//     // Проверка успешности запроса
//     return (err == ESP_OK);
// }

// // Получение JSON-ответа по HTTP GET-запросу с указанного URL
// cJSON* HttpHandler::receiveJson(const char* url) {
//     // Проверка на нулевой указатель URL
//     if (!url) return nullptr;

//     // Буфер для хранения ответа сервера
//     const int buffer_size = 4096;
//     char* buffer = (char*)calloc(1, buffer_size);  // Инициализация нулями
//     if (!buffer) return nullptr;

//     // Настройка конфигурации HTTP клиента
//     esp_http_client_config_t config = {};
//     config.url = url;
//     config.method = HTTP_METHOD_GET;

//     // Инициализация клиента
//     esp_http_client_handle_t client = esp_http_client_init(&config);
//     if (!client) {
//         free(buffer);  // Очистка памяти в случае ошибки
//         return nullptr;
//     }

//     // Выполнение GET-запроса
//     esp_err_t err = esp_http_client_perform(client);
//     if (err != ESP_OK) {
//         // Ошибка запроса, освобождение ресурсов
//         esp_http_client_cleanup(client);
//         free(buffer);
//         return nullptr;
//     }

//     // Чтение ответа сервера в буфер
//     int len = esp_http_client_read_response(client, buffer, buffer_size - 1);
//     esp_http_client_cleanup(client);  // Очистка клиента после чтения

//     // Проверка на пустой ответ
//     if (len <= 0) {
//         free(buffer);
//         return nullptr;
//     }

//     // Завершение строки нулевым символом
//     buffer[len] = '\0';

//     // Парсинг строки в JSON-объект
//     cJSON* json = cJSON_Parse(buffer);
//     free(buffer);  // Очистка буфера

//     // Возврат JSON-объекта (nullptr в случае ошибки)
//     return json;
// }

// void sendHttpJson() {
//     // Создаем экземпляр обработчика HTTP
//     HttpHandler http;

//     // Создаем JSON-объект
//     cJSON* root = cJSON_CreateObject();
//     if (!root) {
//         printf("Ошибка: не удалось создать JSON-объект\n");
//         return;
//     }

//     // Добавляем пары ключ-значение в JSON
//     cJSON_AddStringToObject(root, "device", "growbox-1");
//     cJSON_AddNumberToObject(root, "temperature", 24.5);
//     cJSON_AddNumberToObject(root, "humidity", 70);

//     // Отправляем JSON на сервер
//     const char* url = "http://207.244.250.144:8080/api/command";  // замени на нужный URL
//     bool result = http.sendJson(url, root);

//     // Проверяем результат
//     if (result) {
//         printf("JSON успешно отправлен\n");
//     } else {
//         printf("Ошибка при отправке JSON\n");
//     }

//     // Освобождаем память, выделенную под JSON
//     cJSON_Delete(root);
// }

// void sendHttpCommand(String CMDtoFarm) {

//     // Создаем JSON-объект
//     cJSON* root = cJSON_CreateObject();
//     cJSON_AddStringToObject(root, "command", CMDtoFarm.c_str());
//     cJSON_AddStringToObject(root, "ask_comm", "12");  // можно и cJSON_AddNumberToObject, если сервер примет

//     // Указываем целевой URL
//     const char* url = "http://207.244.250.144:8080/command/send_cmd";  // замените на нужный

//      // Создаем экземпляр обработчика HTTP   
//     HttpHandler http;
//     bool result = http.sendJson(url, root);

//     // Проверяем результат
//     if (result) {
//         printf("Команда успешно отправлена: %s\n", root);
//     } else {
//         printf("Ошибка при отправке команды\n");
//     }

//     // Освобождаем память, выделенную под JSON
//     cJSON_Delete(root);
// }