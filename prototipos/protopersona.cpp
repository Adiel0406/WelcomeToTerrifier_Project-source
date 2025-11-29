#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <cmath>
#include <fstream>
#include <random>
#include <chrono>

// ==================== SISTEMA DE ESCAPE ====================
class SistemaDeEscape {
private:
    enum class FaseEscape { INACTIVO, ACECHO, VENTANA_ESCAPE, FALLO };
    
    FaseEscape faseActual;
    float tiempoFase;
    float duracionAcecho;
    float duracionVentana;
    bool escapeExitoso;
    std::mt19937 generador;

public:
    SistemaDeEscape() 
        : faseActual(FaseEscape::INACTIVO),
          tiempoFase(0),
          duracionAcecho(2.5f),
          duracionVentana(1.5f),
          escapeExitoso(false)
    {
        generador.seed(std::chrono::system_clock::now().time_since_epoch().count());
    }

    void iniciarAcecho() {
        faseActual = FaseEscape::ACECHO;
        tiempoFase = 0;
        escapeExitoso = false;
    }

    void actualizar(float deltaTime) {
        if (faseActual == FaseEscape::INACTIVO) return;

        tiempoFase += deltaTime;

        switch (faseActual) {
            case FaseEscape::ACECHO:
                if (tiempoFase >= duracionAcecho) {
                    faseActual = FaseEscape::VENTANA_ESCAPE;
                    tiempoFase = 0;
                }
                break;

            case FaseEscape::VENTANA_ESCAPE:
                if (tiempoFase >= duracionVentana) {
                    faseActual = FaseEscape::FALLO;
                }
                break;

            case FaseEscape::FALLO:
                if (tiempoFase >= 1.0f) {
                    reiniciar();
                }
                break;
        }
    }

    void intentarMovimiento(bool jugadorSeMovio) {
        if (faseActual == FaseEscape::INACTIVO) return;

        switch (faseActual) {
            case FaseEscape::ACECHO:
                if (jugadorSeMovio) {
                    faseActual = FaseEscape::FALLO;
                    tiempoFase = 0;
                }
                break;

            case FaseEscape::VENTANA_ESCAPE:
                if (jugadorSeMovio) {
                    escapeExitoso = true;
                    faseActual = FaseEscape::INACTIVO;
                }
                break;
        }
    }

    bool isAcechoActivo() const { return faseActual == FaseEscape::ACECHO; }
    bool isVentanaEscapeActiva() const { return faseActual == FaseEscape::VENTANA_ESCAPE; }
    bool isFallo() const { return faseActual == FaseEscape::FALLO; }
    bool isEscapeExitoso() const { return escapeExitoso; }
    bool isEscapeActivo() const { return faseActual != FaseEscape::INACTIVO; }

    void reiniciar() {
        faseActual = FaseEscape::INACTIVO;
        tiempoFase = 0;
        escapeExitoso = false;
    }

    std::string getMensajeFase() const {
        switch (faseActual) {
            case FaseEscape::ACECHO: return "¡No te muevas! La entidad te observa...";
            case FaseEscape::VENTANA_ESCAPE: return "¡AHORA! ¡Muévete para escapar!";
            case FaseEscape::FALLO: return "Fallaste... La entidad te atrapa";
            default: return "";
        }
    }
};

// ==================== SISTEMA DE ALERTAS ====================
class SistemaAlerta {
private:
    bool alertaActiva;
    float tiempoAlerta;
    float duracionAlerta;
    std::string mensajeAlerta;
    std::mt19937 generador;

public:
    SistemaAlerta() 
        : alertaActiva(false), 
          tiempoAlerta(0), 
          duracionAlerta(3.0f) 
    {
        generador.seed(std::chrono::system_clock::now().time_since_epoch().count());
    }

    void activarAlerta(const std::string& mensaje) {
        alertaActiva = true;
        mensajeAlerta = mensaje;
        tiempoAlerta = 0;
    }

