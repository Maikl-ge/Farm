import asyncio
import json
import logging
import time  # Добавлено
from typing import Set
from message_handler import MessageHandler
from farm_http_handler import FarmHTTPHandler
from logging_protocol import LoggingWebSocketServerProtocol
import websockets

# Настройка логирования
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    handlers=[
        logging.StreamHandler()
    ]
)

logger = logging.getLogger(__name__)

class FarmWebSocketHandler:
    def __init__(self, db_manager, ping_interval=5, ping_timeout=30):
        self.db_manager = db_manager
        self.ping_interval = ping_interval
        self.ping_timeout = ping_timeout
        self.connected_clients: Set[LoggingWebSocketServerProtocol] = set()
        self.websocket_state = "disconnected"
        self.frqs_data = None
        self.logger = logger
        self.message_handler = MessageHandler(db_manager, self)

    async def update_websocket_state(self, new_state: str) -> None:
        """Обновление состояния WebSocket и оповещение клиентов"""
        if self.websocket_state != new_state:
            self.websocket_state = new_state
            self.logger.info(f"WebSocket state changed to: {new_state}")
            try:
                state_message = json.dumps({"state": new_state})
                await self.broadcast_message(state_message)
                self.logger.debug(f"Sent state update to {len(self.connected_clients)} clients: {state_message}")
            except Exception as e:
                self.logger.error(f"Failed to broadcast state update: {e}")

    async def check_websocket_state(self) -> None:
        DISCONNECT_THRESHOLD = 5
        last_connected_time = None

        while True:
            try:
                await asyncio.sleep(1)
                if self.connected_clients:
                    last_connected_time = time.time()
                    await self.update_websocket_state("connected")
                else:
                    if last_connected_time is None:
                        last_connected_time = time.time()
                    elif (time.time() - last_connected_time) > DISCONNECT_THRESHOLD:
                        await self.update_websocket_state("disconnected")
            except asyncio.CancelledError:
                self.logger.info("WebSocket state check task cancelled")
                break
            except websockets.exceptions.ConnectionClosed as e:
                self.logger.error(f"Connection closed error in check_websocket_state: {e}")
                await asyncio.sleep(1)
            except Exception as e:
                self.logger.error(f"Unexpected error in check_websocket_state: {e}", exc_info=True)
                await asyncio.sleep(1)

    async def broadcast_message(self, message: str) -> None:
        if not self.connected_clients:
            return

        clients = self.connected_clients.copy()
        disconnected_clients = set()

        for client in clients:
            try:
                if len(message) > 2048:
                    raise ValueError("Message size exceeds limit")
                if client.open:
                    await client.send(message)
                else:
                    disconnected_clients.add(client)
                    self.logger.warning(f"Cannot send message, connection closed for client {id(client)}")
            except websockets.exceptions.ConnectionClosed:
                disconnected_clients.add(client)
                self.logger.info(f"Client {id(client)} connection closed during broadcast")
            except Exception as e:
                disconnected_clients.add(client)
                self.logger.error(f"Error sending message to client {id(client)}: {e}")

        for client in disconnected_clients:
            if client in self.connected_clients:
                self.connected_clients.remove(client)
                self.logger.info(f"Removed disconnected client: {id(client)}")

        if not self.connected_clients:
            await self.update_websocket_state("disconnected")

    async def send_command(self, command: str) -> bool:
        try:
            if not isinstance(command, str):
                raise ValueError(f"Expected command to be a str, got {type(command)}")
            if len(command) > 2048:
                raise ValueError("Command size exceeds limit")
            await self.broadcast_message(command)
            self.logger.debug(f"Command {command} sent to {len(self.connected_clients)} clients")
            return True
        except Exception as e:
            self.logger.error(f"Error sending command: {e}")
            return False

    async def send_raw_command(self, command) -> bool:
        try:
            if not isinstance(command, (str, bytes)):
                raise ValueError(f"Expected command to be str or bytes, got {type(command)}")
            if len(command) > 2048:
                raise ValueError("Command size exceeds limit")
            await self.broadcast_message(command)
            self.logger.info(f"Raw command sent: {command}")
            return True
        except Exception as e:
            self.logger.error(f"Error sending raw command: {e}")
            return False

    async def command_to_farm(self, parameter: dict) -> bool:
        try:
            if not isinstance(parameter, dict):
                raise ValueError(f"Expected parameter to be a dict, got {type(parameter)}")
            command_str = json.dumps(parameter)
            if len(command_str) > 2048:
                raise ValueError("Command size exceeds limit")
            await self.broadcast_message(command_str)
            self.logger.info(f"Command sent to farm: {command_str}")
            return True
        except Exception as e:
            self.logger.error(f"Error sending command to farm: {e}")
            return False

    async def handle_connection(self, websocket: LoggingWebSocketServerProtocol, path: str) -> None:
        client_id = id(websocket)
        self.logger.info(f"New client connected: {client_id}")
        try:
            websocket.ping_timeout = self.ping_timeout
            self.connected_clients.add(websocket)
            await self.update_websocket_state("connected")
            async for message in websocket:
                try:
                    if len(message) > 2048:
                        raise ValueError("Message size exceeds limit")
                    await self.message_handler.handle_message(message, websocket)
                except json.JSONDecodeError as e:
                    self.logger.error(f"JSON decode error from client {client_id}: {e}")
                except ValueError as e:
                    self.logger.error(f"Value error: {e}")
                except Exception as e:
                    self.logger.error(f"Error processing message from client {client_id}: {e}")
        except websockets.exceptions.ConnectionClosed as e:
            self.logger.warning(f"Client {client_id} connection closed with error: {e.code} - {e.reason}")
        except Exception as e:
            self.logger.error(f"Error in connection handler: {e}")
        finally:
            if websocket in self.connected_clients:
                self.connected_clients.remove(websocket)
            await websocket.close()
            self.logger.info(f"Client {client_id} connection closed normally")
            await self.reset_state()

    async def get_frqs_data(self):
        return self.frqs_data

    async def reset_state(self):
        self.frqs_data = None
        self.logger.info("WebSocket handler state reset")
        if not self.connected_clients:
            await asyncio.sleep(5)
            if not self.connected_clients:
                self.websocket_state = "disconnected"