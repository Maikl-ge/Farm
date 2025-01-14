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
import tracemalloc
from datetime import datetime
import concurrent.futures
import uvloop
import socket
from concurrent.futures import ThreadPoolExecutor

async def main():
    # Устанавливаем uvloop как event loop по умолчанию
    uvloop.install()
    
    # Оптимизация настроек event loop
    loop = asyncio.get_running_loop()
    
    # Создаем пул потоков с ограниченным количеством воркеров
    executor = ThreadPoolExecutor(max_workers=4)
    loop.set_default_executor(executor)
    
    # Оптимизация сокетов
    def configure_socket(sock):
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)
        # Увеличиваем размеры буферов
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 262144)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, 262144)
        # Устанавливаем неблокирующий режим
        sock.setblocking(False)
    
    # Включаем отслеживание памяти
    tracemalloc.start(25)  # Отслеживаем 25 фреймов для точности
    logger = logging.getLogger(__name__)

    async def monitor_memory():
        while True:
            current, peak = tracemalloc.get_traced_memory()
            logger.info(f"Current memory usage: {current / 10**6:.1f}MB; Peak: {peak / 10**6:.1f}MB")
            
            # Получаем топ-10 источников утечек памяти
            snapshot = tracemalloc.take_snapshot()
            top_stats = snapshot.statistics('lineno')
            
            logger.info("Top 10 memory allocations:")
            for stat in top_stats[:10]:
                logger.info(stat)
                
            await asyncio.sleep(300)  # Проверка каждые 5 минут

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
        app = web.Application(
            client_max_size=1024**2,  # Ограничиваем размер запроса
            middlewares=[
                web.normalize_path_middleware(),
            ]
        )
        app['db_manager'] = db_manager

        # Настраиваем шаблонизатор
        aiohttp_jinja2.setup(app, loader=jinja2.FileSystemLoader('./templates'))

        # Создаем обработчики
        websocket_handler = FarmWebSocketHandler(
            db_manager,
            ping_interval=5,
            ping_timeout=5
        )
        http_handler = FarmHTTPHandler(db_manager, websocket_handler)

        # Добавляем маршруты
        app.router.add_get("/", http_handler.index)
        app.router.add_get("/command", http_handler.command_page)
        app.router.add_get("/data", http_handler.show_data)
        app.router.add_get("/data.html", http_handler.show_data)
        app.router.add_get("/api/data", http_handler.get_data)
        app.router.add_get('/api/command', http_handler.get_command)
        app.router.add_post('/api/command', http_handler.set_command)
        app.router.add_get('/api/frqs', http_handler.get_frqs)
        app.router.add_get('/api/websocket-state', http_handler.get_websocket_state)
        app.router.add_get('/parameters', http_handler.show_parameters)
        app.router.add_get('/parameters/{id}/edit', http_handler.edit_parameters)
        app.router.add_post('/parameters/{id}/edit', http_handler.edit_parameters)

        # Добавляем обработку больших операций в отдельный пул
        async def handle_heavy_operation(func):
            loop = asyncio.get_running_loop()
            return await loop.run_in_executor(None, func)
        
        # Оптимизация обработчика WebSocket
        async def optimized_ws_handler(websocket, path):
            try:
                # Устанавливаем таймауты для операций
                websocket.connection_timeout = 30
                await asyncio.wait_for(
                    websocket_handler.handle_connection(websocket, path),
                    timeout=30
                )
            except asyncio.TimeoutError:
                logging.warning("WebSocket timeout")
            except Exception as e:
                logging.error(f"WebSocket error: {e}")
            finally:
                await websocket.close()

        # Настройка WebSocket сервера
        async def create_websocket_server():
            return await websockets.serve(
                websocket_handler.handle_connection,
                "0.0.0.0",
                websocket_port,
                ping_interval=30,
                ping_timeout=30,
                max_size=2**20,
                max_queue=100,
                read_limit=2**16,
                write_limit=2**16,
                create_protocol=websockets.WebSocketServerProtocol
            )

        # Запускаем HTTP сервер
        try:
            runner = web.AppRunner(app, access_log=None)  # Отключаем access_log для производительности
            await runner.setup()
            site = web.TCPSite(runner, "0.0.0.0", http_port)
            await site.start()
            logger.info(f"HTTP server started on port {http_port}")
        except Exception as e:
            logger.error(f"Failed to start HTTP server: {e}")
            raise

        # Запускаем WebSocket сервер
        try:
            websocket_server = await create_websocket_server()
            for sock in websocket_server.sockets:
                configure_socket(sock)
            logger.info(f"WebSocket server started on port {websocket_port}")
        except Exception as e:
            logger.error(f"Failed to start WebSocket server: {e}")
            raise

        # Запускаем фоновые задачи
        async def safe_task(task, name):
            try:
                await task
            except asyncio.CancelledError:
                logger.info(f"Task '{name}' was cancelled.")
            except Exception as e:
                logger.error(f"Task '{name}' error: {e}")

        # Добавляем мониторинг памяти к фоновым задачам
        background_tasks = [
            asyncio.create_task(safe_task(websocket_handler.check_websocket_state(), "check_websocket_state")),
            asyncio.create_task(safe_task(monitor_memory(), "memory_monitor"))
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
        logger.exception(f"Application encountered an error: {e}")
    finally:
        # Закрываем соединения
        await db_manager.close_pools()
        if 'runner' in locals():
            await runner.cleanup()
        logger.info("Application shutdown complete")
        # Останавливаем отслеживание памяти
        tracemalloc.stop()

if __name__ == "__main__":
    try:
        # Просто используем asyncio.run(), который сам создаст и закроет loop
        asyncio.run(main(), debug=True)  # debug=True для более подробных сообщений об ошибках
    except KeyboardInterrupt:
        logging.info("Application interrupted by user.")
    except Exception as e:
        logging.exception(f"Fatal error: {e}")
