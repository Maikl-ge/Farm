#include <Arduino.h>
#include <drive.h>
#include <pinout.h>
#include <globals.h>
#include <Profile.h>

// Константы и настройки
const int STEPS_PER_REVOLUTION = 200;  // Количество шагов на один полный оборот (для полного шага)
const int MIN_RPM = 1;                 // Минимальная скорость вращения (RPM)
const int MAX_RPM = 200;               // Максимальная скорость вращения (RPM), зависит от драйвера и двигателя

// Глобальные переменные для управления шаговым двигателем
unsigned long lastStepTime = 0;        // Время последнего шага (в микросекундах)
unsigned long stepInterval = 0;        // Интервал между шагами (в микросекундах)
bool motorEnabled = false;             // Состояние двигателя (включён/выключен)
bool motorDirection = HIGH;            // Направление вращения (HIGH - по часовой, LOW - против)

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

// Функция для режима "Soak" - цикл шагов
void soakRotation() {
    unsigned long currentTime = micros();

    // Проверка и установка скорости вращения
    float rpm = constrain(abs(currentRotation), MIN_RPM, MAX_RPM); // Ограничиваем RPM
    stepInterval = (unsigned long)(1000000.0 / ((rpm * STEPS_PER_REVOLUTION) / 60.0));
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
                //Serial.println("INIT_LEFT_25");
                if (currentTime - lastStepTime >= stepInterval) {
                    digitalWrite(STEP_PIN, HIGH);
                    delayMicroseconds(2);
                    digitalWrite(STEP_PIN, LOW);
                    lastStepTime = currentTime;
                    soakStepCount++;
                    if (soakStepCount >= 25) {
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
            //Serial.println("PAUSE_1");
            if (millis() - soakPauseStart >= 5000) {
                soakState = RIGHT_50;
            }
            break;

        case RIGHT_50: // 50 шагов вправо
            digitalWrite(DIR_PIN, HIGH); // Вправо
            //Serial.println("RIGHT_50");
            if (currentTime - lastStepTime >= stepInterval) {
                digitalWrite(STEP_PIN, HIGH);
                delayMicroseconds(2);
                digitalWrite(STEP_PIN, LOW);
                lastStepTime = currentTime;
                soakStepCount++;
                if (soakStepCount >= 50) {
                    soakStepCount = 0;
                    soakPauseStart = millis();
                    soakState = PAUSE_2;
                }
            }
            break;

        case PAUSE_2: // Пауза 5 секунд
            //Serial.println("PAUSE_2");
            if (millis() - soakPauseStart >= 5000) {
                soakState = LEFT_50;
            }
            break;

        case LEFT_50: // 50 шагов влево
            digitalWrite(DIR_PIN, LOW); // Влево
            //Serial.println("LEFT_50");
            if (currentTime - lastStepTime >= stepInterval) {
                digitalWrite(STEP_PIN, HIGH);
                delayMicroseconds(2);
                digitalWrite(STEP_PIN, LOW);
                lastStepTime = currentTime;
                soakStepCount++;
                if (soakStepCount >= 50) {
                    soakStepCount = 0;
                    soakPauseStart = millis();
                    soakState = PAUSE_3;
                }
            }
            break;

        case PAUSE_3: // Пауза 5 секунд
            //Serial.println("PAUSE_3");
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

    // Проверка и установка скорости вращения
    float rpm = constrain(abs(currentRotation), MIN_RPM, MAX_RPM); // Ограничиваем RPM
    if (currentRotation == 0) {
        // Если скорость 0, выключаем двигатель
        if (motorEnabled) {
            digitalWrite(ENABLE_PIN, HIGH); // Выключаем двигатель
            motorEnabled = false;
        }
        return;
    }

    // Установка направления
    bool newDirection = (currentRotation > 0) ? HIGH : LOW;
    if (newDirection != motorDirection) {
        motorDirection = newDirection;
        digitalWrite(DIR_PIN, motorDirection);
    }

    // Включение двигателя, если он выключен
    if (!motorEnabled) {
        digitalWrite(ENABLE_PIN, LOW); // Включаем двигатель (LOW - вкл для большинства драйверов)
        motorEnabled = true;
    }

    // Расчёт интервала между шагами (в микросекундах)
    stepInterval = (unsigned long)(1000000.0 / ((rpm * STEPS_PER_REVOLUTION) / 60.0));
    STEP = stepInterval; // Установка интервала шагов

    // Генерация импульсов для шага
    if (currentStepTime - lastStepTime >= stepInterval) {
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(2); // Минимальная длительность импульса (зависит от драйвера)
        digitalWrite(STEP_PIN, LOW);
        lastStepTime = currentStepTime;
    }
}