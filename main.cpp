#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <map>
#include "brenett.h"
#include "adiel.h"
#include "israel_collisions.hpp"
#include "Art.h"
#include <iostream>
#if defined(__has_include)
    #if __has_include(<filesystem>)
        #include <filesystem>
        #if defined(__cpp_lib_filesystem)
            namespace fs = std::filesystem;
        #else
            // header present but library not available; try experimental
            #if __has_include(<experimental/filesystem>)
                #include <experimental/filesystem>
                namespace fs = std::experimental::filesystem;
            #else
                #define NO_FILESYSTEM
            #endif
        #endif
    #elif __has_include(<experimental/filesystem>)
        #include <experimental/filesystem>
        namespace fs = std::experimental::filesystem;
    #else
        #define NO_FILESYSTEM
    #endif
#else
    #define NO_FILESYSTEM
#endif

// Prototipo de la función para evitar error de declaración
void generarMueblesConColision(
    const sf::FloatRect& mapBounds,
    std::vector<sf::Texture>& mueblesTextures,
    std::vector<sf::Sprite>& mueblesSprites,
    std::vector<oc::Collider>& mueblesColliders,
    std::vector<sf::RectangleShape>& obstaclesShapes,
    std::vector<sf::FloatRect>& obstaclesRects,
    int cantidad
);



