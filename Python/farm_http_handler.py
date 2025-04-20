import aiohttp_jinja2
from aiohttp import web
import logging
import json
import traceback
from datetime import datetime

# Определяем логгер внутри модуля
logger = logging.getLogger(__name__)

# Глобальная переменная
farm_id = '255'

cmd_start = "SCMD";       # Команда на запуск
cmd_stop = "SCMS";        # Команда на остановку
cmd_restart = "SCMR";     # Команда на перезапуск
cmd_update = "SCMU";      # Команда на обновление ПО
cmd_settings = "SCME";    # Команда на обновление настроек

req_status = "SRST";      # Запрос о статусе фермы
req_data = "SRDT";        # Запрос данных фермы
req_settings = "SRSE";    # Запрос о настройках фермы Профиля настроек сохраненных 
req_parameters = "SRPM";  # Запрос о параметрах фермы
req_profile = "SRPF";     # Запрос о профиле фермы
req_current = "SRCU";     # Запрос  текущих данных фермы

class FarmHTTPHandler:
    def __init__(self, db_manager, websocket_handler):
        self.db_manager = db_manager
        self.websocket_handler = websocket_handler
        self.logger = logger

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

    async def show_parameters(self, request):
        """Отображение списка профилей параметров"""
        try:
            async with self.db_manager.params_pool.acquire() as conn:
                params = await conn.fetch('''
                    SELECT * FROM profile_phases 
                    ORDER BY profileid ASC 
                    LIMIT 10 OFFSET 0
                ''')

            context = {'params': params}
            return aiohttp_jinja2.render_template(
                'parameters.html',
                request,
                context
            )
        except Exception as e:
            self.logger.error(f"Error showing parameters: {e}")
            return web.Response(text=str(e), status=500)
            
    async def select_parameter(self, request):
        """Метод для выбора параметра"""
        try:
            profile_id = request.match_info.get('profileid')
            async with self.db_manager.params_pool.acquire() as conn:
                params = await conn.fetchrow('''
                    SELECT * FROM profile_phases 
                    WHERE profileid = $1
                ''', int(profile_id))
                              
                                    
                # Данные для формирования команды
                farm_id = "255"
                req_settings = "SCME"
                params_dict = dict(params)

                # Формируем строку команды
                command_str = f"{farm_id} {req_settings} {params_dict}"
                # Отправляем команду через WebSocket
                result = await self.websocket_handler.send_raw_command(command_str)

                print(f"Selected parameters for send: {command_str}")
                
                if not params:
                    raise web.HTTPNotFound(text="Profile not found")
                
                return aiohttp_jinja2.render_template(
                    'parameters.html',
                    request,
                    {'params': dict(params)}
                )

        except web.HTTPNotFound:
            raise
        except web.HTTPBadRequest as e:
            self.logger.error(f"Bad request: {e}")
            return web.Response(text=str(e), status=400)
        except Exception as e:
            self.logger.error(f"Error selecting parameter: {e}")
            return web.Response(text=str(e), status=500)          
            

    async def edit_parameters(self, request):
        """Редактирование профиля параметров"""
        try:
            profile_id = request.match_info.get('id')

            async with self.db_manager.params_pool.acquire() as conn:
                params = await conn.fetchrow('''
                    SELECT * FROM profile_phases 
                    WHERE profileid = $1
                ''', int(profile_id))

                if not params:
                    raise web.HTTPNotFound(text="Profile not found")

                if request.method == 'POST':
                    data = await request.post()
                    
                    sunrise = data['sunrise'].split(':')
                    sunrise_minutes = int(sunrise[0]) * 60 + int(sunrise[1])
                    
                    sunset = data['sunset'].split(':')
                    sunset_minutes = int(sunset[0]) * 60 + int(sunset[1])
                    
                    await conn.execute('''
                        UPDATE profile_phases 
                        SET nameprofile = $1,
                            cycle = $2,
                            sunrise = $3,
                            sunset = $4,
                            phase1_duration = $5,
                            phase1_temp = $6,
                            phase1_hum = $7,
                            phase1_water_temp = $8, 
                            phase1_light = $9,
                            phase1_circ = $10,
                            phase1_vent = $11,
                            phase1_watering = $12,
                            phase1_draining = $13,
                            phase1_rot = $14,
                            phase2_duration = $15,
                            phase2_temp = $16,
                            phase2_hum = $17,
                            phase2_water_temp = $18,
                            phase2_light = $19,
                            phase2_circ = $20,
                            phase2_vent = $21,
                            phase2_watering = $22,
                            phase2_draining = $23,
                            phase2_rot = $24,
                            phase3_duration = $25,
                            phase3_temp = $26,
                            phase3_hum = $27,
                            phase3_water_temp = $28,
                            phase3_light = $29,
                            phase3_circ = $30,
                            phase3_vent = $31,
                            phase3_watering = $32,
                            phase3_draining = $33,
                            phase3_rot = $34,
                            phase4_duration = $35,
                            phase4_temp = $36,
                            phase4_hum = $37,
                            phase4_water_temp = $38,
                            phase4_light = $39,
                            phase4_circ = $40,
                            phase4_vent = $41,
                            phase4_watering = $42,
                            phase4_draining = $43,
                            phase4_rot = $44,
                            phase5_duration = $45,
                            phase5_temp = $46,
                            phase5_hum = $47,
                            phase5_water_temp = $48,
                            phase5_light = $49,
                            phase5_circ = $50,
                            phase5_vent = $51,
                            phase5_watering = $52,
                            phase5_draining = $53,
                            phase5_rot = $54,
                            phase6_duration = $55,
                            phase6_temp = $56,
                            phase6_hum = $57,
                            phase6_water_temp = $58,
                            phase6_light = $59,
                            phase6_circ = $60,
                            phase6_vent = $61,
                            phase6_watering = $62,
                            phase6_draining = $63,
                            phase6_rot = $64
                        WHERE profileid = $65
                    ''',
                    
                    data['nameProfile'],
                    int(data['cycle']),
                    sunrise_minutes,
                    sunset_minutes,
                    int(data['phase1_duration']),
                    float(data['phase1_temp']),
                    float(data['phase1_hum']),
                    float(data['phase1_water_temp']),
                    int(data['phase1_light']),
                    int(data['phase1_circ']),
                    int(data['phase1_vent']),
                    int(data['phase1_watering']),
                    int(data['phase1_draining']),
                    int(data['phase1_rot']),
                    int(data['phase2_duration']),
                    float(data['phase2_temp']),
                    float(data['phase2_hum']),
                    float(data['phase2_water_temp']),
                    int(data['phase2_light']),
                    int(data['phase2_circ']),
                    int(data['phase2_vent']),
                    int(data['phase2_watering']),
                    int(data['phase2_draining']),
                    int(data['phase2_rot']),
                    int(data['phase3_duration']),
                    float(data['phase3_temp']),
                    float(data['phase3_hum']),
                    float(data['phase3_water_temp']),
                    int(data['phase3_light']),
                    int(data['phase3_circ']),
                    int(data['phase3_vent']),
                    int(data['phase3_watering']),
                    int(data['phase3_draining']),
                    int(data['phase3_rot']),
                    int(data['phase4_duration']),
                    float(data['phase4_temp']),
                    float(data['phase4_hum']),
                    float(data['phase4_water_temp']),
                    int(data['phase4_light']),
                    int(data['phase4_circ']),
                    int(data['phase4_vent']),
                    int(data['phase4_watering']),
                    int(data['phase4_draining']),
                    int(data['phase4_rot']),
                    int(data['phase5_duration']),
                    float(data['phase5_temp']),
                    float(data['phase5_hum']),
                    float(data['phase5_water_temp']),
                    int(data['phase5_light']),
                    int(data['phase5_circ']),
                    int(data['phase5_vent']),
                    int(data['phase5_watering']),
                    int(data['phase5_draining']),
                    int(data['phase5_rot']),
                    int(data['phase6_duration']),
                    float(data['phase6_temp']),
                    float(data['phase6_hum']),
                    float(data['phase6_water_temp']),
                    int(data['phase6_light']),
                    int(data['phase6_circ']),
                    int(data['phase6_vent']),
                    int(data['phase6_watering']),
                    int(data['phase6_draining']),
                    int(data['phase6_rot']),
                    int(profile_id),                     )
                    
                    raise web.HTTPFound('/parameters')

            params = dict(params)
            params['sunrise'] = f"{params['sunrise'] // 60:02d}:{params['sunrise'] % 60:02d}"
            params['sunset'] = f"{params['sunset'] // 60:02d}:{params['sunset'] % 60:02d}"

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
            
            

    async def send_cmd(self, request):
        """Метод для отправки команд по клику"""
        try:
            data = await request.json()  # Получаем JSON из запроса
            cmd = data.get("command")  # Извлекаем команду
            ask_end = data.get("ask_comm")  # Извлекаем команду            

            if not cmd:
                raise web.HTTPBadRequest(text="Missing 'command' parameter")

            farm_id = "255"
            command_str = f"{farm_id} {cmd} {ask_end}"  # Формируем строку команды

            if self.websocket_handler:
                result = await self.websocket_handler.send_raw_command(command_str)
            else:
                raise web.HTTPInternalServerError(text="WebSocket handler is not initialized")

            #print(f"Sent command: {command_str}")

            return aiohttp_jinja2.render_template(
                'command.html', request, {'command': command_str, 'result': result}
            )

        except Exception as e:
            print(f"Error sending command: {e}")
            raise web.HTTPInternalServerError(text=f"Internal Server Error: {str(e)}")
            
            
    async def handle_http_report(self, request: web.Request):
        """Обрабатывает полученное HTTP-сообщение"""
        client_ip = request.remote
        body = await request.text()

        #print(f"\nHTTP message from {client_ip} ===\n{body}\n")
        self.logger.info(f"Received HTTP message from {client_ip}: {body}")

        parts = body.split(' ', 3)
        if len(parts) < 4:
            return web.json_response({'error': 'Invalid message format'}, status=400)

        id_farm = parts[0]
        type_msg = parts[1]
        json_length = parts[2]

        try:
            data = json.loads(parts[3])
        except json.JSONDecodeError:
            return web.json_response({'error': 'Invalid JSON payload'}, status=400)

        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        #print(f"[DEBUG] datetime = {datetime}, type = {type(datetime)}")
        
        #self.logger.info(f"{timestamp} - Получено сообщение по HTTP от {client_ip}: {body}")

        if type_msg == "FRQS":
            self.websocket_handler.frqs_data = data
            self.logger.info(f"FRQS data updated: {data}")
        elif type_msg == "FLIN":
            success = await self.db_manager.save_sensor_data(data, timestamp)
            if success:
                self.logger.info(f"{timestamp} - Данные по http от клиента {id_farm} успешно сохранены")
            else:
                self.logger.error(f"{timestamp} - Ошибка при сохранении данных  по http")
        elif type_msg == "FDST":
            success = await self.db_manager.save_status_data(data, timestamp)
            if success:
                self.logger.info(f"{timestamp} - Статусные по http данные от клиента {id_farm} успешно сохранены")
            else:
                self.logger.error(f"{timestamp} - Ошибка при сохранении статусных данных по http ")
        else:
            self.logger.warning(f"Unknown HTTP message type from {client_ip}: {type_msg}")
            return web.json_response({'error': 'Unknown message type'}, status=400)

        # ⬇️ финальный return здесь, после всех условий
        return web.json_response({'status': 'ok'})
