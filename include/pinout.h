#ifndef PINOUT_H
#define PINOUT_H

// Пины цифрового ввода/вывода (GPIO)
#define LED_BUILTIN 2 // Светодиод на плате (GPIO2, нога 10)

// Пины для подключения I2C
#define SDA_PIN 21  // Шина данных I2C (SDA) (GPIO21, нога 40)
#define SCL_PIN 22  // Шина тактирования I2C (SCL) (GPIO22, нога 41)

// RTC DS1302
#define DS1302_CLK 14 // Пин для тактовой линии (CLK) RTC DS1302 (GPIO14, нога 23)
#define DS1302_IO  27 // Пин для данных (IO) RTC DS1302 (GPIO27, нога 19)
#define DS1302_CE  26 // Пин для выбора RTC DS1302 (CE) (GPIO26, нога 18)

// 1-Wire для DS18B20
#define ONE_WIRE_BUS 4  // Шина 1-Wire для трех DS18B20 (GPIO4, нога 24)

// Кнопки
#define BUTTON1_PIN 16 // Первая кнопка (GPIO16, нога 8)
#define BUTTON2_PIN 17 // Вторая кнопка (GPIO17, нога 9)
#define BUTTON3_PIN 18 // Третья кнопка (GPIO18, нога 12)

// Датчики pH и TDS
#define PH_SENSOR_PIN ADC1_CH0  // Аналоговый вход для pH (GPIO36, нога 3)
#define TDS_SENSOR_PIN ADC1_CH1 // Аналоговый вход для TDS (GPIO37, нога 4)

// Датчики Холла (уровень воды)
#define HALL_SENSOR1_PIN 32 // Первый датчик Холла (GPIO32, нога 1)
#define HALL_SENSOR2_PIN 33 // Второй датчик Холла (GPIO33, нога 2)
#define HALL_SENSOR3_PIN 25 // Третий датчик Холла (GPIO25, нога 6)
#define HALL_SENSOR4_PIN 26 // Четвертый датчик Холла (GPIO26, нога 7)

// Контроль питающей сети
#define POWER_MONITOR_PIN ADC1_CH2 // Аналоговый вход для мониторинга сети (GPIO38, нога 5)

// Пины для SD-карты (SPI)
#define SD_MOSI SPI_MOSI // MOSI для SD-карты (GPIO23, нога 11)
#define SD_MISO SPI_MISO // MISO для SD-карты (GPIO19, нога 13)
#define SD_SCK SPI_SCK   // SCK для SD-карты (GPIO18, нога 12)
#define SD_CS 5          // Выбор устройства (CS) для SD-карты (GPIO5, нога 29)

// Аналоговые входы (ADC)
#define ADC1_CH0 36  // Аналоговый вход 1 (GPIO36, нога 3)
#define ADC1_CH1 37  // Аналоговый вход 2 (GPIO37, нога 4)
#define ADC1_CH2 38  // Аналоговый вход 3 (GPIO38, нога 5)
#define ADC1_CH3 39  // Аналоговый вход 4 (GPIO39, нога 38)
#define ADC2_CH0 34  // Аналоговый вход 5 (GPIO34, нога 36)
#define ADC2_CH1 35  // Аналоговый вход 6 (GPIO35, нога 37)
#define ADC2_CH2 32  // Аналоговый вход 7 (GPIO32, нога 1)
#define ADC2_CH3 33  // Аналоговый вход 8 (GPIO33, нога 2)

// PWM выходы (можно настраивать для разных пинов)
#define PWM1_PIN 25  // PWM-выход 1 (GPIO25, нога 6)
#define PWM2_PIN 26  // PWM-выход 2 (GPIO26, нога 7)
#define PWM3_PIN 27  // PWM-выход 3 (GPIO27, нога 19)
#define PWM4_PIN 14  // PWM-выход 4 (GPIO14, нога 23)

// Дополнительные GPIO
#define GPIO0   0    // GPIO0 (можно использовать для кнопки BOOT) (GPIO0, нога 21)
#define GPIO4   4    // GPIO4 (часто используется для дополнительных функций) (GPIO4, нога 24)
#define GPIO5   5    // GPIO5 (можно использовать для дополнительных функций) (GPIO5, нога 29)
#define GPIO12  12   // GPIO12 (часто используется для SPI или PWM) (GPIO12, нога 17)
#define GPIO13  13   // GPIO13 (часто используется для SPI или PWM) (GPIO13, нога 16)
#define GPIO14  14   // GPIO14 (часто используется для SPI) (GPIO14, нога 23)
#define GPIO15  15   // GPIO15 (часто используется для SPI, CS) (GPIO15, нога 22)
#define GPIO16  16   // GPIO16 (можно использовать для дополнительных функций) (GPIO16, нога 8)
#define GPIO17  17   // GPIO17 (можно использовать для дополнительных функций) (GPIO17, нога 9)
#define GPIO18  18   // GPIO18 (часто используется для SPI, SCK) (GPIO18, нога 12)
#define GPIO19  19   // GPIO19 (часто используется для SPI, MISO) (GPIO19, нога 13)
#define GPIO20  20   // GPIO20 (можно использовать для I2C, SCL) (GPIO20, резерв)

// Используемые для встроенной памяти
// GPIO6-11: Рекомендуется не использовать для ввода/вывода, так как они подключены к встроенной флеш-памяти.

#endif // PINOUT_H
