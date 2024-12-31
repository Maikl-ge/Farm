#include "ota_module.h"
#include <ArduinoOTA.h>
#include <WiFi.h>

void setupOTA() {
    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "Sketch";
        } else { // U_SPIFFS
            type = "Filesystem";
        }
        Serial.println("Начало OTA: " + type);
    });

    ArduinoOTA.onEnd([]() {
        Serial.println("\nOTA завершено!");
    });

    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Ошибка [%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
            Serial.println("Ошибка аутентификации");
        } else if (error == OTA_BEGIN_ERROR) {
            Serial.println("Ошибка начала");
        } else if (error == OTA_CONNECT_ERROR) {
            Serial.println("Ошибка соединения");
        } else if (error == OTA_RECEIVE_ERROR) {
            Serial.println("Ошибка получения");
        } else if (error == OTA_END_ERROR) {
            Serial.println("Ошибка завершения");
        }
    });

    ArduinoOTA.begin();
    Serial.println("OTA готово!");
}
