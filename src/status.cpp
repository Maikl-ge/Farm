// status.cpp
#include <Arduino.h>
#include "status.h"
#include "TimeModule.h"
#include "Profile.h"
#include <EEPROM.h>
#include <globals.h>

uint16_t totalMinutesElapsed = 0;
uint16_t longPhacse1 = 0;
uint16_t longPhacse2 = 0;
uint16_t longPhacse3 = 0;
uint16_t longPhacse4 = 0;
uint16_t longPhacse5 = 0;
uint16_t longPhacse6 = 0;
uint16_t wateringInterval = 0;
uint16_t wateringDraining = 0;
int phaseToGrowe = -1;


// Определение текущего статуса фермы
void CurrentStatusFarm() {
    EEPROMRead();  // Чтение Параметров из EEPROM
    if(statusFarm == "Work" || statusFarm == "Stop") {
        CheckStatusFarm();   // Проверка фазы роста
    }
    if(phaseToGrowe == 1) {
        wateringInterval = PHASE1_WATERING;
        wateringDraining = PHASE1_DRAINING;
        currentCirculation = PHASE1_CIRCULATION;
        currentVentilation = PHASE1_VENTILATION;
        currentRotation = PHASE1_ROTATION;
        Serial.println("Текущая фаза - 01");
    } 
    else if (phaseToGrowe == 2) { 
        wateringInterval = PHASE2_WATERING; 
        wateringDraining = PHASE2_DRAINING;  
        currentCirculation = PHASE2_CIRCULATION;
        currentVentilation = PHASE2_VENTILATION; 
        currentRotation = PHASE2_ROTATION;    
        Serial.println("Текущая фаза - 02");
    } 
    else if (phaseToGrowe == 3) { 
        wateringInterval = PHASE3_WATERING;
        wateringDraining = PHASE3_DRAINING;
        currentCirculation = PHASE3_CIRCULATION;
        currentVentilation = PHASE3_VENTILATION; 
        currentRotation = PHASE3_ROTATION;
        Serial.println("Текущая фаза - 03");  
    } 
    else if (phaseToGrowe == 4) {
        wateringInterval = PHASE4_WATERING; 
        wateringDraining = PHASE4_DRAINING;
        currentCirculation = PHASE4_CIRCULATION;
        currentVentilation = PHASE4_VENTILATION;
        currentRotation = PHASE4_ROTATION; 
        Serial.println("Текущая фаза - 04");  
    } 
    else if (phaseToGrowe == 5) {
        wateringInterval = PHASE5_WATERING; 
        wateringDraining = PHASE5_DRAINING;
        currentCirculation = PHASE5_CIRCULATION;
        currentVentilation = PHASE5_VENTILATION;
        currentRotation = PHASE5_ROTATION; 
        Serial.println("Текущая фаза - 05");  
    } 
    else if (phaseToGrowe == 6) {
        wateringInterval = PHASE6_WATERING;
        wateringDraining = PHASE6_DRAINING;
        currentCirculation = PHASE6_CIRCULATION;
        currentVentilation = PHASE6_VENTILATION;  
        currentRotation = PHASE6_ROTATION;
        Serial.println("Текущая фаза - 06");  
    } 
    else {
        Serial.println("Ошибка: Фаза не определена.");
    } 
}

// Функция для чтения двух байт из EEPROM и объединения их в uint16_t
uint16_t readFromEEPROM(int address) {
    return EEPROM.read(address) | (EEPROM.read(address + 1) << 8);
}
void saveStringToEEPROM(int address, String& statusFarm);

// Определение текущего статуса фермы
void CheckStatusFarm() {
    printCurrentTime();
    longPhacse1 = PHASE1_DURATION * 60;
    longPhacse2 = PHASE1_DURATION * 60 + PHASE2_DURATION * 60;
    longPhacse3 = PHASE1_DURATION * 60 + PHASE2_DURATION * 60 + PHASE3_DURATION * 60;
    longPhacse4 = PHASE1_DURATION * 60 + PHASE2_DURATION * 60 + PHASE3_DURATION * 60 + PHASE4_DURATION * 60;
    longPhacse5 = PHASE1_DURATION * 60 + PHASE2_DURATION * 60 + PHASE3_DURATION * 60 + PHASE4_DURATION * 60 + PHASE5_DURATION * 60;
    longPhacse6 = PHASE1_DURATION * 60 + PHASE2_DURATION * 60 + PHASE3_DURATION * 60 + PHASE4_DURATION * 60 + PHASE5_DURATION * 60 + PHASE6_DURATION * 60;
    
    currentTimeInMinutes = getCurrentTimeInMinutes();  // Получаем текущее время в минутах
    uint16_t currentDate = getCurrentDate(); // Получаем текущую дату в днях с 1 января 1970
    GROWE_MODE_DATE = readFromEEPROM(EEPROM_GROWE_MODE_DATE_ADDRESS);
    GROWE_MODE_TIME = readFromEEPROM(EEPROM_GROWE_MODE_TIME_ADDRESS);


    uint16_t daysElapsed = currentDate - GROWE_MODE_DATE;
    if(daysElapsed == 0) {
        totalMinutesElapsed = (currentTimeInMinutes - GROWE_MODE_TIME);
    } 
    else if (daysElapsed == 1) {  
        totalMinutesElapsed = currentTimeInMinutes + (1440 - GROWE_MODE_TIME);   
    } 
    else {
        totalMinutesElapsed = ((daysElapsed -1) * 1440) + currentTimeInMinutes + (1440 - GROWE_MODE_TIME);
    }

    checkPhaseToGrowe();  // Проверка фазы роста
}

uint16_t convertTimeToMinutes(uint16_t time) {
    uint8_t hours = time / 100;
    uint8_t minutes = time % 100;
    return hours * 60 + minutes;
}

void checkPhaseToGrowe() {
    Serial.println("Полное время выращивания       " + String(longPhacse6) + " минут или " + String(longPhacse6 / 60) + " часов");
    Serial.println("Прошло время со старта         " + String(totalMinutesElapsed) + " минут"); 
    Serial.println("Осталось времени до завершения " + String(longPhacse6 - totalMinutesElapsed) + " минут или " + String((longPhacse6 - totalMinutesElapsed) / 60) + " часов");

    uint16_t phaseEndTimes[] = {
        longPhacse1,
        longPhacse2,
        longPhacse3,
        longPhacse4,
        longPhacse5,
        longPhacse6
    };

    const char* phaseMessages[] = {
        "Текущая фаза 1 ",
        "Текущая фаза 2 ",
        "Текущая фаза 3 ",
        "Текущая фаза 4 ",
        "Текущая фаза 5 ",
        "Текущая фаза 6 "
    };

    // Если выращивание завершилось
    if ((longPhacse6 - totalMinutesElapsed) == 0 || ((longPhacse6 - totalMinutesElapsed) + 1) == 0) {
        statusFarm = "End";
        saveStringToEEPROM(EEPROM_STATUS_BOX_ADDRESS, statusFarm);
        EEPROM.commit();
        Serial.println("Выращивание завершено. Статус фермы: " + statusFarm);
    }

    // Обнуляем phaseToGrowe перед проверкой
    phaseToGrowe = -1;

    for (int i = 0; i < 6; i++) {
        if (totalMinutesElapsed <= phaseEndTimes[i]) { 
            phaseToGrowe = i + 1; // Начинаем фазы с 1, а не 0
            break;
        }
    }
    if (phaseToGrowe == -1) {
        Serial.println("Ошибка: totalMinutesElapsed больше всех фаз! Фаза не определена.");
    }
}