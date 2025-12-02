#ifndef ART_H
#define ART_H

#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <string>

class Art {
private:
    // Posición y movimiento
    float x, y;
    float velocidad;
    float rangoDeteccion;
    
    // Direcciones
    enum Direccion { ABAJO, ARRIBA, IZQUIERDA, DERECHA };
    Direccion direccionActual;
    
    // Sprites (1 quieto + 4 caminar por cada dirección)
    sf::Texture quietoAbajo;
    sf::Texture quietoArriba;
    sf::Texture quietoIzquierda;
    sf::Texture quietoDerecha;
    
    std::vector<sf::Texture> caminarAbajo;
    std::vector<sf::Texture> caminarArriba;
    std::vector<sf::Texture> caminarIzquierda;
    std::vector<sf::Texture> caminarDerecha;
    
    // Sprite y animación
    sf::Sprite sprite;
    int frameActual;
    float tiempoPorFrame;
    float tiempoAnimacion;
    bool caminando;
    
    // Métodos privados
    float calcularDistancia(float px, float py) const;
    void actualizarAnimacion(float deltaTime);
    void actualizarDireccion(float diffX, float diffY);

public:
    // Constructor
    Art(float startX = 0.0f, float startY = 0.0f);
    
    // Método principal de actualización
    void actualizar(float deltaTime, float playerX, float playerY);
    
    // Cargar texturas (SILENCIOSO - sin mensajes de consola)
    void cargarSprites(
        const std::string& quietoAbajoPath,
        const std::string& quietoArribaPath,
        const std::string& quietoIzquierdaPath,
        const std::string& quietoDerechaPath,
        const std::vector<std::string>& caminarAbajoPaths,    // 4 frames
        const std::vector<std::string>& caminarArribaPaths,   // 4 frames
        const std::vector<std::string>& caminarIzquierdaPaths, // 4 frames
        const std::vector<std::string>& caminarDerechaPaths   // 4 frames
    );
    
    // Getters
    float getX() const { return x; }
    float getY() const { return y; }
    sf::Sprite& getSprite() { return sprite; }
    
    // Setters
    void setPosicion(float nuevaX, float nuevaY);
    void setVelocidad(float nuevaVelocidad) { velocidad = nuevaVelocidad; }
    void setRangoDeteccion(float nuevoRango) { rangoDeteccion = nuevoRango; }
};

#endif