// Project: Terrifier (main + fixed israel_collisions.hpp)
// Este archivo contiene dos secciones: 1) israel_collisions.hpp (corregido)
// y 2) main.cpp (parche mínimo para integrar correctamente la librería de colisiones).

/* ==================== israel_collisions.hpp ==================== */

#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cmath>

namespace oc {

// Struct del Collider
struct Collider {
    sf::RectangleShape shape; // en caso de no tener sprite
    sf::Sprite sprite;
    const sf::Texture* texture = nullptr;
    bool hasSprite = false; //puntero no valido
    sf::Vector2f spriteOffset{0.f, 0.f}; //posicion del sprite
    sf::Vector2f velocity{0.f,0.f}; // velocidad del collider (pixels/seg)

    //Tamaño del collider
    void setSize(const sf::Vector2f &size) {
        shape.setSize(size);
        // usamos origen centrado para colliders movibles (coincide con muchos sprites)
        shape.setOrigin(size * 0.5f);
        if (hasSprite && texture) {
            // no hacemos nada extra aquí
        }
    }

    void setPosition(const sf::Vector2f &pos) {
        // pos se interpreta como la posición del centro del collider
        shape.setPosition(pos);
        if (hasSprite) {
            sprite.setPosition(pos + spriteOffset);
        }
    }

    sf::Vector2f getPosition() const { return shape.getPosition(); } //posicion del collider (centro)
    sf::FloatRect getBounds() const { return shape.getGlobalBounds(); } //bordes del collider (top-left, width, height)

    void draw(sf::RenderTarget &rt) const {
        if (hasSprite) rt.draw(sprite);
        else rt.draw(shape);
    }

    // Con el modo debug, se pueden ver los bordes de los colliders
    void drawDebug(sf::RenderTarget &rt, const sf::Color &color = sf::Color::Green) const {
        sf::RectangleShape out(shape.getSize());
        out.setOrigin(shape.getOrigin());
        out.setPosition(shape.getPosition());
        out.setFillColor(sf::Color::Transparent);
        out.setOutlineColor(color);
        out.setOutlineThickness(1.f);
        rt.draw(out);
    }

    // --- Sprite support API ---

    void setTexture(const sf::Texture &tex) {
        texture = &tex;
        sprite.setTexture(tex);
        hasSprite = true;
        sf::FloatRect lb = sprite.getLocalBounds();
        sprite.setOrigin(lb.left + lb.width * 0.5f, lb.top + lb.height * 0.5f);
        sprite.setPosition(shape.getPosition() + spriteOffset);
    }

    void setSpriteOffset(const sf::Vector2f &offset) {
        spriteOffset = offset;
        if (hasSprite) sprite.setPosition(shape.getPosition() + spriteOffset);
    }

