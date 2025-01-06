#include <Arduino.h>
#include "SensorsModule.h"
#include "Pinout.h" // Подключаем Pinout.h
#include <Wire.h> // Для работы с I2C
#include <DallasTemperature.h> // Для работы с DS18B20
#include <OneWire.h> // Для работы с 1-Wire
#include <Adafruit_HTU21DF.h>
#include <PCF8574.h> // Для работы с I2C экспандером PCF8574T

#define TERMO_SENSOR_1_ADDRESS 0x40  // Адрес 1го датчика температуры и влажности
#define TERMO_SENSOR_2_ADDRESS 0x41  // Адрес 2го датчика температуры и влажности
#define TERMO_SENSOR_3_ADDRESS 0x42  // Адрес 3го датчика температуры и влажности
#define TERMO_SENSOR_4_ADDRESS 0x43  // Адрес 4го датчика температуры и влажности
#define TERMO_SENSOR_5_ADDRESS 0x44  // Адрес 5го датчика температуры и влажности

// Настройки для DS18B20
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20(&oneWire);

// Адрес I2C экспандера PCF8574T
#define PCF8574_ADDRESS 0x20

// Создание объекта для I2C экспандера
PCF8574 pcf8574(PCF8574_ADDRESS);

// Определение переменных состояния кнопок
volatile bool startButtonPressed = 0b0;
volatile bool stopButtonPressed = 0b0;
volatile bool modeButtonPressed = 0b0;

// Определение переменных состояния датчиков уровня воды
bool max_osmo_level = 0b0;
bool min_osmo_level = 0b0;
bool max_water_level = 0b0;
bool min_water_level = 0b0;

// Определение переменных состояния датчиков HDC1080
float temperature_1 = 0.0;
float humidity_1 = 0.0;
float temperature_2 = 0.0;
float humidity_2 = 0.0;
float temperature_3 = 0.0;
float humidity_3 = 0.0;
float temperature_4 = 0.0;
float humidity_4 = 0.0;
float temperature_5 = 0.0;
float humidity_5 = 0.0;

// Создание объектов для каждого датчика HTU21D
Adafruit_HTU21DF htu1;
Adafruit_HTU21DF htu2;
Adafruit_HTU21DF htu3;
Adafruit_HTU21DF htu4;
Adafruit_HTU21DF htu5;

float water_temperature_osmo = 0.0;
float water_temperature_watering = 0.0;
float air_temperature_outdoor = 0.0;
float air_temperature_inlet = 0.0;

// Определение переменных для датчиков качества воды
float ph_osmo = 0.0;
float tds_osmo = 0.0;

// Определение переменных для мониторинга питающей сети
bool power_monitor = 0b0;

// Аппаратный таймер
hw_timer_t *timer = NULL;

// Глобальная переменная для хранения состояния PCF8574
uint8_t state = 0;

// Инициализация всех сенсоров
void initializeSensors() {
    Wire.begin();
    ds18b20.begin();

    // Инициализация I2C экспандера
    if (pcf8574.begin()) {
        Serial.println("PCF8574 initialized successfully");
    } else {
        Serial.println("Failed to initialize PCF8574");
    }

    // Инициализация датчиков температуры и влажности HTU21D
    if (!htu1.begin()) {
        Serial.println("Couldn't find HTU21D sensor 1");
    } else {
        Serial.println("HTU21D sensor 1 initialized");
    }

    if (!htu2.begin()) {
        Serial.println("Couldn't find HTU21D sensor 2");
    } else {
        Serial.println("HTU21D sensor 2 initialized");
    }

    if (!htu3.begin()) {
        Serial.println("Couldn't find HTU21D sensor 3");
    } else {
        Serial.println("HTU21D sensor 3 initialized");
    }

    if (!htu4.begin()) {
        Serial.println("Couldn't find HTU21D sensor 4");
    } else {
        Serial.println("HTU21D sensor 4 initialized");
    }

    if (!htu5.begin()) {
        Serial.println("Couldn't find HTU21D sensor 5");
    } else {
        Serial.println("HTU21D sensor 5 initialized");
    }

    // Инициализация датчика pH
//    pinMode(PH_SENSOR_PIN, INPUT);

// Установка периода опроса датчиков PCF8574
    max_water_level = 0;
    min_water_level = 0;
    max_osmo_level = 0;
    min_osmo_level = 0;
    power_monitor = 0;
    startButtonPressed = 0;
    stopButtonPressed = 0;
    modeButtonPressed = 0;
}

