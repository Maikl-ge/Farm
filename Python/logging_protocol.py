import logging
from websockets import WebSocketServerProtocol

# Настройка логирования
logging.basicConfig(
    level=logging.DEBUG,  # Уровень DEBUG для получения всех сообщений
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler("websocket_handler.log"),
        logging.StreamHandler()
    ]
)

logger = logging.getLogger(__name__)

class LoggingWebSocketServerProtocol(WebSocketServerProtocol):
    def connection_made(self, transport):
        """Логирование открытия соединения"""
        logger.debug("WebSocket connection opened")
        super().connection_made(transport)

    def connection_lost(self, exc):
        """Логирование закрытия соединения"""
        logger.debug(f"WebSocket connection closed with exception: {exc}")
        super().connection_lost(exc)

    async def process_ping(self, data: bytes) -> None:
        """Обработка входящего пинга"""
        logger.debug(f"Received ping from client: {data}")
        await super().process_ping(data)

    async def process_pong(self, data: bytes) -> None:
        """Обработка входящего понга"""
        logger.debug(f"Received pong from client: {data}")
        await super().process_pong(data)

    async def process_request(self, path, request_headers):
        """Обработка запроса на подключение"""
        logger.debug(f"Processing request for path: {path}, headers: {request_headers}")
        return None  # Возвращаем None для успешного продолжения хендшейка

    async def send(self, message):
        """Отправка сообщения клиенту"""
        logger.debug(f"Sending message to client: {message}")
        await super().send(message)

    async def recv(self):
        """Получение сообщения от клиента"""
        message = await super().recv()
        logger.debug(f"Received message from client: {message}")
        return message

    async def close(self, code=1000, reason=''):
        """Закрытие соединения"""
        logger.debug(f"Closing connection with code: {code}, reason: {reason}")
        await super().close(code, reason)