    void setSpriteScaleToMatchCollider(bool keepAspect = true) {
        if (!hasSprite || !texture) return;
        sf::FloatRect lb = sprite.getLocalBounds();
        if (lb.width <= 0.f || lb.height <= 0.f) return;
        sf::Vector2f cs = shape.getSize();
        float sx = cs.x / lb.width;
        float sy = cs.y / lb.height;
        if (keepAspect) {
            float s = std::min(sx, sy);
            sprite.setScale(s, s);
        } else {
            sprite.setScale(sx, sy);
        }

        sprite.setPosition(shape.getPosition() + spriteOffset);
    }
};

inline bool aabbIntersects(const sf::FloatRect &a, const sf::FloatRect &b) { // Esta funcion detecta cuando dos colliders... colisionan 
    return a.left < b.left + b.width && a.left + a.width > b.left &&
           a.top  < b.top  + b.height && a.top  + a.height > b.top;
}

// Helper: construye bounding box del collider a partir de su centro y tamaño
inline sf::FloatRect buildColliderBBFromCenter(const sf::Vector2f &center, const sf::Vector2f &size) {
    return sf::FloatRect(center.x - size.x * 0.5f,
                         center.y - size.y * 0.5f,
                         size.x, size.y);
}

// resolveCollisionsAxisSeparated: mueve primero en X y corrige, luego en Y y corrige.
// Evita corner-clipping y es robusto frente a rectangulos cuyos origins pueden variar.
inline void resolveCollisionsAxisSeparated(Collider &moving, const std::vector<sf::FloatRect> &statics, float dt)
{
    if (dt <= 0.f) return;

    // Tamaño y posición central del collider
    sf::Vector2f size = moving.shape.getSize();
    sf::Vector2f center = moving.getPosition();

    // Desplazamiento deseado (pixels)
    sf::Vector2f disp = moving.velocity * dt;

    const float EPS = 0.0001f; // pequeño margen para evitar "sticking"

    // --- MOVER EN X ---
    if (std::abs(disp.x) > 0.f) {
        center.x += disp.x;
        // comprobar colisiones con una caja candidata
        sf::FloatRect bb = buildColliderBBFromCenter(center, size);
        for (const auto &s : statics) {
            if (bb.intersects(s)) {
                if (disp.x > 0.f) {
                    // moviendo a la derecha: colocar la caja a la izquierda del obstáculo
                    center.x = s.left - (size.x * 0.5f) - EPS;
                } else {
                    // moviendo a la izquierda: colocar la caja a la derecha del obstáculo
                    center.x = s.left + s.width + (size.x * 0.5f) + EPS;
                }
                // actualizar la bbox con la nueva posición (y seguir comprobando otros obstaculos)
                bb = buildColliderBBFromCenter(center, size);
            }
        }
    }

    // aplicar la corrección parcial en X
    moving.setPosition(center);

    // --- MOVER EN Y ---
    if (std::abs(disp.y) > 0.f) {
        center.y += disp.y;
        sf::FloatRect bb = buildColliderBBFromCenter(center, size);
        for (const auto &s : statics) {
            if (bb.intersects(s)) {
                if (disp.y > 0.f) {
                    // moviendo hacia abajo: colocar encima del obstáculo
                    center.y = s.top - (size.y * 0.5f) - EPS;
                } else {
                    // moviendo hacia arriba: colocar debajo del obstáculo
                    center.y = s.top + s.height + (size.y * 0.5f) + EPS;
                }
                bb = buildColliderBBFromCenter(center, size);
            }
        }
    }

    // aplicar la posición final
    moving.setPosition(center);

    // Opcional: si la colisión anuló el movimiento en algún eje, puedes ajustar la velocidad
    // Si la diferencia entre la posición esperada y final es mínima, no cambiamos la velocidad.
}

// Creacion de obstáculos
inline void makeObstacle(std::vector<sf::RectangleShape> &outShapes,
                         std::vector<sf::FloatRect> &outRects,
                         const sf::Vector2f &pos,
                         const sf::Vector2f &size,
                         const sf::Color &color = sf::Color::Red)
{
    // En esta versión interpretamos 'pos' como la esquina superior izquierda (top-left)
    sf::RectangleShape s(size);
    s.setFillColor(color);
    s.setOrigin({0.f, 0.f});
    s.setPosition(pos);
    outShapes.push_back(s);
    outRects.push_back(s.getGlobalBounds());
}

// Recalcula outRects a partir de shapes (si modificas shapes en runtime)
inline void updateObstacleRects(const std::vector<sf::RectangleShape> &shapes, std::vector<sf::FloatRect> &outRects)
{
    outRects.clear();
    outRects.reserve(shapes.size());
    for (const auto &s : shapes) outRects.push_back(s.getGlobalBounds());
}

//Cargar obstáculos desde un archivo de texto
inline void loadObstaclesFromFile(const std::string &path,
                                  std::vector<sf::RectangleShape> &outShapes,
                                  std::vector<sf::FloatRect> &outRects,
                                  bool transparent = false)
{
    outShapes.clear();
    outRects.clear();

    std::ifstream f(path);
    if (!f.is_open()) {
        std::cerr << "oc::loadObstaclesFromFile: No se pudo abrir " << path << ' ';
        return;
    }

    std::string line;
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        if (line[0] == '#') continue;

        std::istringstream ss(line);
        float x,y,w,h;
        int r = 255, g = 0, b = 0;
        if (!(ss >> x >> y >> w >> h)) {
            std::cerr << "oc::loadObstaclesFromFile: malformed line: " << line << ' ';
            continue;
        }
        ss >> r >> g >> b; // color optional

        sf::RectangleShape s({w,h});
        // Interpretamos las coordenadas del archivo como top-left (x,y).
        s.setOrigin({0.f, 0.f});
        s.setPosition({x,y});
        s.setFillColor(
            transparent ?
                sf::Color(r, g, b, 0) :      // totalmente transparente
                sf::Color(r, g, b, 255)      // color original
        );
        outShapes.push_back(s);
        outRects.push_back(s.getGlobalBounds());
    }
}

} // namespace oc