#include <Arduino.h>
#include <drive.h>
#include <pinout.h>
#include <globals.h>
#include <Profile.h>

// Константы и настройки
const int STEPS_PER_REVOLUTION = 200 * 16;  // Количество шагов на один полный оборот (для полного шага)
// микрошаг 1/16
const int MIN_RPM = 0.1;                 // Минимальная скорость вращения (RPM)
const int MAX_RPM = 5;                // Уменьшаем максимальную скорость для большей плавности
const int ACCELERATION_STEPS = 2000;   // Увеличиваем количество шагов для более плавного разгона/торможения
const float ACCELERATION_RATE = (MAX_RPM - MIN_RPM) / ACCELERATION_STEPS; // Скорость изменения RPM

// Глобальные переменные для управления шаговым двигателем
unsigned long lastStepTime = 0;        // Время последнего шага (в микросекундах)
unsigned long stepInterval = 0;        // Интервал между шагами (в микросекундах)
bool motorEnabled = false;             // Состояние двигателя (включён/выключен)
bool motorDirection = HIGH;            // Направление вращения (HIGH - по часовой, LOW - против)
float currentRPM = currentRotation; //MIN_RPM;            // Текущая скорость в RPM
float targetRPM = MIN_RPM;             // Целевая скорость в RPM
int accelerationStep = 0;              // Счётчик шагов для разгона/торможения

// Переменные для soakRotation
int soakStepCount = 0;                 // Счётчик шагов в текущей фазе
enum SoakState { INIT_LEFT_25, PAUSE_1, RIGHT_50, PAUSE_2, LEFT_50, PAUSE_3 }; // Состояния цикла
SoakState soakState = INIT_LEFT_25;    // Текущее состояние
unsigned long soakPauseStart = 0;      // Время начала паузы
bool soakInitialized = false;          // Флаг инициализации режима Soak

// Настройка пинов и инициализация
void setupStepper() {
    pinMode(STEP_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);
    pinMode(ENABLE_PIN, OUTPUT);

    // Инициализация состояния
    digitalWrite(STEP_PIN, LOW);
    digitalWrite(DIR_PIN, motorDirection);
    digitalWrite(ENABLE_PIN, HIGH); // Выключен по умолчанию (HIGH - выкл для большинства драйверов)
}

// Функция для плавного изменения скорости
float accelerate(float target) {
    if (abs(currentRPM - target) < 0.01) { // Более точная проверка достижения целевой скорости
        currentRPM = target;
        accelerationStep = 0;
        return currentRPM;
    }

    if (accelerationStep < ACCELERATION_STEPS) {
        if (currentRPM < target) {
            currentRPM += ACCELERATION_RATE; // Разгон
        } else {
            currentRPM -= ACCELERATION_RATE; // Торможение
        }
        accelerationStep++;
    } else {
        currentRPM = target; // Убеждаемся, что достигли целевой скорости
    }

    currentRPM = constrain(currentRPM, MIN_RPM, MAX_RPM);
    return currentRPM;
}

