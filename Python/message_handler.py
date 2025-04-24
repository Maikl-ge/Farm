"""
Модуль для обработки сообщений WebSocket.
"""

import json
import logging
from datetime import datetime

class MessageHandler:
    """
    Класс для обработки сообщений, полученных через WebSocket.
    """

    def __init__(self, db_manager, websocket_handler, logger=None):
        self.db_manager = db_manager
        self.websocket_handler = websocket_handler
        self.logger = logger or logging.getLogger(__name__)

    async def handle_frqs(self, data, id_farm, timestamp):
        self.websocket_handler.frqs_data = data
        self.logger.info("FRQS data updated: %s", data)
        self.logger.info(
            "%s - Параметры от клиента %s получены и сохранены в буфер",
            timestamp,
            id_farm
        )

    async def handle_message(self, message: str, websocket):
        parts = message.split()
        if len(parts) < 4:
            self.logger.warning("Недостаточно данных в сообщении: %s", message)
            return

        id_farm = parts[0]
        type_msg = parts[1]
        ack_message = f"{id_farm} {type_msg} ACK"

        # Отправляем ACK
        if websocket.open:
            await websocket.send(ack_message)
            self.logger.info("Отправляем ACK: %s", ack_message)
        else:
            self.logger.warning("Cannot send ACK, connection closed for client %s", id_farm)

        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        self.logger.info("%s - Получено сообщение от %s: %s", timestamp, id_farm, message)

        data = json.loads(parts[3])

        if type_msg == "FRQS":
            await self.handle_frqs(data, id_farm, timestamp)
        elif type_msg == "FLIN":
            success = await self.db_manager.save_sensor_data(data, timestamp)
            if success:
                self.logger.info("%s - Данные от клиента %s успешно сохранены", timestamp, id_farm)
            else:
                self.logger.error("%s - Ошибка при сохранении данных", timestamp)
        elif type_msg == "FDST":
            success = await self.db_manager.save_status_data(data, timestamp)
            if success:
                self.logger.info("%s - Статусные данные от клиента %s успешно сохранены", timestamp, id_farm)
            else:
                self.logger.error("%s - Ошибка при сохранении статусных данных", timestamp)
        else:
            self.logger.warning("Unknown message type from client %s: %s", id_farm, type_msg)