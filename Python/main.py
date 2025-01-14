import asyncio
from aiohttp import web
import websockets
import logging
from database import DatabaseManager
from websocket_handler import FarmWebSocketHandler
from http_handler import FarmHTTPHandler
import aiohttp_jinja2
import jinja2
import os

async def main():
    # Базовая настройка логирования
    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
        handlers=[
            logging.StreamHandler()
        ]
    )
    
    # Отключаем логи websockets по умолчанию
    logging.getLogger('websockets').setLevel(logging.WARNING)
    
    # Создаем логгер для main
    logger = logging.getLogger(__name__)
    logger.propagate = False

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
            loader=jinja2.FileSystemLoader('./templates'))

        # Создаем обработчики
        websocket_handler = FarmWebSocketHandler(
            db_manager,
            ping_interval=5,
            ping_timeout=5
        )
        http_handler = FarmHTTPHandler(db_manager, websocket_handler)

        # Добавляем маршруты
        app.router.add_get("/", http_handler.index)  # Редирект на /command
        app.router.add_get("/command", http_handler.command_page)  # Страница управления командами
        app.router.add_get("/data", http_handler.show_data)  # Страница с данными
        app.router.add_get("/data.html", http_handler.show_data)  # Альтернативный путь к данным
        app.router.add_get("/api/data", http_handler.get_data)  # API для получения данных
        app.router.add_get('/api/command', http_handler.get_command)  # API для получения команды
        app.router.add_post('/api/command', http_handler.set_command)  # API для установки команды
        app.router.add_get('/api/frqs', http_handler.get_frqs)  # API для получения FRQS данных
        app.router.add_get('/api/websocket-state', http_handler.get_websocket_state)  # API для состояния WebSocket
        app.router.add_get('/parameters', http_handler.show_parameters)
        app.router.add_get('/parameters/{id}/edit', http_handler.edit_parameters)
        app.router.add_post('/parameters/{id}/edit', http_handler.edit_parameters)

        # Запускаем HTTP сервер
        runner = web.AppRunner(app)
        await runner.setup()
        site = web.TCPSite(runner, "0.0.0.0", http_port)
        await site.start()
        logger.info(f"HTTP server started on port {http_port}")

        # Запускаем WebSocket сервер
        websocket_server = await websockets.serve(
            websocket_handler.handle_connection,
            "0.0.0.0", 
            websocket_port,
            ping_interval=5,
            ping_timeout=5
        )
        logger.info(f"WebSocket server started on port {websocket_port}")

        # Запускаем фоновые задачи
        background_tasks = [
            asyncio.create_task(websocket_handler.check_websocket_state())
        ]

        try:
            # Ждем завершения работы серверов
            await websocket_server.wait_closed()
        finally:
            # Отменяем фоновые задачи при завершении
            for task in background_tasks:
                task.cancel()
            await asyncio.gather(*background_tasks, return_exceptions=True)
        
    except Exception as e:
        logger.error(f"Application error: {e}")
    finally:
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
        'Mozilla',  # Обычные браузеры
        'Chrome',
        'Firefox',
        'Safari',
        'Edge',
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
    asyncio.run(main())