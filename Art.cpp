#include "Art.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>

Art::Art(float startX, float startY)
    : x(startX), y(startY), velocidad(150.f), rangoDeteccion(300.f),
      estadoActual(PATRULLANDO), direccionActual(ABAJO), frameActual(0),
      tiempoPorFrame(0.15f), tiempoAnimacion(0.f), caminando(false),
      waypointActual(0), distanciaUmbral(15.f), temporizadorEspera(0.f),
      esperando(false), patrullaCiclica(true),
      memoriaTiempo(5.0f), // Recuerda al jugador por 5 segundos
      jugadorVistoRecientemente(false),
      tiempoDesdeUltimaVista(0.0f),
      ultimaPosJugador(startX, startY),
      siguiendoRastro(false),        
      tiempoEnRastro(0.0f),          
      destinoRastro(startX, startY)
{
    sf::Texture tex; tex.create(45,45);
    sprite.setTexture(tex);
    sprite.setPosition(x,y);
    sprite.setOrigin(22.5f,22.5f);
}

void Art::actualizar(float deltaTime, float playerX, float playerY) {
    actualizarRastros(deltaTime);
    
    // Lógica de detección del jugador
    float distanciaAlJugador = calcularDistancia(playerX, playerY);
    
    if (distanciaAlJugador <= rangoDeteccion) {
        if (estadoActual != PERSEGUIR) {
            ultimaPosJugador = sf::Vector2f(playerX, playerY);
            jugadorVistoRecientemente = true;
            tiempoDesdeUltimaVista = 0.0f;  // resetea el contador de memoria
        }
        estadoActual = PERSEGUIR;
        siguiendoRastro = false; // Dejar de seguir rastros si ve al jugador
    } else {
        if (estadoActual == PERSEGUIR) {
            estadoActual = REGRESANDO;
            // Marcar última posición como rastro
            marcarMovimientoJugador(playerX, playerY);
        }
        
        // Si no está siguiendo un rastro, comportamiento normal
        if (!siguiendoRastro) {
            switch (estadoActual) {
                case PATRULLANDO: actualizarPatrulla(deltaTime); break;
                case REGRESANDO: 
                    // Ir al último waypoint
                    if (waypoints.empty()) {
                        estadoActual = PATRULLANDO;
                    } else {
                        actualizarRegreso(deltaTime);
                    }
                    break;
            }
        }
    }
    
    // Si está persiguiendo directamente al jugador
    if (estadoActual == PERSEGUIR) {
        actualizarPersecucion(deltaTime, playerX, playerY);
    }
    
    actualizarAnimacion(deltaTime);
    sprite.setPosition(x, y);
}

void Art::actualizarPatrulla(float deltaTime) {
    if (waypoints.empty()) { caminando=false; return; } //si no hay waypoints se queda quieto
    if (esperando) {
        temporizadorEspera -= deltaTime;
        if (temporizadorEspera <= 0.f) {
            esperando = false;
            waypointActual++;   //avanza al siguiente waypoint
            if (waypointActual >= waypoints.size()) {   //ciclo de patrulla 
                if (patrullaCiclica) waypointActual = 0;
                else { waypointActual = waypoints.size()-1; caminando=false; return; }
            }
        }
        caminando = false; return;
    }

    Waypoint& wp = waypoints[waypointActual];
    float distancia = calcularDistanciaAlPunto(wp.posicion);
    if (distancia <= distanciaUmbral) {
        esperando = true; temporizadorEspera = wp.tiempoEspera; caminando=false; return;
    }
    moverHaciaPunto(wp.posicion.x, wp.posicion.y, deltaTime);
    caminando = true;
}

void Art::actualizarPersecucion(float deltaTime, float playerX, float playerY) {
    float diffX = playerX - x; float diffY = playerY - y;
    float distancia = calcularDistancia(playerX, playerY);
    ultimaPosJugador = sf::Vector2f(playerX, playerY);
    // Inteligencia: si el jugador está cerca pero moviéndose rápido, predecir posición
    float velocidadJugadorEstimada = 200.0f; // Velocidad estimada del jugador
    float tiempoPrediccion = 0.3f; // Predecir 0.3 segundos adelante
    actualizarDireccion(diffX, diffY);
    if (distancia > 20.f) {
        float dirX = diffX / distancia; float dirY = diffY / distancia;
        // Aumentar velocidad si el jugador está lejos
        float multiplicadorVelocidad = 1.3f;
        if (distancia > 200.f) { multiplicadorVelocidad = 1.8f; // Corre más si está lejos
        }
        x += dirX * velocidad * multiplicadorVelocidad * deltaTime;
        y += dirY * velocidad * multiplicadorVelocidad * deltaTime;
        caminando = true;
        
    } else {
        caminando = false;
        // Si está muy cerca, intentar rodear
        if (std::rand() % 100 < 30) { // 30% de probabilidad de moverse lateralmente
            float angulo = (std::rand() % 360) * 3.14159f / 180.0f;
            x += std::cos(angulo) * velocidad * 0.5f * deltaTime;
            y += std::sin(angulo) * velocidad * 0.5f * deltaTime;
        }
    }
}

