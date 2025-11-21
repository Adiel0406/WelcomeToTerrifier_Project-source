#include <SFML/Graphics.hpp>

int main() {

    sf::RenderWindow window(sf::VideoMode(800, 600), "Movimiento + Linterna");
    window.setFramerateLimit(60);

    // TEXTURAS DEL PERSONAJE
    sf::Texture texDown, texLeft, texRight;
    texDown.loadFromFile("assets/sprites/sienna_sprites/sienna_down.png");
    texLeft.loadFromFile("assets/sprites/sienna_sprites/sienna_left.png");
    texRight.loadFromFile("assets/sprites/sienna_sprites/sienna_right.png");

    sf::Sprite player;
    player.setTexture(texDown);
    player.setPosition(300, 200);

    sf::FloatRect pb = player.getLocalBounds();
    player.setOrigin(pb.width / 2.f, pb.height / 2.f);

    //---------------------------------------------
    //                 SHADER DE LUZ
    //---------------------------------------------
    sf::Shader flashlight;
    flashlight.loadFromFile("light.frag", sf::Shader::Fragment);

    float lightRadius = 250.f;   // RADIO DE LUZ
    float innerRadius = 60.f;    // TOLERANCIA (zona sin sombra alrededor del jugador)
    sf::Vector2f lightDir(0.f, 1.f); // Dirección (cambia con WASD)

    //---------------------------------------------
    //                  VIEW (camara)
    //---------------------------------------------
    sf::View view = window.getDefaultView();
    view.zoom(0.55f);
    view.setCenter(player.getPosition());

    const float velocidad = 3.0f;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        //---------------------------------------------
        //           MOVIMIENTO + DIRECCION
        //---------------------------------------------
        sf::Vector2f movement(0.f, 0.f);

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            movement.y -= velocidad;
            player.setTexture(texDown);
            lightDir = sf::Vector2f(0, 1);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            movement.y += velocidad;
            player.setTexture(texDown);
            lightDir = sf::Vector2f(0, -1);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            movement.x -= velocidad;
            player.setTexture(texLeft);
            lightDir = sf::Vector2f(-1, 0);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            movement.x += velocidad;
            player.setTexture(texRight);
            lightDir = sf::Vector2f(1, 0);
        }

        player.move(movement);

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

        window.clear();
        window.draw(screen, &flashlight);
        window.draw(player);
        window.display();
    }

    return 0;
}
