#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cmath>

namespace oc { // obstacle-collision

// Minimal collider used for moving objects.
// Now with optional sprite support: puedes asignar una textura con setTexture(...)
struct Collider {
    // visual storage: fallback shape used as hitbox visual if no sprite
    sf::RectangleShape shape;

    // optional sprite visual (texture lifetime MUST outlive this collider)
    sf::Sprite sprite;
    const sf::Texture* texture = nullptr;
    bool hasSprite = false;

    // offset between collider center and the sprite position (en unidades del mundo).
    // por defecto (0,0) => sprite centered on collider center
    sf::Vector2f spriteOffset{0.f, 0.f};

    // movement
    sf::Vector2f velocity{0.f,0.f}; // units per second

    // Set collider size (width, height) and keep origin centered.
    // Also updates fallback shape origin.
    void setSize(const sf::Vector2f &size) {
        shape.setSize(size);
        shape.setOrigin(size * 0.5f);
        // keep sprite origin to center if texture exists
        if (hasSprite && texture) {
            // sprite origin already set in setTexture; no change needed here
        }
    }

    // Set position (center). Updates sprite position too (taking into account spriteOffset).
    void setPosition(const sf::Vector2f &pos) {
        shape.setPosition(pos);
        if (hasSprite) {
            sprite.setPosition(pos + spriteOffset);
        }
    }

    sf::Vector2f getPosition() const { return shape.getPosition(); }
    sf::FloatRect getBounds() const { return shape.getGlobalBounds(); }

    // Draw: sprite if available, otherwise the shape.
    void draw(sf::RenderTarget &rt) const {
        if (hasSprite) rt.draw(sprite);
        else rt.draw(shape);
    }

    // Draw debug outline of hitbox
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

    // Assign a texture to use as sprite visual.
    // IMPORTANT: The texture pointer is stored (not copied). Ensure the texture variable you pass
    // lives longer than this Collider instance.
    void setTexture(const sf::Texture &tex) {
        texture = &tex;
        sprite.setTexture(tex);
        hasSprite = true;
        // Set sprite origin to its center (using local bounds)
        sf::FloatRect lb = sprite.getLocalBounds();
        sprite.setOrigin(lb.left + lb.width * 0.5f, lb.top + lb.height * 0.5f);
        // Position sprite at collider position (respecting spriteOffset)
        sprite.setPosition(shape.getPosition() + spriteOffset);
    }

    // Set a positional offset (in world units) between collider center and sprite drawing center.
    // Útil si visual != hitbox center.
    void setSpriteOffset(const sf::Vector2f &offset) {
        spriteOffset = offset;
        if (hasSprite) sprite.setPosition(shape.getPosition() + spriteOffset);
    }

    // Scale the sprite so its size approximately matches the collider's size.
    // If keepAspect is true, sprite is scaled uniformly preserving aspect ratio.
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
        // Make sure sprite origin remains centered after scaling (origin is in local coords)
        // and reposition sprite to collider center + offset
        sprite.setPosition(shape.getPosition() + spriteOffset);
    }
};

// AABB intersection helper
inline bool aabbIntersects(const sf::FloatRect &a, const sf::FloatRect &b) {
    return a.left < b.left + b.width && a.left + a.width > b.left &&
           a.top  < b.top  + b.height && a.top  + a.height > b.top;
}

// Axis-separated collision resolution: mueve 'moving' por (velocity * dt)
// primero en X y resuelve colisiones con 'statics', luego en Y.
// Al colisionar anula la componente de velocidad en ese eje.
inline void resolveCollisionsAxisSeparated(Collider &moving, const std::vector<sf::FloatRect> &statics, float dt)
{
    sf::Vector2f move = moving.velocity * dt;

    // Move X
    if (move.x != 0.f) {
        sf::Vector2f pos = moving.getPosition();
        pos.x += move.x;
        moving.setPosition(pos);
        sf::FloatRect mb = moving.getBounds();

        for (const auto &s : statics) {
            if (mb.intersects(s)) {
                if (move.x > 0.f) {
                    // moved right -> place moving so its right == s.left
                    float newCenterX = s.left - mb.width * 0.5f;
                    moving.setPosition({newCenterX, moving.getPosition().y});
                } else {
                    // moved left -> place moving so its left == s.left + s.width
                    float newCenterX = s.left + s.width + mb.width * 0.5f;
                    moving.setPosition({newCenterX, moving.getPosition().y});
                }
                moving.velocity.x = 0.f;
                mb = moving.getBounds(); // actualizar bounds
            }
        }
    }

    // Move Y
    if (move.y != 0.f) {
        sf::Vector2f pos = moving.getPosition();
        pos.y += move.y;
        moving.setPosition(pos);
        sf::FloatRect mb = moving.getBounds();

        for (const auto &s : statics) {
            if (mb.intersects(s)) {
                if (move.y > 0.f) {
                    // moved down -> bottom == s.top
                    float newCenterY = s.top - mb.height * 0.5f;
                    moving.setPosition({moving.getPosition().x, newCenterY});
                } else {
                    // moved up -> top == s.top + s.height
                    float newCenterY = s.top + s.height + mb.height * 0.5f;
                    moving.setPosition({moving.getPosition().x, newCenterY});
                }
                moving.velocity.y = 0.f;
                mb = moving.getBounds();
            }
        }
    }
}

// Helper: crea y registra un obstáculo (shape visual + rect para colisión)
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

// Load obstacles from a text file with lines: x y width height [r g b]
// Lines beginning with '#' are ignored.
inline void loadObstaclesFromFile(const std::string &path,
                                  std::vector<sf::RectangleShape> &outShapes,
                                  std::vector<sf::FloatRect> &outRects)
{
    outShapes.clear();
    outRects.clear();

    std::ifstream f(path);
    if (!f.is_open()) {
        std::cerr << "oc::loadObstaclesFromFile: couldn't open " << path << '\n';
        return;
    }

    std::string line;
    while (std::getline(f, line)) {
        // skip empty / comment lines
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