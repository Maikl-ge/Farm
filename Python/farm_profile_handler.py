import json
from aiohttp import web
import logging

class FarmProfileHandler:
    def __init__(self, db_manager):
        self.db_manager = db_manager
        self.logger = logging.getLogger(__name__)

    async def save_profile(self, request):
        """Сохранение профиля из CBOR-данных"""
        try:
            cbor_data = await request.read()  # Используем read() для получения байтов
            success = await self.db_manager.save_profile_from_cbor(cbor_data)
            return web.json_response({"status": "success" if success else "error"})
        except Exception as e:
            return web.json_response({"error": str(e)}, status=500)

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
            self.logger.error(f"Error saving profile data: {e}")
            return web.json_response({"error": str(e)}, status=500)

    async def show_save_profile(self, request):
        """Отображение страницы сохранения профиля"""
        try:
            return web.FileResponse('./templates/save_profile.html')
        except Exception as e:
            self.logger.error(f"Error loading save_profile.html: {e}")
            return web.Response(text="Error loading save_profile.html", status=500)

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
                profile_data['sunrise'] = f"{profile_data['sunrise'] // 60:02d}:{profile_data['sunrise'] % 60:02d}"
                profile_data['sunset'] = f"{profile_data['sunset'] // 60:02d}:{profile_data['sunset'] % 60:02d}"
                profile_data['phases'] = [dict(phase) for phase in phases]
                
                profile_json = json.dumps(profile_data, ensure_ascii=False)
                print(f"Прочитано: {profile_json}")
                return web.Response(text=profile_json, content_type="application/json")
            
        except Exception as e:
            return web.Response(text=str(e), status=500)