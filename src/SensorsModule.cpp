#include <Arduino.h>
#include "SensorsModule.h"
#include "Pinout.h" // Подключаем Pinout.h
#include <Wire.h> // Для работы с I2C
#include <DallasTemperature.h> // Для работы с DS18B20
#include <OneWire.h> // Для работы с 1-Wire
#include <Adafruit_HTU21DF.h>

#define TERMO_SENSOR_1_ADDRESS 0x40  // Адрес 1го датчика температуры и влажности
#define TERMO_SENSOR_2_ADDRESS 0x41  // Адрес 2го датчика температуры и влажности
#define TERMO_SENSOR_3_ADDRESS 0x42  // Адрес 3го датчика температуры и влажности
#define TERMO_SENSOR_4_ADDRESS 0x43  // Адрес 4го датчика температуры и влажности
#define TERMO_SENSOR_5_ADDRESS 0x44  // Адрес 5го датчика температуры и влажности

// Настройки для DS18B20
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20(&oneWire);

// Определение переменных состояния кнопок
volatile bool startButtonPressed = false;
volatile bool stopButtonPressed = false;
volatile bool modeButtonPressed = false;

// Определение переменных состояния датчиков уровня воды
bool max_osmo_level = false;
bool min_osmo_level = false;
bool max_water_level = false;
bool min_water_level = false;

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
bool power_monitor = false;

// Аппаратный таймер
hw_timer_t *timer = NULL;

// Обработчик таймера
void IRAM_ATTR handleTimer() {
    if (digitalRead(BUTTON1_PIN) == LOW) startButtonPressed = true;
    if (digitalRead(BUTTON2_PIN) == LOW) modeButtonPressed = true;
    if (digitalRead(BUTTON3_PIN) == LOW) stopButtonPressed = true;
}

// Инициализация всех сенсоров
void initializeSensors() {
    Wire.begin();
    ds18b20.begin();

    // Инициализация кнопок
    pinMode(BUTTON1_PIN, INPUT_PULLUP);
    pinMode(BUTTON2_PIN, INPUT_PULLUP);
    pinMode(BUTTON3_PIN, INPUT_PULLUP);

    // Настройка аппаратного таймера
    timer = timerBegin(0, 80, true); // Timer 0, прескалер 80 (1 мс для 80 МГц)
    timerAttachInterrupt(timer, &handleTimer, true);
    timerAlarmWrite(timer, 30000, true); // Срабатывание каждые 1000 мкс (1 мс)
    timerAlarmEnable(timer);

    // Инициализация датчиков Холла
    pinMode(HALL_SENSOR1_PIN, INPUT_PULLUP);
    pinMode(HALL_SENSOR2_PIN, INPUT_PULLUP);
    pinMode(HALL_SENSOR3_PIN, INPUT_PULLUP);
    pinMode(HALL_SENSOR4_PIN, INPUT_PULLUP);

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
}

// Опрос кнопок
void updateButtonState() {
    readHallSensors();

    // if (modeButtonPressed) {
    //     modeButtonPressed = false; // Сброс флага
    //     Serial.println("Mode button pressed");
    // }
    if (modeButtonPressed && startButtonPressed) {
        modeButtonPressed = false; // Сброс флага
        startButtonPressed = false; // Сброс флага
        Serial.println("START");
    }   
    if (modeButtonPressed && stopButtonPressed) {
        modeButtonPressed = false; // Сброс флага
        stopButtonPressed = false; // Сброс флага
        Serial.println("STOP");
    }  
    if (startButtonPressed) {
        startButtonPressed = false; // Сброс флага
        Serial.println("Start button pressed");
    }
    if (stopButtonPressed) {
        stopButtonPressed = false; // Сброс флага
        Serial.println("Stop button pressed");
    }  
}

// Чтение состояния датчиков уровня воды
void readHallSensors() {
    max_osmo_level = !digitalRead(HALL_SENSOR1_PIN); // A3144: LOW = магнит обнаружен
    min_osmo_level = !digitalRead(HALL_SENSOR2_PIN); // A3144: LOW = магнит обнаружен
    max_water_level = !digitalRead(HALL_SENSOR3_PIN); // A3144: LOW = магнит обнаружен
    min_water_level = !digitalRead(HALL_SENSOR4_PIN); // A3144: LOW = магнит обнаружен
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
                //data.temperature = isnan(temp) ? -0.7 : temp;
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
    readHallSensors();
    readAllHTU21D();
    readAllDS18B20();
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
