// status.cpp
#include <Arduino.h>
#include "status.h"
#include "TimeModule.h"
#include "Profile.h"
#include <EEPROM.h>
#include <globals.h>
#include <DataSender.h>
#include <TimeModule.h>

uint16_t totalMinutesElapsed = 0;
uint16_t longPhacse = 0;
uint16_t longPhacse1 = 0;
uint16_t longPhacse2 = 0;
uint16_t longPhacse3 = 0;
uint16_t longPhacse4 = 0;
uint16_t longPhacse5 = 0;
uint16_t longPhacse6 = 0;
uint16_t longPhacseEnd = 0;
uint16_t wateringInterval = 0;
uint16_t wateringDraining = 0;
int phaseToGrowe = -1;
bool pintStatusFarm = false;

void breakTime();

// Определение текущего статуса фермы
void CurrentStatusFarm() {

    if(statusFarm == "End") {
        return; 
    }
    
    //EEPROMRead();  // Чтение Параметров из EEPROM
    if(statusFarm == "Work" || statusFarm == "Pause"  || statusFarm == "Stop" || statusFarm == "Ready" && statusFarm != "End") {
        CheckStatusFarm();   // Проверка фазы роста
    }
    if(phaseToGrowe == 1) {
        currentlongPhacse = PHASE1_DURATION;
        wateringInterval = PHASE1_WATERING;
        wateringDraining = PHASE1_DRAINING;
        currentCirculation = PHASE1_CIRCULATION;
        currentVentilation = PHASE1_VENTILATION;
        currentRotation = PHASE1_ROTATION;
        currentTemperatura = PHASE1_TEMP / 10.0;
        currentHumidity = PHASE1_HUMIDITY / 10.0;
        currentWaterTemperatura = PHASE1_WATER_TEMP / 10;
        currentLight = PHASE1_LIGHT;
        currentPhase = "Soak";  // Замачивание
        //Serial.println("Текущая фаза - 01  " + String(currentPhase) + "  " + String(statusFarm));
    } 
    else if (phaseToGrowe == 2) { 
        currentlongPhacse = PHASE2_DURATION;
        wateringInterval = PHASE2_WATERING; 
        wateringDraining = PHASE2_DRAINING;  
        currentCirculation = PHASE2_CIRCULATION;
        currentVentilation = PHASE2_VENTILATION; 
        currentRotation = PHASE2_ROTATION; 
        currentTemperatura = PHASE2_TEMP / 10.0;
        currentHumidity = PHASE2_HUMIDITY / 10.0;  
        currentWaterTemperatura = PHASE2_WATER_TEMP /10;
        currentLight = PHASE2_LIGHT;
        currentPhase = "Germ";  // Рост
        //Serial.println("Текущая фаза - 02  " + String(currentPhase) + "  " + String(statusFarm));  
    } 
    else if (phaseToGrowe == 3) { 
        currentlongPhacse = PHASE3_DURATION;
        wateringInterval = PHASE3_WATERING;
        wateringDraining = PHASE3_DRAINING;
        currentCirculation = PHASE3_CIRCULATION;
        currentVentilation = PHASE3_VENTILATION; 
        currentRotation = PHASE3_ROTATION;
        currentTemperatura = PHASE3_TEMP / 10.0;
        currentHumidity = PHASE3_HUMIDITY / 10.0;  
        currentWaterTemperatura = PHASE3_WATER_TEMP / 10;
        currentLight = PHASE3_LIGHT;
        currentPhase = "Act";  // Рост
        //Serial.println("Текущая фаза - 03  " + String(currentPhase) + "  " + String(statusFarm)); 
    } 
    else if (phaseToGrowe == 4) {
        currentlongPhacse = PHASE4_DURATION;
        wateringInterval = PHASE4_WATERING; 
        wateringDraining = PHASE4_DRAINING;
        currentCirculation = PHASE4_CIRCULATION;
        currentVentilation = PHASE4_VENTILATION;
        currentRotation = PHASE4_ROTATION; 
        currentTemperatura = PHASE4_TEMP / 10.0;
        currentHumidity = PHASE4_HUMIDITY / 10.0;  
        currentWaterTemperatura = PHASE4_WATER_TEMP / 10;
        currentLight = PHASE4_LIGHT;
        currentPhase = "Early";  // Рост
        //Serial.println("Текущая фаза - 04  " + String(currentPhase) + "  " + String(statusFarm));  
    } 
    else if (phaseToGrowe == 5) {
        currentlongPhacse = PHASE5_DURATION;
        wateringInterval = PHASE5_WATERING; 
        wateringDraining = PHASE5_DRAINING;
        currentCirculation = PHASE5_CIRCULATION;
        currentVentilation = PHASE5_VENTILATION;
        currentRotation = PHASE5_ROTATION; 
        currentTemperatura = PHASE5_TEMP / 10.0;
        currentHumidity = PHASE5_HUMIDITY / 10.0;  
        currentWaterTemperatura = PHASE5_WATER_TEMP / 10;
        currentLight = PHASE5_LIGHT;
        currentPhase = "Grow";  // Рост
        //Serial.println("Текущая фаза - 05  " + String(currentPhase) + "  " + String(statusFarm));  
    } 
    else if (phaseToGrowe == 6) {
        currentlongPhacse = PHASE6_DURATION;
        wateringInterval = PHASE6_WATERING;
        wateringDraining = PHASE6_DRAINING;
        currentCirculation = PHASE6_CIRCULATION;
        currentVentilation = PHASE6_VENTILATION;  
        currentRotation = PHASE6_ROTATION;
        currentTemperatura = PHASE6_TEMP / 10.0;
        currentHumidity = PHASE6_HUMIDITY / 10.0;  
        currentWaterTemperatura = PHASE6_WATER_TEMP / 10;
        currentLight = PHASE6_LIGHT;
        currentPhase = "Finish";  // Рост
        //Serial.println("Текущая фаза - 06  " + String(currentPhase) + "  " + String(statusFarm));  
    }  
    else if (phaseToGrowe == 7) {  
        statusFarm = "End";
        Serial.println("Остановленно.");    
    }
    else {
        EEPROMRead();  // Чтение Параметров из EEPROM
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
    longPhacseEnd = longPhacse6 + 1;
    
    if(statusFarm == "Work" || statusFarm == "Pause" ) {        
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
    }

    checkPhaseToGrowe();  // Проверка фазы роста    
}

void checkPhaseToGrowe() {
    uint8_t hours = totalMinutesElapsed / 60;
    uint8_t minutes = totalMinutesElapsed % 60;

    if(totalMinutesElapsed <= longPhacse1 && totalMinutesElapsed >= 0) {
        phaseToGrowe = 1;
        currentPhase = "Soak"; 
        Serial.println("Текущая фаза - " + String(currentPhase) + "  " + String(statusFarm));  
    }

    if(totalMinutesElapsed <= longPhacse2 && totalMinutesElapsed > longPhacse1) {
        phaseToGrowe = 2;
        currentPhase = "Germ"; 
        Serial.println("Текущая фаза - " + String(currentPhase) + "  " + String(statusFarm)); 
    }

    if(totalMinutesElapsed <= longPhacse3 && totalMinutesElapsed > longPhacse2) {
        phaseToGrowe = 3;
        currentPhase = "Act"; 
        Serial.println("Текущая фаза - " + String(currentPhase) + "  " + String(statusFarm)); 

    }

    if(totalMinutesElapsed <= longPhacse4 && totalMinutesElapsed > longPhacse3) {
        phaseToGrowe = 4;
        currentPhase = "Early"; 
        Serial.println("Текущая фаза - " + String(currentPhase) + "  " + String(statusFarm));  
    }

    if(totalMinutesElapsed <= longPhacse5 && totalMinutesElapsed > longPhacse4) {
        phaseToGrowe = 5;
        currentPhase = "Grow"; 
        Serial.println("Текущая фаза - " + String(currentPhase) + "  " + String(statusFarm));  

    }

    if(totalMinutesElapsed <= longPhacse6 && totalMinutesElapsed > longPhacse5) {
        phaseToGrowe = 6;
        currentPhase = "Finish"; 
        Serial.println("Текущая фаза - " + String(currentPhase) + "  " + String(statusFarm)); 
    }

    if(totalMinutesElapsed >= longPhacse6 || totalMinutesElapsed >= longPhacseEnd) {
        phaseToGrowe = 7;
        currentPhase = "End"; 
        statusFarm = "End";
        Serial.println("Текущая фаза - " + String(currentPhase) + "  " + String(statusFarm));  
        saveStringToEEPROM(EEPROM_STATUS_BOX_ADDRESS, statusFarm);
        EEPROM.commit();
        esp_restart();
        Serial.println("Цикл роста завершился успешно. Ферма остановлена - "  + statusFarm);
        Serial.println("Время завершения цикла роста: " + String(GROWE_MODE_TIME));
    }

    if(pintStatusFarm == true) {
        Serial.println("Полное время выращивания       " + String(longPhacse6) + " минут или " + String(longPhacse6 / 60) + ":" + String(totalMinutesElapsed % 60 ));
        Serial.println("Прошло время со старта         " + String(totalMinutesElapsed) + " минут или " + String(hours) + ":" + String(minutes));
        Serial.println("Осталось времени до завершения " + String(longPhacse6 - totalMinutesElapsed) + " минут или " + String((longPhacse6 - totalMinutesElapsed) / 60) + " часов");
        pintStatusFarm == false;
    }
}