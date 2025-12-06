#include "Art.h"
#include <iostream>
#include <fstream>
#include <sstream>

Art::Art(float startX, float startY) 
    : x(startX), y(startY),
      velocidad(150.0f),
      rangoDeteccion(300.0f),
      estadoActual(PATRULLANDO),
      direccionActual(ABAJO),
      frameActual(0),
      tiempoPorFrame(0.15f),
      tiempoAnimacion(0.0f),
      caminando(false),
      waypointActual(0),
      distanciaUmbral(15.0f),
      temporizadorEspera(0.0f),
      esperando(false),
      patrullaCiclica(true)
{
    // Crear sprite básico
    sf::Texture tex;
    tex.create(45, 45);
    sprite.setTexture(tex);
    sprite.setPosition(x, y);
    sprite.setOrigin(22.5f, 22.5f);
}

void Art::actualizar(float deltaTime, float playerX, float playerY) {
    // Calcular distancia al jugador
    float distanciaAlJugador = calcularDistancia(playerX, playerY);
    
    // Cambiar estado según distancia
    if (distanciaAlJugador <= rangoDeteccion) {
        estadoActual = PERSEGUIR;
    } else {
        estadoActual = PATRULLANDO;
    }
    
    // Actualizar comportamiento según estado
    if (estadoActual == PERSEGUIR) {
        actualizarPersecucion(deltaTime, playerX, playerY);
    } else {
        actualizarPatrulla(deltaTime);
    }
    
    // Actualizar animación
    actualizarAnimacion(deltaTime);
    
    // Actualizar posición del sprite
    sprite.setPosition(x, y);
}

void Art::actualizarPatrulla(float deltaTime) {
    if (waypoints.empty()) {
        caminando = false;
        return;
    }
    
    // Si está esperando en un waypoint
    if (esperando) {
        temporizadorEspera -= deltaTime;
        if (temporizadorEspera <= 0) {
            esperando = false;
            
            // Avanzar al siguiente waypoint
            waypointActual++;
            
            // Si llegó al final
            if (waypointActual >= waypoints.size()) {
                if (patrullaCiclica) {
                    waypointActual = 0; // Volver al inicio
                } else {
                    waypointActual = waypoints.size() - 1; // Quedarse en el último
                    caminando = false;
                    return;
                }
            }
        }
        caminando = false;
        return;
    }
    
    // Obtener waypoint actual
    Waypoint& wp = waypoints[waypointActual];
    
    // Calcular distancia al waypoint
    float distancia = calcularDistanciaAlPunto(wp.posicion);
    
    // Si llegó al waypoint
    if (distancia <= distanciaUmbral) {
        // Iniciar espera
        esperando = true;
        temporizadorEspera = wp.tiempoEspera;
        
        // Mantener el sprite quieto con la dirección actual
        caminando = false;
        return;
    }
    
    // Mover hacia el waypoint
    moverHaciaPunto(wp.posicion.x, wp.posicion.y, deltaTime);
    caminando = true;
}

void Art::actualizarPersecucion(float deltaTime, float playerX, float playerY) {
    float diffX = playerX - x;
    float diffY = playerY - y;
    float distancia = calcularDistancia(playerX, playerY);
    
    // Actualizar dirección visual
    actualizarDireccion(diffX, diffY);
    
    // Moverse hacia el jugador si no está demasiado cerca
    if (distancia > 20.0f) {
        float dirX = diffX / distancia;
        float dirY = diffY / distancia;
        
        x += dirX * velocidad * 1.3f * deltaTime; // 20% más rápido al perseguir
        y += dirY * velocidad * 1.3f * deltaTime;
        caminando = true;
    } else {
        caminando = false;
    }
}

void Art::moverHaciaPunto(float targetX, float targetY, float deltaTime) {
    float diffX = targetX - x;
    float diffY = targetY - y;
    float distancia = std::sqrt(diffX * diffX + diffY * diffY);
    
    // Actualizar dirección visual
    actualizarDireccion(diffX, diffY);
    
    if (distancia > 0) {
        float dirX = diffX / distancia;
        float dirY = diffY / distancia;
        
        x += dirX * velocidad * deltaTime;
        y += dirY * velocidad * deltaTime;
    }
}

void Art::moverHaciaPunto(float targetX, float targetY, float deltaTime) {
    float diffX = targetX - x;
    float diffY = targetY - y;
    float distancia = std::sqrt(diffX * diffX + diffY * diffY);
    
    // Actualizar dirección visual
    actualizarDireccion(diffX, diffY);
    
    if (distancia > 0) {
        float dirX = diffX / distancia;
        float dirY = diffY / distancia;
        
        x += dirX * velocidad * deltaTime;
        y += dirY * velocidad * deltaTime;
    }
}


void Art::agregarWaypoint(float x, float y, float tiempoEspera) {
    waypoints.emplace_back(x, y, tiempoEspera);
}

void Art::limpiarWaypoints() {
    waypoints.clear();
    waypointActual = 0;
    esperando = false;
}

