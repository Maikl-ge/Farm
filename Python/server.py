import asyncio
import websockets
import json
from datetime import datetime
import asyncpg
from aiohttp import web

# Строка подключения к PostgreSQL
pg_dsn = 'postgresql://CytiFarm:Farm@localhost:5432/sensor_data'

# Создание объекта приложения
app = web.Application()

# Асинхронная обработка WebSocket-соединений
async def handle_connection(websocket, path):
    print(f"Клиент подключен по пути: {path}")

    # Подключение к базе данных PostgreSQL
    conn = await asyncpg.connect(dsn=pg_dsn)

    try:
        async for message in websocket:
            timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')  # Получаем текущую метку времени
            print(f"{timestamp} - Получено сообщение: {message}")

            # Отправляем подтверждение сразу
            await websocket.send("Ok:200")

            # Обработка сообщения выполняется асинхронно
            asyncio.create_task(process_message(message, conn, timestamp))
    except websockets.exceptions.ConnectionClosed as e:
        print(f"Клиент отключился: {e}")
    finally:
        await conn.close()

# Функция для асинхронной обработки сообщения
async def process_message(message, conn, timestamp):
    try:
        data = json.loads(message)  # Парсим JSON
        print(f"{timestamp} - Распарсенные данные:")

        # Извлекаем данные из JSON и заменяем None на допустимые значения
        def safe_get(data, key, default=0):
            return data.get(key) if data.get(key) is not None else default

        current_date = safe_get(data, "DF")
        current_time = safe_get(data, "TF")
        start_button = safe_get(data, "start_Button")
        stop_button = safe_get(data, "stop_Button")
        mode_button = safe_get(data, "mode_Button")
        max_osmo_level = safe_get(data, "max_osmo_level")
        min_osmo_level = safe_get(data, "min_osmo_level")
        max_water_level = safe_get(data, "max_water_level")
        min_water_level = safe_get(data, "min_water_level")
        temperature_1 = safe_get(data, "T1")
        humidity_1 = safe_get(data, "H1")
        temperature_2 = safe_get(data, "T2")
        humidity_2 = safe_get(data, "H2")
        temperature_3 = safe_get(data, "T3")
        humidity_3 = safe_get(data, "H3")
        temperature_4 = safe_get(data, "T4")
        humidity_4 = safe_get(data, "H4")
        temperature_5 = safe_get(data, "T5")
        humidity_5 = safe_get(data, "H5")
        water_temperature_osmo = safe_get(data, "WTO")
        water_temperature_watering = safe_get(data, "WTW")
        air_temperature_outdoor = safe_get(data, "ATO")
        air_temperature_inlet = safe_get(data, "ATI")
        ph_osmo = safe_get(data, "ph")
        tds_osmo = safe_get(data, "tds")
        power_monitor = safe_get(data, "pm")

        # Сохраняем данные в PostgreSQL
        await conn.execute('''
            INSERT INTO sensor_data(
                timestamp, "current_date", "current_time", start_button, stop_button, mode_button,
                max_osmo_level, min_osmo_level, max_water_level, min_water_level,
                temperature_1, humidity_1, temperature_2, humidity_2, temperature_3, humidity_3,
                temperature_4, humidity_4, temperature_5, humidity_5,
                water_temperature_osmo, water_temperature_watering,
                air_temperature_outdoor, air_temperature_inlet, ph_osmo, tds_osmo, power_monitor
            ) VALUES($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13, $14, $15, $16, $17, $18, $19, $20, $21, $22, $23, $24, $25, $26, $27)
        ''', timestamp, current_date, current_time, start_button, stop_button, mode_button,
             max_osmo_level, min_osmo_level, max_water_level, min_water_level,
             temperature_1, humidity_1, temperature_2, humidity_2, temperature_3, humidity_3,
             temperature_4, humidity_4, temperature_5, humidity_5,
             water_temperature_osmo, water_temperature_watering,
             air_temperature_outdoor, air_temperature_inlet, ph_osmo, tds_osmo, power_monitor)
        
        print(f"{timestamp} - Данные сохранены в PostgreSQL")
    except json.JSONDecodeError as e:
        print(f"{timestamp} - Ошибка при парсинге JSON: {e}")


    except websockets.exceptions.ConnectionClosed as e:
        print(f"Клиент отключился: {e}")
    finally:
        await conn.close()

# Обработчик для отображения данных через HTTP
async def show_data(request):
    # Подключение к базе данных PostgreSQL
    conn = await asyncpg.connect(dsn=pg_dsn)
    # Получаем все сохраненные данные
    records = await conn.fetch('SELECT * FROM sensor_data')
    await conn.close()

    if records:
        # Формируем HTML-ответ
        html_content = "<h1>Сохраненные данные</h1><table border='1'><tr>"
        headers = ["id", "timestamp", "current_date", "current_time", "start_button", "stop_button", "mode_button",
                   "max_osmo_level", "min_osmo_level", "max_water_level", "min_water_level",
                   "temperature_1", "humidity_1", "temperature_2", "humidity_2", "temperature_3", "humidity_3",
                   "temperature_4", "humidity_4", "temperature_5", "humidity_5",
                   "water_temperature_osmo", "water_temperature_watering",
                   "air_temperature_outdoor", "air_temperature_inlet", "ph_osmo", "tds_osmo", "power_monitor"]
        # Заголовки таблицы
        html_content += ''.join([f"<th>{header}</th>" for header in headers])
        html_content += "</tr>"
        # Данные таблицы
        for record in records:
            html_content += "<tr>"
            html_content += ''.join([f"<td>{record[header]}</td>" for header in headers])
            html_content += "</tr>"
        html_content += "</table>"
    else:
        html_content = "<h1>Нет сохраненных данных.</h1>"

    return web.Response(text=html_content, content_type='text/html')

async def main():
    port = 5001  # Порт для WebSocket-сервера
    http_port = 8080  # Порт для HTTP-сервера
    print(f"Сервер запущен на порту {port}...")
    
    # Запуск WebSocket-сервера
    websocket_server = await websockets.serve(handle_connection, "0.0.0.0", port)

    # Запуск HTTP-сервера
    web_server = web.AppRunner(app)
    await web_server.setup()
    site = web.TCPSite(web_server, "0.0.0.0", http_port)  # Сервер для HTTP-запросов
    await site.start()

    # Ожидаем завершения работы серверов
    await websocket_server.wait_closed()

if __name__ == "__main__":
    app.router.add_get("/data", show_data)  # Страница для отображения данных
    asyncio.run(main())