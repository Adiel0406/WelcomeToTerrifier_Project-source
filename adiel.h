#pragma once
#include "objeto.h"
#include <vector>
#include <random>

class ObjectSpawner {
public:
    ObjectSpawner(int minX, int maxX, int minY, int maxY);

    std::vector<Objeto> generarObjetos();
    
private:
    std::mt19937 rng;
    std::uniform_int_distribution<int> distX;
    std::uniform_int_distribution<int> distY;

    bool validarZona(int x, int y, const Rect& zona);
};