int main() {

    sf::RenderWindow window(sf::VideoMode(1280, 720), "Welcome to TERRIFIER");
    window.setFramerateLimit(60);

    sf::RenderTexture lightMask;
    if (!lightMask.create(window.getSize().x, window.getSize().y)) {
        std::cerr << "Error creando render texture para la luz" << std::endl;
    }
    lightMask.setSmooth(true);

    // --- MENÚ PRINCIPAL ---
    sf::Font menuFont;
    if (!menuFont.loadFromFile("assets/fonts/karmatic_arcade.ttf")) {
        // fallback: intentar otra fuente
        menuFont.loadFromFile("assets/fonts/fs-pixel-sans-unicode-regular.ttf");
    }
    // --- Audio: música de inicio y fondo, y sonido de persecución ---
    sf::Music menuMusic;
    if (!menuMusic.openFromFile("assets/audio/inicio.mp3")) {
        std::cerr << "No se pudo cargar assets/audio/inicio.mp3" << std::endl;
    } else {
        menuMusic.setLoop(true);
        menuMusic.play();
    }

    sf::Music bgMusic;
    if (!bgMusic.openFromFile("assets/audio/fondo.mp3")) {
        std::cerr << "No se pudo cargar assets/audio/fondo.mp3" << std::endl;
    } else {
        bgMusic.setLoop(true);
        // no reproducir aún; se iniciará al entrar al juego
    }

    sf::SoundBuffer chaseBuf;
    bool chaseLoaded = false;
    if (chaseBuf.loadFromFile("assets/audio/persecucion_art.mp3")) {
        chaseLoaded = true;
    } else {
        std::cerr << "No se pudo cargar assets/audio/persecucion_art.mp3" << std::endl;
    }
    sf::Sound chaseSound;
    if (chaseLoaded) {
        chaseSound.setBuffer(chaseBuf);
        chaseSound.setLoop(true);
        chaseSound.setVolume(0.f);
    }

    // Sonido adicional para persecución/alerta de Art (archivo tercer sonido)
    sf::SoundBuffer persecucionArtBuf;
    bool persecucionArtLoaded = false;
    sf::Sound persecucionArtSound;
    if (persecucionArtLoaded) {
        persecucionArtSound.setBuffer(persecucionArtBuf);
        // Este sonido será controlado por distancia: loop y volumen inicial 0
        persecucionArtSound.setLoop(true);
        persecucionArtSound.setVolume(0.f);
    }

    bool inMenu = true;
start_menu:
    while (inMenu) {
        sf::Event me;
        bool clickInMenu = false;
        sf::Vector2f clickPosMenu(0.f,0.f);
        while (window.pollEvent(me)) {
            if (me.type == sf::Event::Closed) return 0;
            if (me.type == sf::Event::MouseButtonPressed && me.mouseButton.button == sf::Mouse::Left) {
                clickInMenu = true;
                clickPosMenu = window.mapPixelToCoords({me.mouseButton.x, me.mouseButton.y});
            }
        }

        window.clear(sf::Color(20,20,30));

        // Título
        sf::Text title("Welcome to TERRIFIER", menuFont, 48);
        title.setFillColor(sf::Color::White);
        title.setStyle(sf::Text::Bold);
        title.setPosition((1280 - title.getLocalBounds().width) / 2.f, 140.f);
        window.draw(title);

        // Botones
        sf::Vector2f btnSize(300.f, 70.f);
        sf::RectangleShape playBtn(btnSize);
        playBtn.setOrigin(btnSize.x/2, btnSize.y/2);
        playBtn.setPosition(1280/2.f, 300.f);
        playBtn.setFillColor(sf::Color(80,80,140));

        sf::RectangleShape exitBtn(btnSize);
        exitBtn.setOrigin(btnSize.x/2, btnSize.y/2);
        exitBtn.setPosition(1280/2.f, 400.f);
        exitBtn.setFillColor(sf::Color(140,80,80));

        // Textos de botones
            sf::Text playText("Jugar", menuFont, 30);
        playText.setFillColor(sf::Color::White);
        playText.setPosition(playBtn.getPosition().x - playText.getLocalBounds().width/2.f - 5.f,
                             playBtn.getPosition().y - playText.getLocalBounds().height/2.f - 10.f);

        sf::Text exitText("Salir", menuFont, 30);
        exitText.setFillColor(sf::Color::White);
        exitText.setPosition(exitBtn.getPosition().x - exitText.getLocalBounds().width/2.f - 5.f,
                             exitBtn.getPosition().y - exitText.getLocalBounds().height/2.f - 10.f);

        // Hover effect
        sf::Vector2i mp = sf::Mouse::getPosition(window);
        sf::Vector2f mposf = window.mapPixelToCoords(mp);
        if (playBtn.getGlobalBounds().contains(mposf)) playBtn.setFillColor(sf::Color(120,120,200));
        if (exitBtn.getGlobalBounds().contains(mposf)) exitBtn.setFillColor(sf::Color(200,120,120));

        // Click handling (evento)
        if (clickInMenu) {
            if (playBtn.getGlobalBounds().contains(clickPosMenu)) {
                inMenu = false; // iniciar juego
            }
            if (exitBtn.getGlobalBounds().contains(clickPosMenu)) {
                return 0; // salir
            }
        }

        window.draw(playBtn);
        window.draw(exitBtn);
        window.draw(playText);
        window.draw(exitText);

        window.display();
    }

    // Al salir del menú: detener música de inicio y reproducir música de fondo
    if (menuMusic.getStatus() == sf::Music::Playing) menuMusic.stop();
    if (bgMusic.getStatus() != sf::Music::Playing) bgMusic.play();

    // Mapa
    sf::Texture mapyay; 
    mapyay.loadFromFile("assets/sprites/map.png");
    sf::Sprite map;
    map.setTexture(mapyay);
    // Aplicar un tinte leve frío para sensación nocturna
    map.setColor(sf::Color(180, 185, 220));
    map.setPosition(300, 200);
    //sf::FloatRect mapBound = sf::FloatRect(0, 0, mapyay.getSize().x, mapyay.getSize().y); se repite abajo OOPS!!!

    // TEXTURAS DEL PERSONAJE
    sf::Texture texDown, texLeft, texRight, texUp;
    texDown.loadFromFile("assets/sprites/sienna_sprites/IdleDown.png");
    texLeft.loadFromFile("assets/sprites/sienna_sprites/IdleLeft.png");
    texRight.loadFromFile("assets/sprites/sienna_sprites/IdleRight.png");
    texUp.loadFromFile("assets/sprites/sienna_sprites/IdleUp.png");

    //OBJETOS: crear spawner dentro de los límites del mapa (con margen)
    sf::FloatRect mapBounds = map.getGlobalBounds();
    const int margin = 25; // margen desde bordes del mapa (reducido para spawn un poco más grande)
    int minX = static_cast<int>(mapBounds.left) + margin;
    int maxX = static_cast<int>(mapBounds.left + mapBounds.width) - margin;
    int minY = static_cast<int>(mapBounds.top) + margin;
    int maxY = static_cast<int>(mapBounds.top + mapBounds.height) - margin;
    ObjectSpawner spawner(minX, maxX, minY, maxY);
    auto objetos = spawner.generarObjetos();
    std::map<std::string, sf::Texture> texturas;

    auto loadOrPlaceholder = [&](const std::string &path)->sf::Texture {
        sf::Texture t;
        if (t.loadFromFile(path)) return t;
        // fallback: textura transparente (evita el cuadrado magenta)
        sf::Image img;
        img.create(32,32,sf::Color(0,0,0,0));
        t.loadFromImage(img);
        return t;
    };

    texturas["LlaveA"] = loadOrPlaceholder("assets/sprites/items_sprites/llave_mansion.png");
    texturas["LlaveB"] = loadOrPlaceholder("assets/sprites/items_sprites/llave_cristal.png");
    texturas["LlaveC"] = loadOrPlaceholder("assets/sprites/items_sprites/llave.png");
    texturas["Palanca"] = loadOrPlaceholder("assets/sprites/items_sprites/palanca.png");

    for(int i=1;i<=10;i++){
        std::string key = "Objeto" + std::to_string(i);
        std::string path = "assets/sprites/items_sprites/obj" + std::to_string(i) + ".png";
        texturas[key] = loadOrPlaceholder(path);
    }

    //---------------------------
    // ASOCIAR SPRITES
    //---------------------------
    for(auto& o : objetos){
        o.sprite.setTexture(texturas[o.nombre]);
        o.sprite.setPosition(o.x, o.y);
    }

    // Contar cuántos objetos son "no llave" y "no palanca" para el mensaje
    int totalNonKeyObjects = 0;
    for (auto &o : objetos) {
        if (o.nombre.find("Llave") == std::string::npos && o.nombre != "Palanca") totalNonKeyObjects++;
    }
    int collectedNonKeyObjects = 0;
    std::string pickupMessage = "";
    float pickupMessageTimer = 0.f;
    const float pickupMessageDuration = 2.0f;

    // Lambda para regenerar objetos en reinicio
    auto regenerarObjetos = [&]() {
        objetos = spawner.generarObjetos();
        for (auto &o : objetos) {
            o.sprite.setTexture(texturas[o.nombre]);
            o.sprite.setPosition(o.x, o.y);
        }
        // Recalcular totalNonKeyObjects
        totalNonKeyObjects = 0;
        for (auto &o : objetos) {
            if (o.nombre.find("Llave") == std::string::npos && o.nombre != "Palanca") totalNonKeyObjects++;
        }
        collectedNonKeyObjects = 0;
    };

    std::vector<sf::Texture>walkDown(4);
    walkDown[0].loadFromFile("assets/sprites/sienna_sprites/WalkDown1.png");
    walkDown[1].loadFromFile("assets/sprites/sienna_sprites/WalkDown2.png");
    walkDown[2].loadFromFile("assets/sprites/sienna_sprites/WalkDown3.png");
    walkDown[3].loadFromFile("assets/sprites/sienna_sprites/WalkDown4.png");

    std::vector<sf::Texture>walkLeft(4);
    walkLeft[0].loadFromFile("assets/sprites/sienna_sprites/WalkLeft1.png");
    walkLeft[1].loadFromFile("assets/sprites/sienna_sprites/WalkLeft2.png");
    walkLeft[2].loadFromFile("assets/sprites/sienna_sprites/WalkLeft3.png");
    walkLeft[3].loadFromFile("assets/sprites/sienna_sprites/WalkLeft4.png");

    std::vector<sf::Texture>walkRight(4);
    walkRight[0].loadFromFile("assets/sprites/sienna_sprites/WalkRight1.png");
    walkRight[1].loadFromFile("assets/sprites/sienna_sprites/WalkRight2.png");
    walkRight[2].loadFromFile("assets/sprites/sienna_sprites/WalkRight3.png");
    walkRight[3].loadFromFile("assets/sprites/sienna_sprites/WalkRight4.png");

    std::vector<sf::Texture>walkUp(4);
    walkUp[0].loadFromFile("assets/sprites/sienna_sprites/WalkUp1.png");
    walkUp[1].loadFromFile("assets/sprites/sienna_sprites/WalkUp2.png");
    walkUp[2].loadFromFile("assets/sprites/sienna_sprites/WalkUp3.png");
    walkUp[3].loadFromFile("assets/sprites/sienna_sprites/WalkUp4.png");

    sf::Sprite player;
    player.setTexture(texDown);
    player.setPosition(400.f, 1450.f);

    int currentFrame = 0;
    float frameTime = 0.15f;  // cambio cada 150ms
    float timer = 0.f;

    sf::FloatRect pb = player.getLocalBounds();
    player.setOrigin(pb.width / 2.f, pb.height / 2.f);

    // Estado para recoger objetos con Q
    bool qPrev = false;
    const float pickupRadius = 48.f;
    // Estado para la mecánica de escapar/seguir a Art
    bool escaping = false;
    float escapeTimer = 0.f;
    float escapeDuration = 0.f;
    bool warningShown = false;
    bool canFlee = false;
    float warningTimer = 0.f;
    const float warningWindow = 3.0f; // tiempo para huir tras aparecer la advertencia
    bool gameOver = false;
    bool victory = false; // estado de victoria
    const float activationRadius = 150.f; // radio para iniciar la secuencia
    const float followRadius = 180.f; // radio en el que el jugador debe permanecer cerca de Art (mayor que activationRadius)
    float proximityLostTimer = 0.f;
    const float proximityLostDelay = 2.0f; // segundos que puede salirse antes de perder (grace)
    const float instaKillRadius = 40.f; // si el jugador se acerca a esta distancia, comienza el temporizador
    float instaKillTimer = 0.f;
    const float instaKillDelay = 1.5f; // segundos que debe permanecer dentro para morir
    // Temporizador para evitar reacciones repetidas al quedar atrapado
    float artStuckTimer = 0.f;
    
    // Zona objetivo (convertida a coordenadas de mundo: sumar 300,200 del offset del mapa)
    const float goalMinX = 1110.f + 300.f; // 1410
    const float goalMaxX = 1171.f + 300.f; // 1471
    const float goalMinY = 2238.f + 200.f; // 2438
    const float goalMaxY = 2277.f + 200.f; // 2477
    const int objectivesRequired = 10;

    // Sistema de pausa e inventario
    bool isPaused = false;
    bool pPrev = false;
    bool iPrev = false;
    bool showInventory = false;
    int keyCount = 0;
    bool hasPalanca = false;

    //---------------------------------------------
    //                 IA ART
    //---------------------------------------------
    Art art(650.f, 850.f);  // Posición inicial de Art (150+500,150+700)
    
    art.cargarSprites(
        // Sprites QUIETOS (1 por dirección)
        "assets/sprites/Art_Sprites/idleDown.png",
        "assets/sprites/Art_Sprites/idleUp.png",
        "assets/sprites/Art_Sprites/idleLeft.png",
        "assets/sprites/Art_Sprites/idleRight.png",
        
        // Animación CAMINAR ABAJO (4 frames)
        {
            "assets/sprites/Art_Sprites/WalkDown1.png",
            "assets/sprites/Art_Sprites/WalkDown2.png",
            "assets/sprites/Art_Sprites/WalkDown3.png",
            "assets/sprites/Art_Sprites/WalkDown4.png"
        },
        
        // Animación CAMINAR ARRIBA (4 frames)
        {
            "assets/sprites/Art_Sprites/WalkUp1.png",
            "assets/sprites/Art_Sprites/WalkUp2.png",
            "assets/sprites/Art_Sprites/WalkUp3.png",
            "assets/sprites/Art_Sprites/WalkUp4.png"
        },
        
        // Animación CAMINAR IZQUIERDA (4 frames)
        {
            "assets/sprites/Art_Sprites/WalkLeft1.png",
            "assets/sprites/Art_Sprites/WalkLeft2.png",
            "assets/sprites/Art_Sprites/WalkLeft3.png",
            "assets/sprites/Art_Sprites/WalkLeft4.png"
        },
        
        // Animación CAMINAR DERECHA (4 frames)
        {
            "assets/sprites/Art_Sprites/WalkRight1.png",
            "assets/sprites/Art_Sprites/WalkRight2.png",
            "assets/sprites/Art_Sprites/WalkRight3.png",
            "assets/sprites/Art_Sprites/WalkRight4.png"
        }
    );

    // Configuración de comportamiento: si `artIsHostile` es false, Art NO perseguirá al jugador
    // sino que vagará por todo el mapa como un NPC (waypoints aleatorios).
    bool artIsHostile = false; // cambiar a true para restaurar persecución hacia el jugador

    if (artIsHostile) {
        art.setVelocidad(120.0f);  // Un poco más lento que el jugador
        art.setRangoDeteccion(400.0f);  // Rango de detección amplio
        art.setDistanciaUmbral(25.0f);
        art.setPatrullaCiclica(false);
        art.setMemoriaTiempo(7.0f); 

        if (!art.cargarWaypointsDesdeArchivo("assets/waypoints_art.txt")) {
            // Si falla, crear waypoints manualmente
            art.agregarWaypoint(300, 300, 1.5);
            art.agregarWaypoint(800, 300, 1.0);
            art.agregarWaypoint(800, 800, 2.0);
            art.agregarWaypoint(300, 800, 1.5);
            art.agregarWaypoint(550, 550, 2.5);
        }
    } else {
        // Modo NPC: desactivar detección y generar waypoints aleatorios por todo el mapa
        art.setVelocidad(80.0f);
        art.setRangoDeteccion(0.f); // nunca entra en estado PERSEGUIR
        art.setDistanciaUmbral(20.0f);
        art.setPatrullaCiclica(true);

        // Generar waypoints aleatorios dentro de los límites del mapa
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        art.limpiarWaypoints();
        int numWaypoints = 8;
        int rangeX = maxX - minX;
        int rangeY = maxY - minY;
        if (rangeX <= 0) rangeX = 1;
        if (rangeY <= 0) rangeY = 1;
        for (int i = 0; i < numWaypoints; ++i) {
            int rx = minX + (std::rand() % rangeX);
            int ry = minY + (std::rand() % rangeY);
            art.agregarWaypoint(static_cast<float>(rx), static_cast<float>(ry), 1.0f, "");
        }
    }

    // Cargar sprite de advertencia (esquina inferior izquierda)
    sf::Texture warnTex;
    bool warnLoaded = false;
    if (warnTex.loadFromFile("assets/sprites/advertencia.png")) {
        warnLoaded = true;
    } else {
        std::cerr << "No se pudo cargar assets/sprites/advertencia.png" << std::endl;
    }
    sf::Sprite warnSprite;
    if (warnLoaded) {
        warnSprite.setTexture(warnTex);
        // lo dibujaremos en la vista de pantalla (HUD), en la esquina inferior izquierda
        warnSprite.setScale(0.6f, 0.6f);
    }
    // fallback: crear forma y texto por si no existe la imagen
    sf::RectangleShape warnFallbackRect;
    sf::Text warnFallbackText;
    if (!warnLoaded) {
        // fallback más grande y colorido para asegurar visibilidad
        warnFallbackRect.setSize({260.f, 120.f});
        warnFallbackRect.setFillColor(sf::Color(255, 80, 20, 230));
        warnFallbackRect.setOutlineColor(sf::Color(255, 220, 50));
        warnFallbackRect.setOutlineThickness(6.f);
        // usar la fuente del menú si está disponible
        warnFallbackText.setFont(menuFont);
        warnFallbackText.setString("! ADVERTENCIA !");
        warnFallbackText.setCharacterSize(28);
        warnFallbackText.setFillColor(sf::Color::White);
        warnFallbackText.setStyle(sf::Text::Bold);
    }

    //---------------------------------------------
    //                 SHADER DE LUZ
    //---------------------------------------------
    sf::Shader flashlight;
    flashlight.loadFromFile("assets/shaders/light.frag", sf::Shader::Fragment);

    float lightRadius = 1280.f;   // RADIO DE LUZ
    float innerRadius = 50.f;    // TOLERANCIA (zona sin sombra alrededor del jugador)
    sf::Vector2f lightDir(0.f, 0.f); // Dirección (cambia con WASD)

    //---------------------------------------------
    //                  VIEW (camara)
    //---------------------------------------------
    sf::View view = window.getDefaultView();
    view.zoom(0.55f);
    view.setCenter(player.getPosition());

    // Movement parameters (speed in pixels/second)
    const float maxSpeed = 200.f;    // velocidad máxima
    const float accel = 800.f;       // aceleración (pixels/s^2)
    const float decel = 1000.f;      // desaceleración
    sf::Vector2f velocity(0.f, 0.f);

    sf::Clock clock;

    std::vector<sf::RectangleShape> obstaclesShapes;
    std::vector<sf::FloatRect> obstaclesRects;
    
    oc::Collider col;
    col.setSize({20.f, 40.f});
    col.setPosition(player.getPosition());
    col.shape.setFillColor(sf::Color(0, 0, 0, 0));

    // Colisionador para Art
    oc::Collider artCol;
    artCol.setSize({20.f, 40.f});
    artCol.setPosition(art.getSprite().getPosition());
    artCol.shape.setFillColor(sf::Color(0, 0, 0, 0));

    oc::loadObstaclesFromFile("assets/obstacles.txt", obstaclesShapes, obstaclesRects, true);

    // 1. DECLARA LOS VECTORES DE MUEBLES (después de obstaclesRects, antes del bucle principal)
    std::vector<sf::Texture> mueblesTextures;
    std::vector<sf::Sprite> mueblesSprites;
    std::vector<oc::Collider> mueblesColliders;

    // 2. LLAMA A LA FUNCIÓN PARA GENERAR MUEBLES (después de crear el mapa y mapBounds)
    generarMueblesConColision(
        mapBounds,
        mueblesTextures,
        mueblesSprites,
        mueblesColliders,
        obstaclesShapes,
        obstaclesRects,
        75 // cantidad de muebles, ajusta a tu gusto
    );

    while (window.isOpen()) {
        sf::Event event;
        float dt = clock.restart().asSeconds();
        // Decrementar cooldown de stuck de Art
        if (artStuckTimer > 0.f) {
            artStuckTimer -= dt;
            if (artStuckTimer < 0.f) artStuckTimer = 0.f;
        }

        // manejar eventos (un único loop). También capturamos clicks de ratón para uso en UI

        sf::Vector2f movement(0.f, 0.f);
        bool moving = false;
        enum Direction {UP, DOWN, LEFT, RIGHT};
        Direction dir = DOWN;
        bool clickPressed = false;
        sf::Vector2f clickPos(0.f, 0.f);
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                clickPressed = true;
                clickPos = window.mapPixelToCoords({event.mouseButton.x, event.mouseButton.y});
            }
        }
        
        

        // --- Entrada que SIEMPRE se procesa (Q para recoger, P para pausa) ---
        bool qNow = sf::Keyboard::isKeyPressed(sf::Keyboard::Q);
        if (qNow && !qPrev) {
            sf::Vector2f ppos = player.getPosition();
            int bestIdx = -1;
            float bestDistSq = pickupRadius * pickupRadius;
            sf::Vector2f objetoPosRecogido(0, 0);

            for (int i = 0; i < (int)objetos.size(); ++i) {
                sf::Vector2f op = objetos[i].sprite.getPosition();
                float dx = op.x - ppos.x;
                float dy = op.y - ppos.y;
                float d2 = dx*dx + dy*dy;
                if (d2 < bestDistSq) {
                    bestDistSq = d2;
                    bestIdx = i;
                    objetoPosRecogido = op;
                }
            }
            if (bestIdx != -1) {
                std::string recogido = objetos[bestIdx].nombre;
                std::cout << "Recogido: " << recogido << "\n";

                if (recogido.find("Llave") == std::string::npos && recogido != "Palanca") {
                art.marcarRecoleccionObjeto(objetoPosRecogido.x, objetoPosRecogido.y);
                }
                // Contar llaves y palanca. Si no es llave ni palanca, aumentar contador especial
                if (recogido.find("Llave") != std::string::npos) {
                    keyCount++;
                } else if (recogido == "Palanca") {
                    hasPalanca = true;
                } else {
                    // Objeto genérico recogido
                        collectedNonKeyObjects++;
                        pickupMessage = std::to_string(collectedNonKeyObjects) + " de 10";
                    pickupMessageTimer = pickupMessageDuration;
                    // No reproducir aquí: el sonido 'persecucion_art' se controla por distancia
                }
                objetos.erase(objetos.begin() + bestIdx);
            }
        }
        qPrev = qNow;

        // Iniciar secuencia de escape automáticamente cuando el jugador esté lo suficientemente cerca de Art
        if (!escaping && !gameOver) {
            sf::Vector2f ppos = player.getPosition();
            sf::Vector2f artPos = art.getSprite().getPosition();
            float dx = artPos.x - ppos.x; float dy = artPos.y - ppos.y;
            float d = std::sqrt(dx*dx + dy*dy);
            if (d <= activationRadius) {
                escaping = true;
                escapeTimer = 0.f;
                // duración aleatoria entre 2.0 y 5.0 segundos
                escapeDuration = 2.0f + static_cast<float>(std::rand()) / (static_cast<float>(RAND_MAX) / 3.0f);
                warningShown = false;
                canFlee = false;
                warningTimer = 0.f;
            }
        }

        // --- Pausa con tecla P ---
        bool pNow = sf::Keyboard::isKeyPressed(sf::Keyboard::P);
        if (pNow && !pPrev) {
            isPaused = !isPaused;
        }
        pPrev = pNow;

        // --- Mostrar Inventario con tecla I ---
        bool iNow = sf::Keyboard::isKeyPressed(sf::Keyboard::I);
        if (iNow && !iPrev) {
            showInventory = !showInventory;
        }
        iPrev = iNow;

        // Actualizar temporizador del mensaje de pickup
        if (pickupMessageTimer > 0.f) {
            pickupMessageTimer -= dt;
            if (pickupMessageTimer < 0.f) pickupMessageTimer = 0.f;
        }

        // Verificar condición de victoria: jugador en zona objetivo con 10 objetos recogidos
        if (!gameOver && !victory) {
            sf::Vector2f ppos = player.getPosition();
            if (ppos.x >= goalMinX && ppos.x <= goalMaxX && ppos.y >= goalMinY && ppos.y <= goalMaxY) {
                if (collectedNonKeyObjects >= objectivesRequired) {
                    victory = true;
                }
            }
        }

        // Si NO está en pausa, ejecutar lógica de actualización
        if (!isPaused) {
            //---------------------------------------------
            //           MOVIMIENTO + DIRECCION
            //---------------------------------------------
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
                movement.y -= 1.f;
                dir = UP;
                moving = true;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
                movement.y += 1.f;
                dir = DOWN;
                moving = true;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
                movement.x -= 1.f;
                dir = LEFT;
                moving = true;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
                movement.x += 1.f;
                dir = RIGHT;
                moving = true;
            }

            if (moving) {
                timer += dt;
                if (timer >= frameTime) {
                    timer = 0;
                    currentFrame = (currentFrame + 1) % walkDown.size(); // número de frames
                }

                // Cambiar textura según dirección
                switch (dir) {
                    case DOWN:  player.setTexture(walkDown[currentFrame]); break;
                    case UP:    player.setTexture(walkUp[currentFrame]);   break;
                    case LEFT:  player.setTexture(walkLeft[currentFrame]); break;
                    case RIGHT: player.setTexture(walkRight[currentFrame]);break;
                }
            } 
            else {
                // Quieto
                switch (dir) {
                    case DOWN:  player.setTexture(texDown); break;
                    case UP:    player.setTexture(texUp); break;
                    case LEFT:  player.setTexture(texLeft); break;
                    case RIGHT: player.setTexture(texRight); break;
                }
            }

            // Normalizar input para evitar velocidad mayor en diagonal
            float inputLen = std::sqrt(movement.x*movement.x + movement.y*movement.y);
            sf::Vector2f inputDir(0.f,0.f);
            if (inputLen > 0.001f) inputDir = sf::Vector2f(movement.x/inputLen, movement.y/inputLen);

            // Objetivo de velocidad
            sf::Vector2f targetVel = inputDir * (moving ? maxSpeed : 0.f);
            col.velocity = velocity * dt;

            // Interpolar velocity hacia targetVel por componentes
            // X
            if (velocity.x < targetVel.x) {
                velocity.x += accel;
                if (velocity.x > targetVel.x){ velocity.x = targetVel.x;}
            } else if (velocity.x > targetVel.x) {
                velocity.x -= decel;
                if (velocity.x < targetVel.x){ velocity.x = targetVel.x;}
            }
            // Y
            if (velocity.y < targetVel.y) {
                velocity.y += accel;
                if (velocity.y > targetVel.y){velocity.y = targetVel.y;} 
            } else if (velocity.y > targetVel.y) {
                velocity.y -= decel;
                if (velocity.y < targetVel.y){ velocity.y = targetVel.y;}
            }

            // Movimiento final (aplica dt)
            sf::Vector2f finalMovement = velocity * dt;
            //player.move(finalMovement);

            col.setPosition(player.getPosition());
            col.velocity = velocity;
            oc::resolveCollisionsAxisSeparated(col, obstaclesRects, dt);
            player.setPosition(col.getPosition());
            
            // ============================================
            // RASTREO  DE MOVIMIENTO DEL JUGADOR
            // ============================================
            static float tiempoUltimoRastro = 0.0f;
            tiempoUltimoRastro += dt;

            // Cada 1.5 segundos, marcar posición si el jugador se mueve
            if (tiempoUltimoRastro > 1.5f && moving && !escaping && !gameOver && !victory) {
                art.marcarMovimientoJugador(player.getPosition().x, player.getPosition().y);
                tiempoUltimoRastro = 0.0f;
            }

            //---------------------------------------------
            //          ACTUALIZAR IA ART
            //---------------------------------------------
            // Guardar posición anterior de Art
            sf::Vector2f artPosBefore = art.getSprite().getPosition();

            // Actualizar la IA (esto actualizará la posición interna del sprite)
            art.actualizar(dt, player.getPosition().x, player.getPosition().y);

            // Calcular movimiento (delta) producido por la IA
            sf::Vector2f artPosAfter = art.getSprite().getPosition();
            sf::Vector2f artMovement = artPosAfter - artPosBefore;

            // Aplicar colisiones a Art para que respete los obstáculos usando el collider
            // Colocamos el collider en la posición anterior y le damos la velocidad calculada
            artCol.setPosition(artPosBefore);
            if (dt > 0.f) artCol.velocity = artMovement / dt; else artCol.velocity = {0.f, 0.f};
            oc::resolveCollisionsAxisSeparated(artCol, obstaclesRects, dt);
            // Si la posición final del collider difiere de la posición esperada, hubo colisión
            sf::Vector2f artFinalPos = artCol.getPosition();
            float dxCol = artFinalPos.x - artPosAfter.x;
            float dyCol = artFinalPos.y - artPosAfter.y;
            float colDist = std::sqrt(dxCol*dxCol + dyCol*dyCol);
            const float COLLISION_EPS = 0.5f;
            if (colDist > COLLISION_EPS) {
                // Hubo corrección por colisión
                const float ART_STUCK_COOLDOWN = 0.8f;
                const float ART_NUDGE = 24.f;
                if (artStuckTimer <= 0.f) {
                    // Nudge: retroceder ligeramente en la dirección opuesta al movimiento
                    sf::Vector2f mv = artMovement;
                    float mvlen = std::sqrt(mv.x*mv.x + mv.y*mv.y);
                    sf::Vector2f mvNorm;
                    if (mvlen > 0.0001f) mvNorm = { mv.x / mvlen, mv.y / mvlen };
                    else {
                        // si no hay movimiento, elegir dirección aleatoria
                        float ang = (static_cast<float>(std::rand()) / RAND_MAX) * 2.f * 3.14159265f;
                        mvNorm = { std::cos(ang), std::sin(ang) };
                    }
                    sf::Vector2f nudgedPos = artFinalPos - mvNorm * ART_NUDGE;
                    artCol.setPosition(nudgedPos);
                    // Asegurar que la nueva posición no quede dentro de un obstáculo
                    oc::resolveCollisionsAxisSeparated(artCol, obstaclesRects, dt);
                    sf::Vector2f finalAfterNudge = artCol.getPosition();
                    art.setPosicion(finalAfterNudge.x, finalAfterNudge.y);

                    // Elegir un waypoint lejano aleatorio para evitar volver a la misma habitación
                    art.invertirDireccion();
                    artStuckTimer = ART_STUCK_COOLDOWN;
                } else {
                    // Si estamos en cooldown, simplemente aplicar la posición final
                    art.setPosicion(artFinalPos.x, artFinalPos.y);
                }
            }
            // Actualizar la posición real de Art con la resultante del collider
            art.setPosicion(artFinalPos.x, artFinalPos.y);

            // Proximidad inmediata: iniciar temporizador de insta-kill en lugar de matar al instante
            {
                sf::Vector2f artPosIK = art.getSprite().getPosition();
                sf::Vector2f pPosIK = player.getPosition();
                float dxIK = artPosIK.x - pPosIK.x; float dyIK = artPosIK.y - pPosIK.y;
                float distIK = std::sqrt(dxIK*dxIK + dyIK*dyIK);
                if (!warningShown) {
                    if (distIK <= instaKillRadius) {
                        instaKillTimer += dt;
                        // dar una pequeña ventana para que el jugador se aleje
                        if (instaKillTimer >= instaKillDelay) {
                            gameOver = true;
                        }
                    } else {
                        // si se aleja, resetear timer
                        instaKillTimer = 0.f;
                    }
                } else {
                    // si la advertencia ya apareció, no aplicar insta-kill
                    instaKillTimer = 0.f;
                }
            }

            // Lógica de la secuencia de escape:
            if (escaping && !warningShown && !gameOver) {
                sf::Vector2f artPos = art.getSprite().getPosition();
                sf::Vector2f pPos = player.getPosition();
                float dx = artPos.x - pPos.x; float dy = artPos.y - pPos.y;
                float dist = std::sqrt(dx*dx + dy*dy);
                // debe mantenerse cerca de Art
                if (dist <= followRadius) {
                    escapeTimer += dt;
                    // resetear timer de perdida de proximidad si vuelve a estar dentro
                    proximityLostTimer = 0.f;
                    if (escapeTimer >= escapeDuration) {
                        // Mostrar advertencia y permitir huir por un tiempo limitado
                        warningShown = true;
                        canFlee = true;
                        warningTimer = warningWindow;
                    }
                } else {
                    // Si se aleja antes de que aparezca la advertencia, pierde inmediatamente
                    gameOver = true;
                }
            }

            // Si ya apareció la advertencia, contamos su ventana
            if (warningShown && canFlee && !gameOver) {
                warningTimer -= dt;
                if (warningTimer <= 0.f) {
                    // No huyó a tiempo -> Game Over
                    gameOver = true;
                }
                // Si presiona SPACE mientras puede huir, se libera
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
                    if (canFlee) {
                        escaping = false;
                        warningShown = false;
                        canFlee = false;
                        warningTimer = 0.f;
                        // éxito: permitir al jugador escapar (se puede añadir lógica extra)
                    }
                }
            }

            // --- Actualizar volumen del sonido de persecución según distancia ---
            if (chaseLoaded) {
                sf::Vector2f artPos = art.getSprite().getPosition();
                sf::Vector2f pPos = player.getPosition();
                float dx = artPos.x - pPos.x;
                float dy = artPos.y - pPos.y;
                float dist = std::sqrt(dx*dx + dy*dy);
                const float maxHearDist = 800.f; // distancia a la que ya no se oye
                float vol = 0.f;
                if (dist < maxHearDist) vol = 100.f * (1.0f - dist / maxHearDist);
                if (vol < 0.f) vol = 0.f; if (vol > 100.f) vol = 100.f;
                chaseSound.setVolume(vol);
                if (vol > 1.f) {
                    if (chaseSound.getStatus() != sf::Sound::Playing) chaseSound.play();
                } else {
                    if (chaseSound.getStatus() == sf::Sound::Playing) chaseSound.pause();
                }
            }

            // Actualizar volumen del tercer sonido (persecucion_art) según proximidad gradual
            if (persecucionArtLoaded) {
                sf::Vector2f artPos = art.getSprite().getPosition();
                sf::Vector2f pPos = player.getPosition();
                float dx2 = artPos.x - pPos.x;
                float dy2 = artPos.y - pPos.y;
                float dist2 = std::sqrt(dx2*dx2 + dy2*dy2);
                const float maxHearDistArt = 600.f; // distancia donde ya no se oye
                float targetVolArt = 0.f;
                if (dist2 < maxHearDistArt) targetVolArt = 100.f * (1.0f - dist2 / maxHearDistArt);
                if (targetVolArt < 0.f) targetVolArt = 0.f; if (targetVolArt > 100.f) targetVolArt = 100.f;
                // Suavizado simple para evitar saltos bruscos
                float curVol = persecucionArtSound.getVolume();
                float smooth = 0.08f; // factor de mezcla hacia target
                float newVol = curVol * (1.0f - smooth) + targetVolArt * smooth;
                persecucionArtSound.setVolume(newVol);
                if (newVol > 1.f) {
                    if (persecucionArtSound.getStatus() != sf::Sound::Playing) persecucionArtSound.play();
                } else {
                    if (persecucionArtSound.getStatus() == sf::Sound::Playing) persecucionArtSound.pause();
                }
            }

            //---------------------------------------------
            //          DIRECCION DE LUZ CON MOUSE
            //---------------------------------------------
            // Obtener posición del mouse en coordenadas del mundo
            sf::Vector2i mousePixelPos = sf::Mouse::getPosition(window);
            sf::Vector2f mouseWorldPos = window.mapPixelToCoords(mousePixelPos, view);
            
            // Calcular dirección desde el jugador hacia el mouse
            sf::Vector2f playerPos = player.getPosition();
            lightDir.x = mouseWorldPos.x - playerPos.x;
            lightDir.y = -(mouseWorldPos.y - playerPos.y);
            
            // Normalizar la dirección (IMPORTANTE para el shader)
            float length = std::sqrt(lightDir.x * lightDir.x + lightDir.y * lightDir.y);
            if (length != 0) {
                lightDir.x /= length;
                lightDir.y /= length;
            }

            //---------------------------------------------
            //                UPDATE CAMARA
            //---------------------------------------------
            view.setCenter(player.getPosition());
            limitCam(view,player,mapBounds);
            window.setView(view);

            //---------------------------------------------
            //             UPDATE SHADER PARAMS
            //---------------------------------------------
            sf::Vector2i playerScreenPos = window.mapCoordsToPixel(player.getPosition(), view);
            flashlight.setUniform("lightPos", sf::Vector2f(playerScreenPos.x, window.getSize().y - playerScreenPos.y));
            flashlight.setUniform("lightDir", lightDir);
            flashlight.setUniform("lightRadius", lightRadius);
            flashlight.setUniform("innerRadius", innerRadius);
            flashlight.setUniform("resolution", sf::Glsl::Vec2(window.getSize().x, window.getSize().y));

        } // Fin del bloque de no-pausa

        //---------------------------------------------
        //              DRAW
        //---------------------------------------------
        // Fondo nocturno: overlay semitransparente que cubre solo el mapa
        sf::FloatRect mb = map.getGlobalBounds();
        sf::RectangleShape screen(sf::Vector2f(mb.width, mb.height));
        screen.setPosition(mb.left, mb.top);
        screen.setFillColor(sf::Color(0, 0, 0, 200));

        //--------------------------------------------
        //            COLLISION
        //--------------------------------------------
        sf::RectangleShape rec1;
        rec1.setSize(sf::Vector2f(1500,100));
        rec1.setPosition(300,0);


        window.clear();
        window.draw(map);
        // Dibujar overlay oscuro solo sobre el área del mapa
        window.draw(screen);

        // 3. DIBUJA LOS MUEBLES ANTES DE LOS OBJETOS Y EL JUGADOR
        for (auto& m : mueblesSprites) window.draw(m);

        // Dibujar colisiones
        for (auto &s : obstaclesShapes) window.draw(s);
        col.draw(window);
        window.draw(art.getSprite());
    
        for(auto& o : objetos){
            window.draw(o.sprite);
        }
        

        window.draw(screen, &flashlight);
        window.draw(player);
        //---------------------------------------------
        //              HUD - UI
        //---------------------------------------------
        // Cambiar a vista de pantalla para dibujar HUD en coordenadas fijas
        sf::View screenView = window.getDefaultView();
        window.setView(screenView);

        // --- Símbolo de PAUSA (esquina superior derecha) ---
        if (isPaused) {
            sf::Text pauseText("||", menuFont, 48);
            pauseText.setFillColor(sf::Color::Red);
            pauseText.setPosition(1280 - 80.f, 20.f);
            window.draw(pauseText);

            // Overlay semi-transparente oscuro
            sf::RectangleShape pauseOverlay(sf::Vector2f(1280.f, 720.f));
            pauseOverlay.setFillColor(sf::Color(0, 0, 0, 255));
            window.draw(pauseOverlay);

            // Texto "PAUSA" centrado en la pantalla
            sf::Text pauseTitle("PAUSA", menuFont, 80);
            pauseTitle.setFillColor(sf::Color::White);
            pauseTitle.setStyle(sf::Text::Bold);
            // Centrar horizontalmente y verticalmente
            pauseTitle.setPosition(
                (1280.f - pauseTitle.getLocalBounds().width) / 2.f,
                (720.f - pauseTitle.getLocalBounds().height) / 2.f - 50.f
            );
            window.draw(pauseTitle);

            // Texto de instrucción "Presiona P para continuar"
            sf::Text resumeText("Presiona P para continuar", menuFont, 20);
            resumeText.setFillColor(sf::Color(200, 200, 200));
            resumeText.setPosition(
                (1280.f - resumeText.getLocalBounds().width) / 2.f,
                (720.f - resumeText.getLocalBounds().height) / 2.f + 50.f
            );
            window.draw(resumeText);
        }

        // --- Mostrar INVENTARIO (esquina superior izquierda) ---
        if (showInventory) {
            sf::Text invTitle("I", menuFont, 40);
            invTitle.setFillColor(sf::Color::Yellow);
            invTitle.setPosition(20.f, 20.f);
            window.draw(invTitle);

            // Mostrar contenido del inventario
            std::string invText = "Llaves: " + std::to_string(keyCount) + "\n";
            invText += "Palanca: " + std::string(hasPalanca ? "SI" : "NO");
            
            sf::Text invContent(invText, menuFont, 20);
            invContent.setFillColor(sf::Color::White);
            invContent.setPosition(20.f, 70.f);
            window.draw(invContent);
        }

        // Si hay advertencia visible, dibujarla en la esquina inferior izquierda (HUD view)
        if (warningShown) {
            // Cambiar a vista HUD
            sf::View screenView = window.getDefaultView();
            window.setView(screenView);
            float sx = 20.f;
            if (warnLoaded) {
                float sy = 720.f - warnSprite.getGlobalBounds().height - 20.f;
                warnSprite.setPosition(sx, sy);
                window.draw(warnSprite);
            } else {
                // dibujar fallback
                float sy = 720.f - warnFallbackRect.getSize().y - 20.f;
                warnFallbackRect.setPosition(sx, sy);
                window.draw(warnFallbackRect);
                if (warnFallbackText.getString() != "") {
                    // centrar texto dentro del rect
                    sf::FloatRect r = warnFallbackRect.getGlobalBounds();
                    sf::FloatRect t = warnFallbackText.getLocalBounds();
                    warnFallbackText.setPosition(r.left + (r.width - t.width) / 2.f - t.left, r.top + (r.height - t.height) / 2.f - t.top);
                    window.draw(warnFallbackText);
                }
            }
            // Restaurar la vista anterior (la vista del juego)
            window.setView(view);
        }

        // Si hay victoria, mostrar pantalla y permitir reinicio con R
        if (victory) {
            // Vista HUD
            sf::View screenView = window.getDefaultView();
            window.setView(screenView);

            sf::RectangleShape overlay(sf::Vector2f(1280.f, 720.f));
            overlay.setFillColor(sf::Color(0,0,0,200));
            window.draw(overlay);

            sf::Text vic("Ganaste ... o ...", menuFont, 96);
            vic.setFillColor(sf::Color::Green);
            vic.setStyle(sf::Text::Bold);
            vic.setPosition((1280.f - vic.getLocalBounds().width)/2.f, (720.f - vic.getLocalBounds().height)/2.f - 40.f);
            window.draw(vic);

            // Botones: Volver al menú principal / Salir del juego
            float btnW = 300.f, btnH = 70.f;
            sf::RectangleShape menuBtn({btnW, btnH});
            menuBtn.setOrigin(btnW/2.f, btnH/2.f);
            menuBtn.setPosition(1280.f/2.f - 200.f, 720.f/2.f + 60.f);
            menuBtn.setFillColor(sf::Color(70,120,200));

            sf::RectangleShape quitBtn({btnW, btnH});
            quitBtn.setOrigin(btnW/2.f, btnH/2.f);
            quitBtn.setPosition(1280.f/2.f + 200.f, 720.f/2.f + 60.f);
            quitBtn.setFillColor(sf::Color(140,80,80));

            sf::Text menuText("Volver", menuFont, 20);
            menuText.setFillColor(sf::Color::White);
            menuText.setPosition(menuBtn.getPosition().x - menuText.getLocalBounds().width/2.f, menuBtn.getPosition().y - menuText.getLocalBounds().height/2.f - 6.f);

            sf::Text quitText("Salir", menuFont, 24);
            quitText.setFillColor(sf::Color::White);
            quitText.setPosition(quitBtn.getPosition().x - quitText.getLocalBounds().width/2.f, quitBtn.getPosition().y - quitText.getLocalBounds().height/2.f - 6.f);

            window.draw(menuBtn);
            window.draw(quitBtn);
            window.draw(menuText);
            window.draw(quitText);

            // Manejo de input del ratón por evento (evita múltiples activaciones)
            if (clickPressed) {
                if (menuBtn.getGlobalBounds().contains(clickPos)) {
                    // Volver al menú principal: detener música de fondo y reproducir la del menú
                    bgMusic.stop();
                    if (menuMusic.getStatus() != sf::Music::Playing) menuMusic.play();
                    inMenu = true;
                    // Ir al inicio del menú
                    goto start_menu;
                }
                if (quitBtn.getGlobalBounds().contains(clickPos)) {
                    window.close();
                }
            }
            // Presentar el frame y saltar al siguiente (sin procesar más)
            window.display();
            continue;
        }

        // Si hay Game Over, mostrar pantalla y permitir reinicio con R
        if (gameOver) {
            // Vista HUD
            sf::View screenView = window.getDefaultView();
            window.setView(screenView);

            sf::RectangleShape overlay(sf::Vector2f(1280.f, 720.f));
            overlay.setFillColor(sf::Color(0,0,0,200));
            window.draw(overlay);

            sf::Text go("GAME OVER", menuFont, 96);
            go.setFillColor(sf::Color::Red);
            go.setStyle(sf::Text::Bold);
            go.setPosition((1280.f - go.getLocalBounds().width)/2.f, (720.f - go.getLocalBounds().height)/2.f - 40.f);
            window.draw(go);

            // Botones: Volver al menú principal / Salir del juego
            float btnW = 320.f, btnH = 70.f;
            sf::RectangleShape menuBtn({btnW, btnH});
            menuBtn.setOrigin(btnW/2.f, btnH/2.f);
            menuBtn.setPosition(1280.f/2.f - 200.f, 720.f/2.f + 60.f);
            menuBtn.setFillColor(sf::Color(70,120,200));

            sf::RectangleShape quitBtn({btnW, btnH});
            quitBtn.setOrigin(btnW/2.f, btnH/2.f);
            quitBtn.setPosition(1280.f/2.f + 200.f, 720.f/2.f + 60.f);
            quitBtn.setFillColor(sf::Color(140,80,80));

            sf::Text menuText("Volver", menuFont, 20);
            menuText.setFillColor(sf::Color::White);
            menuText.setPosition(menuBtn.getPosition().x - menuText.getLocalBounds().width/2.f, menuBtn.getPosition().y - menuText.getLocalBounds().height/2.f - 6.f);

            sf::Text quitText("Salir", menuFont, 24);
            quitText.setFillColor(sf::Color::White);
            quitText.setPosition(quitBtn.getPosition().x - quitText.getLocalBounds().width/2.f, quitBtn.getPosition().y - quitText.getLocalBounds().height/2.f - 6.f);

            window.draw(menuBtn);
            window.draw(quitBtn);
            window.draw(menuText);
            window.draw(quitText);

            // Manejo de input del ratón por evento (evita múltiples activaciones)
            if (clickPressed) {
                if (menuBtn.getGlobalBounds().contains(clickPos)) {
                    // Volver al menú principal: detener música de fondo y reproducir la del menú
                    bgMusic.stop();
                    if (menuMusic.getStatus() != sf::Music::Playing) menuMusic.play();
                    inMenu = true;
                    // Ir al inicio del menú
                    goto start_menu;
                }
                if (quitBtn.getGlobalBounds().contains(clickPos)) {
                    window.close();
                }
            }
            // Presentar el frame y saltar al siguiente (sin procesar más)
            window.display();
            continue;
        }

        // Mensaje temporal al recoger objetos (no llaves ni palanca)
        if (pickupMessageTimer > 0.f && !pickupMessage.empty()) {
            sf::Text msg(pickupMessage, menuFont, 28);
            msg.setFillColor(sf::Color::White);
            // Centrar el texto en la parte superior
            msg.setPosition((1280.f - msg.getLocalBounds().width) / 2.f, 100.f);
            window.draw(msg);
        }

        // Mini-texto "Faltan X objetos" cuando el jugador está en la zona objetivo pero sin suficientes objetos
        bool inGoalZone = (player.getPosition().x >= goalMinX && player.getPosition().x <= goalMaxX &&
                           player.getPosition().y >= goalMinY && player.getPosition().y <= goalMaxY);
        if (inGoalZone && collectedNonKeyObjects < objectivesRequired) {
            int faltanObjetos = objectivesRequired - collectedNonKeyObjects;
            std::string faltanText = "Faltan " + std::to_string(faltanObjetos) + " objetos";
            sf::Text faltanMsg(faltanText, menuFont, 20);
            faltanMsg.setFillColor(sf::Color::Yellow);
            faltanMsg.setStyle(sf::Text::Bold);
            // Mostrar en la parte inferior derecha
            faltanMsg.setPosition(1280.f - faltanMsg.getLocalBounds().width - 20.f, 680.f - faltanMsg.getLocalBounds().height);
            window.draw(faltanMsg);
        }

        window.display();
    }

    return 0;
}

