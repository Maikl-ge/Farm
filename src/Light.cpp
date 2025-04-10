#include <Arduino.h>
#include <pinout.h>
#include <globals.h>
#include <TimeModule.h>
#include <Profile.h>
#include <status.h>

uint16_t transitionTime = 15 * 60; // Длительность (в секундах) перехода
uint16_t currentTimeMinutes;
unsigned long lastUpdateLightTime = 0;
unsigned long brightnessInterval = 0;
const float GAMMA = 2.2;

// Глобальные переменные
int currentBrightness = 0;

// Константы
const int MIN_BRIGHTNESS = 0;
const int pwmLightChannel = 0;
const int pwmFrequency = 20000;
const int pwmResolution = 10;

void setupLightControl() {
    ledcSetup(pwmLightChannel, pwmFrequency, pwmResolution);
    ledcAttachPin(LIGHT_PIN, pwmLightChannel);
    ledcWrite(pwmLightChannel, MIN_BRIGHTNESS);
}

int applyGammaCorrection(int rawValue, int maxValue) {
    float normalized = float(rawValue) / maxValue;
    float corrected = pow(normalized, GAMMA);
    return corrected * maxValue;
}

void updateLightBrightness() {
    if(statusFarm == "Stop" || statusFarm == "End" || statusFarm == "Abort") {
        ledcWrite(pwmLightChannel, MIN_BRIGHTNESS);
        LIGHT = MIN_BRIGHTNESS;  
        return;  
    }
    if (currentLight <= 0) {
        ledcWrite(pwmLightChannel, MIN_BRIGHTNESS);
        currentBrightness = MIN_BRIGHTNESS;
        LIGHT = MIN_BRIGHTNESS;
        return;
    }

    printCurrentTime();

    long currentLightTimeSeconds = (CurrentTime / 10000) * 3600 + ((CurrentTime / 100) % 100) * 60 + (CurrentTime % 100);

    int targetBrightness = 0;

    long sunriseStart = (SUNRISE * 60) - (transitionTime / 2);
    long sunriseEnd = SUNRISE * 60;
    long sunsetStart = (SUNSET * 60) - (transitionTime / 2);
    long sunsetEnd = SUNSET * 60;

    brightnessInterval = (transitionTime * 1000) / currentLight;

    if (currentLightTimeSeconds >= sunriseStart && currentLightTimeSeconds <= sunriseEnd) {
        float progress = float(currentLightTimeSeconds - sunriseStart) / (sunriseEnd - sunriseStart);
        targetBrightness = MIN_BRIGHTNESS + progress * (currentLight - MIN_BRIGHTNESS);
    } else if (currentLightTimeSeconds >= sunsetStart && currentLightTimeSeconds <= sunsetEnd) {
        float progress = float(currentLightTimeSeconds - sunsetStart) / (sunsetEnd - sunsetStart);
        targetBrightness = currentLight - progress * (currentLight - MIN_BRIGHTNESS);
    } else if (currentLightTimeSeconds > sunriseEnd && currentLightTimeSeconds < sunsetStart) {
        targetBrightness = currentLight;
    } else {
        targetBrightness = MIN_BRIGHTNESS;
    }

    int correctedBrightness = applyGammaCorrection(targetBrightness, currentLight);

    if (currentBrightness != correctedBrightness) {
        currentBrightness = correctedBrightness;
        ledcWrite(pwmLightChannel, currentBrightness);
        LIGHT = currentBrightness;
    }
}