#include <Arduino.h>
#include <airBox.h>
#include <pinout.h>
#include <globals.h>
#include <PID_v1.h>
#include <SensorsModule.h>

// Константы и настройки
const int PWM_FREQUENCY = 5000;       // Частота ШИМ (Гц)
const int PWM_RESOLUTION = 10;        // Разрешение ШИМ (10 бит = 0-1023)
const int HITER_AIR_CHANNEL = 0;      // Канал ШИМ для нагревателя
const int FAN_INLET_CHANNEL = 1;      // Канал ШИМ для приточного вентилятора
const float TEMP_TOLERANCE = 0.5;     // Допуск температуры (±0.5°C)
const float HUM_TOLERANCE = 1.0;      // Допуск влажности (±1%)
const int MIN_PWM = 50;               // Минимальное значение ШИМ (для старта устройств)
const int MAX_PWM = 1023;             // Максимальное значение ШИМ

// PID параметры (настройте под вашу систему)
const double KP_TEMP = 40.0;          // Пропорциональный коэффициент для температуры
const double KI_TEMP = 0.1;           // Интегральный коэффициент для температуры
const double KD_TEMP = 10.0;          // Дифференциальный коэффициент для температуры
const double KP_FAN = 30.0;           // Пропорциональный коэффициент для вентилятора
const double KI_FAN = 0.05;           // Интегральный коэффициент для вентилятора
const double KD_FAN = 5.0;            // Дифференциальный коэффициент для вентилятора

// Глобальные переменные для PID
double tempInput = 0.0;               // Текущая температура (вход для PID)
double tempOutput = 0.0;              // Выход PID для нагревателя (0-1023)
double tempSetpoint = 0.0;            // Целевая температура (из currentTemperatura)
double fanInput = 0.0;                // Текущая температура (вход для PID вентилятора)
double fanOutput = 0.0;               // Выход PID для вентилятора (0-1023)
double fanSetpoint = 0.0;             // Целевая температура для вентилятора (из currentTemperatura)

// PID объекты
PID tempPID(&tempInput, &tempOutput, &tempSetpoint, KP_TEMP, KI_TEMP, KD_TEMP, DIRECT);
PID fanPID(&fanInput, &fanOutput, &fanSetpoint, KP_FAN, KI_FAN, KD_FAN, DIRECT);

// Время последнего обновления
unsigned long lastUpdateTime = 0;
const unsigned long UPDATE_INTERVAL = 1000; // Интервал обновления (1 секунда)

// Настройка пинов и инициализация
void setupClimateControl() {
    // Установка целевых значений
    currentTemperatura = 23.0;
    currentHumidity = 75.0;
    // Настройка ШИМ для нагревателя
    ledcSetup(HITER_AIR_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(HITER_AIR_PIN, HITER_AIR_CHANNEL);
    ledcWrite(HITER_AIR_CHANNEL, 0); // Выключен по умолчанию

    // Настройка ШИМ для приточного вентилятора
    ledcSetup(FAN_INLET_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(FAN_INLET_PIN, FAN_INLET_CHANNEL);
    ledcWrite(FAN_INLET_CHANNEL, 0); // Выключен по умолчанию

    // Настройка пина увлажнителя
    pinMode(STEAM_IN_PIN, OUTPUT);
    digitalWrite(STEAM_IN_PIN, LOW); // Выключен по умолчанию

    // Инициализация PID
    tempPID.SetMode(AUTOMATIC);
    tempPID.SetOutputLimits(0, MAX_PWM); // Диапазон ШИМ для нагревателя
    fanPID.SetMode(AUTOMATIC);
    fanPID.SetOutputLimits(0, MAX_PWM); // Диапазон ШИМ для вентилятора
}

// Обновление управления климатом
void updateClimateControl() {
    unsigned long currentTime = millis();
    if (currentTime - lastUpdateTime < UPDATE_INTERVAL) {
        return; // Обновляем не чаще, чем раз в секунду
    }

    // Обновление целевых значений из глобальных переменных
    tempSetpoint = currentTemperatura;
    fanSetpoint = currentTemperatura;

    // Чтение датчиков
    tempInput = temperature_1;  // Текущая температура
    fanInput = temperature_1;  // Используем ту же температуру для вентилятора

    // Регулирование температуры
    if (tempInput < (tempSetpoint - TEMP_TOLERANCE)) {
        // Температура ниже целевой - включаем нагреватель
        fanPID.SetMode(MANUAL);     // Выключаем PID вентилятора
        fanOutput = 0;              // Выключаем вентилятор
        tempPID.SetMode(AUTOMATIC); // Включаем PID нагревателя
        tempPID.Compute();          // Расчёт ШИМ для нагревателя
        ledcWrite(HITER_AIR_CHANNEL, (int)tempOutput >= MIN_PWM ? (int)tempOutput : 0);
        ledcWrite(FAN_INLET_CHANNEL, 0);
    } else if (tempInput > (tempSetpoint + TEMP_TOLERANCE)) {
        // Температура выше целевой - включаем приточный вентилятор
        tempPID.SetMode(MANUAL);    // Выключаем PID нагревателя
        tempOutput = 0;             // Выключаем нагреватель
        fanPID.SetMode(AUTOMATIC);  // Включаем PID вентилятора
        fanPID.Compute();           // Расчёт ШИМ для вентилятора
        ledcWrite(FAN_INLET_CHANNEL, (int)fanOutput >= MIN_PWM ? (int)fanOutput : 0);
        ledcWrite(HITER_AIR_CHANNEL, 0);
    } else {
        // Температура в пределах допуска - выключаем оба устройства
        tempPID.SetMode(MANUAL);
        fanPID.SetMode(MANUAL);
        tempOutput = 0;
        fanOutput = 0;
        ledcWrite(HITER_AIR_CHANNEL, 0);
        ledcWrite(FAN_INLET_CHANNEL, 0);
    }

    // Регулирование влажности
    if (humidity_1 < (currentHumidity - HUM_TOLERANCE)) {
        // Влажность ниже целевой - включаем увлажнитель
        digitalWrite(STEAM_IN_PIN, HIGH);
    } else if (humidity_1 > (currentHumidity + HUM_TOLERANCE)) {
        // Влажность выше целевой - вытесняем влажный воздух вентилятором
        digitalWrite(STEAM_IN_PIN, LOW);
        if (tempInput <= (tempSetpoint + TEMP_TOLERANCE)) {
            // Включаем вентилятор только если температура не превышает цель
            fanPID.SetMode(AUTOMATIC);
            fanPID.Compute();
            ledcWrite(FAN_INLET_CHANNEL, (int)fanOutput >= MIN_PWM ? (int)fanOutput : 0);
        }
    } else {
        // Влажность в пределах допуска - выключаем увлажнитель
        digitalWrite(STEAM_IN_PIN, LOW);
    }

    // Обновление глобальных переменных
    HITER_AIR = (int)tempOutput;
    FAN_INLET = (int)fanOutput;

    // Вывод в Serial Monitor
    Serial.print("Hiter PWM: "); Serial.print(HITER_AIR);
    Serial.print(" | Fan PWM: "); Serial.print(FAN_INLET);
    Serial.print(" | Steam State: "); Serial.print(digitalRead(STEAM_IN_PIN) ? "ON" : "OFF");
    Serial.print(" | temperature_1: "); Serial.print(temperature_1);
    Serial.print(" | humidity_1: "); Serial.print(humidity_1);
    Serial.print(" | Target Temp: "); Serial.print(currentTemperatura);
    Serial.print(" | Target Hum: "); Serial.println(currentHumidity);

    lastUpdateTime = currentTime;
}