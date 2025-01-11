import asyncpg

# Строка подключения к PostgreSQL
pg_dsn = 'postgresql://CytiFarm:Farm@localhost:5432/sensor_data'
pg_dsp = 'postgresql://CytiFarm:Farm@localhost:5432/SystemParams'

async def create_db_pool():
    return await asyncpg.create_pool(dsn=pg_dsn)

# Функция для сохранения данных сенсоров в базу данных
async def save_sensor_data(conn, data, timestamp):
    await conn.execute('''
        INSERT INTO sensor_data(
            timestamp, "current_date", "current_time", start_button, stop_button, mode_button,
            max_osmo_level, min_osmo_level, max_water_level, min_water_level,
            temperature_1, humidity_1, temperature_2, humidity_2, temperature_3, humidity_3,
            temperature_4, humidity_4, temperature_5, humidity_5,
            water_temperature_osmo, water_temperature_watering,
            air_temperature_outdoor, air_temperature_inlet, ph_osmo, tds_osmo, power_monitor
        ) VALUES($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13, $14, $15, $16, $17, $18, $19, $20, $21, $22, $23, $24, $25, $26, $27)
    ''', timestamp, data["DF"], data["TF"], data["start_Button"], data["stop_Button"], data["mode_Button"],
       data["max_osmo_level"], data["min_osmo_level"], data["max_water_level"], data["min_water_level"],
       data["T1"], data["H1"], data["T2"], data["H2"], data["T3"], data["H3"],
       data["T4"], data["H4"], data["T5"], data["H5"],
       data["WTO"], data["WTW"], data["ATO"], data["ATI"], data["ph"], data["tds"], data["pm"])