from aiohttp import web
import logging
import json
import aiohttp_jinja2
import jinja2
import os

class FarmHTTPHandler:
    def __init__(self, db_manager, websocket_handler=None):
        self.db_manager = db_manager
        self.websocket_handler = websocket_handler
        self.logger = logging.getLogger(__name__)

    async def index(self, request):
        """Перенаправление на страницу команд"""
        raise web.HTTPFound('/command')

    async def command_page(self, request):
        """Страница управления командами"""
        try:
            return web.FileResponse('./templates/command.html')
        except Exception as e:
            self.logger.error(f"Error serving command page: {e}")
            return web.Response(text="Error loading command page", status=500)

    async def show_data(self, request):
        """Отображение данных сенсоров"""
        try:
            async with self.db_manager.sensor_pool.acquire() as conn:
                records = await conn.fetch('SELECT * FROM sensor_data ORDER BY timestamp DESC LIMIT 100')

            # Проверяем, запрошен ли JSON формат
            if request.query.get('format') == 'json':
                return web.json_response([dict(record) for record in records])

            # Для HTML-запроса возвращаем файл data.html
            try:
                return web.FileResponse('./templates/data.html')
            except Exception as e:
                self.logger.error(f"Error serving data page: {e}")
                return web.Response(text="Error loading data page", status=500)

        except Exception as e:
            self.logger.error(f"Error showing data: {e}")
            return web.Response(text=str(e), status=500)

    async def get_data(self, request):
        """API для получения данных в формате JSON"""
        try:
            async with self.db_manager.sensor_pool.acquire() as conn:
                records = await conn.fetch('SELECT * FROM sensor_data ORDER BY timestamp DESC LIMIT 100')
            return web.json_response([dict(record) for record in records])
        except Exception as e:
            self.logger.error(f"Error getting data: {e}")
            return web.json_response({"error": str(e)}, status=500)

    async def get_command(self, request):
        """API: Получение текущей команды"""
        if not self.websocket_handler:
            return web.json_response(
                {"error": "WebSocket handler not initialized"},
                status=500
            )
        
        return web.json_response({
            "command": self.websocket_handler.command_to_farm
        })

    async def set_command(self, request):
        """API: Установка новой команды"""
        if not self.websocket_handler:
            return web.json_response(
                {"error": "WebSocket handler not initialized"},
                status=500
            )
        
        try:
            data = await request.json()
            new_command = data.get('command')
            
            if not new_command:
                return web.json_response(
                    {"error": "Command is required"},
                    status=400
                )
            
            success = self.websocket_handler.set_command(new_command)
            
            if success:
                return web.json_response({
                    "status": "success",
                    "command": new_command
                })
            else:
                return web.json_response(
                    {"error": "Failed to set command"},
                    status=500
                )
                
        except json.JSONDecodeError:
            return web.json_response(
                {"error": "Invalid JSON"},
                status=400
            )
        except Exception as e:
            return web.json_response(
                {"error": str(e)},
                status=500
            )

    async def get_frqs(self, request):
        """API: Получение FRQS данных"""
        if not self.websocket_handler:
            return web.json_response(
                {"error": "WebSocket handler not initialized"},
                status=500
            )
        
        frqs_data = await self.websocket_handler.get_frqs_data()
        if frqs_data is None:
            return web.json_response({"error": "No FRQS data available"}, status=404)
        
        return web.json_response(frqs_data)

    async def get_websocket_state(self, request):
        """API: Получение состояния WebSocket"""
        if not self.websocket_handler:
            return web.json_response(
                {"error": "WebSocket handler not initialized"},
                status=500
            )
        
        return web.json_response({
            "state": self.websocket_handler.websocket_state
        })