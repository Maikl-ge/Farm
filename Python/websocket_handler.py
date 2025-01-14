import asyncio
import websockets
import json
import logging
from datetime import datetime
from typing import Optional, Set

# Глобальный логгер для модуля
_logger = None

class FarmWebSocketHandler:
    def __init__(self, db_manager, ping_interval=5, ping_timeout=30):
        """
        Инициализация WebSocket обработчика
        ping_interval=5 - интервал пинга в секундах
        ping_timeout=10 - таймаут в секундах
        """
        global _logger
        
        self.db_manager = db_manager
        self.ping_interval = ping_interval
        self.ping_timeout = ping_timeout
        self.connected_clients: Set[websockets.WebSocketServerProtocol] = set()
        self.websocket_state = "disconnected"
        self.command_to_farm = ""
        self.should_send_command = False
        self.frqs_data = None  # Добавляем хранение FRQS данных
        
        # Используем глобальный логгер или создаем новый
        if _logger is None:
            _logger = logging.getLogger('websocket_handler')
            if not _logger.handlers:
                _logger.setLevel(logging.INFO)
                handler = logging.StreamHandler()
                handler.setFormatter(logging.Formatter(
                    '%(asctime)s - %(name)s - %(levelname)s - %(message)s'
                ))
                _logger.addHandler(handler)
                _logger.propagate = False
        
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

    async def send_command(self) -> None:
        """Отправка команды всем подключенным клиентам"""
        if self.connected_clients and self.should_send_command:
            await self.broadcast_message(self.command_to_farm)
            self.should_send_command = False
            self.logger.debug(f"Command {self.command_to_farm} sent to {len(self.connected_clients)} clients")

    def set_command(self, new_command: str) -> bool:
        """Установка новой команды и флага для отправки"""
        try:
            self.command_to_farm = new_command
            self.should_send_command = True
            self.logger.info(f"Command updated to: {new_command}")
            asyncio.create_task(self.send_command())
            return True
        except Exception as e:
            self.logger.error(f"Error setting command: {e}")
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
        self.command_to_farm = ""
        self.logger.info("WebSocket handler state reset")      