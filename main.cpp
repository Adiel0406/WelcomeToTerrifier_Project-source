#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <cmath>
#include <vector>
#include <map>
#include "brenett.h"
#include "adiel.h"
#include "israel_collisions.hpp"
#include "Art.h"
#include <iostream>



int main() {

    sf::RenderWindow window(sf::VideoMode(1280, 720), "Welcome to Terrifier (TEST VER.)");
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
    if (chaseBuf.loadFromFile("assets/audio/persecucion.mp3")) {
        chaseLoaded = true;
    } else {
        std::cerr << "No se pudo cargar assets/audio/persecucion.mp3" << std::endl;
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
    if (persecucionArtBuf.loadFromFile("assets/audio/persecucion_art.mp3")) {
        persecucionArtLoaded = true;
    } else {
        std::cerr << "No se pudo cargar assets/audio/persecucion_art.mp3" << std::endl;
    }
    sf::Sound persecucionArtSound;
    if (persecucionArtLoaded) {
        persecucionArtSound.setBuffer(persecucionArtBuf);
        // Este sonido será controlado por distancia: loop y volumen inicial 0
        persecucionArtSound.setLoop(true);
        persecucionArtSound.setVolume(0.f);
    }

    bool inMenu = true;
    while (inMenu) {
        sf::Event me;
        while (window.pollEvent(me)) {
            if (me.type == sf::Event::Closed) return 0;
        }

        window.clear(sf::Color(20,20,30));

        // Título
        sf::Text title("Welcome to Terrifier", menuFont, 48);
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

        // Click handling
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            if (playBtn.getGlobalBounds().contains(mposf)) {
                inMenu = false; // iniciar juego
            }
            if (exitBtn.getGlobalBounds().contains(mposf)) {
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
    Art art(150.f, 150.f);  // Posición inicial de Art
    
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

    art.setVelocidad(100.0f);  // Un poco más lento que el jugador
    art.setRangoDeteccion(400.0f);  // Rango de detección amplio
    art.setDistanciaUmbral(20.0f);
    art.setPatrullaCiclica(true);

    if (!art.cargarWaypointsDesdeArchivo("assets/waypoints_art.txt")) {
    // Si falla, crear waypoints manualmente
    
    art.agregarWaypoint(300, 300, 2.0);  // Espera 2 segundos
    art.agregarWaypoint(500, 300, 1.5);  // Espera 1.5 segundos
    art.agregarWaypoint(500, 500, 3.0);  // Espera 3 segundos
    art.agregarWaypoint(300, 500, 2.0);  // Espera 2 segundos
}

    //---------------------------------------------
    //                 SHADER DE LUZ
    //---------------------------------------------
    sf::Shader flashlight;
    flashlight.loadFromFile("assets/shaders/light.frag", sf::Shader::Fragment);

    float lightRadius = 250.f;   // RADIO DE LUZ
    float innerRadius = 60.f;    // TOLERANCIA (zona sin sombra alrededor del jugador)
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
    col.shape.setFillColor(sf::Color::Cyan);

    oc::loadObstaclesFromFile("assets/obstacles.txt", obstaclesShapes, obstaclesRects);

    while (window.isOpen()) {
        sf::Event event;
        float dt = clock.restart().asSeconds();

        sf::Event ev;
        while(window.pollEvent(ev)){
            if(ev.type == sf::Event::Closed)
                window.close();
        }

        sf::Vector2f movement(0.f, 0.f);
        bool moving = false;
        enum Direction {UP, DOWN, LEFT, RIGHT};
        Direction dir = DOWN;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        
        

        // --- Entrada que SIEMPRE se procesa (Q para recoger, P para pausa) ---
        bool qNow = sf::Keyboard::isKeyPressed(sf::Keyboard::Q);
        if (qNow && !qPrev) {
            sf::Vector2f ppos = player.getPosition();
            int bestIdx = -1;
            float bestDistSq = pickupRadius * pickupRadius;
            for (int i = 0; i < (int)objetos.size(); ++i) {
                sf::Vector2f op = objetos[i].sprite.getPosition();
                float dx = op.x - ppos.x;
                float dy = op.y - ppos.y;
                float d2 = dx*dx + dy*dy;
                if (d2 < bestDistSq) {
                    bestDistSq = d2;
                    bestIdx = i;
                }
            }
            if (bestIdx != -1) {
                std::string recogido = objetos[bestIdx].nombre;
                std::cout << "Recogido: " << recogido << "\n";
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


            //---------------------------------------------
            //          ACTUALIZAR IA ART
            //---------------------------------------------
            art.actualizar(dt, player.getPosition().x, player.getPosition().y);

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
        
        window.draw(rec1);
        

        // Dibujar colisiones
        for (auto &s : obstaclesShapes) window.draw(s);
        col.draw(window);
        window.draw(player);
        window.draw(art.getSprite());
    
        for(auto& o : objetos){
            window.draw(o.sprite);
        }
        

        //window.draw(screen, &flashlight);

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
            pauseOverlay.setFillColor(sf::Color(0, 0, 0, 150));
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

        // Mensaje temporal al recoger objetos (no llaves ni palanca)
        if (pickupMessageTimer > 0.f && !pickupMessage.empty()) {
            sf::Text msg(pickupMessage, menuFont, 28);
            msg.setFillColor(sf::Color::White);
            // Centrar el texto en la parte superior
            msg.setPosition((1280.f - msg.getLocalBounds().width) / 2.f, 100.f);
            window.draw(msg);
        }

        window.display();
    }

    return 0;
}