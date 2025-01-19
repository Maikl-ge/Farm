#ifndef MENU_H
#define MENU_H

#include <Arduino.h>
#include <globals.h>
#include <pinout.h>

// Объявление глобальных переменных
extern uint8_t buttonState;
extern float power_monitor;

// Объявление функций
void initializeMenu();
void updateButtonState();
void updatePowerMonitor();

#endif // MENU_H