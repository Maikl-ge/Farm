import asyncio
import websockets
import json
from datetime import datetime
import redis
from aiohttp import web

# Настройки Redis
redis_host = 'localhost'
redis_port = 6379
redis_db = 0

# Создаем подключение к Redis
r = redis.StrictRedis(host=redis_host, port=redis_port, db=redis_db, decode_responses=True)

# Создание объекта приложения
app = web.Application()

# Асинхронная обработка WebSocket-соединений
async def handle_connection(websocket, path):
    print(f"Клиент подключен по пути: {path}")
    try:
        async for message in websocket:
            timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')  # Получаем текущую метку времени
            print(f"{timestamp} - Получено сообщение: {message}")

            # Преобразуем полученное сообщение из JSON
            try:
                data = json.loads(message)  # Парсим JSON
                print(f"{timestamp} - Распарсенные данные:")
                temperature = data.get("температура")
                humidity = data.get("влажность")
                waterLevel = data.get("уровень воды")
                if temperature is not None and humidity is not None and waterLevel is not None:
                    print(f"{timestamp} - температура: {temperature}, влажность: {humidity}, уровень воды:  {waterLevel}")
                    
                    # Сохраняем данные в Redis
                    redis_key = f"data:{timestamp}"  # Ключ для сохранения данных
                    r.hset(redis_key, mapping={"время": timestamp, "температура": temperature, "влажность": humidity, "уровень воды": waterLevel})
                    print(f"{timestamp} - Данные сохранены в Redis: {redis_key}")
                else:
                    print(f"{timestamp} - Некорректные данные (температура или влажность отсутствуют).")
            except json.JSONDecodeError as e:
                print(f"{timestamp} - Ошибка при парсинге JSON: {e}")

            # Отправляем эхо-сообщение
            await websocket.send(f"{timestamp} - Эхо: {message}")
    except websockets.exceptions.ConnectionClosed as e:
        print(f"Клиент отключился: {e}")

# Обработчик для отображения данных через HTTP
async def show_data(request):
    # Получаем все сохраненные ключи
    keys = r.keys("data:*")
    if keys:
        # Извлекаем данные из Redis
        html_content = "<h1>Сохраненные данные</h1><ul>"
        for key in keys:
            data = r.hgetall(key)
            html_content += f"<li>Ключ: {key}, данные: {data}</li>"
        html_content += "</ul>"
    else:
        html_content = "<h1>Нет сохраненных данных в Redis.</h1>"

    return web.Response(text=html_content, content_type='text/html')

async def main():
    port = 5001
    print(f"Сервер запущен на порту {port}...")
    
    # Запуск WebSocket-сервера
    websocket_server = await websockets.serve(handle_connection, "0.0.0.0", port)

    # Запуск HTTP-сервера
    web_server = web.AppRunner(app)
    await web_server.setup()
    site = web.TCPSite(web_server, "0.0.0.0", 8080)  # Сервер для HTTP-запросов
    await site.start()

    # Ожидаем завершения работы серверов
    await websocket_server.wait_closed()

if __name__ == "__main__":
    app.router.add_get("/data", show_data)  # Страница для отображения данных
    asyncio.run(main())
