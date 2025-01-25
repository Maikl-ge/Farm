#include <Arduino.h>
#include <pinout.h>
#include <SPI.h>
#include <SdFat.h>
#include <globals.h>
#include <sdios.h>
#include <CurrentProfile.h>
#include <TimeModule.h>
#include <DataSender.h>
#include <SDCard.h>

SdFat sd;
SdFile file;
SdFile root;

// Очередь для хранения сообщений
long enqueueIndex = 0;
long dequeueIndex = 0;

// Имя файла для хранения индексов 
const char* indexFilename = "index.txt";
void setupCDcard();

// Инициализация SD-карты
void setupCDcard() {
    // Настройка SPI с указанием кастомных пинов
    SPI.begin(SD_SCK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);
    // Инициализация SD-карты
    if (!sd.begin(SD_CS_PIN, SD_SCK_MHZ(18))) {
        Serial.println("Не удалось инициализировать SD-карту!");
        return;
    }
    // Вывод информации о карте
    Serial.print("SD-карта успешно инициализирована. Card type: ");
    switch (sd.card()->type()) {
        case SD_CARD_TYPE_SD1:
            Serial.println("SD1");
         break;
        case SD_CARD_TYPE_SD2:
            Serial.println("SD2");
        break;
        case SD_CARD_TYPE_SDHC:
            Serial.println("SDHC");
        break;
    default:
      Serial.println("Unknown");
    }

    // Чтение индексов из файла
    if (sd.exists(indexFilename)) {
        SdFile indexFile;
        if (indexFile.open(indexFilename, O_READ)) {
            char buffer[32];
            int bytesRead = indexFile.read(buffer, sizeof(buffer) - 1);
            buffer[bytesRead] = '\0'; // Завершаем строку нулевым символом
            sscanf(buffer, "%d %d", &enqueueIndex, &dequeueIndex);
            indexFile.close();
        }
    }
    Serial.printf("Индексы: enqueueIndex = %d, dequeueIndex = %d\n", enqueueIndex, dequeueIndex);
    counterFilesSD();
}

void counterFilesSD() {
    unsigned long timeCounting = millis();
    // Открываем корневой каталог
    if (!root.open("/")) {
        Serial.println("Ошибка открытия корневого каталога.");
        return;
    }

    // Переменная для хранения количества файлов
    int fileCount = 0;

    // Перебираем все файлы в корневом каталоге и считаем их количество
    SdFile file;
    while (file.openNext(&root, O_RDONLY)) {
        if (!file.isDir()) {
            fileCount++;
        }
    file.close();
    }

    // Закрываем корневой каталог
    root.close();

    // Выводим количество файлов
    Serial.print("Количество файлов на SD карте: ");
    Serial.print(fileCount);
    Serial.println("  " + (String(millis() - timeCounting) + " ms"));
}

void saveIndices(SdFat& sd, int enqueueIndex, int dequeueIndex) {
    SdFile indexFile;
    if (indexFile.open(indexFilename, O_WRITE | O_CREAT | O_TRUNC)) {
        char buffer[32];
        int bytesWritten = snprintf(buffer, sizeof(buffer), "%d %d", enqueueIndex, dequeueIndex);
        indexFile.write(buffer, bytesWritten);
        indexFile.close();
    }
}

void enqueue(SdFs& sd, const String& item) {
    char filename[20];
    sprintf(filename, "queue_%ld.txt", enqueueIndex); 

    SdFile file;
    if (file.open(filename, O_WRITE | O_CREAT | O_TRUNC)) {
        file.println(item);
        file.close();
        Serial.println("Элемент добавлен в очередь: " + String(enqueueIndex));
        enqueueIndex++;
        saveIndices(sd, enqueueIndex, dequeueIndex);
    } else {
        Serial.println("Ошибка при добавлении элемента в очередь.");
    }
}

void dequeue() {
    if (dequeueIndex >= enqueueIndex) {
        if(dequeueIndex > 0) {
        Serial.println("Индексы очереди сброшены.");            
        dequeueIndex = enqueueIndex = 0;
        saveIndices(sd, enqueueIndex, dequeueIndex);
        }     
        Serial.println("Очередь совсем пуста.");          
        return;
    }

    char filename[20];
    sprintf(filename, "queue_%ld.txt", dequeueIndex); 

    SdFile file;
    if (file.open(filename, O_READ)) {
        char buffer[1024];
        int bytesRead = file.read(buffer, sizeof(buffer) - 1);
        buffer[bytesRead] = '\0';
        Serial.println("Элемент из очереди: " + String(dequeueIndex));
        sendMessageOK = true;
        enqueueASK = "send";
        //Serial.print("Очередь ");
        transmitionTime = millis();  // Время начала передачи
        sendWebSocketMessage(buffer);
        // Удаление файла
        unsigned long dequeueWait = millis();
        while(millis() - dequeueWait < 4000) {  // ждем 4000 мс
            if (enqueueASK == "sendOk") {
            //Serial.println("enqueueASK " + enqueueASK);  
            if (sd.remove(filename)) {
                Serial.println(String(filename) + "Файл успешно удален.");
            } else {
                Serial.println("Ошибка при удалении файла " + String(filename));
            }
            file.close();
            dequeueIndex++;
            //Serial.println("Индекс оставшихся " + String(dequeueIndex));
            saveIndices(sd, enqueueIndex, dequeueIndex);
                sendMessageOK = false;
                enqueueASK = "empty";
            }
            delay(1);  // Небольшая задержка чтобы не нагружать процессор
        }
    } else {
        enqueueASK = "empty";
        Serial.println("Ошибка при извлечении элемента из очереди.");
    }
    enqueueASK = "empty";
    Serial.println("Индексы сохранены " + String(enqueueIndex) + " " + String(dequeueIndex));
}
