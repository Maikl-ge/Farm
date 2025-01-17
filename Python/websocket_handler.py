import asyncio
import websockets
import json
import logging
import aiohttp_jinja2
from datetime import datetime
from typing import Optional, Set

# Настройка логирования
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',  # Исправлено 'levellevel' на 'levelname'
    handlers=[
        logging.FileHandler("websocket_handler.log"),
        logging.StreamHandler()
    ]
)

logger = logging.getLogger(__name__)

# Пример использования логирования
logger.info("WebSocket handler initialized")
logger.error("Error in WebSocket handler")

# Глобальный логгер для модуля
_logger = logging.getLogger('websocket_handler')
if not _logger.handlers:
    _logger.setLevel(logging.INFO)
    handler = logging.StreamHandler()
    handler.setFormatter(logging.Formatter(
        '%(asctime)s - %(name)s - %(levelname)s - %(message)s'  # Исправлено 'levellevel' на 'levelname'
    ))
    _logger.addHandler(handler)
    _logger.propagate = False

class FarmWebSocketHandler:
    def __init__(self, db_manager, ping_interval=5, ping_timeout=15):
        """
        Инициализация WebSocket обработчика
        ping_interval=5 - интервал пинга в секундах
        ping_timeout=15 - таймаут в секундах
        """
        self.db_manager = db_manager
        self.ping_interval = ping_interval
        self.ping_timeout = ping_timeout
        self.connected_clients: Set[websockets.WebSocketServerProtocol] = set()
        self.websocket_state = "disconnected"
        self.frqs_data = None
        self.logger = _logger

    def update_websocket_state(self, new_state: str) -> None:
        """Обновление состояния WebSocket"""
        if self.websocket_state != new_state:
            self.websocket_state = new_state
            self.logger.info(f"WebSocket state changed to: {new_state}")

    async def check_websocket_state(self) -> None:
        """Проверка состояния WebSocket соединений"""
        while True:
            try:
                await asyncio.sleep(5)
                if self.connected_clients:
                    self.update_websocket_state("connected")
                else:
                    self.update_websocket_state("disconnected")
            except asyncio.CancelledError:
                self.logger.info("WebSocket state check task cancelled")
                break
            except Exception as e:
                self.logger.error(f"Error in check_websocket_state: {e}")
                await asyncio.sleep(5)

    async def broadcast_message(self, message: str) -> None:
        """Отправка сообщения всем подключенным клиентам"""
        if not self.connected_clients:
            return

        clients = self.connected_clients.copy()
        disconnected_clients = set()

        for client in clients:
            try:
                if len(message) > 1024:  # Ограничение на длину сообщения
                    raise ValueError("Message size exceeds limit")
                await client.send(message)
            except websockets.exceptions.ConnectionClosed:
                disconnected_clients.add(client)
                self.logger.info(f"Client {id(client)} connection closed during broadcast")
            except Exception as e:
                disconnected_clients.add(client)
                self.logger.error(f"Error sending message to client {id(client)}: {e}")

        # Удаляем отключенные клиенты
        for client in disconnected_clients:
            if client in self.connected_clients:
                self.connected_clients.remove(client)
                self.logger.info(f"Removed disconnected client: {id(client)}")

        if not self.connected_clients:
            self.update_websocket_state("disconnected")

    async def send_command(self, command: str) -> bool:
        """Отправка команды всем подключенным клиентам"""
        try:
            if not isinstance(command, str):
                raise ValueError(f"Expected command to be a str, got {type(command)}")

            if len(command) > 1024:  # Ограничение на длину команды
                raise ValueError("Command size exceeds limit")
            await self.broadcast_message(command)
            self.logger.debug(f"Command {command} sent to {len(self.connected_clients)} clients")
            return True
        except Exception as e:
            self.logger.error(f"Error sending command: {e}")
            return False

    async def command_to_farm(self, parameter: dict) -> bool:
        """Асинхронная отправка команды на ферму"""
        try:
            if not isinstance(parameter, dict):
                raise ValueError(f"Expected parameter to be a dict, got {type(parameter)}")
            command_str = json.dumps(parameter)
            
            if len(command_str) > 1024:  # Ограничение на длину команды
                raise ValueError("Command size exceeds limit")
            await self.broadcast_message(command_str)
            self.logger.info(f"Command sent to farm: {command_str}")
            return True
        except Exception as e:
            self.logger.error(f"Error sending command to farm: {e}")
            return False

    async def handle_connection(self, websocket: websockets.WebSocketServerProtocol, path: str) -> None:
        """Обработка WebSocket соединения"""
        client_id = id(websocket)
        self.logger.info(f"New client connected: {client_id}")

        try:
            websocket.ping_timeout = self.ping_timeout  # Устанавливаем таймаут для ping
            self.connected_clients.add(websocket)
            self.update_websocket_state("connected")

            async for message in websocket:
                try:
                    if len(message) > 1024:  # Ограничение на длину сообщения
                        raise ValueError("Message size exceeds limit")

                    parts = message.split(' ', 3)
                    if len(parts) < 4:
                        continue

                    # Сразу отправляем ACK, проверив только формат сообщения
                    id_farm = parts[0]
                    type_msg = parts[1]
                    ack_message = f"{id_farm} {type_msg} ACK"
                    await websocket.send(ack_message)

                    # Теперь можно логировать и обрабатывать данные
                    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                    self.logger.info(f"{timestamp} - Получено сообщение от {client_id}: {message}")

                    json_length = parts[2]
                    data = json.loads(parts[3])

                    if type_msg == "FRQS":
                        self.frqs_data = data
                        self.logger.info(f"FRQS data updated: {data}")
                    elif type_msg == "FLIN":
                        success = await self.db_manager.save_sensor_data(data, timestamp)
                        if success:
                            self.logger.info(f"{timestamp} - Данные от клиента {id_farm} успешно сохранены")
                        else:
                            self.logger.error(f"{timestamp} - Ошибка при сохранении данных")
                    else:
                        self.logger.warning(f"Unknown message type from client {client_id}: {type_msg}")

                except json.JSONDecodeError as e:
                    self.logger.error(f"JSON decode error from client {client_id}: {e}")
                except ValueError as e:
                    self.logger.error(f"Value error: {e}")
                except Exception as e:
                    self.logger.error(f"Error processing message from client {client_id}: {e}")

        except websockets.exceptions.ConnectionClosed:
            self.logger.info(f"Client {client_id} connection closed")
            if websocket in self.connected_clients:
                self.connected_clients.remove(websocket)
            if not self.connected_clients:
                self.reset_state()  # Очищаем состояние если нет подключенных клиентов

        except Exception as e:
            self.logger.error(f"Error in connection handler: {e}")
            if websocket in self.connected_clients:
                self.connected_clients.remove(websocket)
            if not self.connected_clients:
                self.reset_state()

    async def get_frqs_data(self):
        """Получение текущих FRQS данных"""
        return self.frqs_data

    def reset_state(self):
        """Очистка состояния WebSocket обработчика"""
        self.connected_clients.clear()
        self.websocket_state = "disconnected"
        self.frqs_data = None
        self.logger.info("WebSocket handler state reset")