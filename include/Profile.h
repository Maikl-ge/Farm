#ifndef SETTINGS_MODULE_H
#define SETTINGS_MODULE_H

// Инициализация модуля настроек
void initializeSettingsModule();
void saveUint16ToEEPROM();
void EEPROMRead();
void saveSettingsToEEPROM();
uint16_t readUint16FromEEPROM(int address);
void serializeSettings();
void fetchAndSaveSettings();

#endif // SETTINGS_MODULE_H