    void actualizar(float deltaTime) {
        if (alertaActiva) {
            tiempoAlerta += deltaTime;
            if (tiempoAlerta >= duracionAlerta) {
                alertaActiva = false;
            }
        }
    }

    bool generarEventoAleatorio(float probabilidad) {
        std::uniform_real_distribution<float> distribucion(0.0f, 1.0f);
        return distribucion(generador) < probabilidad;
    }

    bool isAlertaActiva() const { return alertaActiva; }
    std::string getMensajeAlerta() const { return mensajeAlerta; }
};

// ==================== SISTEMA DE INVENTARIO ====================
class Inventario {
private:
    std::vector<std::string> items;
    int capacidadMaxima;

public:
    Inventario(int capacidad = 10) : capacidadMaxima(capacidad) {}

    void agregarItem(const std::string& item) {
        if (items.size() < capacidadMaxima) {
            items.push_back(item);
            std::cout << "Item agregado: " << item << std::endl;
        } else {
            std::cout << "Inventario lleno!" << std::endl;
        }
    }

    void mostrarInventario() const {
        std::cout << "=== INVENTARIO ===" << std::endl;
        for (size_t i = 0; i < items.size(); ++i) {
            std::cout << i + 1 << ". " << items[i] << std::endl;
        }
        std::cout << "==================" << std::endl;
    }

    size_t getCantidadItems() const { return items.size(); }
    int getCapacidadMaxima() const { return capacidadMaxima; }
    const std::vector<std::string>& getItems() const { return items; }
};

// ==================== SISTEMA DE VIDA ====================
class BarraVida {
private:
    float vidaMaxima;
    float vidaActual;

public:
    BarraVida(float maxVida = 100.0f) : vidaMaxima(maxVida), vidaActual(maxVida) {}

    void recibirDano(float dano) {
        vidaActual = std::max(0.0f, vidaActual - dano);
    }

    void curar(float curacion) {
        vidaActual = std::min(vidaMaxima, vidaActual + curacion);
    }

    float getPorcentajeVida() const {
        return (vidaActual / vidaMaxima) * 100.0f;
    }

    bool estaVivo() const {
        return vidaActual > 0;
    }

    float getVidaActual() const { return vidaActual; }
    float getVidaMaxima() const { return vidaMaxima; }
};

// ==================== SISTEMA DE ANIMACIONES ====================
enum class EstadoAnimacion {
    QUIETO, CAMINAR, ATACAR, PERSEGUIR, ALERTA, ACEECHO
};

class Sprites {
private:
    std::map<EstadoAnimacion, std::vector<sf::Texture>> animaciones;
    std::map<EstadoAnimacion, std::vector<std::string>> rutasTexturas;
    EstadoAnimacion estadoActual;
    int frameActual;
    float tPorFrame;
    float tAcumulado;
    sf::Sprite spriteActual;

public:
    Sprites() 
        : estadoActual(EstadoAnimacion::QUIETO), 
          frameActual(0), 
          tPorFrame(0.1f), 
          tAcumulado(0) 
    {}

    void agregarAnimacion(EstadoAnimacion estado, const std::vector<std::string>& rutasArchivos) {
        rutasTexturas[estado] = rutasArchivos;
        std::vector<sf::Texture> frames;
        
        for (const auto& ruta : rutasArchivos) {
            sf::Texture textura;
            if (textura.loadFromFile(ruta)) {
                frames.push_back(textura);
            } else {
                // Textura de fallback
                sf::Texture fallback;
                fallback.create(32, 32);
                frames.push_back(fallback);
            }
        }
        
        animaciones[estado] = frames;
        
        if (!frames.empty()) {
            spriteActual.setTexture(frames[0]);
            spriteActual.setOrigin(frames[0].getSize().x / 2, frames[0].getSize().y / 2);
        }
    }

