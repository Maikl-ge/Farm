import asyncio
import logging
from aiohttp import web
import websockets
from database_manager import DatabaseManager
from websocket_handler import FarmWebSocketHandler
from http_handler import HttpRequestHandler

# Конфигурация серверов
WS_HOST = '0.0.0.0'
WS_PORT = 8765
HTTP_HOST = '0.0.0.0'
HTTP_PORT = 8080

def setup_routes(app, handler):
    """Настройка маршрутов"""
    app.router.add_get('/', handler.index)
    app.router.add_get('/command', handler.command_page)
    app.router.add_get('/parameters', handler.parameters_page)
    app.router.add_get('/parameters/{id}/select', handler.select_parameter)

@web.middleware
async def security_middleware(request, handler):
    """Middleware для базовой защиты от сканирования"""
    # Список разрешенных User-Agent
    allowed_agents = [
        'Mozilla',
        'Chrome',
        'Firefox',
        'Safari',
        'Edge',
        'ESP32'
    ]
    
    # Проверка User-Agent
    user_agent = request.headers.get('User-Agent', '')
    if not any(agent in user_agent for agent in allowed_agents):
        return web.Response(status=403)

    # Блокировка подозрительных запросов
    if request.method == 'UNKNOWN' or 'PRI' in str(request.method):
        return web.Response(status=403)

    # Блокировка сканеров
    if 'CensysInspect' in user_agent or 'bot' in user_agent.lower():
        return web.Response(status=403)

    try:
        response = await handler(request)
        return response
    except web.HTTPException:
        raise
    except Exception as e:
        logging.error(f"Unexpected error: {e}")
        return web.Response(status=500)

async def start_server():
    """Запуск сервера с автоматическим восстановлением"""
    logger = logging.getLogger(__name__)
    runner = None
    db_manager = None
    ws_server = None
    
    while True:
        try:
            # Инициализация базы данных
            db_manager = DatabaseManager()
            await db_manager.init_pools()
            
            # Инициализация WebSocket обработчика
            websocket_handler = FarmWebSocketHandler(
                db_manager,
                ping_interval=5,
                ping_timeout=30
            )
            
            # Инициализация HTTP обработчика
            http_handler = HttpRequestHandler(db_manager, websocket_handler)
            
            # Создание приложения
            app = web.Application(middlewares=[security_middleware])
            setup_routes(app, http_handler)
            
            # Запуск WebSocket сервера
            ws_server = await websockets.serve(
                websocket_handler.handle_connection,
                WS_HOST,
                WS_PORT,
                ping_interval=5,
                ping_timeout=30
            )
            
            # Запуск HTTP сервера
            runner = web.AppRunner(app)
            await runner.setup()
            site = web.TCPSite(runner, HTTP_HOST, HTTP_PORT)
            await site.start()
            
            logger.info(f"Server started on http://{HTTP_HOST}:{HTTP_PORT}")
            logger.info(f"WebSocket server started on ws://{WS_HOST}:{WS_PORT}")
            
            # Держим серверы запущенными
            while True:
                await asyncio.sleep(3600)
                    
        except Exception as e:
            logger.error(f"Server error: {e}")
            logger.info("Attempting to restart server in 10 seconds...")
            await asyncio.sleep(10)
            continue
        
        finally:
            # Очистка ресурсов при остановке
            try:
                if runner:
                    await runner.cleanup()
                if ws_server:
                    ws_server.close()
                    await ws_server.wait_closed()
                if db_manager and db_manager.parameters_pool:
                    await db_manager.parameters_pool.close()
            except Exception as e:
                logger.error(f"Error during cleanup: {e}")

if __name__ == "__main__":
    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
    )
    
    try:
        asyncio.run(start_server())
    except KeyboardInterrupt:
        logging.info("Server stopped by user")
    except Exception as e:
        logging.error(f"Fatal error: {e}")