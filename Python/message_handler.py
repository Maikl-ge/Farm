import json
import logging
from datetime import datetime

class MessageHandler:
    def __init__(self, db_manager, websocket_handler):
        self.db_manager = db_manager
        self.websocket_handler = websocket_handler
        self.logger = logging.getLogger(__name__)

    async def handle_message(self, message: str, websocket):
        """Обрабатывает полученное сообщение"""
        client_id = id(websocket)
        self.logger.info(f"Received message from client {client_id}: {message}")

        parts = message.split(' ', 3)
        if len(parts) < 4:
            return

        id_farm = parts[0]
        type_msg = parts[1]
        ack_message = f"{id_farm} {type_msg} ACK"

        # Отправляем ACK
        if websocket.open:
            await websocket.send(ack_message)
        else:
            self.logger.warning(f"Cannot send ACK, connection closed for client {client_id}")

        # Логирование и обработка данных
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        self.logger.info(f"{timestamp} - Получено сообщение от {client_id}: {message}")

        json_length = parts[2]
        data = json.loads(parts[3])

        if type_msg == "FRQS":
            self.websocket_handler.frqs_data = data
            self.logger.info(f"FRQS data updated: {data}")
        elif type_msg == "FLIN":
            success = await self.db_manager.save_sensor_data(data, timestamp)
            if success:
                self.logger.info(f"{timestamp} - Данные от клиента {id_farm} успешно сохранены")
            else:
                self.logger.error(f"{timestamp} - Ошибка при сохранении данных")
        elif type_msg == "FDST":
            success = await self.db_manager.save_status_data(data, timestamp)
            if success:
                self.logger.info(f"{timestamp} - Статусные данные от клиента {id_farm} успешно сохранены")
            else:
                self.logger.error(f"{timestamp} - Ошибка при сохранении статусных данных")
        else:
            self.logger.warning(f"Unknown message type from client {client_id}: {type_msg}")