#main.py
import asyncio
from aiohttp import web
import websockets
import logging
from database import DatabaseManager
from websocket_handler import FarmWebSocketHandler
from farm_http_handler import FarmHTTPHandler
from farm_iframe_handler import FarmIframeHandler
from farm_api_handler import FarmAPIHandler
from farm_profile_handler import FarmProfileHandler
from message_handler import MessageHandler

import aiohttp_jinja2
import jinja2
import tracemalloc
import aiomonitor
import requests
from datetime import datetime, timezone


async def main():
    # Старт отслеживания утечек памяти
    tracemalloc.start()
    



    BOT_TOKEN = '7707296799:AAEgP4vmsSxfDBYYp9hK18Bolr03geKvNuY'
    CHAT_ID = '306069126'
    TEXT = 'Server Farm started 🚀'

    url = f"https://api.telegram.org/bot{BOT_TOKEN}/sendMessage"
    payload = {
        'chat_id': CHAT_ID,
        'text': TEXT
    }

    response = requests.post(url, data=payload)

    # Проверка успешности
    if response.status_code == 200:
        print("✅ Сообщение отправлено!")
    else:
        print(f"❌ Ошибка: {response.status_code} — {response.text}")








    
    # Базовая настройка логирования
    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
        handlers=[
            logging.StreamHandler()  # Лог только в консоль
        ]
    )  

    # Отключаем логи websockets по умолчанию
    logging.getLogger('websockets').setLevel(logging.WARNING)

    # Создаем логгер для main
    logger = logging.getLogger(__name__)
    logger.propagate = False

    # Логируем время старта сервера
    start_time = datetime.now(timezone.utc)
    logger.info(f"Server started at {start_time}")

    # Порты для серверов
    websocket_port = 5001
    http_port = 8080
    logger.info(f"Starting servers - WebSocket port: {websocket_port}, HTTP port: {http_port}")

    # Создаем менеджер баз данных
    db_manager = DatabaseManager()

    try:
        # Инициализируем пулы подключений
        await db_manager.init_pools()

        # Создаем приложение aiohttp
        app = web.Application(middlewares=[security_middleware])
        app['db_manager'] = db_manager

        # Настраиваем шаблонизатор
        aiohttp_jinja2.setup(app,
            loader=jinja2.FileSystemLoader('./html'))

        # Создаем обработчики
        websocket_handler = FarmWebSocketHandler(
            db_manager,
            ping_interval=5,
            ping_timeout=5
        )
        message_handler = MessageHandler(db_manager, websocket_handler)
        http_handler = FarmHTTPHandler(db_manager, websocket_handler, message_handler)
        iframe_handler = FarmIframeHandler(db_manager, websocket_handler)
        api_handler = FarmAPIHandler(db_manager, websocket_handler)
        profile_handler = FarmProfileHandler(db_manager)
        #dashboard_handler = FarmDashboardHandler(db_manager)

        # Добавляем маршруты
        app.router.add_get("/", http_handler.index)  # Редирект на /command
        app.router.add_get("/command", http_handler.command_page)  # Страница управления командами
        
        app.router.add_get("/data", api_handler.show_data)  # Страница с данными
        app.router.add_get("/data.html", api_handler.show_data)  # Альтернативный путь к данным
        app.router.add_get("/api/data", api_handler.get_data)  # API для получения данных
        
        app.router.add_get('/api/command', api_handler.get_command)  # API для получения команды
        app.router.add_post('/api/command', api_handler.set_command)  # API для установки команды
        
        app.router.add_post('/command/send_cmd', http_handler.send_cmd)
        
        app.router.add_post('/report', http_handler.http_report)
        app.router.add_get('/latest_sensor', http_handler.http_latest_sensor)  # Новый маршрут
        app.router.add_get('/latest_status', http_handler.http_latest_status)
        
        app.router.add_get('/api/frqs', api_handler.get_frqs)  # API для получения FRQS данных
        app.router.add_get('/api/websocket-state', api_handler.get_websocket_state)  # API для состояния WebSocket
        
        app.router.add_get('/parameters', http_handler.show_parameters)
        app.router.add_get('/parameters.html', http_handler.show_parameters)        
        app.router.add_get('/parameters/{profileid}/select', http_handler.select_parameter)
        app.router.add_post('/parameters/{profileid}/select', http_handler.select_parameter)        
        app.router.add_get('/parameters/{id}/edit', http_handler.edit_parameters)
        app.router.add_post('/parameters/{id}/edit', http_handler.edit_parameters)
        app.router.add_get('/select_parameter/{id}', http_handler.select_parameter)
        
        app.router.add_post('/save_profile', profile_handler.save_profile)  # Сохранение профиля
        app.router.add_get('/save_profile', profile_handler.show_save_profile)  # Страница сохранения профиля
        app.router.add_get('/read_profile_db', profile_handler.read_profile_db)  # Чтение профиля из БД

        app.router.add_get("/data_watering", iframe_handler.show_data_watering)  # Data page
        app.router.add_get("/data_watering.html", iframe_handler.show_data_watering)  # Alternative path for data
        app.router.add_get("/api/data_watering", iframe_handler.get_data_watering)  # API to get data

        # Запускаем HTTP сервер
        runner = web.AppRunner(app)
        await runner.setup()
        site = web.TCPSite(runner, "0.0.0.0", http_port)
        await site.start()
        logger.info(f"HTTP server started on port {http_port}")

        # Ограничение количества одновременных подключений к WebSocket серверу
        max_connections = 100
        connected_clients = set()

        async def limited_connection_handler(websocket, path):
            if len(connected_clients) >= max_connections:
                await websocket.close(code=websockets.CloseCode.GOING_AWAY, reason="Too many connections")
                return
            connected_clients.add(websocket)
            try:
                await websocket_handler.handle_connection(websocket, path)
            finally:
                connected_clients.remove(websocket)

        # Запускаем WebSocket сервер
        websocket_server = await websockets.serve(
            limited_connection_handler,
            "0.0.0.0", 
            websocket_port,
            ping_interval=5,
            ping_timeout=5,
            max_size=2**20,  # Максимальный размер сообщения 1MB
            max_queue=32,  # Максимальная длина очереди сообщений
        )
        logger.info(f"WebSocket server started on port {websocket_port}")

        # Запускаем фоновые задачи
        background_tasks = [
            asyncio.create_task(websocket_handler.check_websocket_state())
        ]

        try:
            # Ждем завершения работы серверов
            with aiomonitor.start_monitor(loop=asyncio.get_event_loop()):
                await websocket_server.wait_closed()
        finally:
            # Отменяем фоновые задачи при завершении
            for task in background_tasks:
                task.cancel()
            await asyncio.gather(*background_tasks, return_exceptions=True)

    except Exception as e:
        logger.error(f"Application error: {e}")
    finally:
        # Логируем время остановки сервера
        stop_time = datetime.now(timezone.utc)  # Используем timezone-aware datetime
        elapsed_time = stop_time - start_time
        logger.info(f"Server stopped at {stop_time}")
        logger.info(f"Server uptime: {elapsed_time}")

        # Закрываем соединения
        await db_manager.close_pools()
        if 'runner' in locals():
            await runner.cleanup()
        logger.info("Application shutdown complete")

@web.middleware
async def security_middleware(request, handler):
    """Middleware для базовой защиты от сканирования"""
    # Список разрешенных User-Agent
    allowed_agents = [
        'Chrome',
        'Safari',
        'ESP32'    # Ваши устройства
    ]
    
    # Проверка User-Agent
    user_agent = request.headers.get('User-Agent', '')
    if not any(agent in user_agent for agent in allowed_agents):
        return web.Response(status=403)  # Forbidden

    # Блокировка подозрительных запросов
    if request.method == 'UNKNOWN' or 'PRI' in str(request.method):
        return web.Response(status=403)

    # Блокировка сканеров
    if 'CensysInspect' in user_agent or 'bot' in user_agent.lower():
        return web.Response(status=403)

    try:
        response = await handler(request)
        return response
    except web.HTTPException as ex:
        raise
    except Exception as e:
        logging.error(f"Unexpected error: {e}")
        return web.Response(status=500)

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        logging.info("Server stopped by user")