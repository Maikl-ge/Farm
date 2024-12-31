#include "SensorsModule.h"

// Переменная для отслеживания времени последнего опроса
static unsigned long lastSensorReadTime = 0;

void initializeSensors() {
    // Настраиваем пины датчиков
    pinMode(DIGITAL_SENSOR_PIN, INPUT);
    pinMode(ANALOG_SENSOR_PIN, INPUT);

    Serial.println("Датчики инициализированы.");
}

SensorData readSensors() {
    SensorData data;

    // Проверяем, прошло ли достаточно времени с последнего опроса
    if (millis() - lastSensorReadTime >= SENSOR_READ_INTERVAL) {
        lastSensorReadTime = millis();

        // Считываем данные с аналогового датчика
        data.analogValue = analogRead(ANALOG_SENSOR_PIN);

        // Считываем состояние цифрового датчика
        data.digitalState = digitalRead(DIGITAL_SENSOR_PIN);

        // Вывод данных для отладки
        Serial.printf("Analog Value: %d\n", data.analogValue);
        Serial.printf("Digital State: %s\n", data.digitalState ? "HIGH" : "LOW");
    }

    return data;
}
