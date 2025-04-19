#ifndef ACCESS_POINT_H
#define ACCESS_POINT_H

#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>

class AccessPoint {
public:
    AccessPoint();
    void start();
    void handleClient();  
    String getSSID();
    String getPassword();

private:
    WebServer server;
    Preferences preferences;

    void handleRoot();
    void handleSave();
};

#endif
