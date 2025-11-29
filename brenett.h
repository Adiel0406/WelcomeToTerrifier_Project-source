#include <SFML/Graphics.hpp>
#include <cmath>
//using namespace sf;

float close(float val, float min, float max){
    if (val < min) return min;
    if (val < max) return max;
    else return val;
};

int checker(sf::RectangleShape &R1, sf::CircleShape &C1){
    //s
    float closeX= close(C1.getPosition().x,R1.getPosition().x,R1.getPosition().x+R1.getSize().x);
    float closeY= close(C1.getPosition().y,R1.getPosition().y,R1.getPosition().y+R1.getSize().y);

    float distanceX = C1.getPosition().x + closeX;
    float distanceY = C1.getPosition().y + closeY;

    float dist = (distanceX * distanceX + distanceY * distanceY);
    return 0;
}