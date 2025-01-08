from quart import Quart, render_template, request, jsonify, redirect, url_for
from datetime import datetime
import asyncpg
import logging

app = Quart(__name__)

# Настройка логирования
logging.basicConfig(level=logging.DEBUG)

# Функция для подключения к базе данных PostgreSQL
async def get_db_connection():
    return await asyncpg.create_pool(
        user='CytiFarm', 
        password='Farm', 
        database='SystemParams', 
        host='127.0.0.1'
    )

# Функция для преобразования строки времени (HH:MM) в минуты с начала дня
def time_to_minutes(time_str):
    if time_str:
        try:
            hours, minutes = map(int, time_str.split(":"))
            return hours * 60 + minutes
        except ValueError as e:
            app.logger.error(f"Error parsing time '{time_str}': {e}")
            return None
    return None

# Функция для преобразования минут с начала дня в строку времени (HH:MM)
def minutes_to_time(minutes):
    if minutes is not None:
        try:
            hours = minutes // 60
            minutes = minutes % 60
            return f"{hours:02}:{minutes:02}"
        except ValueError as e:
            app.logger.error(f"Error converting minutes '{minutes}': {e}")
            return None
    return None

# Маршрут для получения настроек из БД
@app.route('/settings', methods=['GET'])
async def get_settings():
    try:
        # Асинхронное подключение к базе данных
        conn = await get_db_connection()
        async with conn.acquire() as connection:
            # Асинхронный запрос для получения данных
            result = await connection.fetchrow("SELECT * FROM system_params WHERE id = 10")  # Получаем одну строку

        if result:
            app.logger.debug(f"Fetched settings from the database: {result}")
            # Логируем все ключи результата
            app.logger.debug(f"Result keys: {list(result.keys())}")

            # Если настройки найдены, возвращаем их в формате JSON
            settings = {
                "nameProfile": result.get('nameprofile'),
                "cycle": result.get('cycle'),
                "sunrise": result.get('sunrise'),
                "sunset": result.get('sunset'),
                "dayTemperatureStart": result.get('daytemperaturestart'),
                "dayTemperatureEnd": result.get('daytemperatureend'),
                "nightTemperatureStart": result.get('nighttemperaturestart'),
                "nightTemperatureEnd": result.get('nighttemperatureend'),
                "dayHumidityStart": result.get('dayhumiditystart'),
                "dayHumidityEnd": result.get('dayhumidityend'),
                "nightHumidityStart": result.get('nighthumiditystart'),
                "nightHumidityEnd": result.get('nighthumidityend'),
                "waterTemperature": result.get('watertemperature'),
                "dayWateringInterval": str(result.get('daywateringinterval')),
                "nightWateringInterval": str(result.get('nightwateringinterval')),
                "dayCirculation": result.get('daycirculation'),
                "nightCirculation": result.get('nightcirculation'),
                "dayVentilation": result.get('dayventilation'),
                "nightVentilation": result.get('nightventilation'),
            }
            return jsonify(settings)  # Отправляем настройки как JSON
        else:
            return jsonify({"error": "Settings not found"}), 404
    except Exception as e:
        # Обработка ошибок
        app.logger.error(f"Error fetching settings: {e}")
        return jsonify({"error": "Internal server error"}), 500

# Главная страница для отображения всех профилей
@app.route("/", methods=["GET"])
async def index():
    try:
        # Чтение данных из базы данных
        conn = await get_db_connection()
        async with conn.acquire() as connection:
            result = await connection.fetch("SELECT * FROM system_params ORDER BY id ASC LIMIT 10")

        if result:
            app.logger.debug(f"Fetched data from the database: {result}")
            params = result
        else:
            app.logger.debug("No data found in the database.")
            params = None

        return await render_template("index.html", params=params)
    except Exception as e:
        app.logger.error(f"Error fetching data: {e}")
        return "Error fetching data", 500