    void actualizar(float deltaTime) {
        tAcumulado += deltaTime;
        
        if (tAcumulado >= tPorFrame && !animaciones[estadoActual].empty()) {
            frameActual = (frameActual + 1) % animaciones[estadoActual].size();
            spriteActual.setTexture(animaciones[estadoActual][frameActual]);
            tAcumulado = 0;
        }
    }

    void cambiarEstado(EstadoAnimacion nuevoEstado) {
        if (estadoActual != nuevoEstado) {
            estadoActual = nuevoEstado;
            frameActual = 0;
            tAcumulado = 0;
            if (!animaciones[estadoActual].empty()) {
                spriteActual.setTexture(animaciones[estadoActual][frameActual]);
            }
        }
    }

    EstadoAnimacion getEstadoActual() const { return estadoActual; }
    void setTiempoPorFrame(float tiempo) { tPorFrame = tiempo; }
    sf::Sprite& getSpriteSFML() { return spriteActual; }
    void setPosicionSprite(float x, float y) { spriteActual.setPosition(x, y); }
};

// ==================== ENTIDAD BASE SIENNA ====================
enum class Direccion { 
    ARRIBA, ABAJO, IZQUIERDA, DERECHA, QUIETO 
};

class Sienna {
protected:
    float x, y;
    std::string nombre;
    Sprites sprite;
    Direccion direccion;

public:
    Sienna(float startX, float startY, const std::string& Nombre) 
        : x(startX), y(startY), nombre(Nombre), direccion(Direccion::ABAJO) 
    {}

    virtual ~Sienna() = default;
    
    virtual void actualizar(float deltaTime) {
        sprite.actualizar(deltaTime);
    }

    // Getters
    float getX() const { return x; }
    float getY() const { return y; }
    std::string getNombre() const { return nombre; }
    Direccion getDireccion() const { return direccion; }
    Sprites& getSprite() { return sprite; }

    // Setters
    void setPosicion(float nuevaX, float nuevaY) {
        x = nuevaX;
        y = nuevaY;
        sprite.setPosicionSprite(x, y);
    }

    void setDireccion(Direccion nuevaDireccion) {
        direccion = nuevaDireccion;
    }
};

// ==================== ENTIDAD BASE ART (PARA IA) ====================
enum class DireccionArt { 
    ARRIBA, ABAJO, IZQUIERDA, DERECHA, QUIETO 
};

class Art {
protected:
    float x, y;
    std::string nombre;
    Sprites sprite;
    DireccionArt direccion;

public:
    Art(float startX, float startY, const std::string& entidadNombre) 
        : x(startX), y(startY), nombre(entidadNombre), direccion(DireccionArt::ABAJO) 
    {}

    virtual ~Art() = default;
    virtual void actualizar(float deltaTime) = 0;

    float getX() const { return x; }
    float getY() const { return y; }
    void setPosicion(float nuevaX, float nuevaY) { 
        x = nuevaX; 
        y = nuevaY; 
        sprite.setPosicionSprite(x, y);
    }
    Sprites& getSprite() { return sprite; }
    DireccionArt getDireccion() const { return direccion; }
    void setDireccion(DireccionArt nuevaDireccion) { direccion = nuevaDireccion; }
    std::string getNombre() const { return nombre; }
};