void Art::actualizarRegreso(float deltaTime) {
    if (waypoints.empty()) { estadoActual = PATRULLANDO; return; }
    Waypoint& wp = waypoints[waypointActual];
    float distancia = calcularDistanciaAlPunto(wp.posicion);
    if (distancia <= distanciaUmbral) { estadoActual = PATRULLANDO; esperando=true; temporizadorEspera=wp.tiempoEspera; caminando=false; return; }
    moverHaciaPunto(wp.posicion.x, wp.posicion.y, deltaTime); caminando=true;
}

size_t Art::encontrarWaypointMasCercano() const {
    if (waypoints.empty()) return 0;
    size_t mejor = 0; float mejorD = calcularDistanciaAlPunto(waypoints[0].posicion);
    for (size_t i=1;i<waypoints.size();++i){ float d=calcularDistanciaAlPunto(waypoints[i].posicion); if (d<mejorD){ mejorD=d; mejor=i; }}
    return mejor;
}

void Art::moverHaciaPunto(float targetX, float targetY, float deltaTime) {
    float diffX = targetX - x; float diffY = targetY - y;
    float distancia = std::sqrt(diffX*diffX + diffY*diffY);
    actualizarDireccion(diffX, diffY);
    if (distancia>0.f){ float dirX=diffX/distancia, dirY=diffY/distancia; x += dirX*velocidad*deltaTime; y += dirY*velocidad*deltaTime; }
}

void Art::invertirDireccion() {
    if (waypoints.empty()) return;

    // Comportamiento mejorado para evitar quedarse atascado en la misma habitación:
    // Elegir un waypoint que esté lo más alejado posible de la posición actual.
    // Si estamos persiguiendo, volvemos a REGRESANDO y apuntamos al waypoint más cercano
    if (estadoActual == PERSEGUIR) {
        estadoActual = REGRESANDO;
        waypointActual = encontrarWaypointMasCercano();
        esperando = false;
        return;
    }

    // Si patrullando o regresando -> seleccionar waypoint lejano
    sf::Vector2f posActual(x, y);
    std::vector<size_t> candidatos;
    const float MIN_DIST = 400.f; // preferir waypoints al menos a esta distancia

    for (size_t i = 0; i < waypoints.size(); ++i) {
        float dx = waypoints[i].posicion.x - posActual.x;
        float dy = waypoints[i].posicion.y - posActual.y;
        float d = std::sqrt(dx*dx + dy*dy);
        if (d >= MIN_DIST) candidatos.push_back(i);
    }

    size_t elegido = waypointActual;
    if (!candidatos.empty()) {
        // elegir al azar entre candidatos lejanos
        size_t r = static_cast<size_t>(std::rand()) % candidatos.size();
        elegido = candidatos[r];
    } else {
        // si no hay candidatos, elegir el waypoint más lejano
        float mejorDist = -1.f;
        for (size_t i = 0; i < waypoints.size(); ++i) {
            float dx = waypoints[i].posicion.x - posActual.x;
            float dy = waypoints[i].posicion.y - posActual.y;
            float d = std::sqrt(dx*dx + dy*dy);
            if (d > mejorDist) { mejorDist = d; elegido = i; }
        }
    }

    waypointActual = elegido;
    estadoActual = PATRULLANDO;
    esperando = false;
    temporizadorEspera = 0.f;
}

void Art::agregarWaypoint(float px, float py, float tiempoEspera, const std::string& accion) {
    waypoints.emplace_back(px, py, tiempoEspera, accion);
}

void Art::limpiarWaypoints(){ waypoints.clear(); waypointActual=0; esperando=false; }

