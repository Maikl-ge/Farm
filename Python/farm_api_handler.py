import json
from aiohttp import web
import logging

class FarmAPIHandler:
    def __init__(self, db_manager, websocket_handler=None):
        self.db_manager = db_manager
        self.websocket_handler = websocket_handler
        self.logger = logging.getLogger(__name__)

    async def show_data(self, request):
        """Отображение данных сенсоров"""
        try:
            async with self.db_manager.sensor_pool.acquire() as conn:
                records = await conn.fetch('SELECT * FROM sensor_data ORDER BY timestamp DESC LIMIT 20')

            if request.query.get('format') == 'json':
                return web.json_response([dict(record) for record in records])

            try:
                return web.FileResponse('./templates/data.html')
            except Exception as e:
                return web.Response(text="Error loading data page", status=500)

        except Exception as e:
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
            self.logger.info(f"Received data for setting command: {data}")

            command_str = data.get('command')

            if not command_str:
                return web.json_response(
                    {"error": "Command is required"},
                    status=400
                )

            success = await self.websocket_handler.send_command(command_str)

            if success:
                return web.json_response({
                    "status": "success",
                    "command": command_str
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
            self.logger.error(f"Error setting command: {e}")
            return web.json_response(
                {"error": str(e)},
                status=500)

    async def get_frqs(self, request):
        """API: Получение FRQS данных"""
        if not self.websocket_handler:
            return web.json_response(
                {"error": "WebSocket handler not initialized"},
                status=500
            )
        
        frqs_data = await self.websocket_handler.get_frqs_data()
        print(f"Parameters: {frqs_data}")
        if frqs_data is None:
            return web.json_response({"error": "No FRQS data available"}, status=404)
        
        return web.json_response(frqs_data)
