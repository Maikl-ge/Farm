# insert_test_data.py
import asyncio
from datetime import datetime, timedelta
from database import DatabaseManager  # Импортируем класс DatabaseManager из database.py

async def main():
    db_manager = DatabaseManager()
    await db_manager.init_pools()

    # Генерация и сохранение тестовых данных
    start_time = datetime.now()
    for i in range(3000):
        timestamp = (start_time + timedelta(minutes=i)).strftime("%Y-%m-%d %H:%M:%S")
        test_data = {
            "DF": int(timestamp.split()[0].replace("-", "")),  # Преобразование даты в целое число
            "TF": int(timestamp.split()[1].replace(":", "")),  # Преобразование времени в целое число
            "start_Button": 0,
            "stop_Button": 0,
            "mode_Button": 0,
            "max_osmo_level": 100,
            "min_osmo_level": 50,
            "max_water_level": 80,
            "min_water_level": 30,
            "T1": 22.5 + i % 10,
            "H1": 65.0 + i % 5,
            "T2": 19.0 + i % 10,
            "H2": 70.0 + i % 5,
            "T3": 24.0 + i % 10,
            "H3": 67.0 + i % 5,
            "T4": 20.0 + i % 10,
            "H4": 68.0 + i % 5,
            "T5": 21.0 + i % 10,
            "H5": 72.0 + i % 5,
            "WTO": 20.0 + i % 5,
            "WTW": 25.0 + i % 5,
            "ATO": 18.0 + i % 5,
            "ATI": 26.0 + i % 5,
            "ph": 6.7 + i % 2,
            "tds": 300 + i % 50,
            "pm": 1
        }
        success = await db_manager.save_sensor_data(test_data, timestamp)
        if not success:
            print(f"Failed to save test data for timestamp {timestamp}")

    await db_manager.close_pools()

# Запуск примера
if __name__ == "__main__":
    asyncio.run(main())