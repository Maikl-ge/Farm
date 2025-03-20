#include "AccessPoint.h"
#include <Arduino.h>

AccessPoint::AccessPoint() : server(80) {}

#define AP_SSID "Farm 255"
#define AP_PASSWORD "87654321" // Минимум 8 символов

void AccessPoint::start() {
    // Инициализация точки доступа
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    Serial.println("Access Point Started");
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());

    // Инициализация веб-сервера
    server.on("/", HTTP_GET, std::bind(&AccessPoint::handleRoot, this));
    server.on("/save", HTTP_POST, std::bind(&AccessPoint::handleSave, this));
    server.begin();
    Serial.println("Web Server Started");
}

String AccessPoint::getSSID() {
    preferences.begin("wifi", true);
    String ssid = preferences.getString("ssid", "");
    preferences.end();
    return ssid;
}

String AccessPoint::getPassword() {
    preferences.begin("wifi", true);
    String password = preferences.getString("password", "");
    preferences.end();
    return password;
}

void AccessPoint::handleRoot() {
    String html = R"rawliteral(
        <!DOCTYPE html>
        <html>
        <head>
            <title>WiFi Configuration</title>
        </head>
        <body>
            <h1>Enter WiFi Credentials</h1>
            <form action="/save" method="POST">
                <label for="ssid">SSID:</label><br>
                <input type="text" id="ssid" name="ssid"><br>
                <label for="password">Password:</label><br>
                <input type="password" id="password" name="password"><br><br>
                <input type="submit" value="Save">
            </form>
        </body>
        </html>
    )rawliteral";
    server.send(200, "text/html", html);
}

void AccessPoint::handleSave() {
    if (server.hasArg("ssid") && server.hasArg("password")) {
        String ssid = server.arg("ssid");
        String password = server.arg("password");

        preferences.begin("wifi", false);
        preferences.putString("ssid", ssid);
        preferences.putString("password", password);
        preferences.end();

        server.send(200, "text/plain", "Credentials saved! Reboot the device.");
        delay(2000);
        ESP.restart();
    } else {
        server.send(400, "text/plain", "Invalid input!");
    }
}
