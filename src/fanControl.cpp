#include <Arduino.h>
#include <fanControl.h>
#include <pinout.h>
#include <globals.h>

// Константы и настройки
const int pwmCirculationChannel = 1;  // Канал PWM для вентилятора циркуляции (FAN_CIRC_PIN)
const int pwmVentilationChannel = 2;  // Канал PWM для приточного вентилятора (FAN_INLET_PIN)
const int pwmFrequency = 30000;        // Частота PWM
const int pwmResolution = 10;         // Разрешение PWM (макс. 1023)

const uint16_t MAX_PWM = 1023;        // Максимальное значение PWM (аппаратный предел)
const int MIN_CIRCULATION_PWM = 50;   // Минимальное значение PWM для вентилятора циркуляции
const int MIN_VENTILATION_PWM = 50;   // Минимальное значение PWM для приточного вентилятора
const int WIND_CHANGE_DELAY = 100;    // Задержка между изменениями PWM (мс)
const int WIND_CHANGE_STEP = 5;       // Максимальный шаг изменения PWM

// Глобальные переменные для вентилятора циркуляции (FAN_CIRC_PIN)
int circulationCurrentPwm = MIN_CIRCULATION_PWM;  // Текущее значение PWM для циркуляции
int circulationTargetPwm = MIN_CIRCULATION_PWM + 50; // Целевое значение PWM для циркуляции (начальное)

// Глобальные переменные для приточного вентилятора (FAN_INLET_PIN)
int ventilationCurrentPwm = MIN_VENTILATION_PWM;  // Текущее значение PWM для притока
int ventilationTargetPwm = MIN_VENTILATION_PWM + 50; // Целевое значение PWM для притока (начальное)

unsigned long lastWateringUpdateTime = 0;     // Время последнего обновления

void setupFan() {
    // Настройка PWM для вентилятора циркуляции (FAN_CIRC_PIN)
    ledcSetup(pwmCirculationChannel, pwmFrequency, pwmResolution);
    ledcAttachPin(FAN_CIRC_PIN, pwmCirculationChannel);
    ledcWrite(pwmCirculationChannel, circulationCurrentPwm); // Инициализация с минимальным значением PWM

    // Настройка PWM для приточного вентилятора (FAN_INLET_PIN)
    ledcSetup(pwmVentilationChannel, pwmFrequency, pwmResolution);
    ledcAttachPin(FAN_VENT_PIN, pwmVentilationChannel);
    ledcWrite(pwmVentilationChannel, ventilationCurrentPwm); // Инициализация с минимальным значением PWM
}

void updateFanControl() {
    unsigned long currentFanTime = millis();

    // Обновление вентиляторов с задержкой
    if (currentFanTime - lastWateringUpdateTime >= WIND_CHANGE_DELAY) {
        // Управление вентилятором циркуляции (FAN_CIRC_PIN) - имитация ветра
        int circMax = min(currentCirculation, MAX_PWM); // Ограничение максимума currentCirculation
        if (currentCirculation < MIN_CIRCULATION_PWM) {
            // Если currentCirculation < MIN_CIRCULATION_PWM, выключаем вентилятор
            circulationCurrentPwm = 0;
            circulationTargetPwm = 0;
            ledcWrite(pwmCirculationChannel, 0);
            FAN_CIRC = 0;
        } else {
            // Нормальная работа - имитация ветра
            if (circulationCurrentPwm < MIN_CIRCULATION_PWM) {
                circulationCurrentPwm = MIN_CIRCULATION_PWM; // Принудительно устанавливаем минимум
            }
            // Если цель некорректна (меньше или равна минимуму), устанавливаем новую
            if (circulationTargetPwm <= MIN_CIRCULATION_PWM) {
                circulationTargetPwm = random(MIN_CIRCULATION_PWM, circMax + 1);
            }
            if (circulationCurrentPwm < circulationTargetPwm) {
                circulationCurrentPwm += random(1, WIND_CHANGE_STEP + 1); // Увеличиваем PWM случайным шагом
                if (circulationCurrentPwm > circulationTargetPwm) {
                    circulationCurrentPwm = circulationTargetPwm;
                }
            } else if (circulationCurrentPwm > circulationTargetPwm) {
                circulationCurrentPwm -= random(1, WIND_CHANGE_STEP + 1); // Уменьшаем PWM случайным шагом
                if (circulationCurrentPwm < circulationTargetPwm) {
                    circulationCurrentPwm = circulationTargetPwm;
                }
            }
            circulationCurrentPwm = constrain(circulationCurrentPwm, MIN_CIRCULATION_PWM, circMax);
            ledcWrite(pwmCirculationChannel, circulationCurrentPwm);
            FAN_CIRC = circulationCurrentPwm;

            // Если достигли цели, устанавливаем новую случайную цель для циркуляции
            if (circulationCurrentPwm == circulationTargetPwm) {
                circulationTargetPwm = random(MIN_CIRCULATION_PWM, circMax + 1); // Новое случайное значение
            }
        }

        // Управление приточным вентилятором (FAN_VENT_PIN) - имитация бриза
        int ventMax = min(currentVentilation, MAX_PWM); // Ограничение максимума currentVentilation
        if (currentVentilation < MIN_VENTILATION_PWM) {
            // Если currentVentilation < MIN_VENTILATION_PWM, выключаем вентилятор
            ventilationCurrentPwm = 0;
            ventilationTargetPwm = 0;
            ledcWrite(pwmVentilationChannel, 0);
            FAN_VENT = 0;
        } else {
            // Нормальная работа - имитация бриза
            if (ventilationCurrentPwm < MIN_VENTILATION_PWM) {
                ventilationCurrentPwm = MIN_VENTILATION_PWM; // Принудительно устанавливаем минимум
            }
            // Если цель некорректна (меньше или равна минимуму), устанавливаем новую
            if (ventilationTargetPwm <= MIN_VENTILATION_PWM) {
                ventilationTargetPwm = random(MIN_VENTILATION_PWM, ventMax + 1);
            }
            if (ventilationCurrentPwm < ventilationTargetPwm) {
                ventilationCurrentPwm += random(1, WIND_CHANGE_STEP + 1); // Увеличиваем PWM случайным шагом
                if (ventilationCurrentPwm > ventilationTargetPwm) {
                    ventilationCurrentPwm = ventilationTargetPwm;
                }
            } else if (ventilationCurrentPwm > ventilationTargetPwm) {
                ventilationCurrentPwm -= random(1, WIND_CHANGE_STEP + 1); // Уменьшаем PWM случайным шагом
                if (ventilationCurrentPwm < ventilationTargetPwm) {
                    ventilationCurrentPwm = ventilationTargetPwm;
                }
            }
            ventilationCurrentPwm = constrain(ventilationCurrentPwm, MIN_VENTILATION_PWM, ventMax);
            ledcWrite(pwmVentilationChannel, ventilationCurrentPwm);
            FAN_VENT = ventilationCurrentPwm;

            // Если достигли цели, устанавливаем новую случайную цель для притока
            if (ventilationCurrentPwm == ventilationTargetPwm) {
                ventilationTargetPwm = random(MIN_VENTILATION_PWM, ventMax + 1); // Новое случайное значение
            }
        }
        lastWateringUpdateTime = currentFanTime;
    }
}