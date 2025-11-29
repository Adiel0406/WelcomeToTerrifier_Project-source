#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include "brenett.h"

int main() {

    sf::RenderWindow window(sf::VideoMode(1280, 720), "Movimiento + Linterna");
    window.setFramerateLimit(60);
    
    // Mapa
    sf::Texture mapyay; 
    mapyay.loadFromFile("assets/sprites/map.png");
    sf::Sprite map;
    map.setTexture(mapyay);
    map.setPosition(300, 200);

    // TEXTURAS DEL PERSONAJE
    sf::Texture texDown, texLeft, texRight, texUp;
    texDown.loadFromFile("assets/sprites/sienna_sprites/IdleDown.png");
    texLeft.loadFromFile("assets/sprites/sienna_sprites/IdleLeft.png");
    texRight.loadFromFile("assets/sprites/sienna_sprites/IdleRight.png");
    texUp.loadFromFile("assets/sprites/sienna_sprites/IdleUp.png");

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
    player.setPosition(300, 200);

    int currentFrame = 0;
    float frameTime = 0.15f;  // cambio cada 150ms
    float timer = 0.f;

    sf::FloatRect pb = player.getLocalBounds();
    player.setOrigin(pb.width / 2.f, pb.height / 2.f);

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

        sf::Vector2f movement(0.f, 0.f);
        bool moving = false;
        enum Direction {UP, DOWN, LEFT, RIGHT};
        Direction dir = DOWN;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

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

        // Interpolar velocity hacia targetVel por componentes
        // X
        if (velocity.x < targetVel.x) {
            velocity.x += accel * dt;
            if (velocity.x > targetVel.x) velocity.x = targetVel.x;
        } else if (velocity.x > targetVel.x) {
            velocity.x -= decel * dt;
            if (velocity.x < targetVel.x) velocity.x = targetVel.x;
        }
        // Y
        if (velocity.y < targetVel.y) {
            velocity.y += accel * dt;
            if (velocity.y > targetVel.y) velocity.y = targetVel.y;
        } else if (velocity.y > targetVel.y) {
            velocity.y -= decel * dt;
            if (velocity.y < targetVel.y) velocity.y = targetVel.y;
        }

        // Movimiento final (aplica dt)
        sf::Vector2f finalMovement = velocity * dt;
        player.move(finalMovement);

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
        window.setView(view);

        //---------------------------------------------
        //             UPDATE SHADER PARAMS
        //---------------------------------------------
        flashlight.setUniform("lightPos", player.getPosition());
        flashlight.setUniform("lightDir", lightDir);
        flashlight.setUniform("lightRadius", lightRadius);
        flashlight.setUniform("innerRadius", innerRadius);
        flashlight.setUniform("resolution", sf::Vector2f(window.getSize()));

        //---------------------------------------------
        //              DRAW
        //---------------------------------------------
        // Fondo oscuro
        sf::RectangleShape screen(sf::Vector2f(window.getSize()));
        screen.setFillColor(sf::Color::Black);

        //--------------------------------------------
        //            COLLISION
        //--------------------------------------------
        sf::RectangleShape rec1;
        rec1.setSize(sf::Vector2f(1500,10));
        rec1.setPosition(sf::Vector2f(300,150));
        

        window.clear();
        window.draw(map);
        window.draw(screen, &flashlight);
        window.draw(rec1);
        window.draw(player);
        window.display();
    }

    return 0;
}