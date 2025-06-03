import json
import logging
from datetime import datetime
from typing import Dict, Any

class MessageHandler:
    def __init__(self, db_manager, websocket_handler):
        self.db_manager = db_manager
        self.websocket_handler = websocket_handler
        self.logger = logging.getLogger(__name__)
        self.last_FRQS: Dict[str, Any] = {}  # Буфер для последних FRQS данных

    async def handle_message(self, message: str, websocket):
        """Обрабатывает полученное сообщение"""
        client_id = id(websocket) if websocket else "HTTP"
        self.logger.info(f"Received message from client {client_id}: {message}")

        try:
            parts = message.split(' ', 3)
            if len(parts) < 2:
                self.logger.error(f"Invalid message format: {message}")
                return

            id_farm = parts[0]
            type_msg = parts[1]
            ack_message = f"{id_farm} {type_msg} ACK"

            # Отправляем ACK, если WebSocket доступен
            if websocket and websocket.open:
                await websocket.send(ack_message)
                self.logger.info(f"Sent ACK: {ack_message}")
            else:
                self.logger.info(f"Skipping ACK for client {client_id} (no WebSocket)")

            # Проверяем, есть ли данные
            if len(parts) < 4:
                self.logger.info(f"No data payload in message: {message}")
                return

            json_length = parts[2]
            try:
                data = json.loads(parts[3])
            except json.JSONDecodeError as e:
                self.logger.error(f"Failed to parse JSON: {e}")
                return

            # Обработка типов сообщений
            if type_msg == "FRQS":
                self.last_FRQS = data
                self.logger.info(f"FRQS updated in buffer: {data}")
                if self.websocket_handler:
                    self.websocket_handler.frqs_data = self.last_FRQS
                    self.logger.info(f"FRQS data updated in websocket_handler: {data}")
            elif type_msg == "FLIN":
                success = await self.db_manager.save_sensor_data(data, datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
                if success:
                    self.logger.info(f"Sensor data from client {id_farm} saved successfully")
                else:
                    self.logger.error(f"Failed to save sensor data for client {id_farm}")
            elif type_msg == "FDST":
                success = await self.db_manager.save_status_data(data, datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
                if success:
                    self.logger.info(f"Status data from client {id_farm} saved successfully")
                else:
                    self.logger.error(f"Failed to save status data for client {id_farm}")
            else:
                self.logger.warning(f"Unknown message type from client {client_id}: {type_msg}")

        except Exception as e:
            self.logger.exception(f"Error processing message: {e}")

    async def send_raw_command(self, command: str):
        """Метод для отправки команд (заглушка, предполагается реализация в websocket_handler)"""
        self.logger.info(f"Sending raw command: {command}")
        # Реализация зависит от websocket_handler, здесь заглушка
        return {"status": "sent", "command": command}