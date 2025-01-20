#ifndef ACCESS_POINT_H
#define ACCESS_POINT_H

#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>

class AccessPoint {
public:
    AccessPoint();
    void start();
    String getSSID();
    String getPassword();
    void handleRoot();
    void handleSave();

private:
    WebServer server;
    Preferences preferences;
};

#endif