bool Art::cargarWaypointsDesdeArchivo(const std::string& rutaArchivo) {
    std::ifstream archivo(rutaArchivo);
    if (!archivo.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo de waypoints: " << rutaArchivo << std::endl;
        return false;
    }
    
    limpiarWaypoints();
    
    std::string linea;
    int numLinea = 0;
    
    while (std::getline(archivo, linea)) {
        numLinea++;
        
        // Saltar líneas vacías o comentarios
        if (linea.empty() || linea[0] == '#') {
            continue;
        }
        
        std::istringstream ss(linea);
        float x, y, wait = 2.0f; // Valor por defecto: 2 segundos
        
        // Leer coordenadas
        if (!(ss >> x >> y)) {
            std::cerr << "Error en línea " << numLinea << ": formato incorrecto" << std::endl;
            continue;
        }
        
        // Leer tiempo de espera opcional
        ss >> wait;
        
        agregarWaypoint(x, y, wait);
    }
    
    archivo.close();
    std::cout << "Waypoints cargados: " << waypoints.size() << " desde " << rutaArchivo << std::endl;
    return true;
}


void Art::actualizarDireccion(float diffX, float diffY) {
    if (std::abs(diffX) > std::abs(diffY)) {
        if (diffX > 0) {
            direccionActual = DERECHA;
        } else {
            direccionActual = IZQUIERDA;
        }
    } else {
        if (diffY > 0) {
            direccionActual = ABAJO;
        } else {
            direccionActual = ARRIBA;
        }
    }
}

void Art::actualizarAnimacion(float deltaTime) {
    tiempoAnimacion += deltaTime;
    
    if (caminando) {
        if (tiempoAnimacion >= tiempoPorFrame) {
            std::vector<sf::Texture>* texturas = nullptr;
            
            switch (direccionActual) {
                case ABAJO: texturas = &caminarAbajo; break;
                case ARRIBA: texturas = &caminarArriba; break;
                case IZQUIERDA: texturas = &caminarIzquierda; break;
                case DERECHA: texturas = &caminarDerecha; break;
            }
            
            if (texturas && !texturas->empty()) {
                frameActual = (frameActual + 1) % texturas->size();
                sprite.setTexture((*texturas)[frameActual]);
            }
            
            tiempoAnimacion = 0.0f;
        }
    } else {
        switch (direccionActual) {
            case ABAJO: 
                if (quietoAbajo.getSize().x > 0) 
                    sprite.setTexture(quietoAbajo); 
                break;
            case ARRIBA: 
                if (quietoArriba.getSize().x > 0) 
                    sprite.setTexture(quietoArriba); 
                break;
            case IZQUIERDA: 
                if (quietoIzquierda.getSize().x > 0) 
                    sprite.setTexture(quietoIzquierda); 
                break;
            case DERECHA: 
                if (quietoDerecha.getSize().x > 0) 
                    sprite.setTexture(quietoDerecha); 
                break;
        }
    }
}

float Art::calcularDistancia(float px, float py) const {
    float diffX = px - x;
    float diffY = py - y;
    return std::sqrt(diffX * diffX + diffY * diffY);
}

float Art::calcularDistanciaAlPunto(const sf::Vector2f& punto) const {
    float diffX = punto.x - x;
    float diffY = punto.y - y;
    return std::sqrt(diffX * diffX + diffY * diffY);
}

void Art::cargarSprites(
    const std::string& quietoAbajoPath,
    const std::string& quietoArribaPath,
    const std::string& quietoIzquierdaPath,
    const std::string& quietoDerechaPath,
    const std::vector<std::string>& caminarAbajoPaths,
    const std::vector<std::string>& caminarArribaPaths,
    const std::vector<std::string>& caminarIzquierdaPaths,
    const std::vector<std::string>& caminarDerechaPaths)
{
    // Cargar sprites QUIETOS (1 por dirección)
    quietoAbajo.loadFromFile(quietoAbajoPath);
    quietoArriba.loadFromFile(quietoArribaPath);
    quietoIzquierda.loadFromFile(quietoIzquierdaPath);
    quietoDerecha.loadFromFile(quietoDerechaPath);
    
    // Cargar animaciones CAMINAR (4 frames por dirección)
    for (const auto& path : caminarAbajoPaths) {
        sf::Texture tex;
        tex.loadFromFile(path);
        caminarAbajo.push_back(tex);
    }
    
    for (const auto& path : caminarArribaPaths) {
        sf::Texture tex;
        tex.loadFromFile(path);
        caminarArriba.push_back(tex);
    }
    
    for (const auto& path : caminarIzquierdaPaths) {
        sf::Texture tex;
        tex.loadFromFile(path);
        caminarIzquierda.push_back(tex);
    }
    
    for (const auto& path : caminarDerechaPaths) {
        sf::Texture tex;
        tex.loadFromFile(path);
        caminarDerecha.push_back(tex);
    }
    
    // Establecer sprite inicial
    sprite.setTexture(quietoAbajo);
    sprite.setOrigin(22.5f, 22.5f);
}

void Art::setPosicion(float nuevaX, float nuevaY) {     //actualizar posición
    x = nuevaX;
    y = nuevaY;
    sprite.setPosition(x, y);   //sincronizar sprite
}
//sin ella no se podria cambiar la posicion de art
