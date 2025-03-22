// status.h
#ifndef STATUS_H
#define STATUS_H

#include <Arduino.h>
#include "TimeModule.h"

void currentStatusFarm();
uint16_t addTimes(uint16_t currentTime, uint16_t startTime);
uint16_t subtractTimes(uint16_t currentTime, uint16_t startTime);

#endif // STATUS_H