// Функция для режима "Soak" - цикл шагов
void soakRotation() {
    unsigned long currentTime = micros();

    // Устанавливаем целевую скорость в зависимости от состояния (более низкие и плавные скорости)
    float targetSpeed = (soakState == INIT_LEFT_25 || soakState == LEFT_50) ? 10.0 : 20.0; // Уменьшаем скорости
    targetRPM = constrain(targetSpeed, MIN_RPM, MAX_RPM);

    // Плавное изменение скорости
    currentRPM = accelerate(targetRPM);

    // Расчёт интервала между шагами на основе текущей скорости (исправляем на 1000000.0)
    stepInterval = (unsigned long)(1000000.0 / ((currentRPM * STEPS_PER_REVOLUTION) / 60.0));
    STEP = stepInterval; // Установка интервала шагов

    // Включение двигателя, если он выключен
    if (!motorEnabled) {
        digitalWrite(ENABLE_PIN, LOW); // Включаем двигатель
        motorEnabled = true;
    }

    switch (soakState) {
        case INIT_LEFT_25: // 25 шагов влево (только один раз при входе)
            if (!soakInitialized) {
                digitalWrite(DIR_PIN, LOW); // Влево
                if (currentTime - lastStepTime >= stepInterval) {
                    digitalWrite(STEP_PIN, HIGH);
                    delayMicroseconds(15); // Увеличиваем длительность импульса для стабильности
                    digitalWrite(STEP_PIN, LOW);
                    lastStepTime = currentTime;
                    soakStepCount++;
                    if (soakStepCount >= STEPS_PER_REVOLUTION / 8) { // 25 шагов (1/8 оборота)
                        soakStepCount = 0;
                        soakPauseStart = millis();
                        soakState = PAUSE_1;
                        soakInitialized = true; // Помечаем, что начальные 25 шагов выполнены
                    }
                }
            } else {
                soakState = PAUSE_1; // Пропускаем INIT_LEFT_25, если уже выполнено
            }
            break;

        case PAUSE_1: // Пауза 5 секунд
            if (millis() - soakPauseStart >= 5000) {
                soakState = RIGHT_50;
            }
            break;

        case RIGHT_50: // 50 шагов вправо
            digitalWrite(DIR_PIN, HIGH); // Вправо
            if (currentTime - lastStepTime >= stepInterval) {
                digitalWrite(STEP_PIN, HIGH);
                delayMicroseconds(15);
                digitalWrite(STEP_PIN, LOW);
                lastStepTime = currentTime;
                soakStepCount++;
                if (soakStepCount >= STEPS_PER_REVOLUTION / 4) { // 50 шагов (1/4 оборота)
                    soakStepCount = 0;
                    soakPauseStart = millis();
                    soakState = PAUSE_2;
                }
            }
            break;

        case PAUSE_2: // Пауза 5 секунд
            if (millis() - soakPauseStart >= 5000) {
                soakState = LEFT_50;
            }
            break;

        case LEFT_50: // 50 шагов влево
            digitalWrite(DIR_PIN, LOW); // Влево
            if (currentTime - lastStepTime >= stepInterval) {
                digitalWrite(STEP_PIN, HIGH);
                delayMicroseconds(15);
                digitalWrite(STEP_PIN, LOW);
                lastStepTime = currentTime;
                soakStepCount++;
                if (soakStepCount >= STEPS_PER_REVOLUTION / 4) { // 50 шагов
                    soakStepCount = 0;
                    soakPauseStart = millis();
                    soakState = PAUSE_3;
                }
            }
            break;

        case PAUSE_3: // Пауza 5 секунд
            if (millis() - soakPauseStart >= 5000) {
                soakState = RIGHT_50; // Возврат к RIGHT_50, минуя INIT_LEFT_25
            }
            break;
    }
}

// Обновление состояния двигателя
void updateStepperControl() {
    if (currentPhase == "Soak") {
        soakRotation();
        return; // Выходим, чтобы не выполнять стандартную логику
    }

    // Сбрасываем флаг инициализации, если вышли из режима Soak
    if (soakInitialized) {
        soakInitialized = false;
        soakState = INIT_LEFT_25; // Сбрасываем состояние для следующего входа в Soak
    }

    unsigned long currentStepTime = micros();

    // Устанавливаем целевую скорость на основе currentRotation
    targetRPM = constrain(abs(currentRotation), MIN_RPM, MAX_RPM);

    // Плавное изменение скорости
    currentRPM = accelerate(targetRPM);

    if (currentRPM == 0) {
        // Если скорость 0, выключаем двигатель
        if (motorEnabled) {
            digitalWrite(ENABLE_PIN, HIGH); // Выключаем двигатель
            motorEnabled = false;
        }
        return;
    }

    // Установка направления
    bool newDirection = ((currentRotation) > 0) ? HIGH : LOW;
    if (newDirection != motorDirection) {
        motorDirection = newDirection;
        digitalWrite(DIR_PIN, motorDirection);
    }

    // Включение двигателя, если он выключен
    if (!motorEnabled) {
        digitalWrite(ENABLE_PIN, LOW); // Включаем двигатель (LOW - вкл для большинства драйверов)
        motorEnabled = true;
    }

    // Расчёт интервала между шагами (в микросекундах) - исправлено на 1000000.0
    stepInterval = (unsigned long)(1000000.0 / ((currentRPM * STEPS_PER_REVOLUTION) / 60.0));
    STEP = stepInterval; // Установка интервала шагов

    // Генерация импульсов для шага с небольшой задержкой для стабильности
    if (currentStepTime - lastStepTime >= stepInterval) {
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(15); // Увеличиваем длительность импульса
        digitalWrite(STEP_PIN, LOW);
        lastStepTime = currentStepTime;
    }
}