// Функция чтения байта состояния с PCF8574
uint8_t readPCF8574() {
    if (!pcf8574.begin()) {
        //Serial.println("Failed to read PCF8574, assigning default state");
        state = 0b10100001;
    } else {
        state = pcf8574.read8();
    }
    // Чтение состояния датчиков уровня воды
    max_osmo_level = state & 0b10000000; // 7 бит
    min_osmo_level = state & 0b01000000; // 6 бит
    max_water_level = state & 0b00100000; // 5 бит
    min_water_level = state & 0b00010000; // 4 бит
    // Чтение состояния кнопок
    startButtonPressed = state & 0b00001000; // 3 бит
    modeButtonPressed = state & 0b00000100; // 2 бит
    stopButtonPressed = state & 0b00000010; // 1 бит
    // Чтение состояния мониторинга питающей сети
    power_monitor = state & 0b00000001; // 0 бит

    return state;
}

// Чтение данных с датчика HTU21D
SensorData readHTU21D(Adafruit_HTU21DF &htu) {
    SensorData data = {0.0, 0.0};

    float temp = htu.readTemperature();
    float hum = htu.readHumidity();

    // Проверяем данные на NaN
    data.temperature = isnan(temp) ? -0.7 : temp;
    data.humidity = isnan(hum) ? -0.7 : hum;

    return data;
}

// Чтение данных с пяти датчиков HTU21D
void readAllHTU21D() {
    SensorData data;

    data = readHTU21D(htu1);
    temperature_1 = data.temperature;
    humidity_1 = data.humidity;

    data = readHTU21D(htu2);
    temperature_2 = data.temperature;
    humidity_2 = data.humidity;

    data = readHTU21D(htu3);
    temperature_3 = data.temperature;
    humidity_3 = data.humidity;

    data = readHTU21D(htu4);
    temperature_4 = data.temperature;
    humidity_4 = data.humidity;

    data = readHTU21D(htu5);
    temperature_5 = data.temperature;
    humidity_5 = data.humidity;
}

// Чтение данных с четырех датчиков DS18B20
void readAllDS18B20() {
    ds18b20.requestTemperatures();

    for (uint8_t i = 0; i < 4; i++) {
        float temperature = ds18b20.getTempCByIndex(i);

        // Присваивание считанных температур переменным
        switch (i) {
            case 0:
                water_temperature_osmo = temperature;
                break;
            case 1:
                water_temperature_watering = temperature;
                break;
            case 2:
                air_temperature_outdoor = temperature;
                break;
            case 3:
                air_temperature_inlet = temperature;
                break;
        }
    }
}

// Обновление состояния датчиков
void updateSensors() {
    readAllHTU21D();
    readAllDS18B20();
    uint8_t sensorState = readPCF8574();
    max_water_level = (sensorState & 0x01) ? 1 : 0;
    min_water_level = (sensorState & 0x02) ? 1 : 0;
//    processButtonStates();
    // Serial.println("Опрос всех сенсоров");
    Serial.print(max_osmo_level);
    Serial.print(" ");
    Serial.print(min_osmo_level);
    Serial.print(" ");
    Serial.print(max_water_level);
    Serial.print(" ");
    Serial.print(min_water_level);
    Serial.print(" - ");
    Serial.print(temperature_1);
    Serial.print(" ");
    Serial.print(humidity_1);
    Serial.print(" ");
    Serial.print(temperature_2);
    Serial.print(" ");
    Serial.println(humidity_2);
}
