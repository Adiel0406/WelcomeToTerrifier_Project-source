#include "Art.h"

Art::Art(float startX, float startY) 
    : x(startX), y(startY),
      velocidad(150.0f),
      rangoDeteccion(300.0f),
      direccionActual(ABAJO),
      frameActual(0),
      tiempoPorFrame(0.15f),
      tiempoAnimacion(0.0f),
      caminando(false)
{
    // Crear sprite básico (fallback - cuadrado blanco)
    sf::Texture tex;
    tex.create(32, 32);
    sprite.setTexture(tex);
    sprite.setPosition(x, y);
    sprite.setOrigin(45.0f, 45.0f);
}

void Art::actualizar(float deltaTime, float playerX, float playerY) {
    // Calcular distancia y dirección al jugador
    float diffX = playerX - x;
    float diffY = playerY - y;
    float distancia = calcularDistancia(playerX, playerY);
    
    // Actualizar dirección visual (que sprite usara)
    actualizarDireccion(diffX, diffY);
    
    // Comportamiento: perseguir si está en rango
    if (distancia <= rangoDeteccion && distancia > 20.0f) {
        // Moverse hacia el jugador
        float dirX = diffX / distancia;
        float dirY = diffY / distancia;
        
        x += dirX * velocidad * deltaTime;
        y += dirY * velocidad * deltaTime;
        caminando = true;
    } else {
        caminando = false;
    }
    
    // Actualizar animación
    actualizarAnimacion(deltaTime);
    
    // Actualizar posición del sprite
    sprite.setPosition(x, y);
}
//sin ella, art no se moveria ni animaria

void Art::actualizarDireccion(float diffX, float diffY) {
    // Determinar dirección basada en el vector hacia el jugador
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
//sin ella no funcionaria la animacion y la direccion (solo miraria hacia abajo)

void Art::actualizarAnimacion(float deltaTime) {
    tiempoAnimacion += deltaTime;
    
    if (caminando) {
        // ANIMACIÓN CAMINAR
        if (tiempoAnimacion >= tiempoPorFrame) {
            // Seleccionar vector de texturas según dirección
            std::vector<sf::Texture>* texturas = nullptr;    //Permite cambiar a qué vector apunta según dirección (no apunta a nada inicialmente)
            
            switch (direccionActual) {
                case ABAJO: texturas = &caminarAbajo; break;
                case ARRIBA: texturas = &caminarArriba; break;
                case IZQUIERDA: texturas = &caminarIzquierda; break;
                case DERECHA: texturas = &caminarDerecha; break;
            }
            
            // Si hay texturas, cambiar frame
            if (texturas && !texturas->empty()) {
                frameActual = (frameActual + 1) % texturas->size(); // ciclo entre frames
                sprite.setTexture((*texturas)[frameActual]); // actualizar textura del sprite
            }
            
            tiempoAnimacion = 0.0f;
        }
    } else {
        // SPRITE QUIETO (solo 1 frame)
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
//sin ella, el tiempo no se acumularia y no habria cambio de frame o seria muy rapido, y puede llegarse a crashear si no hay texturas cargadas

float Art::calcularDistancia(float px, float py) const { //al usar const no se puede modificar ningun atributo de la clase, solo lee
    float diffX = px - x;
    float diffY = py - y;
    return std::sqrt(diffX * diffX + diffY * diffY); //teorema de pitagoras (no se usa pow por eficiencia)
}
//sin ella no se podria calcular la distancia al jugador

// ==================== CARGAR SPRITES ====================

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
    sprite.setOrigin(sprite.getLocalBounds().width / 2.0f, sprite.getLocalBounds().height / 2.0f);
}

void Art::setPosicion(float nuevaX, float nuevaY) {     //actualizar posición
    x = nuevaX;
    y = nuevaY;
    sprite.setPosition(x, y);   //sincronizar sprite
}
//sin ella no se podria cambiar la posicion de art