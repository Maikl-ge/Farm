#include <Arduino.h>
#include <drive.h>
#include <pinout.h>
#include <globals.h>
#include <Profile.h>
#include <cmath>

// Константы и настройки
const int STEPS_PER_REVOLUTION = 200 * 16;  // 3200 шагов на оборот (микрошаг 1/16)
const float MIN_RPM = 0.1;                 // Минимальная скорость вращения (RPM)
const float MAX_RPM = 50.0;                // Уменьшенная максимальная скорость
const int ACCELERATION_STEPS = 8000;       // Больше шагов для плавности
const float ACCELERATION_RATE = (MAX_RPM - MIN_RPM) / (float)ACCELERATION_STEPS;

// Глобальные переменные
unsigned long lastStepTime = 0;
unsigned long stepInterval = 0;
bool motorEnabled = false;
bool motorDirection = HIGH;
float currentRPM = MIN_RPM;
float targetRPM = currentRotation;
int accelerationStep = 0;
int stepsToTarget = 0;
bool currentDir = false;
int statusStep = 0;

// Переменные для soakRotation
int soakStepCount = 0;
enum SoakState { INIT_LEFT_25, PAUSE_1, RIGHT_50, PAUSE_2, LEFT_50, PAUSE_3 };
SoakState soakState = INIT_LEFT_25;
unsigned long soakPauseStart = 0;
bool soakInitialized = false;
void rightStep();
void leftStep();
void pauseLeft();
void pauseRight();

void setupStepper() {
    pinMode(STEP_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);
    pinMode(ENABLE_PIN, OUTPUT);

    digitalWrite(STEP_PIN, LOW);
    digitalWrite(DIR_PIN, motorDirection);
    digitalWrite(ENABLE_PIN, HIGH); // Выключен по умолчанию
}

float accelerate(float target) {
    static unsigned long lastTime = millis();
    unsigned long currentTime = millis();
    float timeDelta = (currentTime - lastTime) / 1000.0;

    if (abs(currentRPM - target) < 0.1) { // Увеличенный порог
        currentRPM = target;
        accelerationStep = 0;
    } else if (accelerationStep < ACCELERATION_STEPS) {
        float rate = ACCELERATION_RATE * timeDelta;
        if (currentRPM < target) {
            currentRPM += rate;
        } else {
            currentRPM -= rate;
        }
        accelerationStep++;
    } else {
        currentRPM = target;
    }

    currentRPM = constrain(currentRPM, MIN_RPM, MAX_RPM);
    lastTime = currentTime;
    return currentRPM;
}

void soakRotation() {
    currentStepTime = micros();
    targetRPM = constrain(currentRotation, MIN_RPM, MAX_RPM);
    currentRPM = accelerate(targetRPM);
    stepInterval = (unsigned long)(1000000.0 / ((currentRPM * STEPS_PER_REVOLUTION) / 60.0));

    if (!motorEnabled) {
        digitalWrite(ENABLE_PIN, LOW);
        motorEnabled = true;
    }

    switch (statusStep) {
        case 0: pauseLeft(); break;
        case 1: stepsToTarget = STEPS_PER_REVOLUTION / 4; currentDir = false; rightStep(); break;
        case 2: pauseRight(); break;
        case 3: stepsToTarget = STEPS_PER_REVOLUTION / 4; currentDir = true; leftStep(); break;
    }
}

void germRotation() {
    currentStepTime = micros();
    targetRPM = constrain(currentRotation, MIN_RPM, MAX_RPM);
    currentRPM = accelerate(targetRPM);

    stepInterval = (unsigned long)(1000000.0 / ((currentRPM * STEPS_PER_REVOLUTION) / 60.0));

    STEP = stepInterval;

    currentDir = false; // Вправо
    digitalWrite(DIR_PIN, currentDir);

    if (!motorEnabled) {
        digitalWrite(ENABLE_PIN, LOW);
        motorEnabled = true;
    }

    if (currentStepTime - lastStepTime >= stepInterval) {
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(50); // Увеличенный импульс
        digitalWrite(STEP_PIN, LOW);
        lastStepTime = currentStepTime;
    }
}

void pauseLeft() {
    if (statusStep == 0) {
        if (millis() - soakPauseStart >= 5000) {
            statusStep = 1;
        }
    }
}

void rightStep() {
    if (statusStep == 1) {
        digitalWrite(DIR_PIN, currentDir);
        if (currentStepTime - lastStepTime >= stepInterval) {
            digitalWrite(STEP_PIN, HIGH);
            delayMicroseconds(50);
            digitalWrite(STEP_PIN, LOW);
            lastStepTime = currentStepTime;
            soakStepCount++;
            if (soakStepCount >= stepsToTarget) {
                soakStepCount = 0;
                soakPauseStart = millis();
                statusStep = 2;
            }
        }
    }
}

void pauseRight() {
    if (statusStep == 2) {
        if (millis() - soakPauseStart >= 5000) {
            statusStep = 3;
        }
    }
}

void leftStep() {
    if (statusStep == 3) {
        digitalWrite(DIR_PIN, currentDir);
        if (currentStepTime - lastStepTime >= stepInterval) {
            digitalWrite(STEP_PIN, HIGH);
            delayMicroseconds(50);
            digitalWrite(STEP_PIN, LOW);
            lastStepTime = currentStepTime;
            soakStepCount++;
            if (soakStepCount >= stepsToTarget) {
                soakStepCount = 0;
                soakPauseStart = millis();
                statusStep = 0;
            }
        }
    }
}

void updateStepperControl() {
    if(statusFarm == "Work" || statusFarm == "Pause") {
        digitalWrite(ENABLE_PIN, HIGH);    
        if (currentPhase == "Soak") {
            soakRotation();
            return;
        } else if (currentPhase == "Germ" || currentPhase == "Act" || currentPhase == "Early" || 
                currentPhase == "Grow" || currentPhase == "Finish") {
            germRotation();
            return;
        }
        digitalWrite(ENABLE_PIN, LOW);
        return;
    }
}