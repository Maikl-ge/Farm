#ifndef SDCARD_H
#define SDCARD_H

#include <Arduino.h>

// Подключи FS и SPIFFS раньше SdFat, чтобы избежать конфликта макросов
#include <FS.h>
#include <SPIFFS.h>

// Удали/переопредели конфликтующие макросы из FS перед SdFat
#ifdef FILE_READ
  #undef FILE_READ
#endif

#ifdef FILE_WRITE
  #undef FILE_WRITE
#endif

#include <SdFat.h>  // Теперь безопасно

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
