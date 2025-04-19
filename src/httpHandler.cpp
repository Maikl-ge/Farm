#include "httpHandler.h"
#include <Arduino.h>
#include <cJSON.h>
#include <esp_http_client.h>
#include <HTTPClient.h>
#include "globals.h"
#include <SDCard.h>


HttpHandler::HttpHandler() {}

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

bool HttpHandler::sendText(const char* url, const char* payload) {
    HTTPClient http;
    http.begin(url);
    http.addHeader("Content-Type", "text/plain");

    int httpResponseCode = http.POST((uint8_t*)payload, strlen(payload));

    if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.printf("HTTP response: %s\n", response.c_str());
    } else {
        Serial.printf("HTTP request failed, error: %s\n", http.errorToString(httpResponseCode).c_str());
    }

    http.end();
    return httpResponseCode > 0;
}

bool sendHttpJsonLikeSocket(const char* url, const char* messageToSend) {
    HttpHandler http;
    bool result = http.sendText(url, messageToSend);
    if(!result){
        enqueue(sd, messageToSend);   // если не Ок то сохраняем в очередь
    }
    Serial.printf("Text send %s\n", result ? "successful" : "failed");
    return result;
}
