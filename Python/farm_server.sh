#!/bin/bash

# Путь к проекту
PROJECT_DIR=~/farm

# Имя сервиса systemd
SERVICE_NAME="farm"

# Функция для запуска сервера
start_server() {
    echo "Запуск сервера..."
    sudo systemctl start "$SERVICE_NAME.service"
    sudo systemctl status "$SERVICE_NAME.service" --no-pager
}

# Функция для остановки сервера
stop_server() {
    echo "Остановка сервера..."
    sudo systemctl stop "$SERVICE_NAME.service"
    echo "Сервер остановлен."
}

# Функция для перезапуска сервера
restart_server() {
    echo "Перезапуск сервера..."
    sudo systemctl restart "$SERVICE_NAME.service"
    sudo systemctl status "$SERVICE_NAME.service" --no-pager
}

# Функция для проверки статуса сервера
check_server() {
    echo "Проверка статуса сервера..."
    sudo systemctl status "$SERVICE_NAME.service" --no-pager
}

# Основной блок для обработки аргументов командной строки
case "$1" in
    run)
        start_server
        ;;
    stop)
        stop_server
        ;;
    rst)
        restart_server
        ;;
    st)
        check_server
        ;;
    *)
        echo "Использование: $0 {run|stop|rst|st}"
        exit 1
        ;;
esac

exit 0