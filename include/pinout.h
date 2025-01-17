#ifndef PINOUT_H
#define PINOUT_H

// Входы от датчиков фермы  20 ----------------------------

// Пины для подключения I2C
#define SDA_PIN 21  // Шина данных I2C (SDA) (GPIO21, нога 33) SHT20
#define SCL_PIN 22  // Шина тактирования I2C (SCL) (GPIO22, нога 36)
// Кнопки 
// Датчики Холла (уровень воды)
// Контроль питающей сети

// 1-Wire для DS18B20
#define ONE_WIRE_BUS 4  // Шина 1-Wire для четырех DS18B20 (GPIO4, нога 26)

// Пины для SD-карты (SPI)
#define SD_MOSI 23 // MOSI для SD-карты (GPIO23, нога 37)
#define SD_MISO 19 // MISO для SD-карты (GPIO19, нога 31)
#define SD_SCK 18  // SCK для SD-карты (GPIO18, нога 30)
#define SD_CS 5    // Выбор устройства (CS) для SD-карты (GPIO5, нога 29)

// Выходы для управления фермой ----------------------------
// Пины для управления устройствами PWM
#define LIGHT_PIN  2  // Свет (PWM) (GPIO02, нога 24)
#define FAN_RACK_PIN 15     // Циркуляция внутри 1 и 2 полки (PWM) (GPIO15, нога 23)
#define FAN_SHELF_PIN 17    // Циркуляция внутри 3 и 4 полки (PWM) (GPIO17, нога 28)
#define FAN_CIRC_PIN 16     // Циркуляция внутри камеры (PWM) (GPIO16, нога 27)
#define FAN_INLET 12        // Подача воздуха из вне (PWM) (GPIO12, нога 13)
#define HITER_AIR_PIN 13    // Обогрев камеры (PWM) (GPIO13, нога 15)
#define HITER_WATER_PIN 14  // Нагрев воды (PWM) (GPIO14, нога 12)
#define FAN_OPTION_PIN 25   //Опциональный вентилятор (GPIO25, нога 9)

// Пины для управления нагрузками ON/OFF
#define OSMOS_ON_PIN 32 // Подача очищенной воды (ON/OFF) (GPIO32, нога 7)
#define PUMP_WATERING_PIN 33   // Полив (ON/OFF) (GPIO33, нога 8)
#define PUMP_TRANSFER_PIN 26   // Подача в бак полива osmo воды (ON/OFF) (GPIO26, нога 10)
#define WATER_OUT_PIN 27 // Слив (ON/OFF) (GPIO27, нога 11)
#define STEAM_IN_PIN 3   // Парогенератор (ON/OFF) (GPIO3, нога 34)

// Шаговый двигатель (Step, Dir, Enable)
#define STEP_PIN 1   // Шаговый двигатель (GPIO1, нога 35)
#define DIR_PIN 0   // Направление (GPIO0, нога 25)
#define ENABLE_PIN   // Включение (GPIO0, нога )

// Используемые для встроенной памяти
// GPIO6-11: Рекомендуется не использовать для ввода/вывода, так как они подключены к встроенной флеш-памяти.
// #define GPIO0   0    // GPIO0 (можно использовать для кнопки BOOT) (GPIO0, нога 25)

#endif // PINOUT_H

// 3,3 volt   нога 1 ~~~~~~~~~~
// Reset      нога 2 ~~~~~~~~~~
// GPIO36     нога 3 
// GPIO39     нога 4 
// GPIO34     нога 5 
// GPIO35     нога 6 
// GPIO32     нога 7  <-- OSMOS_ON_PIN
// GPIO33     нога 8  <-- PUMP_1_PIN
// GPIO25     нога 9  <-- FAN_OPTION_PIN
// GPIO26     нога 10  <-- PUMP_TRANSFER_PIN  
// GPIO27     нога 11  <-- WATER_OUT_PIN  
// GPIO14     нога 12  <-- HITER_WATER_PIN
// GPIO12     нога 13  <-- FAN_INLET      // Подача воздуха из вне (PWM) (FAN_INLET)    
// GNND       нога 14 ~~~~~~~~~~
// GPIO13     нога 15  <-- HITER_AIR_PIN  // Обогрев камеры (PWM) (HITER_AIR_PIN)   
// GPIO09     нога 16   FLASH D2
// GPIO10     нога 17   FLASH D3
// GPIO11     нога 18   FLASH CMD
// Vin 5 volt нога 19 ~~~~~~~~~~
// GPIO06     нога 20   FLASH SCK
// GPIO07     нога 21   FLASH D0
// GPIO08     нога 22   FLASH D1
// GPIO15     нога 23  <-- FAN_RACK_PIN   // Циркуляция внутри 1 и 2 полки (PWM) (FAN_RACK_PIN)
// GPIO02     нога 24  <-- LIGHT_PIN      // Свет (PWM) (LIGHT_PIN)
// GPIO00     нога 25  <-- DIR_PIN        // Направление (DIR_PIN)
// GPIO04     нога 26 --> ONE_WIRE_BUS    // 1-Wire для DS18B20
// GPIO16     нога 27  <-- FAN_CIRC_PIN   // Циркуляция внутри камеры (PWM) (FAN_CIRC_PIN)
// GPIO17     нога 28  <-- FAN_SHELF_PIN  // Циркуляция внутри 3 и 4 полки (PWM) (FAN_SHELF_PIN)
// GPIO05     нога 29 --> SD_CS           // Выбор устройства (CS) для SD-карты 
// GPIO18     нога 30 --> SD_SCK          // Шина тактирования I2C (SCL) (SD_SCK)
// GPIO19     нога 31 --> SD_MISO         // Шина данных I2C (SDA) (SD_MISO)
// GND        нога 32 ~~~~~~~~~~
// GPIO21     нога 33 --> SDA_PIN         // Шина данных I2C 
// GPIO03     нога 34  <-- STEAM_IN_PIN   // Парогенератор (ON/OFF) (STEAM_IN_PIN)
// GPIO01     нога 35 --> STEP_PIN        // Шаговый двигатель (Step) (STEP_PIN)
// GPIO22     нога 36 --> SCL_PIN         // Шина тактирования I2C 
// GPIO23     нога 37 --> SD_MOSI         // MOSI для SD-карты (SD_MOSI)
// GND        нога 38 ~~~~~~~~~~