// Crea un collider para un sprite dado
oc::Collider createColliderForSprite(const sf::Sprite& sprite) {
    oc::Collider collider;
    sf::FloatRect bounds = sprite.getGlobalBounds();
    collider.setSize({bounds.width, bounds.height});
    collider.setPosition({bounds.left, bounds.top});
    collider.shape.setFillColor(sf::Color(0, 0, 0, 0)); // invisible
    return collider;
}

// Genera muebles aleatorios como obstáculos estáticos
void generarMueblesConColision(
    const sf::FloatRect& mapBounds,
    std::vector<sf::Texture>& mueblesTextures,
    std::vector<sf::Sprite>& mueblesSprites,
    std::vector<oc::Collider>& mueblesColliders,
    std::vector<sf::RectangleShape>& obstaclesShapes,
    std::vector<sf::FloatRect>& obstaclesRects,
    int cantidad
) {
    mueblesTextures.clear();
    mueblesSprites.clear();
    mueblesColliders.clear();

    // --- Crear textura placeholder de "cesped alto" ---
    static sf::Texture placeholderTex;
    static bool placeholderCreated = false;
    if (!placeholderCreated) {
        sf::Image img;
        img.create(48, 48, sf::Color(60, 160, 60)); // fondo verde
        for (unsigned int x = 0; x < 48; ++x) {
            if (x % 6 == 0) {
                for (unsigned int y = 0; y < 48; ++y) {
                    img.setPixel(x, y, sf::Color(30, 100 + rand()%80, 30));
                }
            }
            if (x % 8 == 3) {
                for (unsigned int y = 24; y < 48; ++y) {
                    img.setPixel(x, y, sf::Color(20, 80 + rand()%60, 20));
                }
            }
        }
        placeholderTex.loadFromImage(img);
        placeholderCreated = true;
    }

    // Solo cargar texturas de assets/sprites/sprites_muebles
    std::vector<std::string> mueblesPaths;
#ifndef NO_FILESYSTEM
    for (const auto& entry : fs::directory_iterator("assets/sprites/sprites_muebles")) {
        if (entry.is_regular_file()) {
            mueblesPaths.push_back(entry.path().string());
        }
    }
#else
    // No hay soporte filesystem en esta toolchain; no cargamos texturas desde directorio.
#endif
    if (mueblesPaths.empty()) return;

    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    int intentosMax = 100;

    // Vector de punteros a texturas válidas (para fallback)
    std::vector<sf::Texture*> validTextures;
    mueblesTextures.reserve(cantidad + 1); // evitar realocaciones

    for (int i = 0; i < cantidad; ++i) {
        sf::Texture tex;
        bool loaded = false;
        int idx = std::rand() % mueblesPaths.size();
        loaded = tex.loadFromFile(mueblesPaths[idx]);

        sf::Texture* texPtr = nullptr;
        if (loaded) {
            mueblesTextures.push_back(tex);
            texPtr = &mueblesTextures.back();
            validTextures.push_back(texPtr);
        } else if (!validTextures.empty()) {
            // Usa una textura válida ya cargada
            texPtr = validTextures[std::rand() % validTextures.size()];
        } else {
            // Usa el placeholder
            texPtr = &placeholderTex;
        }

        sf::Sprite sprite;
        sprite.setTexture(*texPtr);

        // Escala aleatoria
        float scale = 0.7f + 0.6f * (std::rand() / (float)RAND_MAX);
        sprite.setScale(scale, scale);

        int intentos = 0;
        bool colocado = false;
        while (intentos < intentosMax && !colocado) {
            float x = mapBounds.left + 20 + std::rand() % (int)(mapBounds.width - sprite.getGlobalBounds().width - 40);
            float y = mapBounds.top + 20 + std::rand() % (int)(mapBounds.height - sprite.getGlobalBounds().height - 40);
            sprite.setPosition(x, y);
            sf::FloatRect sprRect = sprite.getGlobalBounds();

            // Verifica superposición con otros muebles y obstáculos
            bool overlap = false;
            for (const auto& c : mueblesColliders) {
                if (c.getBounds().intersects(sprRect)) { overlap = true; break; }
            }
            for (const auto& r : obstaclesRects) {
                if (r.intersects(sprRect)) { overlap = true; break; }
            }
            if (!overlap) {
                colocado = true;
                mueblesSprites.push_back(sprite);
                oc::Collider col = createColliderForSprite(sprite);
                mueblesColliders.push_back(col);
                sf::RectangleShape shape;
                shape.setPosition(sprRect.left, sprRect.top);
                shape.setSize({sprRect.width, sprRect.height});
                shape.setFillColor(sf::Color(120, 80, 40, 0));
                obstaclesShapes.push_back(shape);
                obstaclesRects.push_back(sprRect);
            }
            ++intentos;
        }
    }
}