# Страница для редактирования профиля
@app.route("/edit/<int:id>", methods=["GET", "POST"])
async def edit(id):
    if request.method == "POST":
        try:
            form = await request.form
            nameProfile = form.get("nameProfile", "Default")
            cycle = int(form.get("cycle", 1))
            sunrise = time_to_minutes(form.get("sunrise"))
            sunset = time_to_minutes(form.get("sunset"))
            if sunrise is None or sunset is None:
                return "Error parsing sunrise/sunset times", 400

            dayTemperatureStart = float(form["dayTemperatureStart"])
            dayTemperatureEnd = float(form["dayTemperatureEnd"])
            nightTemperatureStart = float(form["nightTemperatureStart"])
            nightTemperatureEnd = float(form["nightTemperatureEnd"])
            dayHumidityStart = float(form["dayHumidityStart"])
            dayHumidityEnd = float(form["dayHumidityEnd"])
            nightHumidityStart = float(form["nightHumidityStart"])
            nightHumidityEnd = float(form["nightHumidityEnd"])
            waterTemperature = float(form["waterTemperature"])

            # Преобразуем интервалы времени в минуты
            dayWateringInterval = time_to_minutes(form["dayWateringInterval"])
            nightWateringInterval = time_to_minutes(form["nightWateringInterval"])

            app.logger.debug(f"dayWateringInterval: {dayWateringInterval}, nightWateringInterval: {nightWateringInterval}")

            if dayWateringInterval is None or nightWateringInterval is None:
                return "Error parsing watering intervals", 400

            dayCirculation = int(form["dayCirculation"])
            nightCirculation = int(form["nightCirculation"])
            dayVentilation = int(form["dayVentilation"])
            nightVentilation = int(form["nightVentilation"])

            # Логирование данных формы
            app.logger.debug(f"Form data before saving: {form}")

            # Подключаемся к базе данных
            conn = await get_db_connection()
            async with conn.acquire() as connection:
                # Обновляем существующую запись
                app.logger.debug("Updating data in the database...")
                await connection.execute("""
                    UPDATE system_params SET
                        nameProfile = $1, cycle = $2, sunrise = $3, sunset = $4, dayTemperatureStart = $5, dayTemperatureEnd = $6,
                        nightTemperatureStart = $7, nightTemperatureEnd = $8, dayHumidityStart = $9, dayHumidityEnd = $10,
                        nightHumidityStart = $11, nightHumidityEnd = $12, waterTemperature = $13,
                        dayWateringInterval = $14, nightWateringInterval = $15, dayCirculation = $16, nightCirculation = $17,
                        dayVentilation = $18, nightVentilation = $19
                    WHERE id = $20
                """, nameProfile, cycle, sunrise, sunset, dayTemperatureStart, dayTemperatureEnd,
                   nightTemperatureStart, nightTemperatureEnd, dayHumidityStart, dayHumidityEnd,
                   nightHumidityStart, nightHumidityEnd, waterTemperature, dayWateringInterval,
                   nightWateringInterval, dayCirculation, nightCirculation, dayVentilation, nightVentilation, id)
            app.logger.debug("Data successfully updated in the database.")

            return redirect(url_for("index"))
        except Exception as e:
            app.logger.error(f"Error updating data: {e}")
            return "Error updating data", 500

    if request.method == "GET":
        try:
            # Чтение данных из базы данных
            conn = await get_db_connection()
            async with conn.acquire() as connection:
                result = await connection.fetch("SELECT * FROM system_params WHERE id = $1", id)

            if result:
                app.logger.debug(f"Fetched data from the database: {result[0]}")
                params = dict(result[0])

                # Преобразуем интервалы времени из минут в строку времени
                params['sunrise'] = minutes_to_time(params['sunrise'])
                params['sunset'] = minutes_to_time(params['sunset'])
                params['daywateringinterval'] = minutes_to_time(params['daywateringinterval'])
                params['nightwateringinterval'] = minutes_to_time(params['nightwateringinterval'])
            else:
                app.logger.debug("No data found in the database.")
                return "No data found", 404

            return await render_template("edit.html", params=params)
        except Exception as e:
            app.logger.error(f"Error fetching data: {e}")
            return "Error fetching data", 500

if __name__ == "__main__":
    app.run(debug=True, host="0.0.0.0")