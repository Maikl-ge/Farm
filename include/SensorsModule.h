#ifndef SENSORS_MODULE_H
#define SENSORS_MODULE_H

#include <Arduino.h>
#include "Pinout.h" // Подключаем Pinout.h

// Структура для хранения состояния кнопок
struct ButtonState {
    bool button1Pressed;
    bool button2Pressed;
    bool button3Pressed;
};

// Функции модуля
void initializeButtons(); // Инициализация кнопок
ButtonState readButtons(); // Опрос кнопок

#endif // SENSORS_MODULE_H