// ==================== SISTEMA GUI ====================
class GUI {
private:
    sf::Font fuente;
    sf::Text textoVida;
    sf::Text textoInventario;
    sf::Text textoEstado;
    sf::Text textoAlerta;
    sf::Text textoEscape;

public:
    GUI() {
        if (!fuente.loadFromFile("assets/fuentes/arial.ttf")) {
            std::cout << "No se pudo cargar la fuente" << std::endl;
        }
        
        // Configurar textos
        textoVida.setFont(fuente);
        textoVida.setCharacterSize(16);
        textoVida.setFillColor(sf::Color::Green);
        textoVida.setPosition(10, 10);
        
        textoInventario.setFont(fuente);
        textoInventario.setCharacterSize(16);
        textoInventario.setFillColor(sf::Color::White);
        textoInventario.setPosition(10, 30);
        
        textoEstado.setFont(fuente);
        textoEstado.setCharacterSize(16);
        textoEstado.setFillColor(sf::Color::Yellow);
        textoEstado.setPosition(10, 50);
        
        textoAlerta.setFont(fuente);
        textoAlerta.setCharacterSize(18);
        textoAlerta.setFillColor(sf::Color::Red);
        textoAlerta.setPosition(250, 10);
        
        textoEscape.setFont(fuente);
        textoEscape.setCharacterSize(20);
        textoEscape.setFillColor(sf::Color::Cyan);
        textoEscape.setPosition(250, 50);
        textoEscape.setStyle(sf::Text::Bold);
    }

    void actualizarGUI(const BarraVida& vida, const Inventario& inventario, 
                      const std::string& estadoJugador, const SistemaAlerta& alerta,
                      const SistemaEscape& escape) {
        // Actualizar texto de vida
        textoVida.setString("VIDA: " + std::to_string((int)vida.getVidaActual()) + 
                           "/" + std::to_string((int)vida.getVidaMaxima()));
        
        // Actualizar texto de inventario
        textoInventario.setString("INVENTARIO: " + std::to_string(inventario.getCantidadItems()) + 
                                 "/" + std::to_string(inventario.getCapacidadMaxima()));
        
        // Actualizar estado del jugador
        textoEstado.setString("ESTADO: " + estadoJugador);
        
        // Actualizar alertas
        if (alerta.isAlertaActiva()) {
            textoAlerta.setString(alerta.getMensajeAlerta());
        } else {
            textoAlerta.setString("");
        }
        
        // Actualizar sistema de escape
        if (escape.isEscapeActivo()) {
            textoEscape.setString(escape.getMensajeFase());
        } else {
            textoEscape.setString("");
        }
    }

    void dibujar(sf::RenderWindow& ventana) {
        ventana.draw(textoVida);
        ventana.draw(textoInventario);
        ventana.draw(textoEstado);
        ventana.draw(textoAlerta);
        ventana.draw(textoEscape);
    }
};

// ==================== JUGADOR (HEREDA DE SIENNA) ====================
class Jugador : public Sienna {
private:
    // PARÁMETROS DE MOVIMIENTO 
    float velocidadMovimiento;
    bool estaAtacando;
    float tiempoAtaque;
    float tiempoAtaqueActual;
    Inventario inventario;
    BarraVida barraVida;
    SistemaAlerta* sistemaAlerta;
    SistemaEscape* sistemaEscape;
    bool movimientoIntentado;
    bool espadaEquipada;

public:
    // ========== CONSTRUCTOR ==========
    Jugador(float startX, float startY, SistemaAlerta* alertaSystem, SistemaEscape* escapeSystem) 
        : Sienna(startX, startY, "Jugador"),
          velocidadMovimiento(250.0f),
          estaAtacando(false),
          tiempoAtaque(0.3f),
          tiempoAtaqueActual(0.0f),
          inventario(10),
          barraVida(100.0f),
          sistemaAlerta(alertaSystem),
          sistemaEscape(escapeSystem),
          movimientoIntentado(false),
          espadaEquipada(true)
    {
        configurarAnimacionesJugador();
        // Agregar algunos items de ejemplo al inventario
        inventario.agregarItem("Espada de Acero");
        inventario.agregarItem("Poción de Vida");
        inventario.agregarItem("Llave Antigua");
    }

