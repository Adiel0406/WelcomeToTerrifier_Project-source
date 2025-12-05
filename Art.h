#ifndef ART_H
#define ART_H

#include <SFML/Graphics.hpp> //sf
#include <cmath>
#include <vector> //para almacenar las texturas de movimiento
#include <string> //rutas de archivos

class Art {
private:
    // Posición y movimiento
    float x, y;
    float velocidad;
    float rangoDeteccion;
    
    // Direcciones
    enum Direccion { ABAJO, ARRIBA, IZQUIERDA, DERECHA };   //enum convierte las direcciones en numeraciones enteras para que el codigo sea mas legible
    Direccion direccionActual;
    
    // Sprites (1 quieto + 4 caminar por cada dirección)
    sf::Texture quietoAbajo;
    sf::Texture quietoArriba;
    sf::Texture quietoIzquierda;
    sf::Texture quietoDerecha;
    // Vectores para almacenar las texturas de caminar en cada dirección
    std::vector<sf::Texture> caminarAbajo;
    std::vector<sf::Texture> caminarArriba;
    std::vector<sf::Texture> caminarIzquierda;
    std::vector<sf::Texture> caminarDerecha;
    
    // Sprite y animación
    sf::Sprite sprite;      //representación visual de Art
    int frameActual;
    float tiempoPorFrame;   //duración de cada frame en segundos
    float tiempoAnimacion;  //acumulador de tiempo tras el ultimo cambio de frame
    bool caminando; //decide si se queda quieto o se mueve
    
    
    float calcularDistancia(float px, float py) const; //distancia al jugador
    void actualizarAnimacion(float deltaTime);  //gestiona la animación
    void actualizarDireccion(float diffX, float diffY);     //actualiza la dirección según la posición del jugador

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