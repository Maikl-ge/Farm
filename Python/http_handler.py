import json
import aiohttp_jinja2
from aiohttp import web
import logging

class FarmHTTPHandler:
    def __init__(self, db_manager, websocket_handler=None):
        self.db_manager = db_manager
        self.websocket_handler = websocket_handler

    async def save_profile(self, request):
        """Сохранение профиля из CBOR-данных"""
        try:
            cbor_data = await request.read()  # Используем read() для получения байтов
            success = await self.db_manager.save_profile_from_cbor(cbor_data)
            return web.json_response({"status": "success" if success else "error"})
        except Exception as e:
            return web.json_response({"error": str(e)}, status=500)

    async def index(self, request):
        """Перенаправление на страницу команд"""
        raise web.HTTPFound('/command')

    async def command_page(self, request):
        """Страница управления командами"""
        try:
            return web.FileResponse('./templates/command.html')
        except Exception as e:
            return web.Response(text="Error loading command page", status=500)

    async def show_iframe_temp(self, request):
        """iframe Спидометр температуры воды"""
        try:
            return web.FileResponse('./templates/iframe_line_temp.html')
        except Exception as e:
            return web.Response(text="Error loading command page", status=500)

    async def show_iframe_temp_out(self, request):
        """iframe Спидометр температуры чистой воды"""
        try:
            return web.FileResponse('./templates/iframe_line_temp_out.html')
        except Exception as e:
            return web.Response(text="Error loading command page", status=500)

    async def show_iframe_circulation(self, request):
        """iframe Спидометр циркуляция в боксе"""
        try:
            return web.FileResponse('./templates/iframe_line_circulation.html')
        except Exception as e:
            return web.Response(text="Error loading command page", status=500)

    async def show_iframe_inlet(self, request):
        """iframe Спидометр вытяжка из бокса"""
        try:
            return web.FileResponse('./templates/iframe_line_inlet.html')
        except Exception as e:
            return web.Response(text="Error loading command page", status=500)

    async def show_iframe_rotation(self, request):
        """iframe Спидометр вращения барабана"""
        try:
            return web.FileResponse('./templates/iframe_line_rotation.html')
        except Exception as e:
            return web.Response(text="Error loading command page", status=500)

    async def show_iframe_pH(self, request):
        """iframe Уровень pH"""
        try:
            return web.FileResponse('./templates/iframe_line_pH.html')
        except Exception as e:
            return web.Response(text="Error loading command page", status=500)

    async def show_line_TDS(self, request):
        """iframe Уровень TDS"""
        try:
            return web.FileResponse('./templates/iframe_line_TDS.html')
        except Exception as e:
            return web.Response(text="Error loading command page", status=500)

    async def show_line_light(self, request):
        """iframe Уровень света"""
        try:
            return web.FileResponse('./templates/iframe_line_light.html')
        except Exception as e:
            return web.Response(text="Error loading command page", status=500)

    async def show_pump_watering(self, request):
        """iframe Насос полива"""
        try:
            return web.FileResponse('./templates/pump_watering.html')
        except Exception as e:
            return web.Response(text="Error loading command page", status=500)

    async def show_pump_mixing(self, request):
        """iframe Насос смешивания"""
        try:
            return web.FileResponse('./templates/pump_mixing.html')
        except Exception as e:
            return web.Response(text="Error loading command page", status=500)

    async def show_us_humidifier(self, request):
        """iframe Увлажнитель"""
        try:
            return web.FileResponse('./templates/us_humidifier.html')
        except Exception as e:
            return web.Response(text="Error loading command page", status=500)

    async def show_iframe_graf_t_h(self, request):
        """iframe Отображение графиков"""
        try:
            return web.FileResponse('./templates/iframe_graf_t_h.html')
        except Exception as e:
            return web.Response(text="Error loading command page", status=500)

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

    async def get_data(self, request):
        """API для получения данных в формате JSON"""
        try:
            async with self.db_manager.sensor_pool.acquire() as conn:
                records = await conn.fetch('SELECT * FROM sensor_data ORDER BY timestamp DESC LIMIT 100')
            return web.json_response([dict(record) for record in records])
        except Exception as e:
            return web.json_response({"error": str(e)}, status=500)

    async def get_data_watering(self, request):
        """API для получения данных полива в формате JSON"""
        try:
            limit = request.query.get('limit', '40')
            limit = int(limit)

            async with self.db_manager.sensor_pool.acquire() as conn:
                records = await conn.fetch(f'SELECT temperature_1, humidity_1 FROM sensor_data ORDER BY timestamp DESC LIMIT {limit}')
                
            return web.json_response([dict(record) for record in records])
        except Exception as e:
            return web.json_response({"error": str(e)}, status=500)

    async def get_data_iframe(self, request):
        """API для получения данных для iframe в формате JSON"""
        try:
            limit = request.query.get('limit', '40')
            limit = int(limit)

            async with self.db_manager.sensor_pool.acquire() as conn:
                records = await conn.fetch(f'SELECT water_temperature_osmo, water_temperature_watering, ph_osmo, tds_osmo FROM sensor_data ORDER BY timestamp DESC LIMIT {limit}')
                
            return web.json_response([dict(record) for record in records])
        except Exception as e:
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

                if request.method == 'POST':
                    data = await request.post()
                    
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
            return web.Response(text=str(e), status=500)

    async def select_parameter(self, request):
        """Обработка выбора параметра"""
        param_id = request.match_info.get('id')
        id_farm = '255'
        type_msg = 'SCME'

        try:
            async with self.db_manager.params_pool.acquire() as conn:
                settings = await conn.fetchrow(
                    "SELECT * FROM system_params WHERE id = $1",
                    int(param_id)
                )

            if settings:
                settings_json = json.dumps(dict(settings), ensure_ascii=False)
                print(f"Selected parameter ID: {param_id}")
                print(f"Setting command: {settings_json}")

                message = f"{id_farm} {type_msg} {settings_json}"

                await self.websocket_handler.broadcast_message(message)
                return web.HTTPFound('/parameters')
            else:
                return web.Response(text="Parameter not found", status=404)

        except Exception as e:
            return web.Response(text=str(e), status=500)

    async def save_new_profile(self, request):
        """Сохранение нового профиля из JSON-данных в profile_phases """
        try:
            json_data = await request.json()  # Читаем JSON из запроса
            print("Received JSON data:", json_data)
            success = await self.db_manager.save_profile_data(json_data)

            if success:
                return web.json_response({"status": "success"})
            else:
                return web.json_response({"error": "Failed to save profile"}, status=500)

        except json.JSONDecodeError:
            return web.json_response({"error": "Invalid JSON format"}, status=400)
        except Exception as e:
            return web.json_response({"error": str(e)}, status=500)
            
    async def show_save_profile(self, request):
        """Отображение страницы сохранения профиля"""
        try:
            return web.FileResponse('./templates/save_profile.html')
        except Exception as e:
            return web.Response(text="Error loading command page", status=500)
            
    
    async def read_profile_db(self, request):
        profile_id = request.query.get('id')  # Используем параметр запроса
        print(f"ID профиля: {profile_id}")
        
        try:
            async with self.db_manager.params_pool.acquire() as conn:
                # Получаем общие данные профиля
                profile = await conn.fetchrow(
                    "SELECT nameprofile AS \"nameProfile\", cycle, sunrise, sunset FROM system_params WHERE id = $1",
                    int(profile_id)
                )
                if not profile:
                    return web.Response(text="Профиль не найден", status=404)
                
                # Получаем все фазы для профиля
                phases = await conn.fetch(
                    "SELECT duration, day_temperature AS \"dayTemp\", night_temperature AS \"nightTemp\", "
                    "day_humidity AS \"dayHum\", night_humidity AS \"nightHum\", "
                    "day_watering_interval AS \"dayWater\", night_watering_interval AS \"nightWater\", "
                    "water_temperature AS \"waterTemp\", day_ventilation AS \"dayVent\", "
                    "night_ventilation AS \"nightVent\", day_circulation AS \"dayCirc\", "
                    "night_circulation AS \"nightCirc\", day_rotation AS \"dayRot\", "
                    "night_rotation AS \"nightRot\", light_intensity AS \"light\" "
                    "FROM profile_phases WHERE profile_id = $1 ORDER BY phase_number",
                    int(profile_id)
                )
                
                profile_data = dict(profile)
                # Преобразуем sunrise/sunset из минут в формат HH:MM
                profile_data['sunrise'] = f"{profile_data['sunrise'] // 60:02d}:{profile_data['sunrise'] % 60:02d}"
                profile_data['sunset'] = f"{profile_data['sunset'] // 60:02d}:{profile_data['sunset'] % 60:02d}"
                profile_data['phases'] = [dict(phase) for phase in phases]
                
                profile_json = json.dumps(profile_data, ensure_ascii=False)
                print(f"Прочитано: {profile_json}")
                return web.Response(text=profile_json, content_type="application/json")
            
        except Exception as e:
            return web.Response(text=str(e), status=500)
