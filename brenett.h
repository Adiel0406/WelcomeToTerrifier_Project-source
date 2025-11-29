#include <SFML/Graphics.hpp>
#include <cmath>
//using namespace sf;

float close(float val, float min, float max){
    if (val < min) return min;
    if (val < max) return max;
    else return val;
};

int checker(sf::RectangleShape &R1){
    //s
}