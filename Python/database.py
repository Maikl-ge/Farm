import asyncpg
import logging
from typing import Optional, Dict, Any
from datetime import datetime

class DatabaseManager:
    def __init__(self):
        """
        Инициализация менеджера баз данных.
        Настраивает конфигурацию для двух баз данных и логирование.
        """
        # Строки подключения к базам данных
        self.sensor_db_config = {
            'user': 'CytiFarm',
            'password': 'Farm',
            'database': 'sensor_data',
            'host': 'localhost',
            'port': 5432
        }
        
        self.params_db_config = {
            'user': 'CytiFarm',
            'password': 'Farm',
            'database': 'SystemParams',  # Имя базы данных, содержащей таблицу status_farm
            'host': 'localhost',
            'port': 5432        
        }

        # Пулы подключений
        self.sensor_pool: Optional[asyncpg.Pool] = None
        self.params_pool: Optional[asyncpg.Pool] = None
        
        # Настройка логирования
        self.logger = logging.getLogger(__name__)
        self._setup_logging()

    def _setup_logging(self) -> None:
        """Настройка параметров логирования"""
        logging.basicConfig(
            level=logging.INFO,
            format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
        )

    async def init_pools(self) -> None:
        """Инициализация пулов подключений к обеим базам данных"""
        try:
            self.sensor_pool = await asyncpg.create_pool(**self.sensor_db_config)
            self.logger.info("Sensor database pool created successfully")
            
            self.params_pool = await asyncpg.create_pool(**self.params_db_config)
            self.logger.info("Parameters database pool created successfully")
            
        except Exception as e:
            self.logger.error(f"Error creating database pools: {e}")
            raise

    async def close_pools(self) -> None:
        """Закрытие пулов подключений"""
        if self.sensor_pool:
            await self.sensor_pool.close()
        if self.params_pool:
            await self.params_pool.close()
        self.logger.info("Database pools closed")

    async def save_sensor_data(self, data: Dict[str, Any], timestamp: str) -> bool:
        """Сохранение данных сенсоров"""
        if not self.sensor_pool:
            raise RuntimeError("Sensor database pool not initialized")
            
        try:
            # Преобразование строки timestamp в объект datetime
            timestamp_dt = datetime.strptime(timestamp, "%Y-%m-%d %H:%M:%S")
            # Преобразование объекта datetime в строку
            timestamp_str = timestamp_dt.strftime("%Y-%m-%d %H:%M:%S")
            async with self.sensor_pool.acquire() as conn:
                await conn.execute('''
                    INSERT INTO sensor_data(
                        timestamp, "current_date", "current_time", 
                        start_button, stop_button, mode_button,
                        max_osmo_level, min_osmo_level, max_water_level, min_water_level,
                        temperature_1, humidity_1, temperature_2, humidity_2, 
                        temperature_3, humidity_3, temperature_4, humidity_4, 
                        temperature_5, humidity_5, water_temperature_osmo, 
                        water_temperature_watering, air_temperature_outdoor, 
                        air_temperature_inlet, ph_osmo, tds_osmo, power_monitor
                    ) VALUES($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, 
                            $13, $14, $15, $16, $17, $18, $19, $20, $21, $22, 
                            $23, $24, $25, $26, $27)
                ''', timestamp_str, data["DF"], data["TF"], data["start_Button"], 
                    data["stop_Button"], data["mode_Button"], data["max_osmo_level"], 
                    data["min_osmo_level"], data["max_water_level"], data["min_water_level"], 
                    data["T1"], data["H1"], data["T2"], data["H2"], data["T3"], 
                    data["H3"], data["T4"], data["H4"], data["T5"], data["H5"], 
                    data["WTO"], data["WTW"], data["ATO"], data["ATI"], data["ph"], 
                    data["tds"], data["pm"])
                self.logger.info(f"Sensor data saved successfully at {timestamp}")
                return True
        except Exception as e:
            self.logger.error(f"Error saving sensor data: {e}")
            return False

    async def save_status_data(self, data: Dict[str, Any], timestamp: str) -> bool:
        """Сохранение статусных данных"""
        if not self.params_pool:
            raise RuntimeError("Status database pool not initialized")
        
        try:
            # Преобразование строки timestamp в объект datetime
            timestamp_dt = datetime.strptime(timestamp, "%Y-%m-%d %H:%M:%S")
            # Преобразование целочисленных значений в булевые
            data["OSMOS_ON"] = bool(data["OSMOS_ON"])
            data["PUMP_WATERING"] = bool(data["PUMP_WATERING"])
            data["PUMP_TRANSFER"] = bool(data["PUMP_TRANSFER"])
            data["WATER_OUT"] = bool(data["WATER_OUT"])
            data["STEAM_IN"] = bool(data["STEAM_IN"])
            data["ENABLE"] = bool(data["ENABLE"])
            async with self.params_pool.acquire() as conn:
                await conn.execute('''
                    INSERT INTO status_farm(
                        timestamp, osmos_on, pump_watering, pump_transfer, water_out, steam_in,
                        light, fan_rack, fan_shelf, fan_circ, fan_inlet, hiter_air, hiter_water, fan_option,
                        step, dir, enable
                    ) VALUES($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13, $14, $15, $16, $17)
                ''',
                timestamp_dt, data["OSMOS_ON"], data["PUMP_WATERING"], data["PUMP_TRANSFER"], data["WATER_OUT"], data["STEAM_IN"],
                data["LIGHT"], data["FAN_RACK"], data["FAN_SHELF"], data["FAN_CIRC"], data["FAN_INLET"], data["HITER_AIR"], data["HITER_WATER"], data["FAN_OPTION"],
                data["STEP"], data["DIR"], data["ENABLE"])
                self.logger.info(f"Status data saved successfully at {timestamp}")
                return True
        except Exception as e:
            self.logger.error(f"Error saving status data: {e}")
            return False

    async def get_system_params(self, profile_id: int = 10) -> Optional[Dict[str, Any]]:
        """Получение параметров системы"""
        if not self.params_pool:
            raise RuntimeError("Parameters database pool not initialized")
            
        try:
            async with self.params_pool.acquire() as conn:
                result = await conn.fetchrow(
                    "SELECT * FROM system_params WHERE id = $1", profile_id
                )
                if result:
                    return dict(result)
                return None
        except Exception as e:
            self.logger.error(f"Error fetching system parameters: {e}")
            return None

    async def update_system_params(self, profile_id: int, params: Dict[str, Any]) -> bool:
        """Обновление параметров системы"""
        if not self.params_pool:
            raise RuntimeError("Parameters database pool not initialized")
            
        try:
            async with self.params_pool.acquire() as conn:
                result = await conn.execute("""
                    UPDATE system_params SET
                        nameProfile = $1, cycle = $2, sunrise = $3, sunset = $4,
                        dayTemperatureStart = $5, dayTemperatureEnd = $6,
                        nightTemperatureStart = $7, nightTemperatureEnd = $8,
                        dayHumidityStart = $9, dayHumidityEnd = $10,
                        nightHumidityStart = $11, nightHumidityEnd = $12,
                        waterTemperature = $13, dayWateringInterval = $14,
                        nightWateringInterval = $15, dayCirculation = $16,
                        nightCirculation = $17, dayVentilation = $18,
                        nightVentilation = $19
                    WHERE id = $20
                """, params['nameProfile'], params['cycle'], params['sunrise'],
                    params['sunset'], params['dayTemperatureStart'],
                    params['dayTemperatureEnd'], params['nightTemperatureStart'],
                    params['nightTemperatureEnd'], params['dayHumidityStart'],
                    params['dayHumidityEnd'], params['nightHumidityStart'],
                    params['nightHumidityEnd'], params['waterTemperature'],
                    params['dayWateringInterval'], params['nightWateringInterval'],
                    params['dayCirculation'], params['nightCirculation'],
                    params['dayVentilation'], params['nightVentilation'],
                    profile_id)
                
                success = result.lower().endswith('update 1')
                if success:
                    self.logger.info(f"System parameters updated successfully for profile {profile_id}")
                return success
                
        except Exception as e:
            self.logger.error(f"Error updating system parameters: {e}")
            return False

    async def save_profile_data(self, profile_data: Dict[str, Any]) -> bool:
        if 'nameProfile' not in profile_data or 'phases' not in profile_data:
            self.logger.error("Missing required profile fields")
            return False

        try:
            self.logger.info(f"Received profile data: {profile_data}")
            async with self.params_pool.acquire() as conn:
                async with conn.transaction():
                    sunrise_hours, sunrise_minutes = map(int, profile_data['sunrise'].split(':'))
                    sunset_hours, sunset_minutes = map(int, profile_data['sunset'].split(':'))
                    sunrise_total = sunrise_hours * 60 + sunrise_minutes
                    sunset_total = sunset_hours * 60 + sunset_minutes

                    profile_id = profile_data.get('id')
                    if profile_id:
                        # Проверяем, существует ли запись, и обновляем её
                        result = await conn.execute('''
                            UPDATE system_params 
                            SET nameprofile = $1, cycle = $2, sunrise = $3, sunset = $4
                            WHERE id = $5
                        ''', profile_data['nameProfile'], profile_data['cycle'], sunrise_total, sunset_total, profile_id)
                        if result == "UPDATE 0":  # Если запись не найдена
                            self.logger.error(f"No profile found with id {profile_id}")
                            return False
                    else:
                        # Создаем новую запись
                        profile_id = await conn.fetchval('''
                            INSERT INTO system_params (nameprofile, cycle, sunrise, sunset)
                            VALUES ($1, $2, $3, $4)
                            ON CONFLICT (nameprofile) DO UPDATE
                            SET cycle = EXCLUDED.cycle,
                                sunrise = EXCLUDED.sunrise,
                                sunset = EXCLUDED.sunset
                            RETURNING id
                        ''', profile_data['nameProfile'], profile_data['cycle'], sunrise_total, sunset_total)

                    # Удаляем старые фазы
                    await conn.execute('DELETE FROM profile_phases WHERE profile_id = $1', profile_id)

                    # Вставляем новые фазы
                    for i, phase in enumerate(profile_data['phases'], 1):
                        if not isinstance(phase, dict):
                            self.logger.error(f"Phase {i} is not a dictionary: {phase}")
                            return False

                        await conn.execute('''
                            INSERT INTO profile_phases (
                                profile_id, phase_number, duration,
                                day_temperature, night_temperature,
                                day_humidity, night_humidity,
                                day_watering_interval, night_watering_interval,
                                water_temperature, day_ventilation,
                                night_ventilation, day_circulation,
                                night_circulation, day_rotation,
                                night_rotation, light_intensity
                            ) VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13, $14, $15, $16, $17)
                        ''', profile_id, i, phase.get('duration', 0),
                            phase.get('dayTemp', 0.0), phase.get('nightTemp', 0.0),
                            phase.get('dayHum', 0.0), phase.get('nightHum', 0.0),
                            phase.get('dayWater', 0), phase.get('nightWater', 0),
                            phase.get('waterTemp', 0.0), phase.get('dayVent', 0),
                            phase.get('nightVent', 0), phase.get('dayCirc', 0),
                            phase.get('nightCirc', 0), phase.get('dayRot', 0),
                            phase.get('nightRot', 0), phase.get('light', 0))

                    self.logger.info(f"Profile data saved successfully with ID: {profile_id}")
                    return True
        except Exception as e:
            self.logger.error(f"Error saving profile data: {e}")
            return False
        
    async def save_profile_data(self, profile_data: Dict[str, Any]) -> bool:
        if 'nameProfile' not in profile_data or 'phases' not in profile_data:
            self.logger.error("Missing required profile fields")
            return False

        try:
            self.logger.info(f"Received profile data: {profile_data}")
            async with self.params_pool.acquire() as conn:
                async with conn.transaction():
                    sunrise_hours, sunrise_minutes = map(int, profile_data['sunrise'].split(':'))
                    sunset_hours, sunset_minutes = map(int, profile_data['sunset'].split(':'))
                    sunrise_total = sunrise_hours * 60 + sunrise_minutes
                    sunset_total = sunset_hours * 60 + sunset_minutes

                    profile_id = profile_data.get('id')
                    if profile_id:
                        result = await conn.execute('''
                            UPDATE system_params 
                            SET nameprofile = $1, cycle = $2, sunrise = $3, sunset = $4
                            WHERE id = $5
                        ''', profile_data['nameProfile'], profile_data['cycle'], sunrise_total, sunset_total, profile_id)
                        if result == "UPDATE 0":
                            self.logger.error(f"No profile found with id {profile_id}")
                            return False
                    else:
                        profile_id = await conn.fetchval('''
                            INSERT INTO system_params (nameprofile, cycle, sunrise, sunset)
                            VALUES ($1, $2, $3, $4)
                            ON CONFLICT (nameprofile) DO UPDATE
                            SET cycle = EXCLUDED.cycle,
                                sunrise = EXCLUDED.sunrise,
                                sunset = EXCLUDED.sunset
                            RETURNING id
                        ''', profile_data['nameProfile'], profile_data['cycle'], sunrise_total, sunset_total)

                    await conn.execute('DELETE FROM profile_phases WHERE profile_id = $1', profile_id)

                    for phase_num in range(1, 7):
                        phase_key = f"phase{phase_num}"
                        if phase_key not in profile_data:
                            continue
                        phase = profile_data[phase_key]

                        if not isinstance(phase, dict):
                            self.logger.error(f"Phase {phase_num} is not a dictionary: {phase}")
                            return False

                        await conn.execute('''
                            INSERT INTO profile_phases (
                                profile_id, phase_number, duration,
                                day_temperature, night_temperature,
                                day_humidity, night_humidity,
                                day_watering_interval, night_watering_interval,
                                water_temperature, day_ventilation,
                                night_ventilation, day_circulation,
                                night_circulation, day_rotation,
                                night_rotation, light_intensity
                            ) VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13, $14, $15, $16, $17)
                        ''', profile_id, phase_num, 
                            phase.get(f'phase{phase_num}_duration', 0),
                            phase.get(f'phase{phase_num}_temp', 0.0), 
                            phase.get(f'phase{phase_num}_temp', 0.0), 
                            phase.get(f'phase{phase_num}_hum', 0.0),
                            phase.get(f'phase{phase_num}_hum', 0.0),
                            phase.get(f'phase{phase_num}_watering', 0),
                            phase.get(f'phase{phase_num}_watering', 0),
                            phase.get(f'phase{phase_num}_water_temp', 0.0),
                            phase.get(f'phase{phase_num}_vent', 0),
                            phase.get(f'phase{phase_num}_vent', 0),
                            phase.get(f'phase{phase_num}_circ', 0),
                            phase.get(f'phase{phase_num}_circ', 0),
                            phase.get(f'phase{phase_num}_rot', 0),
                            phase.get(f'phase{phase_num}_rot', 0),
                            phase.get(f'phase{phase_num}_light', 0)
                        )

                    self.logger.info(f"Profile data saved successfully with ID: {profile_id}")
                    return True
        except Exception as e:
            self.logger.error(f"Error saving profile data: {e}")
            return False
