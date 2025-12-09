#ifndef ART_H
#define ART_H

#include <SFML/Graphics.hpp> //sf
#include <cmath>
#include <vector> //para almacenar las texturas de movimiento
#include <string> //rutas de archivos
#include <algorithm>

class Art {
private:
    // Posición y movimiento
    float x, y;
    float velocidad;
    float rangoDeteccion;

    //estados
    enum Estado { PATRULLANDO, PERSEGUIR, REGRESANDO };
    Estado estadoActual;
    
    // Direcciones
    enum Direccion { ABAJO, ARRIBA, IZQUIERDA, DERECHA };   //enum convierte las direcciones en numeraciones enteras para que el codigo sea mas legible
    Direccion direccionActual;
    
    float memoriaTiempo;          // Tiempo que recuerda al jugador
    sf::Vector2f ultimaPosJugador; // Última posición conocida del jugador
    bool jugadorVistoRecientemente; // Flag para saber si vio al jugador
    float tiempoDesdeUltimaVista;  // Contador desde que vio al jugador
    struct Rastro {
        sf::Vector2f posicion;
        float frescura;  // 1.0 (nuevo) a 0.0 (viejo)
        std::string tipo; // "objeto" o "movimiento"
        
        Rastro(float x, float y, const std::string& t) 
            : posicion(x, y), frescura(1.0f), tipo(t) {}
    };
    
    std::vector<Rastro> rastros;         
    bool siguiendoRastro;                 
    sf::Vector2f destinoRastro;           
    float tiempoEnRastro;  

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
    
    struct Waypoint {
        sf::Vector2f posicion;
        float tiempoEspera;  // Tiempo a esperar en este waypoint
        std::string accion; // acción opcional a ejecutar al llegar

        Waypoint(float x, float y, float wait = 0.0f, const std::string& acc = "")
            : posicion(x, y), tiempoEspera(wait), accion(acc) {}
    };
    
    std::vector<Waypoint> waypoints;
    size_t waypointActual;
    float distanciaUmbral;
    float temporizadorEspera;
    bool esperando;
    bool patrullaCiclica;
    
    // Punto de inicio (para regresar después de perseguir)
    size_t waypointAntesDePerseguir;
    size_t encontrarWaypointMasCercano() const;
    
    float calcularDistancia(float px, float py) const; //distancia al jugador
    float calcularDistanciaAlPunto(const sf::Vector2f& punto) const;
    void actualizarAnimacion(float deltaTime);  //gestiona la animación
    void actualizarDireccion(float diffX, float diffY);     //actualiza la dirección según la posición del jugador
    void actualizarPatrulla(float deltaTime);
    void actualizarPersecucion(float deltaTime, float playerX, float playerY);
    void actualizarRegreso(float deltaTime);
    void moverHaciaPunto(float targetX, float targetY, float deltaTime);
    void actualizarRastros(float deltaTime);         
    void seguirRastroMasFresco(float deltaTime);
    void actualizarMemoria(float deltaTime, float playerX, float playerY);
    

public:
    // Constructor
    Art(float startX = 0.0f, float startY = 0.0f);
    
    // Método principal de actualización
    void actualizar(float deltaTime, float playerX, float playerY);
    
    // Cargar texturas
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
    
    // Sistema de waypoints
    void agregarWaypoint(float x, float y, float tiempoEspera = 0.0f, const std::string& accion = "");
    void limpiarWaypoints();
    bool cargarWaypointsDesdeArchivo(const std::string& rutaArchivo);
    
    // Getters
    float getX() const { return x; }
    float getY() const { return y; }
    float getMemoriaTiempo() const { return memoriaTiempo; }
    sf::Sprite& getSprite() { return sprite; }
    Estado getEstado() const { return estadoActual; }
    size_t getWaypointActual() const { return waypointActual; }
    size_t getTotalWaypoints() const { return waypoints.size(); }
    
    // Setters
    void setPosicion(float nuevaX, float nuevaY);
    void setVelocidad(float nuevaVelocidad) { velocidad = nuevaVelocidad; }
    void setRangoDeteccion(float nuevoRango) { rangoDeteccion = nuevoRango; }
    void setDistanciaUmbral(float distancia) { distanciaUmbral = distancia; }
    void setPatrullaCiclica(bool ciclico) { patrullaCiclica = ciclico; }
    void setMemoriaTiempo(float tiempo) { memoriaTiempo = tiempo; }
    void setInteligenciaBusqueda(bool activar);
    void marcarRecoleccionObjeto(float x, float y);    
    void marcarMovimientoJugador(float x, float y);
    
    // Para debug
    sf::Vector2f getWaypointPosition(size_t index) const {
        if (index < waypoints.size()) {
            return waypoints[index].posicion;
        }
        return sf::Vector2f(0, 0);
    }

    // Fuerza a Art a cambiar su rumbo (SE usa tras colisión)
    void invertirDireccion();
};

#endif