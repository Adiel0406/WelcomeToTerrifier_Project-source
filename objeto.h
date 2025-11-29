#pragma once
#include <string>
#include <SFML/Graphics.hpp>

struct Rect {
    int x1, y1;
    int x2, y2;

    bool contains(int x, int y) const {
        return (x >= x1 && x <= x2 && y >= y1 && y <= y2);
    }
};

struct Objeto {
    std::string nombre;
    int x, y;
    sf::Sprite sprite; 
};