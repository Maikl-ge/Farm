import asyncio
import websockets
import logging
from database import DatabaseManager
from websocket_handler import FarmWebSocketHandler
from http_handler import FarmHTTPHandler
from aiohttp import web
import aiohttp_jinja2
import jinja2
import tracemalloc
import aiomonitor
from datetime import datetime, timezone
from logging_protocol import LoggingWebSocketServerProtocol

async def main():
    # Старт отслеживания утечек памяти
    tracemalloc.start()

    # Базовая настройка логирования
    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
        handlers=[
            logging.StreamHandler(),  # Лог в консоль
            logging.FileHandler('server.log'),  # Лог в файл
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
        app = web.Application()
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
        http_handler = FarmHTTPHandler(db_manager, websocket_handler)

        # Добавляем маршруты
        app.router.add_get("/", http_handler.index)  # Redirect to /command
        app.router.add_get("/command", http_handler.command_page)  # Command management page
        app.router.add_get("/data", http_handler.show_data)  # Data page
        app.router.add_get("/data.html", http_handler.show_data)  # Alternative path for data
        app.router.add_get("/api/data", http_handler.get_data)  # API to get data
        app.router.add_get('/api/command', http_handler.get_command)  # API to get command
        app.router.add_post('/api/command', http_handler.set_command)  # API to set command
        app.router.add_get('/api/frqs', http_handler.get_frqs)  # API to get FRQS data
        app.router.add_get('/api/websocket-state', http_handler.get_websocket_state)  # API for WebSocket state
        app.router.add_get('/parameters', http_handler.show_parameters)
        app.router.add_get('/parameters/{id}/select', http_handler.select_parameter)
        app.router.add_get('/parameters/{id}/edit', http_handler.edit_parameters)  # Add this route
        app.router.add_post('/parameters/{id}/edit', http_handler.edit_parameters)
        app.router.add_get('/select_parameter/{id}', http_handler.select_parameter)

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
            read_limit=2**16,  # Максимальное количество байт для чтения
            write_limit=2**16,  # Максимальное количество байт для записи
            create_protocol=LoggingWebSocketServerProtocol  # Используем наш протокол
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
        stop_time = datetime.now(timezone.utc)
        elapsed_time = stop_time - start_time
        logger.info(f"Server stopped at {stop_time}")
        logger.info(f"Server uptime: {elapsed_time}")

        # Закрываем соединения
        await db_manager.close_pools()
        if 'runner' in locals():
            await runner.cleanup()
        logger.info("Application shutdown complete")

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        logging.info("Server stopped by user")