    // ========== CONFIGURAR ANIMACIONES ==========
    void configurarAnimacionesJugador() {
        std::vector<std::string> spritesQuieto = {
            "assets/sprites/jugador/quieto/frame_1.png",
            "assets/sprites/jugador/quieto/frame_2.png"
        };
        
        std::vector<std::string> spritesCaminar = {
            "assets/sprites/jugador/caminar/frame_1.png",
            "assets/sprites/jugador/caminar/frame_2.png",
            "assets/sprites/jugador/caminar/frame_3.png"
        };
        
        std::vector<std::string> spritesAtacar = {
            "assets/sprites/jugador/atacar/frame_1.png",
            "assets/sprites/jugador/atacar/frame_2.png",
            "assets/sprites/jugador/atacar/frame_3.png"
        };
        
        sprite.agregarAnimacion(EstadoAnimacion::QUIETO, spritesQuieto);
        sprite.agregarAnimacion(EstadoAnimacion::CAMINAR, spritesCaminar);
        sprite.agregarAnimacion(EstadoAnimacion::ATACAR, spritesAtacar);
        sprite.setTiempoPorFrame(0.1f);
    }

    // ========== ACTUALIZAR (OVERRIDE) ==========
    void actualizar(float deltaTime) override {
        movimientoIntentado = false;
        
        if (estaAtacando) {
            tiempoAtaqueActual += deltaTime;
            if (tiempoAtaqueActual >= tiempoAtaque) {
                estaAtacando = false;
                tiempoAtaqueActual = 0.0f;
                sprite.cambiarEstado(EstadoAnimacion::QUIETO);
            }
        }
        
        Sienna::actualizar(deltaTime);
    }

    // ========== MANEJAR INPUT ==========
    void manejarInput(sf::Keyboard::Key tecla, float deltaTime) {
        float movX = 0, movY = 0;
        bool seMovio = false;

        switch(tecla) {
            case sf::Keyboard::W: 
                movY = -1;
                direccion = Direccion::ARRIBA;
                seMovio = true;
                break;
                
            case sf::Keyboard::S: 
                movY = 1;
                direccion = Direccion::ABAJO;
                seMovio = true;
                break;
                
            case sf::Keyboard::A: 
                movX = -1;
                direccion = Direccion::IZQUIERDA;
                seMovio = true;
                break;
                
            case sf::Keyboard::D: 
                movX = 1;
                direccion = Direccion::DERECHA;
                seMovio = true;
                break;
                
            case sf::Keyboard::Space:  // BARRA ESPACIADORA - ATAQUE
                iniciarAtaque();
                break;
                
            case sf::Keyboard::Q:  // Mostrar inventario
                inventario.mostrarInventario();
                break;

            case sf::Keyboard::R:  // Recibir daño (para prueba)
                barraVida.recibirDano(10.0f);
                sistemaAlerta->activarAlerta("¡Recibiste 10 de daño!");
                break;

            case sf::Keyboard::H:  // Curar (para prueba)
                barraVida.curar(15.0f);
                sistemaAlerta->activarAlerta("¡Te curaste 15 de vida!");
                break;

            case sf::Keyboard::I:  // Agregar item de prueba
                inventario.agregarItem("Poción de Mana");
                break;
        }

        // ===== APLICAR MOVIMIENTO =====
        if (seMovio && !estaAtacando) {
            if (movX != 0 && movY != 0) {
                movX *= 0.7071f;
                movY *= 0.7071f;
            }
            
            x += movX * velocidadMovimiento * deltaTime;
            y += movY * velocidadMovimiento * deltaTime;
            
            sprite.cambiarEstado(EstadoAnimacion::CAMINAR);
        }
        else if (!estaAtacando && !seMovio) {
            sprite.cambiarEstado(EstadoAnimacion::QUIETO);
        }

        // Detectar intento de movimiento durante sistema de escape
        if ((tecla == sf::Keyboard::W || tecla == sf::Keyboard::A || 
             tecla == sf::Keyboard::S || tecla == sf::Keyboard::D) && 
            sistemaEscape->isEscapeActivo()) {
            
            movimientoIntentado = true;
            sistemaEscape->intentarMovimiento(true);
        }
    }

private:
    // ========== INICIAR ATAQUE ==========
    void iniciarAtaque() {
        if (!estaAtacando && espadaEquipada) {
            estaAtacando = true;
            tiempoAtaqueActual = 0.0f;
            sprite.cambiarEstado(EstadoAnimacion::ATACAR);
            sistemaAlerta->activarAlerta("¡Ataque ejecutado!");
        }
    }

public:
    // ========== MÉTODOS PÚBLICOS DEL JUGADOR ==========
    
