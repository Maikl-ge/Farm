#include <Arduino.h>
#include "SensorsModule.h"
#include "Pinout.h" // Подключаем Pinout.h

// Инициализация кнопок
void initializeButtons() {
    pinMode(BUTTON1_PIN, INPUT_PULLUP);
    pinMode(BUTTON2_PIN, INPUT_PULLUP);
    pinMode(BUTTON3_PIN, INPUT_PULLUP);
}

// Опрос кнопок
ButtonState readButtons() {
    ButtonState state;
    state.button1Pressed = digitalRead(BUTTON1_PIN) == LOW;
    state.button2Pressed = digitalRead(BUTTON2_PIN) == LOW;
    state.button3Pressed = digitalRead(BUTTON3_PIN) == LOW;
    return state;
}
