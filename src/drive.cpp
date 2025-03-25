#include <Arduino.h>
#include <drive.h>
#include <pinout.h>
#include <globals.h>

// Константы и настройки
const int STEPS_PER_REVOLUTION = 200;  // Количество шагов на один полный оборот (для полного шага)
const int MIN_RPM = 1;                 // Минимальная скорость вращения (RPM)
const int MAX_RPM = 200;               // Максимальная скорость вращения (RPM), зависит от драйвера и двигателя

// Глобальные переменные для управления шаговым двигателем
unsigned long lastStepTime = 0;        // Время последнего шага (в микросекундах)
unsigned long stepInterval = 0;        // Интервал между шагами (в микросекундах)
bool motorEnabled = false;             // Состояние двигателя (включён/выключен)
bool motorDirection = HIGH;            // Направление вращения (HIGH - по часовой, LOW - против)

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

// Обновление состояния двигателя
void updateStepperControl() {
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
    // Частота шагов (Hz) = (RPM * Шаги_на_оборот) / 60
    // Интервал (мкс) = 1,000,000 / Частота
    stepInterval = (unsigned long)(1000000.0 / ((rpm * STEPS_PER_REVOLUTION) / 60.0));
    STEP = stepInterval;  // Установка интервала шагов
    
    // Генерация импульсов для шага
    if (currentStepTime - lastStepTime >= stepInterval) {
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(2); // Минимальная длительность импульса (зависит от драйвера)
        digitalWrite(STEP_PIN, LOW);
        lastStepTime = currentStepTime;
    }
}