#include <SFML/Graphics.hpp>
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

    art.setVelocidad(180.0f);  // Un poco más lento que el jugador
    art.setRangoDeteccion(400.0f);  // Rango de detección amplio

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
        
        oc::Collider col;
        col.setSize({20.f, 40.f});
        col.setPosition(sf::Vector2f(400, 1450));
        col.shape.setFillColor(sf::Color::Cyan);
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
            velocity.x += accel * dt;
            if (velocity.x > targetVel.x){ velocity.x = targetVel.x;}
        } else if (velocity.x > targetVel.x) {
            velocity.x -= decel * dt;
            if (velocity.x < targetVel.x){ velocity.x = targetVel.x;}
        }
        // Y
        if (velocity.y < targetVel.y) {
            velocity.y += accel * dt;
            if (velocity.y > targetVel.y){velocity.y = targetVel.y;} 
            
        } else if (velocity.y > targetVel.y) {
            velocity.y -= decel * dt;
            if (velocity.y < targetVel.y){ velocity.y = targetVel.y;}
        }

        // Movimiento final (aplica dt)
        sf::Vector2f finalMovement = velocity * dt;
        player.move(finalMovement);

        //---------------------------------------------
        //          ACTUALIZAR IA ART
        //---------------------------------------------
        art.actualizar(dt, player.getPosition().x, player.getPosition().y);

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

        std::vector<sf::RectangleShape> obstaclesShapes;
        std::vector<sf::FloatRect> obstaclesRects;
        oc::resolveCollisionsAxisSeparated(col, obstaclesRects, dt);
        //std::cout<< checker(rec1,c1) <<std::endl;
        

        oc::loadObstaclesFromFile("assets/obstacles.txt", obstaclesShapes, obstaclesRects);


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
        

        window.draw(screen, &flashlight);
        window.display();
    }

    return 0;
}