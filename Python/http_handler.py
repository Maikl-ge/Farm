import json
import aiohttp_jinja2
from aiohttp import web
import logging

# Настройка логирования
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler("http_handler.log"),
        logging.StreamHandler()
    ]
)

logger = logging.getLogger(__name__)

# Пример использования логирования
logger.info("HTTP handler initialized")

class FarmHTTPHandler:
    def __init__(self, db_manager, websocket_handler=None):
        self.db_manager = db_manager
        self.websocket_handler = websocket_handler
        self.logger = logging.getLogger(__name__)
        self.logger.info("FarmHTTPHandler initialized")

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
            self.logger.info(f"Received data for setting command: {data}")

            command_str = data.get('command')

            if not command_str:
                return web.json_response(
                    {"error": "Command is required"},
                    status=400
                )

            # Отправляем команду как строку
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

    async def show_parameters(self, request):
        """Отображение списка профилей параметров"""
        try:
            async with self.db_manager.params_pool.acquire() as conn:
                params = await conn.fetch('''
                    SELECT * FROM system_params 
                    ORDER BY id ASC 
                    LIMIT 10 OFFSET 0
                ''')

            context = {'params': params}
            return aiohttp_jinja2.render_template(
                'parametrs.html',
                request,
                context
            )
        except Exception as e:
            self.logger.error(f"Error showing parameters: {e}")
            return web.Response(text=str(e), status=500)

    async def edit_parameters(self, request):
        """Редактирование профиля параметров"""
        try:
            profile_id = request.match_info.get('id')
            
            async with self.db_manager.params_pool.acquire() as conn:
                params = await conn.fetchrow('''
                    SELECT * FROM system_params 
                    WHERE id = $1
                ''', int(profile_id))

                if not params:
                    raise web.HTTPNotFound(text="Profile not found")

                # Если это POST запрос, обновляем данные
                if request.method == 'POST':
                    data = await request.post()
                    
                    # Конвертируем время в минуты
                    sunrise = data['sunrise'].split(':')
                    sunrise_minutes = int(sunrise[0]) * 60 + int(sunrise[1])
                    
                    sunset = data['sunset'].split(':')
                    sunset_minutes = int(sunset[0]) * 60 + int(sunset[1])
                    
                    day_watering = data['dayWateringInterval'].split(':')
                    day_watering_minutes = int(day_watering[0]) * 60 + int(day_watering[1])
                    
                    night_watering = data['nightWateringInterval'].split(':')
                    night_watering_minutes = int(night_watering[0]) * 60 + int(night_watering[1])

                    await conn.execute('''
                        UPDATE system_params 
                        SET nameprofile = $1,
                            cycle = $2,
                            sunrise = $3,
                            sunset = $4,
                            daytemperaturestart = $5,
                            daytemperatureend = $6,
                            nighttemperaturestart = $7,
                            nighttemperatureend = $8,
                            dayhumiditystart = $9,
                            dayhumidityend = $10,
                            nighthumiditystart = $11,
                            nighthumidityend = $12,
                            watertemperature = $13,
                            daywateringinterval = $14,
                            nightwateringinterval = $15,
                            daycirculation = $16,
                            nightcirculation = $17,
                            dayventilation = $18,
                            nightventilation = $19
                        WHERE id = $20
                    ''',
                    data['nameProfile'],
                    int(data['cycle']),
                    sunrise_minutes,
                    sunset_minutes,
                    float(data['dayTemperatureStart']),
                    float(data['dayTemperatureEnd']),
                    float(data['nightTemperatureStart']),
                    float(data['nightTemperatureEnd']),
                    float(data['dayHumidityStart']),
                    float(data['dayHumidityEnd']),
                    float(data['nightHumidityStart']),
                    float(data['nightHumidityEnd']),
                    float(data['waterTemperature']),
                    day_watering_minutes,
                    night_watering_minutes,
                    int(data['dayCirculation']),
                    int(data['nightCirculation']),
                    int(data['dayVentilation']),
                    int(data['nightVentilation']),
                    int(profile_id)
                    )
                    
                    raise web.HTTPFound('/parameters')

            # Преобразуем минуты в формат HH:MM для отображения
            params = dict(params)
            params['sunrise'] = f"{params['sunrise'] // 60:02d}:{params['sunrise'] % 60:02d}"
            params['sunset'] = f"{params['sunset'] // 60:02d}:{params['sunset'] % 60:02d}"
            params['daywateringinterval'] = f"{params['daywateringinterval'] // 60:02d}:{params['daywateringinterval'] % 60:02d}"
            params['nightwateringinterval'] = f"{params['nightwateringinterval'] // 60:02d}:{params['nightwateringinterval'] % 60:02d}"

            return aiohttp_jinja2.render_template(
                'edit.html',
                request,
                {'params': params}
            )

        except web.HTTPFound:
            raise
        except Exception as e:
            self.logger.error(f"Error editing parameters: {e}")
            return web.Response(text=str(e), status=500)

    async def select_parameter(self, request):
        """Обработка выбора параметра"""
        param_id = request.match_info.get('id')
        id_farm = '255'
        type_msg = 'SCME'

        try:
            # Получаем параметр из БД
            async with self.db_manager.params_pool.acquire() as conn:
                settings = await conn.fetchrow(
                    "SELECT * FROM system_params WHERE id = $1",
                    int(param_id)
                )

            if settings:  # Проверяем, существует ли параметр
                # Формируем JSON-строку из настроек
                settings_json = json.dumps(dict(settings), ensure_ascii=False)
                
                # Выводим в консоль для отладки
                print(f"Selected parameter ID: {param_id}")
                print(f"Setting command: {settings_json}")

                # Формируем сообщение для отправки
                message = f"{id_farm} {type_msg} {settings_json}"

                # Отправляем команду на ферму через WebSocket
                await self.websocket_handler.broadcast_message(message)
                # Перенаправляем обратно на страницу параметров
                return web.HTTPFound('/parameters')
            else:
                return web.Response(text="Parameter not found", status=404)

        except asyncpg.PostgresError as e:
            self.logger.error(f"Database error selecting parameter: {e}")
            return web.Response(text="Database error", status=500)
        except Exception as e:
            self.logger.error(f"Error selecting parameter: {e}")
            return web.Response(text=str(e), status=500)