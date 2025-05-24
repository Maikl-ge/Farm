#include <Arduino.h>
#include "SensorsModule.h"
#include "pinout.h" // Подключаем Pinout.h
#include "globals.h" // Подключаем globals.h
#include <Wire.h> // Для работы с I2C
//#include <DallasTemperature.h> // Для работы с DS18B20
#include <OneWire.h> // Для работы с 1-Wire
#include <Adafruit_HTU21DF.h>
#include <PCF8574.h> // Для работы с I2C экспандером PCF8574T
#include <TimeModule.h>
#include <status.h>

// Адрес I2C экспандера PCF8574T
#define PCF8574_ADDRESS 0x27  // Адрес I2C экспандера PCF8574T проверен

// Создание объекта для I2C экспандера
PCF8574 pcf8574(PCF8574_ADDRESS);

// Определение переменных состояния датчиков уровня воды
bool max_osmo_level = false;
bool min_osmo_level = false;
bool max_water_level = false;
bool min_water_level = false;

// Определение переменных состояния датчиков Бокса
float temperatureInBox = 0.0;
float humidityInBox = 0.0;
bool HTU21D_OFF = false; // Флаг подключенного датчика HTU21D

// Создание объектов для каждого датчика HTU21D
Adafruit_HTU21DF HTU21D;

float water_temperature_osmo = 0.0;
float water_temperature_watering = 0.0;
float air_temperature_outdoor = 0.0;
float air_temperature_inlet = 0.0;

// Определение переменных для датчиков качества воды
float CO2 = 0.0;
float ph_osmo = 0.0;
float tds_osmo = 0.0;

// Глобальная переменная для хранения состояния PCF8574
uint8_t sensorState = 0;
OneWire ds(ONE_WIRE_BUS); // Создаем объект OneWire
// DeviceAddress sensorWateringAddress = {0x28, 0x8B, 0x63, 0x58, 0x00, 0x00, 0x00, 0x97};  // подключен
// DeviceAddress sensorOutdoorAddress = {0x28, 0x2F, 0x1E, 0x49, 0xF6, 0xE6, 0x3C, 0xBF};  // подключен

float readDS18B20Temperature(DeviceAddress sensorAddress);
void initializeSensor(DeviceAddress sensorAddress);

// Инициализация всех сенсоров
void initializeSensors() {

    initializeSensor(sensorWateringAddress);
    initializeSensor(sensorOutdoorAddress);
    Serial.println("All DS18B20 sensors initialized");

    // Инициализация I2C экспандера
    if (pcf8574.begin()) {
        Serial.println("PCF8574 initialized successfully");
    } else {
        Serial.println("Failed to initialize PCF8574");
    }

    // Инициализация датчиков температуры и влажности HTU21D
    if (!HTU21D.begin()) {
        HTU21D_OFF = false; // Датчик HTU21D найден
        Serial.println("Couldn't find HTU21D sensor");
    } else {
        HTU21D_OFF = true; // Датчик HTU21D не найден работаем по AHT10
        Serial.println("HTU21D sensor initialized");
    }
    // Инициализация датчика pH
//    pinMode(PH_SENSOR_PIN, INPUT);

// Назначение портов PCF8574 датчиков 
    max_water_level = false;
    min_water_level = false;
    max_osmo_level = false;
    min_osmo_level = false;
    power_monitor = false;
}

// Функция чтения байта состояния с PCF8574
uint8_t readPCF8574() {
    if (!pcf8574.begin()) {
        Serial.println("Failed to initialize PCF8574, assigning default state");
        sensorState = 0b11101000;
    } else {
        sensorState = pcf8574.read8();
        if (sensorState == 0xFF) {
            Serial.println("Failed to read from PCF8574, assigning default state");
            sensorState = 0b11101000;
        }
    }

    // Инвертируем логические значения, если требуется
    sensorState = ~sensorState;

    // Чтение состояния датчиков уровня воды
    max_osmo_level = (sensorState & 0b10000000) != 0; // 7 бит
    min_osmo_level = (sensorState & 0b01000000) != 0; // 6 бит
    max_water_level = (sensorState & 0b00100000) != 0; // 5 бит
    min_water_level = (sensorState & 0b00010000) != 0; // 4 бит
    // 3 бит всегда в 0, исключить из опроса
    //Serial.print("Sensor state: ");
    //Serial.println(sensorState, BIN);
    return sensorState;
}

// Чтение данных с датчика HTU21D
void readTempAndHum() {
    if(HTU21D_OFF) {
        temperatureInBox = HTU21D.readTemperature();
        humidityInBox = HTU21D.readHumidity();
        temperatureInBox = isnan(temperatureInBox) ? 5.404 : roundf(temperatureInBox * 100) / 100.0;
        humidityInBox = isnan(humidityInBox) ? 32.404 : roundf(humidityInBox * 10) / 10.0; 
        return; // Если датчик HTU21D не найден, выходим из функции
    }

    // Чтение с AHT10
    Wire.beginTransmission(0x38);
    Wire.write(0xAC);  // Запрос измерения
    Wire.write(0x33);
    Wire.write(0x00);
    Wire.endTransmission();
    delay(85); // Даташит требует 75+ мс ожидания

    Wire.requestFrom(0x38, 6);
    if (Wire.available() == 6) {
        uint8_t status = Wire.read();
        uint8_t byte1 = Wire.read();
        uint8_t byte2 = Wire.read();
        uint8_t byte3 = Wire.read();
        uint8_t byte4 = Wire.read();
        uint8_t byte5 = Wire.read();

        // Сбор данных влажности (20 бит)
        uint32_t rawHum = ((uint32_t)byte1 << 12) | ((uint32_t)byte2 << 4) | (byte3 >> 4);

        // Сбор данных температуры (20 бит)
        uint32_t rawTemp = ((uint32_t)(byte3 & 0x0F) << 16) | ((uint32_t)byte4 << 8) | byte5;

        humidityInBox = roundf(rawHum * 100.0 / 1048576.0 * 100.0) / 100.0;
        temperatureInBox = roundf((rawTemp * 200.0 / 1048576.0 - 50.0) * 100.0) / 100.0;
    } else {
        Serial.println("AHT10 read error: insufficient data");
        // Проверяем данные на NaN для определения потерянного датчика
        temperatureInBox = isnan(temperatureInBox) ? 5.404 : roundf(temperatureInBox * 100) / 100.0;
        humidityInBox = isnan(humidityInBox) ? 32.404 : roundf(humidityInBox * 10) / 10.0; 
  
    }
}