    // Control de movimiento
    void setPuedeMoverse(bool puede) { /* Implementar si es necesario */ }
    void forzarInmovilidad() { sprite.cambiarEstado(EstadoAnimacion::QUIETO); }
    void restaurarMovimiento() { /* Implementar si es necesario */ }
    
    // Getters de estado
    bool getMovimientoIntentado() const { return movimientoIntentado; }
    bool getEstaAtacando() const { return estaAtacando; }
    float getVelocidad() const { return velocidadMovimiento; }
    
    // Getters de sistemas
    Inventario& getInventario() { return inventario; }
    BarraVida& getBarraVida() { return barraVida; }
    
    // Estado como string
    std::string getEstadoString() const {
        switch(sprite.getEstadoActual()) {
            case EstadoAnimacion::QUIETO: return "QUIETO";
            case EstadoAnimacion::CAMINAR: return "CAMINANDO";
            case EstadoAnimacion::ATACAR: return "ATACANDO";
            default: return "DESCONOCIDO";
        }
    }

    // Setters
    void setVelocidad(float nuevaVelocidad) { velocidadMovimiento = nuevaVelocidad; }
};

// ==================== ENTIDAD IA (HEREDA DE ART) ====================
class IA : public Art {
private:
    // Entidad que esta IA va a perseguir
    Art* objetivo;
    
    // Velocidad de movimiento
    float velocidad;
    
    // Distancia a la que detecta al objetivo
    float rangoDeteccion;
    
    // Distancia a la que considera que "alcanzó" al objetivo
    float rangoAtaque;
    float rangoAcecho;
    SistemaAlerta* sistemaAlerta;
    SistemaEscape* sistemaEscape;
    bool acechoActivo;
    float tiempoEntreAcechos;
    float tiempoUltimoAcecho;

public:
    // inicializar posición, objetivo y parámetros de IA
    IA(float startX, float startY, Art* objetivoPerseguir, SistemaAlerta* alertaSystem, SistemaEscape* escapeSystem) 
        : Art(startX, startY, "EnemigoIA"),
          objetivo(objetivoPerseguir),
          velocidad(180.0f),
          rangoDeteccion(250.0f),
          rangoAtaque(40.0f),
          rangoAcecho(80.0f),
          sistemaAlerta(alertaSystem),
          sistemaEscape(escapeSystem),
          acechoActivo(false),
          tiempoEntreAcechos(8.0f),
          tiempoUltimoAcecho(0)
    {
        configurarAnimacionesIA();
    }