bool Art::cargarWaypointsDesdeArchivo(const std::string& rutaArchivo){
    std::ifstream archivo(rutaArchivo);
    if(!archivo.is_open()){ std::cerr<<"Error: no se pudo abrir "+rutaArchivo<<std::endl; return false; }
    limpiarWaypoints(); std::string linea; int num=0;
    while(std::getline(archivo,linea)){
        num++; if(linea.empty()||linea[0]=='#') continue;
        std::istringstream ss(linea);
        float px, py; float wait=0.f; std::string accion="";
        if(!(ss>>px>>py)) { std::cerr<<"Error en linea "<<num<<" formato incorrecto\n"; continue; }
        if(!(ss>>wait)) wait=0.f;
        if(!(ss>>accion)) accion="";
        agregarWaypoint(px,py,wait,accion);
    }
    archivo.close(); std::cout<<"Waypoints cargados: "<<waypoints.size()<<" desde "<<rutaArchivo<<std::endl; return true;
}

void Art::actualizarDireccion(float diffX, float diffY){
    if (std::abs(diffX)>std::abs(diffY)) direccionActual = diffX>0?DERECHA:IZQUIERDA;
    else direccionActual = diffY>0?ABAJO:ARRIBA;
}

void Art::actualizarAnimacion(float deltaTime){
    tiempoAnimacion += deltaTime;
    if (caminando){
        if (tiempoAnimacion >= tiempoPorFrame){
            std::vector<sf::Texture>* texs = nullptr;
            switch(direccionActual){ case ABAJO: texs=&caminarAbajo; break; case ARRIBA: texs=&caminarArriba; break; case IZQUIERDA: texs=&caminarIzquierda; break; case DERECHA: texs=&caminarDerecha; break; }
            if (texs && !texs->empty()){ frameActual = (frameActual+1)%texs->size(); sprite.setTexture((*texs)[frameActual]); }
            tiempoAnimacion = 0.f;
        }
    } else {
        switch(direccionActual){ case ABAJO: if(quietoAbajo.getSize().x>0) sprite.setTexture(quietoAbajo); break; case ARRIBA: if(quietoArriba.getSize().x>0) sprite.setTexture(quietoArriba); break; case IZQUIERDA: if(quietoIzquierda.getSize().x>0) sprite.setTexture(quietoIzquierda); break; case DERECHA: if(quietoDerecha.getSize().x>0) sprite.setTexture(quietoDerecha); break; }
    }
}

float Art::calcularDistancia(float px, float py) const{ float dx=px-x, dy=py-y; return std::sqrt(dx*dx+dy*dy); }
float Art::calcularDistanciaAlPunto(const sf::Vector2f& p) const{ float dx=p.x-x, dy=p.y-y; return std::sqrt(dx*dx+dy*dy); }

void Art::cargarSprites(const std::string& quietoAbajoPath, const std::string& quietoArribaPath,
                        const std::string& quietoIzquierdaPath, const std::string& quietoDerechaPath,
                        const std::vector<std::string>& caminarAbajoPaths,const std::vector<std::string>& caminarArribaPaths,
                        const std::vector<std::string>& caminarIzquierdaPaths,const std::vector<std::string>& caminarDerechaPaths)
{
    quietoAbajo.loadFromFile(quietoAbajoPath);
    quietoArriba.loadFromFile(quietoArribaPath);
    quietoIzquierda.loadFromFile(quietoIzquierdaPath);
    quietoDerecha.loadFromFile(quietoDerechaPath);

    caminarAbajo.clear(); for (auto &p: caminarAbajoPaths){ sf::Texture t; t.loadFromFile(p); caminarAbajo.push_back(t); }
    caminarArriba.clear(); for (auto &p: caminarArribaPaths){ sf::Texture t; t.loadFromFile(p); caminarArriba.push_back(t); }
    caminarIzquierda.clear(); for (auto &p: caminarIzquierdaPaths){ sf::Texture t; t.loadFromFile(p); caminarIzquierda.push_back(t); }
    caminarDerecha.clear(); for (auto &p: caminarDerechaPaths){ sf::Texture t; t.loadFromFile(p); caminarDerecha.push_back(t); }

    if (quietoAbajo.getSize().x>0) sprite.setTexture(quietoAbajo);
    sprite.setOrigin(sprite.getLocalBounds().width/2.f, sprite.getLocalBounds().height/2.f);
}

void Art::setPosicion(float nuevaX, float nuevaY){ x=nuevaX; y=nuevaY; sprite.setPosition(x,y); }

void Art::actualizarMemoria(float deltaTime, float playerX, float playerY) {
    if (jugadorVistoRecientemente) {
        tiempoDesdeUltimaVista += deltaTime;
        // Actualizar última posición si el jugador está en rango
        if (calcularDistancia(playerX, playerY) <= rangoDeteccion * 1.5f) {
            ultimaPosJugador = sf::Vector2f(playerX, playerY);
            tiempoDesdeUltimaVista = 0.0f; // Resetear tiempo
        }
    }
}