// Обновление состояния датчиков
void updateSensors() {
    readTempAndHum();
    readPCF8574(); 
    readAllDS18B20();   
    power_monitor = analogRead(POWER_MONITOR_PIN); // Обновление состояния мониторинга питающей сети
}

void readAllDS18B20() {

    float tempWatering = readDS18B20Temperature(sensorWateringAddress);
    float tempOutdoor  = readDS18B20Temperature(sensorOutdoorAddress);
    // Обработка шума и ошибок
        if (tempWatering <= -127.0 || tempWatering > 85.0 || tempWatering == 85.0) {
            Serial.println("Ошибка датчика полива, fallback");
            tempWatering = currentWaterTemperatura + 1.111;
        }
    
        if (tempOutdoor <= -127.0 || tempOutdoor > 85.0 || tempOutdoor == 85.0) {
            Serial.println("Ошибка уличного датчика, fallback");
            tempOutdoor = currentWaterTemperatura + 2.222;
        }
    // Обновляем все температуры
    water_temperature_osmo = tempWatering;
    water_temperature_watering = tempWatering;
    air_temperature_outdoor = tempOutdoor;
    air_temperature_inlet = tempOutdoor;
    printCurrentTime();
    // Выводим данные в Serial Monitor
    Serial.print("Time: ");
    Serial.print(CurrentTime);
    Serial.print(" Осталось " + String(longPhacse6 - totalMinutesElapsed) + " минут ");

    Serial.print("  Temperature: ");
    Serial.print(tempOutdoor);
    Serial.print(" °C  ");
    Serial.print(HITER_WATER);
    Serial.print("  ");
    Serial.print(tempWatering);
    Serial.print("  Temperature:");
    Serial.print(temperatureInBox);
    Serial.print("  Humidity: ");
    Serial.print(humidityInBox);
    Serial.print("  HITER_AIR: ");
    Serial.print(HITER_AIR);
    Serial.print("  STEAM_IN: ");
    Serial.print(STEAM_IN);
    Serial.print("  level: ");
    bool max_watering_level = digitalRead(WATERING_LEVEL_BOX_PIN); 
    Serial.println(max_watering_level);
    
}

float readDS18B20Temperature(DeviceAddress sensorAddress) {
    byte data[9];

    ds.reset();
    ds.select(sensorAddress);
    ds.write(0x44, 1);  // Старт измерения температуры
    delay(300);         // Ждем завершения (под 12 бит – до 750 мс)

    ds.reset();
    ds.select(sensorAddress);
    ds.write(0xBE);     // Чтение scratchpad

    for (int i = 0; i < 9; i++) {
        data[i] = ds.read();
    }

    if (OneWire::crc8(data, 8) != data[8]) {
        Serial.println("CRC check failed");
        return -127.0;  // Ошибка
    }

    int16_t raw = (data[1] << 8) | data[0];
    return (float)raw / 16.0;
}

// Инициализация (применимо ко всем сенсорам)
void initializeSensor(DeviceAddress sensorAddress) {
    ds.reset();
    ds.select(sensorAddress);
    ds.write(0x4E);  // Write Scratchpad
    ds.write(0x00);  // Th
    ds.write(0x00);  // Tl
    ds.write(0x3F);  // 10 бит (0.25 °C), можно заменить на 0x7F для 12 бит

    ds.reset();
    ds.select(sensorAddress);
    ds.write(0x48);  // Copy Scratchpad
    delay(20);
}

// Arduino D5 ---[Диод 1N4148]--> VCC (A3144)
//           |                   |
//           +---[4.7 кОм]---+5 В
//           |                   |
//           +----------------> OUT (A3144)
// Arduino GND ----------------> GND (A3144)

// const int hallPin = 5;  // Пин для питания и считывания сигнала

// void setup() {
//   Serial.begin(115200);  // Инициализация серийного порта
//   pinMode(hallPin, OUTPUT);  // Изначально пин как выход для подачи питания
//   digitalWrite(hallPin, HIGH);  // Подаём питание
// }

// void loop() {
//   // Переключаем пин в режим входа для считывания сигнала
//   pinMode(hallPin, INPUT_PULLUP);  // Включаем внутренний подтягивающий резистор
//   delay(10);  // Задержка для стабилизации сигнала
//   int hallState = digitalRead(hallPin);  // Читаем сигнал

//   // Возвращаем пин в режим выхода для подачи питания
//   pinMode(hallPin, OUTPUT);
//   digitalWrite(hallPin, HIGH);  // Восстанавливаем питание

//   if (hallState == LOW) {
//     Serial.println("Поле обнаружено");
//   } else {
//     Serial.println("Поле не обнаружено");
//   }

//   delay(100);  // Задержка для упрощения наблюдения
// }      