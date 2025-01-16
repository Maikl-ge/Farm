use actix_web::{
    middleware, web, App, Error, HttpRequest, HttpResponse, HttpServer, Responder,
};
use actix_web_actors::ws;
use serde::Serialize;
use sqlx::{Pool, Postgres};
use std::sync::Arc;
use tera::{Context, Tera};
use tokio::sync::Mutex;
use tokio::time::{self, Duration};
use log::{info, warn};

// Глобальное состояние приложения
struct AppState {
    db_pool: Pool<Postgres>,
    websocket_clients: Arc<Mutex<Vec<actix::Addr<WebSocketHandler>>>>,
}

// Старт сервера
#[actix_web::main]
async fn main() -> std::io::Result<()> {
    // Настройка логирования
    env_logger::init();
    let database_url = std::env::var("DATABASE_URL").expect("DATABASE_URL must be set");

    let db_pool = Pool::<Postgres>::connect(&database_url)
        .await
        .expect("Failed to connect to database");

    let websocket_clients = Arc::new(Mutex::new(Vec::new()));

    let state = web::Data::new(AppState {
        db_pool,
        websocket_clients,
    });

    let tera = Tera::new("templates/**/*").expect("Error initializing Tera templates");

    HttpServer::new(move || {
        App::new()
            .app_data(state.clone())
            .app_data(web::Data::new(tera.clone()))
            .wrap(middleware::Logger::default())
            .wrap(middleware::Compress::default())
            .wrap_fn(security_middleware)
            .route("/", web::get().to(index))
            .route("/command", web::get().to(command_page))
            .route("/api/data", web::get().to(api_data))
            .route("/ws/", web::get().to(ws_route))
            .route("/parameters", web::get().to(show_parameters))
            .route("/parameters/{id}/edit", web::post().to(edit_parameters))
    })
    .bind("0.0.0.0:8080")?
    .run()
    .await
}

// Маршруты

async fn index() -> impl Responder {
    HttpResponse::Ok().body("Welcome to the index page!")
}

async fn command_page() -> impl Responder {
    HttpResponse::Ok().body("Command page!")
}

#[derive(Serialize)]
struct ApiResponse {
    message: String,
}

async fn api_data() -> impl Responder {
    HttpResponse::Ok().json(ApiResponse {
        message: "API data response".to_string(),
    })
}

async fn show_parameters() -> impl Responder {
    HttpResponse::Ok().body("Show parameters")
}

async fn edit_parameters() -> impl Responder {
    HttpResponse::Ok().body("Edit parameters")
}

// WebSocket обработчик
struct WebSocketHandler;

impl actix::Actor for WebSocketHandler {
    type Context = ws::WebsocketContext<Self>;
}

impl ws::StreamHandler<Result<ws::Message, ws::ProtocolError>> for WebSocketHandler {
    fn handle(&mut self, msg: Result<ws::Message, ws::ProtocolError>, ctx: &mut Self::Context) {
        if let Ok(ws::Message::Text(text)) = msg {
            ctx.text(format!("Echo: {}", text));
        }
    }
}

async fn ws_route(
    req: HttpRequest,
    stream: web::Payload,
    state: web::Data<AppState>,
) -> Result<HttpResponse, Error> {
    let websocket_clients = state.websocket_clients.clone();
    let handler = WebSocketHandler.start();
    websocket_clients.lock().await.push(handler.clone());
    ws::start(WebSocketHandler, &req, stream)
}

// Мидлвар для защиты от сканирования
async fn security_middleware(
    req: HttpRequest,
    srv: web::Service,
) -> Result<HttpResponse, actix_web::Error> {
    let allowed_agents = ["Mozilla", "Chrome", "Firefox", "Safari", "Edge", "ESP32"];
    let user_agent = req.headers().get("User-Agent").and_then(|ua| ua.to_str().ok());

    if let Some(agent) = user_agent {
        if !allowed_agents.iter().any(|&allowed| agent.contains(allowed)) {
            return Ok(HttpResponse::Forbidden().finish());
        }
    }

    srv.call(req).await
}
