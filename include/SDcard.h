#ifndef SDCARD_H
#define SDCARD_H

#include <Arduino.h>
#include <SdFat.h>

// Объявление глобальных переменных
extern SdFat sd;
extern SdFile file;
extern SdFile root;
extern long enqueueIndex;
extern long dequeueIndex;

// Объявление функций
void dequeue();
void setupCDcard();
void initQueue();
void saveMessageToSDCard(const String& message);
void counterFilesSD();
void saveIndices(SdFat& sd, int enqueueIndex, int dequeueIndex);
void enqueue(SdFs& sd, const String& item);

#endif // SDCARD_H