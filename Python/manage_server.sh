#!/bin/bash

# Путь к проекту
PROJECT_DIR=~/my_project

# Путь к виртуальному окружению
VENV_PATH="$PROJECT_DIR/venv"

# Команда для запуска сервера
SERVER_COMMAND="python3 main.py"

# PID файл для отслеживания процесса сервера
PID_FILE="$PROJECT_DIR/server.pid"

# Функция для активации виртуального окружения
activate_venv() {
    source "$VENV_PATH/bin/activate"
}

# Функция для запуска сервера
start_server() {
    if [ -f "$PID_FILE" ]; then
        echo "Сервер уже запущен."
    else
        echo "Запуск сервера..."
        cd "$PROJECT_DIR" || exit
        activate_venv
        nohup $SERVER_COMMAND &> /dev/null &
        echo $! > "$PID_FILE"
        echo "Сервер запущен с PID $(cat $PID_FILE)"
    fi
}

# Функция для остановки сервера
stop_server() {
    if [ -f "$PID_FILE" ]; then
        echo "Остановка сервера..."
        kill $(cat "$PID_FILE")
        rm -f "$PID_FILE"
        echo "Сервер остановлен."
    else
        echo "Сервер не запущен."
    fi
}

# Функция для перезапуска сервера
restart_server() {
    stop_server
    start_server
}

# Функция для проверки статуса сервера
check_server() {
    if [ -f "$PID_FILE" ]; then
        PID=$(cat "$PID_FILE")
        if ps -p $PID > /dev/null; then
            echo "Сервер работает с PID $PID"
        else
            echo "Сервер не работает, но PID файл существует. Запуск сервера..."
            rm -f "$PID_FILE"
            start_server
        fi
    else
        echo "Сервер не работает. Запуск сервера..."
        start_server
    fi
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