    // 1. Configurar Animaciones Específicas 
    void configurarAnimacionesIA() {
        std::vector<std::string> spritesQuieto = {
            "assets/sprites/enemigo/quieto/frame_1.png",
            "assets/sprites/enemigo/quieto/frame_2.png", 
            "assets/sprites/enemigo/quieto/frame_3.png",
            "assets/sprites/enemigo/quieto/frame_4.png"
        };
        
        std::vector<std::string> spritesPerseguir = {
            "assets/sprites/enemigo/perseguir/frame_1.png",
            "assets/sprites/enemigo/perseguir/frame_2.png",
            "assets/sprites/enemigo/perseguir/frame_3.png", 
            "assets/sprites/enemigo/perseguir/frame_4.png"
        };
        
        std::vector<std::string> spritesAtacar = {
            "assets/sprites/enemigo/atacar/frame_1.png",
            "assets/sprites/enemigo/atacar/frame_2.png",
            "assets/sprites/enemigo/atacar/frame_3.png",
            "assets/sprites/enemigo/atacar/frame_4.png"
        };

        std::vector<std::string> spritesAcecho = {
            "assets/sprites/enemigo/acecho/frame_1.png",
            "assets/sprites/enemigo/acecho/frame_2.png",
            "assets/sprites/enemigo/acecho/frame_3.png"
        };

        std::vector<std::string> spritesAlerta = {
            "assets/sprites/enemigo/alerta/frame_1.png",
            "assets/sprites/enemigo/alerta/frame_2.png"
        };
        
        sprite.agregarAnimacion(EstadoAnimacion::QUIETO, spritesQuieto);
        sprite.agregarAnimacion(EstadoAnimacion::PERSEGUIR, spritesPerseguir);
        sprite.agregarAnimacion(EstadoAnimacion::ATACAR, spritesAtacar);
        sprite.agregarAnimacion(EstadoAnimacion::ACEECHO, spritesAcecho);
        sprite.agregarAnimacion(EstadoAnimacion::ALERTA, spritesAlerta);
        
        sprite.setTiempoPorFrame(0.15f);
    }

    // 2. Actualizar Dirección Visual
    void actualizarDireccion() {
        float diffX = objetivo->getX() - x;
        float diffY = objetivo->getY() - y;
        
        if (std::abs(diffX) > std::abs(diffY)) {
            direccion = (diffX > 0) ? DireccionArt::DERECHA : DireccionArt::IZQUIERDA;
        } else {
            direccion = (diffY > 0) ? DireccionArt::ABAJO : DireccionArt::ARRIBA;
        }
    }

    // 3. Actualizar (Implementación)
    void actualizar(float deltaTime) override {
        float diffX = objetivo->getX() - x;
        float diffY = objetivo->getY() - y;
        float distancia = std::sqrt(diffX * diffX + diffY * diffY);
        
        actualizarDireccion();
        
        tiempoUltimoAcecho += deltaTime;

        // LÓGICA PRINCIPAL DE COMPORTAMIENTO
        if (distancia <= rangoDeteccion) {
            if (sistemaEscape->isEscapeActivo()) {
                mantenerPosicionAcecho(diffX, diffY, distancia);
            }
            else if (distancia <= rangoAcecho && tiempoUltimoAcecho >= tiempoEntreAcechos) {
                iniciarAcecho();
            }
            else if (distancia > rangoAtaque) {
                perseguirObjetivo(deltaTime, diffX, diffY, distancia);
            } else {
                atacarObjetivo();
            }
        } else {
            patrullar(deltaTime);
        }
        
        sprite.actualizar(deltaTime);
    }

private:
    // 4. Perseguir Objetivo
    void perseguirObjetivo(float deltaTime, float diffX, float diffY, float distancia) {
        float direccionX = diffX / distancia;
        float direccionY = diffY / distancia;
        
        x += direccionX * velocidad * deltaTime;
        y += direccionY * velocidad * deltaTime;
        
        sprite.cambiarEstado(EstadoAnimacion::PERSEGUIR);
    }

    // 5. Atacar Objetivo
    void atacarObjetivo() {
        sprite.cambiarEstado(EstadoAnimacion::ATACAR);
        sistemaAlerta->activarAlerta("¡La entidad te ataca!");
    }

    // 6. Iniciar Acecho
    void iniciarAcecho() {
        sistemaEscape->iniciarAcecho();
        acechoActivo = true;
        tiempoUltimoAcecho = 0;
        sistemaAlerta->activarAlerta("¡La entidad está muy cerca! No te muevas...");
        sprite.cambiarEstado(EstadoAnimacion::ACEECHO);
    }

