#include "WebControl.h"
#include "httpHandler.h"  // для sendHttpJsonLikeSocket
#include <Arduino.h>
#include <WebSocketHandler.h>
#include <DataSender.h>

const char* html_page = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>ESP32 Control</title>
  <style>
    body {
      text-align: center;
      font-family: sans-serif;
      background-color: #1e1e1e;
    }
    h2 {
      margin-top: 20px;
        font-weight: 400;
        font-size: 18px;
        color: #fff;
    }
    button {
    width: 100%; /* Полная ширина контейнера с ограничением ниже */
    max-width: 190px; /* Максимальная ширина кнопки */
    padding: 5px; /* Внутренние отступы (влияют на общий размер) */
    font-size: clamp(12px, 3vw, 14px); /* Размер шрифта, влияет на высоту и читаемость */
    margin: 4px; /* Внешние отступы, влияют на расстояние между кнопками */
            background: #3e3e3e;
            color: #aaa;
            border: 1px solid #3e3e3e;
            border-radius: 4px;
            cursor: pointer;
            transition: border-color 0.3s, background 0.3s;
            box-sizing: border-box;
    }
    button:hover {
            border-color: #27ae60;
            background: #4e4e4e;
    }
  </style>
</head>
<body>
  <h2>Control FARM</h2>
  <button onclick="sendCommand(1)">Start  GROWING</button><br>
  <button onclick="sendCommand(2)">Stop  Growing</button><br>
  <button onclick="sendCommand(3)">Send  Data</button><br>
  <button onclick="sendCommand(4)">Send Status</button>
  <button onclick="ReBoot(5)">Reboot</button>
    <script>
        function sendCommand(num) {
            fetch("/btn" + num)
                .then(response => {
                    if (!response.ok) {
                        console.error("Ошибка запроса:", response.statusText);
                    }
                })
                .catch(error => console.error("Ошибка соединения:", error));
        }
    </script>  
</body>
</html>
)rawliteral";

WebServer server(80);  // определение глобального объекта

void handleRoot() {
    server.send(200, "text/html", html_page);
}

void handleButton(int num) {
    if (num == 1) {
        startGrowe();
        Serial.println("Start GROWING direct command");
    } else if (num == 2) {
        stopGrowe();
        Serial.println("Stop GROWING direct command");
    } else if (num == 3) {
        sendDataIfNeeded();
        Serial.println("Send Data direct command");
    } else if (num == 4) {
        serializeStatus();
        Serial.println("Send Status direct command");
    } else if (num == 5) {
        Serial.println("Send Status direct command");
        esp_restart();
    } else if (num == 6) {

    }

    server.send(200, "text/plain", "OK");
}

void setupWebServer() {
    server.on("/", handleRoot);
    server.on("/btn1", []() { handleButton(1); });
    server.on("/btn2", []() { handleButton(2); });
    server.on("/btn3", []() { handleButton(3); });
    server.on("/btn4", []() { handleButton(4); });
    server.on("/btn5", []() { handleButton(5); });
    server.on("/btn6", []() { handleButton(6); });
    server.begin();
    Serial.println("HTTP сервер запущен");
}
