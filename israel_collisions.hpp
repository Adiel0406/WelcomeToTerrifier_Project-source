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
    sf::Vector2f velocity{0.f,0.f}; // velocidad del collider

    //Tamaño del collider
    void setSize(const sf::Vector2f &size) {
        shape.setSize(size);
        shape.setOrigin(size * 0.5f);
        if (hasSprite && texture) {
        }
    }

    void setPosition(const sf::Vector2f &pos) {
        shape.setPosition(pos);
        if (hasSprite) {
            sprite.setPosition(pos + spriteOffset);
        }
    }

    sf::Vector2f getPosition() const { return shape.getPosition(); } //posicion del collider
    sf::FloatRect getBounds() const { return shape.getGlobalBounds(); } //bordes del collider

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


inline void resolveCollisionsAxisSeparated(Collider &moving, const std::vector<sf::FloatRect> &statics, float dt) // Esta funcion hace que los dos colliders se detengan
{
    sf::Vector2f move = moving.velocity * dt;

    if (move.x != 0.f) {
        sf::Vector2f pos = moving.getPosition();
        pos.x += move.x;
        moving.setPosition(pos);
        sf::FloatRect mb = moving.getBounds();

        for (const auto &s : statics) {
            if (mb.intersects(s)) {
                if (move.x > 0.f) {
                    float newCenterX = s.left - mb.width * 0.5f;
                    moving.setPosition({newCenterX, moving.getPosition().y});
                } else {
                    float newCenterX = s.left + s.width + mb.width * 0.5f;
                    moving.setPosition({newCenterX, moving.getPosition().y});
                }
                moving.velocity.x = 0.f;
                mb = moving.getBounds();
            }
        }
    }

    
    if (move.y != 0.f) {
        sf::Vector2f pos = moving.getPosition();
        pos.y += move.y;
        moving.setPosition(pos);
        sf::FloatRect mb = moving.getBounds();

        for (const auto &s : statics) {
            if (mb.intersects(s)) {
                if (move.y > 0.f) {
                    float newCenterY = s.top - mb.height * 0.5f;
                    moving.setPosition({moving.getPosition().x, newCenterY});
                } else {
                    float newCenterY = s.top + s.height + mb.height * 0.5f;
                    moving.setPosition({moving.getPosition().x, newCenterY});
                }
                moving.velocity.y = 0.f;
                mb = moving.getBounds();
            }
        }
    }
}

// Creacion de obstáculos
inline void makeObstacle(std::vector<sf::RectangleShape> &outShapes,
                         std::vector<sf::FloatRect> &outRects,
                         const sf::Vector2f &pos,
                         const sf::Vector2f &size,
                         const sf::Color &color = sf::Color::Red)
{
    sf::RectangleShape s(size);
    s.setFillColor(color);
    s.setOrigin(size * 0.5f);
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
                                  std::vector<sf::FloatRect> &outRects)
{
    outShapes.clear();
    outRects.clear();

    std::ifstream f(path);
    if (!f.is_open()) {
        std::cerr << "oc::loadObstaclesFromFile: No se pudo abrir " << path << '\n';
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
            std::cerr << "oc::loadObstaclesFromFile: malformed line: " << line << '\n';
            continue;
        }
        ss >> r >> g >> b; // color optional

        sf::RectangleShape s({w,h});
        s.setOrigin({w*0.5f, h*0.5f});
        s.setPosition({x,y});
        s.setFillColor(sf::Color(static_cast<sf::Uint8>(r),
                                 static_cast<sf::Uint8>(g),
                                 static_cast<sf::Uint8>(b)));
        outShapes.push_back(s);
        outRects.push_back(s.getGlobalBounds());
    }
}

} // namespace oc
