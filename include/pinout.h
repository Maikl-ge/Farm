#ifndef PINOUT_H
#define PINOUT_H

// Пины цифрового ввода/вывода (GPIO)
#define LED_BUILTIN  // Светодиод на плате (GPIO2, нога 10)

// Входы от датчиков фермы  20 ----------------------------

// Пины для подключения I2C
#define SDA_PIN 21  // Шина данных I2C (SDA) (GPIO21, нога 33) SHT20
#define SCL_PIN 22  // Шина тактирования I2C (SCL) (GPIO22, нога 36)

// 1-Wire для DS18B20
#define ONE_WIRE_BUS 4  // Шина 1-Wire для четырех DS18B20 (GPIO4, нога 26)

// Кнопки
#define BUTTON1_PIN 16 // Первая кнопка (GPIO16, нога 8)
#define BUTTON2_PIN 17 // Вторая кнопка (GPIO17, нога 9)
#define BUTTON3_PIN 20 // Третья кнопка (GPIO20, нога 40)

// Датчики pH и TDS
// #define PH_SENSOR_PIN 36  // Аналоговый вход для pH (GPIO36, нога 3)
// #define TDS_SENSOR_PIN 37 // Аналоговый вход для TDS (GPIO37, нога 4)

// Датчики Холла (уровень воды)
#define HALL_SENSOR1_PIN 34 // Первый датчик Холла (GPIO34, нога 5)
#define HALL_SENSOR2_PIN 35 // Второй датчик Холла (GPIO35, нога 6)
#define HALL_SENSOR3_PIN 36 // Третий датчик Холла (GPIO36, нога 3) 
#define HALL_SENSOR4_PIN 39 // Четвертый датчик Холла (GPIO39, нога 4) 

// Контроль питающей сети
#define POWER_MONITOR_PIN 3 // Аналоговый вход для мониторинга сети (GPIO3, нога 34)

// Пины для SD-карты (SPI)
#define SD_MOSI 23 // MOSI для SD-карты (GPIO23, нога 37)
#define SD_MISO 19 // MISO для SD-карты (GPIO19, нога 31)
#define SD_SCK 18  // SCK для SD-карты (GPIO18, нога 30)
#define SD_CS 5    // Выбор устройства (CS) для SD-карты (GPIO5, нога 29)

// Выходы для управления фермой ----------------------------
// Пины для управления устройствами PWM
#define LIGHT_PIN  2  // Свет (PWM) (GPIO02, нога 24)
#define FAN_RACK_PIN 26     // Циркуляция внутри 1 и 2 полки (PWM) (GPIO26, нога 10)
#define FAN_SHELF_PIN 27    // Циркуляция внутри 3 и 4 полки (PWM) (GPIO27, нога 11
#define FAN_CIRC_PIN 15     // Циркуляция внутри камеры (PWM) (GPI5, нога 23)
#define FAN_INLET 12        // Подача воздуха из вне (PWM) (GPIO12, нога 13)
#define HITER_AIR_PIN 13    // Обогрев камеры (PWM) (GPIO13, нога 15)
#define HITER_WATER_PIN 14  // Нагрев воды (PWM) (GPIO14, нога 12)

// Пины для управления клапанами ON/OFF
#define OSMOS_ON_PIN  // Подача очищенной воды (ON/OFF) (GPIO17, нога 9)
#define PUMP_1_PIN    // Полив (ON/OFF) (GPIO32, нога 7)
#define PUMP_2_PIN    // Перемешивание (ON/OFF) (GPIO33, нога 8)
#define WATER_OUT_PIN   // Слив (ON/OFF) (GPIO4, нога 24)
#define STEAM_IN_PIN    // Парогенератор (ON/OFF) (GPIO5, нога 29)

// Шаговый двигатель (Step, Dir, Enable)
#define STEP_PIN     // Шаговый двигатель (GPIO2, нога 10)
#define DIR_PIN     // Направление (GPIO15, нога 23)
#define ENABLE_PIN   // Включение (GPIO0, нога 21)

// Используемые для встроенной памяти
// GPIO6-11: Рекомендуется не использовать для ввода/вывода, так как они подключены к встроенной флеш-памяти.
// #define GPIO0   0    // GPIO0 (можно использовать для кнопки BOOT) (GPIO0, нога 25)

#endif // PINOUT_H

// 3,3 volt   нога 1 ~~~~~~~~~~
// Reset      нога 2 ~~~~~~~~~~
// GPIO36     нога 3 --> HALL_SENSOR_3
// GPIO39     нога 4 --> HALL_SENSOR_4
// GPIO34     нога 5 --> HALL_SENSOR_1
// GPIO35     нога 6 --> HALL_SENSOR_2
// GPIO32     нога 7
// GPIO33     нога 8
// GPIO25     нога 9
// GPIO26     нога 10  <-- FAN_RACK_PIN
// GPIO27     нога 11  <-- FAN_SHELF_PIN
// GPIO14     нога 12  <-- HITER_WATER_PIN
// GPIO12     нога 13  <-- FAN_INLET
// GNND       нога 14 ~~~~~~~~~~
// GPIO13     нога 15  <-- HITER_AIR_PIN
// GPIO09     нога 16   FLASH D2
// GPIO10     нога 17   FLASH D3
// GPIO11     нога 18   FLASH CMD
// Vin 5 volt нога 19 ~~~~~~~~~~
// GPIO06     нога 20   FLASH SCK
// GPIO07     нога 21   FLASH D0
// GPIO08     нога 22   FLASH D1
// GPIO15     нога 23  <-- FAN_CIRC_PIN
// GPIO02     нога 24  <-- LIGHT_PIN
// GPIO00     нога 25
// GPIO04     нога 26 --> ONE_WIRE_BUS
// GPIO16     нога 27
// GPIO17     нога 28
// GPIO05     нога 29 --> SD_CS
// GPIO18     нога 30 --> SD_SCK
// GPIO19     нога 31 --> SD_MISO
// GND        нога 32 ~~~~~~~~~~
// GPIO21     нога 33 --> SDA_PIN
// GPIO03     нога 34 --> POWER_MONITOR_PIN
// GPIO01     нога 35
// GPIO22     нога 36 --> SCL_PIN
// GPIO23     нога 37 --> SD_MOSI
// GND        нога 38 ~~~~~~~~~~