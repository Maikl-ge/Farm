#include <Arduino.h>
#include <globals.h>
#include <pinout.h>
#include "menu.h"
#include "AccessPoint.h"
#include <WebSocketHandler.h>

// Константы времени нажатий (в циклах опроса)
const uint8_t SHORT_PRESS_THRESHOLD = 0; // Порог короткого нажатия
const uint8_t MEDIUM_PRESS_THRESHOLD = 3; // Порог среднего нажатия
const uint8_t LONG_PRESS_THRESHOLD = 27;   // Порог длинного нажатия
bool startButton = false;
bool stopButton = false;
void parceMessageFromServer(const String& messageFromServer);

// Структура для кнопок
struct Button {
    uint8_t pin;             // Номер пина кнопки
    bool currentState;       // Текущее состояние кнопки
    uint8_t holdCounter;     // Счётчик удержания кнопки
    bool released;           // Флаг отпускания кнопки
};

// Время последнего чтения кнопок
unsigned long lastButtonReadTime = 0;

// Константа для блокировки чтения кнопок (0,6 секунды)
const unsigned long BUTTON_READ_INTERVAL = 500;

// Инициализация кнопок
Button buttons[] = {
    {START_BUTTON_PIN, false, 0, false},
    {STOP_BUTTON_PIN, false, 0, false},
};

const uint8_t buttonCount = sizeof(buttons) / sizeof(Button);

// Переменные состояния
uint8_t buttonState = 0; // Байт состояния кнопок
// Определение переменных для мониторинга питающей сети
float power_monitor = 0;


// Установка пинов в режим входа
void initializeMenu() {
    for (uint8_t i = 0; i < buttonCount; i++) {
        pinMode(buttons[i].pin, INPUT);
    }
    pinMode(POWER_MONITOR_PIN, INPUT); // Установка пина мониторинга питающей сети в режим входа
}

// Функция обработки состояния кнопки
void processButton(Button &button, uint8_t buttonIndex) {
    // Чтение текущего состояния кнопки
    button.currentState = analogRead(button.pin) < 1000;

    if (button.currentState) { // Кнопка нажата
        button.holdCounter++;
        button.released = false; // Сброс флага отпускания
    } else { // Кнопка отпущена
        if (!button.released && button.holdCounter > 0) {
            // Фиксируем длительность нажатия
            if (button.holdCounter >= LONG_PRESS_THRESHOLD) {
                buttonState |= (1 << 5); // Длинное нажатие
            } else if (button.holdCounter >= MEDIUM_PRESS_THRESHOLD) {
                buttonState |= (1 << 4); // Среднее нажатие
            } else if (button.holdCounter >= SHORT_PRESS_THRESHOLD) {
                buttonState |= (1 << 3); // Короткое нажатие
            }

            buttonState |= (1 << buttonIndex); // Фиксируем кнопку
            button.holdCounter = 0; // Сбрасываем счётчик удержания
            button.released = true; // Устанавливаем флаг отпускания
        }
    }
}

// Функция обработки всех кнопок
void updateButtonState() {

    buttonState = 0; // Сброс состояния кнопок

    for (uint8_t i = 0; i < buttonCount; i++) {
        processButton(buttons[i], i);
    }

    // Обрвботка коротких нажатий
    if (buttonState == 0b00001001) {  // Короткое нажатие START 
        startButton = true;
        //Serial.println("Клавиатура: короткий START");
        //Serial.println("Клавиатура: короткий START");
    }
    if (buttonState == 0b00001010) {  // Короткое нажатие STOP
        stopButton = true;
        //Serial.println("Клавиатура: короткий STOP");
        //Serial.println("Клавиатура: короткий STOP");
    }
    if (buttonState == 0b00001011) {  // Короткое нажатие START + STOP
        Serial.println("Клавиатура: короткий START + STOP");
    }

    // Обработка средних нажатий
    if (buttonState == 0b00010001) {  // Среднее нажатие START
        startButton = true;
    }
    if (buttonState == 0b00010010) {  // Среднее нажатие STOP
        stopButton = true;
    }
    if (buttonState == 0b00010011) {  // Среднее нажатие START + STOP
        Serial.println("Остановка цикла роста по нажатию двух кнопок");
    }

    // Обработка длинных нажатий
    if (buttonState == 0b00100001) {  // Длинное нажатие START
        Serial.println("Клавиатура: длинный START");
    }
    if (buttonState == 0b00100010) {  // Длинное нажатие STOP 
        Serial.println("Клавиатура: длинный STOP");
    }
    if (buttonState == 0b00100011) {  // Длинное нажатие START + STOP
        Serial.println("Клавиатура: длинный START + STOP");
        //accessPoint.start();
    }
}

// Состояние байта buttonState при коротком нажатии
// 00001000	Короткое нажатие одной или комбинации кнопок
// 00001001	Короткое нажатие START
// 00001010	Короткое нажатие STOP
// 00001011	Короткое нажатие START + STOP


// Состояние байта buttonState при среднем нажатии
// 00010000	Среднее нажатие одной или комбинации кнопок
// 00010001	Среднее нажатие START
// 00010010	Среднее нажатие STOP
// 00010011	Среднее нажатие START + STOP

// Состояние байта buttonState при длинном нажатии
// 00100000	Длинное нажатие одной или комбинации кнопок
// 00100001	Длинное нажатие START
// 00100010	Длинное нажатие STOP
// 00100011	Длинное нажатие START + STOP