// ============================================
// NUEVO SISTEMA DE RASTROS
// ============================================

void Art::marcarRecoleccionObjeto(float x, float y) {
    // Crear rastro con prioridad alta
    rastros.emplace_back(x, y, "objeto");
    
    // Si está cerca, empezar a seguir inmediatamente
    float dist = calcularDistancia(x, y);
    if (dist < 500.0f && estadoActual != PERSEGUIR) {
        siguiendoRastro = true;
        destinoRastro = sf::Vector2f(x, y);
        estadoActual = REGRESANDO; // Cambiar a modo "investigar"
    }
    
    // Limitar cantidad de rastros
    if (rastros.size() > 20) {
        rastros.erase(rastros.begin()); // Eliminar el más viejo
    }
}

void Art::marcarMovimientoJugador(float x, float y) {
    // Solo marcar si no está muy cerca de rastros existentes
    for (const auto& rastro : rastros) {
        float dx = rastro.posicion.x - x;
        float dy = rastro.posicion.y - y;
        if (std::sqrt(dx*dx + dy*dy) < 100.0f) {
            return; // Ya hay un rastro cercano
        }
    }
    
    rastros.emplace_back(x, y, "movimiento");
    
    // Limitar cantidad
    if (rastros.size() > 15) {
        rastros.erase(rastros.begin());
    }
}

void Art::actualizarRastros(float deltaTime) {
    // Actualizar frescura de rastros (se desvanecen con el tiempo)
    for (auto& rastro : rastros) {
        rastro.frescura -= 0.1f * deltaTime; // Se desvanece en 10 segundos
    }
    
    // Eliminar rastros muy viejos
    rastros.erase(
        std::remove_if(rastros.begin(), rastros.end(),
            [](const Rastro& r) { return r.frescura <= 0.0f; }),
        rastros.end()
    );
    
    // Si no está persiguiendo y hay rastros frescos, seguir uno
    if (estadoActual != PERSEGUIR && inteligenciaBusqueda && !rastros.empty() && !siguiendoRastro) {
        // Encontrar el rastro más fresco
        float mejorFrescura = 0.0f;
        size_t mejorIndice = 0;
        
        for (size_t i = 0; i < rastros.size(); ++i) {
            // Los objetos recogidos tienen prioridad
            float bonus = (rastros[i].tipo == "objeto") ? 0.3f : 0.0f;
            if (rastros[i].frescura + bonus > mejorFrescura) {
                mejorFrescura = rastros[i].frescura + bonus;
                mejorIndice = i;
            }
        }
        
        if (mejorFrescura > 0.3f) { // Solo si es relativamente fresco
            siguiendoRastro = true;
            destinoRastro = rastros[mejorIndice].posicion;
            tiempoEnRastro = 0.0f;
            estadoActual = REGRESANDO; // Entrar en modo "investigar"
        }
    }
    
    // Si está siguiendo un rastro, delegar a helper
    if (siguiendoRastro) {
        seguirRastroMasFresco(deltaTime);
    }
}

void Art::seguirRastroMasFresco(float deltaTime) {
    tiempoEnRastro += deltaTime;

    // Calcular distancia al destino del rastro
    float distancia = calcularDistancia(destinoRastro.x, destinoRastro.y);

    if (distancia <= distanciaUmbral * 1.5f) { // Un poco más de margen
        // Llegó al rastro - "investigar" por 2 segundos
        caminando = false;

        if (tiempoEnRastro > 2.0f) {
            siguiendoRastro = false;
            estadoActual = PATRULLANDO;

            // Eliminar este rastro ya investigado
            auto it = std::find_if(rastros.begin(), rastros.end(),
                [this](const Rastro& r) {
                    float dx = r.posicion.x - destinoRastro.x;
                    float dy = r.posicion.y - destinoRastro.y;
                    return std::sqrt(dx*dx + dy*dy) < 10.0f;
                });
            if (it != rastros.end()) {
                rastros.erase(it);
            }
        }
    } else {
        // Moverse hacia el rastro
        moverHaciaPunto(destinoRastro.x, destinoRastro.y, deltaTime);
        caminando = true;
    }

    // Si lleva mucho tiempo buscando (8 seg), abandonar
    if (tiempoEnRastro > 8.0f) {
        siguiendoRastro = false;
        estadoActual = PATRULLANDO;
    }
}

void Art::setInteligenciaBusqueda(bool activar) {
    inteligenciaBusqueda = activar;
    if (!activar) {
        // abandonar rastros si se desactiva
        siguiendoRastro = false;
    }
}