    // 7. Mantener posición durante acecho
    void mantenerPosicionAcecho(float diffX, float diffY, float distancia) {
        float distanciaDeseada = rangoAcecho * 0.8f;
        
        if (distancia > distanciaDeseada) {
            float direccionX = diffX / distancia;
            float direccionY = diffY / distancia;
            x += direccionX * velocidad * 0.5f;
            y += direccionY * velocidad * 0.5f;
        }

        if (sistemaEscape->isVentanaEscapeActiva()) {
            sistemaAlerta->activarAlerta("¡ALERTA! ¡Muévete para escapar!");
        }
    }

    // 8. Patrullar
    void patrullar(float deltaTime) {
        static float tiempoCambio = 0;
        static float direccionX = 0, direccionY = 0;
        
        tiempoCambio += deltaTime;
        
        if (tiempoCambio >= 2.0f) {
            direccionX = (std::rand() % 3 - 1);
            direccionY = (std::rand() % 3 - 1);
            tiempoCambio = 0;
        }
        
        x += direccionX * velocidad * 0.4f * deltaTime;
        y += direccionY * velocidad * 0.4f * deltaTime;
        
        sprite.cambiarEstado(EstadoAnimacion::QUIETO);
    }
};

// ==================== CLASE PRINCIPAL DEL JUEGO ====================
class JuegoTerror {
private:
    sf::RenderWindow ventana;
    Jugador* jugador;
    IA* entidadTerrorifica;
    SistemaAlerta sistemaAlerta;
    SistemaEscape sistemaEscape;
    GUI gui;

public:
    JuegoTerror() : ventana(sf::VideoMode(800, 600), "Sistema Unificado de Terror") {
        ventana.setFramerateLimit(60);
        
        jugador = new Jugador(400, 300, &sistemaAlerta, &sistemaEscape);
        entidadTerrorifica = new IA(100, 100, jugador, &sistemaAlerta, &sistemaEscape);
    }

    ~JuegoTerror() {
        delete jugador;
        delete entidadTerrorifica;
    }

    void ejecutar() {
        sf::Clock reloj;
        
        while (ventana.isOpen()) {
            float deltaTime = reloj.restart().asSeconds();
            
            procesarEventos();
            actualizar(deltaTime);
            renderizar();
        }
    }

private:
    void procesarEventos() {
        sf::Event evento;
        while (ventana.pollEvent(evento)) {
            if (evento.type == sf::Event::Closed) {
                ventana.close();
            }
            
            if (evento.type == sf::Event::KeyPressed) {
                jugador->manejarInput(evento.key.code, 0.016f);
            }
        }
    }

    void actualizar(float deltaTime) {
        // Actualizar sistemas
        sistemaEscape.actualizar(deltaTime);
        sistemaAlerta.actualizar(deltaTime);
        
        // Controlar movilidad del jugador según fase de escape
        if (sistemaEscape.isAcechoActivo()) {
            jugador->forzarInmovilidad();
        } else if (sistemaEscape.isVentanaEscapeActiva()) {
            // El jugador ya puede moverse normalmente
        } else if (sistemaEscape.isFallo()) {
            jugador->forzarInmovilidad();
        } else if (sistemaEscape.isEscapeExitoso()) {
            sistemaEscape.reiniciar();
        }
        
        // Actualizar entidades
        jugador->actualizar(deltaTime);
        entidadTerrorifica->actualizar(deltaTime);
        
        // Actualizar GUI
        gui.actualizarGUI(jugador->getBarraVida(), jugador->getInventario(), 
                         jugador->getEstadoString(), sistemaAlerta, sistemaEscape);
    }

    void renderizar() {
        ventana.clear(sf::Color(20, 20, 40));
        
        // Dibujar entidades
        ventana.draw(entidadTerrorifica->getSprite().getSpriteSFML());
        ventana.draw(jugador->getSprite().getSpriteSFML());
        
        // Dibujar GUI
        gui.dibujar(ventana);
        
        ventana.display();
    }
};

// ==================== FUNCIÓN MAIN ====================
int main() {
    JuegoTerror juego;
    juego.ejecutar();
    return 0;
}