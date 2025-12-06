#include "Art.h"
#include <iostream>

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
      patrullaCiclica(true),
      posicionInicial(startX, startY),
      waypointInicioPersecucion(0)
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
    
    // Máquina de estados
    switch (estadoActual) {
        case PATRULLANDO:
            if (distanciaAlJugador <= rangoDeteccion) {
                estadoActual = PERSEGUIR;
                waypointInicioPersecucion = waypointActual; // Guardar posición actual
            } else {
                actualizarPatrulla(deltaTime);
            }
            break;
            
        case PERSEGUIR:
            if (distanciaAlJugador > rangoDeteccion * 1.5f) { // Mayor umbral para dejar de perseguir
                estadoActual = REGRESANDO;
            } else {
                actualizarPersecucion(deltaTime, playerX, playerY);
            }
            break;
            
        case REGRESANDO:
            if (calcularDistanciaAlPunto(posicionInicial) < distanciaUmbral) {
                estadoActual = PATRULLANDO;
                waypointActual = waypointInicioPersecucion;
            } else {
                actualizarRegreso(deltaTime);
            }
            break;
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
        if (wp.tiempoEspera > 0) {
            esperando = true;
            temporizadorEspera = wp.tiempoEspera;
        }
        
        // Ejecutar acción si existe
        if (!wp.accion.empty()) {
            ejecutarAccionWaypoint(wp.accion);
        }
        
        // Si no tiene tiempo de espera, pasar inmediatamente al siguiente
        if (wp.tiempoEspera <= 0) {
            waypointActual++;
            if (waypointActual >= waypoints.size()) {
                if (patrullaCiclica) {
                    waypointActual = 0;
                } else {
                    caminando = false;
                    return;
                }
            }
        }
        
        caminando = (wp.tiempoEspera <= 0); // Seguir caminando si no espera
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
        
        x += dirX * velocidad * 1.2f * deltaTime; // 20% más rápido al perseguir
        y += dirY * velocidad * 1.2f * deltaTime;
        caminando = true;
    } else {
        caminando = false;
    }
}

void Art::actualizarRegreso(float deltaTime) {
    // Regresar a la posición inicial (o al waypoint donde empezó la persecución)
    moverHaciaPunto(posicionInicial.x, posicionInicial.y, deltaTime);
    caminando = true;
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

void Art::ejecutarAccionWaypoint(const std::string& accion) {
    // Aquí puedes implementar diferentes acciones
    if (accion == "mirar_izquierda") {
        direccionActual = IZQUIERDA;
    } else if (accion == "mirar_derecha") {
        direccionActual = DERECHA;
    } else if (accion == "mirar_arriba") {
        direccionActual = ARRIBA;
    } else if (accion == "mirar_abajo") {
        direccionActual = ABAJO;
    }
    // Puedes agregar más acciones como sonidos, cambiar animación, etc.
}

// ==================== MÉTODOS DE WAYPOINTS ====================

void Art::agregarWaypoint(float x, float y, float tiempoEspera, const std::string& accion) {
    waypoints.emplace_back(x, y, tiempoEspera, accion);
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
        float x, y, wait = 0.0f;
        std::string accion = "";
        
        // Leer coordenadas
        if (!(ss >> x >> y)) {
            std::cerr << "Error en línea " << numLinea << ": formato incorrecto" << std::endl;
            continue;
        }
        
        // Leer tiempo de espera opcional
        if (ss >> wait) {
            // Leer acción opcional
            std::string accionTemp;
            if (ss >> accionTemp) {
                accion = accionTemp;
            }
        }
        
        agregarWaypoint(x, y, wait, accion);
    }
    
    archivo.close();
    std::cout << "Waypoints cargados: " << waypoints.size() << " desde " << rutaArchivo << std::endl;
    return true;
}

void Art::iniciarPatrullaDesdeWaypoint(size_t indice) {
    if (indice < waypoints.size()) {
        waypointActual = indice;
        esperando = false;
        temporizadorEspera = 0.0f;
    }
}

// ==================== MÉTODOS EXISTENTES (mantener) ====================

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

// Los métodos cargarSprites y setPosicion se mantienen iguales...
// [Mantén el código original de estos métodos]