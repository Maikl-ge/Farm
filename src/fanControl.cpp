#include <Arduino.h>
#include <fanControl.h>

// Константы и настройки
const int FAN_RACK_CHANNEL = 1;       // Канал PWM для вентилятора
const int FAN_RACK_PIN = 15;           // Пин вентилятора
const int PWM_FREQUENCY = 5000;       // Частота PWM
const int PWM_RESOLUTION = 10;        // Разрешение PWM (макс. 1023)
const int MAX_PWM = 1023;             // Максимальное значение PWM
const int MIN_PWM = 5;              // Минимальное значение PWM (для ограничения)
const int WIND_CHANGE_DELAY = 100;    // Задержка между изменениями PWM (мс)
const int WIND_CHANGE_STEP = 5;      // Максимальный шаг изменения PWM

// Глобальные переменные
int currentPwm = MIN_PWM;             // Текущее значение PWM
int pwmTarget = MAX_PWM;              // Целевое значение PWM

void setupFan() {
    // Настройка PWM для вентилятора
    ledcSetup(FAN_RACK_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION); 
    ledcAttachPin(FAN_RACK_PIN, FAN_RACK_CHANNEL);
    ledcWrite(FAN_RACK_CHANNEL, currentPwm); // Инициализация с минимальным значением PWM
}

void updateFanControl() {
    // Проверка и коррекция текущего значения PWM для плавного перехода к цели
    if (currentPwm < pwmTarget) {
        currentPwm += random(1, WIND_CHANGE_STEP); // Увеличиваем PWM случайным шагом
        if (currentPwm > pwmTarget) {
            currentPwm = pwmTarget;
        }
    } else if (currentPwm > pwmTarget) {
        currentPwm -= random(1, WIND_CHANGE_STEP); // Уменьшаем PWM случайным шагом
        if (currentPwm < pwmTarget) {
            currentPwm = pwmTarget;
        }
    }

    // Запись нового значения PWM
    ledcWrite(FAN_RACK_CHANNEL, currentPwm);

    // Если достигли цели, устанавливаем новую случайную цель
    if (currentPwm == pwmTarget) {
        pwmTarget = random(MIN_PWM, MAX_PWM); // Новое случайное значение в заданном диапазоне
    }
}
