#include <Arduino.h>
#include "SensorsModule.h"

void setupWater() {
    pinMode(OSMOS_ON_PIN, OUTPUT);
    pinMode(PUMP_2_PIN, OUTPUT);
    digitalWrite (OSMOS_ON_PIN, LOW);
    digitalWrite (PUMP_2_PIN, LOW);

}

// Обработка нажатия кнопок
void updateButtonState() {
    if (modeButtonPressed && startButtonPressed) {
        modeButtonPressed = false; // Сброс флага
        startButtonPressed = false; // Сброс флага
        Serial.println("START");
    }   
    if (modeButtonPressed && stopButtonPressed) {
        modeButtonPressed = false; // Сброс флага
        stopButtonPressed = false; // Сброс флага
        Serial.println("STOP");
    }  
    if (startButtonPressed) {
        startButtonPressed = false; // Сброс флага
        Serial.println("Start button pressed");
    }
    if (stopButtonPressed) {
        stopButtonPressed = false; // Сброс флага
        Serial.println("Stop button pressed");
    }  
}

void sendMessagetoStatus() {
    Serial.println("Power monitor: ERROR");
}

//Обработка состояния датчиков холла
void controlWaterLevel() {
    if (max_osmo_level == 1 && min_osmo_level == 0) {
       digitalWrite (OSMOS_ON_PIN, LOW);
    }

    if (min_osmo_level == 0 && max_osmo_level == 1) {
        digitalWrite (OSMOS_ON_PIN, HIGH);
    }
    
    if (max_water_level == 1 && min_water_level == 0) {
        digitalWrite (PUMP_2_PIN, LOW);
    }
    
    if (min_water_level == 0 && max_water_level == 1) {
        digitalWrite (PUMP_2_PIN, HIGH);
    }
}

    void powerMonitor() {
    // Чтение состояния мониторинга питающей сети
    if (power_monitor == 0) {
        sendMessagetoStatus();
    }
}

    void updateButtonWater() {
        readPCF8574();
        controlWaterLevel();
        updateButtonState();
        powerMonitor();

    }


