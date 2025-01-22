# watchdog.py
import psutil
import time
import subprocess
import logging

# Настройка логирования
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

# Конфигурация сторожевой собаки
CPU_THRESHOLD = 80  # Пороговое значение для использования CPU в процентах
MEMORY_THRESHOLD = 80  # Пороговое значение для использования памяти в процентах
CHECK_INTERVAL = 5  # Интервал проверки в секундах
SERVER_COMMAND = "python main.py"  # Команда для запуска сервера

def is_server_running():
    """Проверяет, запущен ли сервер."""
    for process in psutil.process_iter(['pid', 'name']):
        if 'python' in process.info['name']:
            return True
    return False

def restart_server():
    """Перезапускает сервер."""
    logging.info("Перезапуск сервера...")
    subprocess.Popen(SERVER_COMMAND, shell=True)

def monitor_resources():
    """Мониторинг ресурсов сервера."""
    while True:
        cpu_usage = psutil.cpu_percent(interval=1)
        memory_usage = psutil.virtual_memory().percent

        logging.info(f"Использование CPU: {cpu_usage}%")
        logging.info(f"Использование памяти: {memory_usage}%")

        if cpu_usage > CPU_THRESHOLD or memory_usage > MEMORY_THRESHOLD:
            logging.warning("Превышены пороговые значения ресурсов!")
            if is_server_running():
                logging.info("Сервер работает. Перезапуск не требуется.")
            else:
                logging.info("Сервер не работает. Перезапуск...")
                restart_server()

        time.sleep(CHECK_INTERVAL)

if __name__ == "__main__":
    logging.info("Запуск сторожевой собаки...")
    if not is_server_running():
        logging.info("Сервер не работает. Запуск...")
        restart_server()

    monitor_resources()