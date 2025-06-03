from aiohttp import web
import logging

# Определяем логгер внутри модуля
logger = logging.getLogger(__name__)

class FarmIframeHandler:
    def __init__(self, db_manager, websocket_handler):
        self.db_manager = db_manager
        self.websocket_handler = websocket_handler
        self.logger = logger

    async def show_iframe_temp(self, request):
        """iframe Спидометр температуры воды"""
        try:
            return web.FileResponse('./templates/iframe_line_temp.html')
        except Exception as e:
            self.logger.error(f"Error loading iframe_line_temp.html: {e}")
            return web.Response(text="Error loading iframe_line_temp.html", status=500)

    async def show_iframe_temp_out(self, request):
        """iframe Спидометр температуры чистой воды"""
        try:
            return web.FileResponse('./templates/iframe_line_temp_out.html')
        except Exception as e:
            self.logger.error(f"Error loading iframe_line_temp_out.html: {e}")
            return web.Response(text="Error loading iframe_line_temp_out.html", status=500)

    async def show_iframe_circulation(self, request):
        """iframe Спидометр циркуляция в боксе"""
        try:
            return web.FileResponse('./templates/iframe_line_circulation.html')
        except Exception as e:
            self.logger.error(f"Error loading iframe_line_circulation.html: {e}")
            return web.Response(text="Error loading iframe_line_circulation.html", status=500)

    async def show_iframe_inlet(self, request):
        """iframe Спидометр вытяжка из бокса"""
        try:
            return web.FileResponse('./templates/iframe_line_inlet.html')
        except Exception as e:
            self.logger.error(f"Error loading iframe_line_inlet.html: {e}")
            return web.Response(text="Error loading iframe_line_inlet.html", status=500)

    async def show_iframe_rotation(self, request):
        """iframe Спидометр вращения барабана"""
        try:
            return web.FileResponse('./templates/iframe_line_rotation.html')
        except Exception as e:
            self.logger.error(f"Error loading iframe_line_rotation.html: {e}")
            return web.Response(text="Error loading iframe_line_rotation.html", status=500)

    async def show_iframe_pH(self, request):
        """iframe Уровень pH"""
        try:
            return web.FileResponse('./templates/iframe_line_pH.html')
        except Exception as e:
            self.logger.error(f"Error loading iframe_line_pH.html: {e}")
            return web.Response(text="Error loading iframe_line_pH.html", status=500)

    async def show_line_TDS(self, request):
        """iframe Уровень TDS"""
        try:
            return web.FileResponse('./templates/iframe_line_TDS.html')
        except Exception as e:
            self.logger.error(f"Error loading iframe_line_TDS.html: {e}")
            return web.Response(text="Error loading iframe_line_TDS.html", status=500)

    async def show_line_light(self, request):
        """iframe Уровень света"""
        try:
            return web.FileResponse('./templates/iframe_line_light.html')
        except Exception as e:
            self.logger.error(f"Error loading iframe_line_light.html: {e}")
            return web.Response(text="Error loading iframe_line_light.html", status=500)

    async def show_pump_watering(self, request):
        """iframe Насос полива"""
        try:
            return web.FileResponse('./templates/pump_watering.html')
        except Exception as e:
            self.logger.error(f"Error loading pump_watering.html: {e}")
            return web.Response(text="Error loading pump_watering.html", status=500)

    async def show_pump_mixing(self, request):
        """iframe Насос смешивания"""
        try:
            return web.FileResponse('./templates/pump_mixing.html')
        except Exception as e:
            self.logger.error(f"Error loading pump_mixing.html: {e}")
            return web.Response(text="Error loading pump_mixing.html", status=500)

    async def show_us_humidifier(self, request):
        """iframe Увлажнитель"""
        try:
            return web.FileResponse('./templates/us_humidifier.html')
        except Exception as e:
            self.logger.error(f"Error loading us_humidifier.html: {e}")
            return web.Response(text="Error loading us_humidifier.html", status=500)

    async def show_iframe_graf_t_h(self, request):
        """iframe Отображение графиков"""
        try:
            return web.FileResponse('./templates/iframe_graf_t_h.html')
        except Exception as e:
            self.logger.error(f"Error loading iframe_graf_t_h.html: {e}")
            return web.Response(text="Error loading iframe_graf_t_h.html", status=500)
            
            
    async def show_data_watering(self, request):
        """Отображение данных сенсоров полива"""
        try:
            async with self.db_manager.sensor_pool.acquire() as conn:
                records = await conn.fetch('SELECT * FROM sensor_data ORDER BY timestamp DESC LIMIT 1')

            if request.query.get('format') == 'json':
                return web.json_response([dict(record) for record in records])

            try:
                return web.FileResponse('./templates/data_watering.html')
            except Exception as e:
                return web.Response(text="Error loading data page", status=500)

        except Exception as e:
            return web.Response(text=str(e), status=500)

    async def get_data_watering(self, request):
        """API для получения данных полива в формате JSON"""
        try:
            limit = request.query.get('limit', '40')
            limit = int(limit)

            async with self.db_manager.sensor_pool.acquire() as conn:
                records = await conn.fetch(f'SELECT "current_time", temperature_1, humidity_1 FROM sensor_data ORDER BY timestamp DESC LIMIT {limit}')
                
            return web.json_response([dict(record) for record in records])
        except Exception as e:
            return web.json_response({"error": str(e)}, status=500)